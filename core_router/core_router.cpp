// core_router.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/multi_array.hpp>
#include <boost/foreach.hpp>

#include <set>
#include <vector>
#include <iostream>

#include <queue>
#include <boost/assign/list_of.hpp>
#include <map>

///////////////////////////////////////////////////////////////////////////////////////////

struct COORDINATE
{
	size_t layer;
	size_t x;
	size_t y;

	bool operator < (const COORDINATE& coo) const
	{
		if (layer != coo.layer)
			return layer < coo.layer;

		if (x != coo.x)
			return x < coo.x;

		return y < coo.y;
	}

	bool operator == (const COORDINATE& coo) const
	{
		return layer == coo.layer && x == coo.x && y == coo.y;
	}
};

std::ostream& operator<< ( std::ostream& o , const COORDINATE& c) 
{
	o << '[' << c.layer << "," << c.x << "," << c.y << "]";
	return o;
}

/////////////////////////////////////////////////////////////////

struct NETCOORDINATES
{
	size_t netid;
	COORDINATE coo1, coo2;
};

typedef std::vector<NETCOORDINATES> NETLIST;

/////////////////////////////////////////////////////////////////

enum DIRECTION
{
	UNDEFINED = -1,
	NONE,
	N,S,E,W,U,D
};

bool is_bend(DIRECTION d1, DIRECTION d2)
{
	if ((d1 == N || d1 == S) && (d2 == E || d2 == W))
		return true;

	if ((d1 == E || d1 == W) && (d2 == N || d2 == S))
		return true;

	return false;
}

bool is_via (DIRECTION d)
{
	return (d == U || d == D);
}

///////////////////////////////////////////////////////

int const COST_UNDEFINED = -2;
int const COST_BLOCKED = -1;

struct WAVE_CELL
{
	int cost;
	DIRECTION pred;

	WAVE_CELL (void)
	{
		cost = COST_UNDEFINED;
		pred = UNDEFINED;
	}
};

typedef std::map<COORDINATE, WAVE_CELL> WAVEFRONT;
typedef std::vector<COORDINATE> PATH;
typedef std::set<COORDINATE> SETCOO;
typedef std::map<size_t, PATH> ROUTING;
typedef std::map<COORDINATE, int> COSTS;

int get_cost (const COSTS& costs, const COORDINATE& coo )
{
	COSTS::const_iterator it = costs.find( coo );
	if (it == costs.end())
		return 0;

	return it->second;
}

bool is_blocked (const COSTS& costs, const COORDINATE& coo)
{
	return get_cost(costs, coo) == COST_BLOCKED;
}

void block (COSTS& costs, const COORDINATE& coo)
{
	costs[coo] = COST_BLOCKED;
}

void unblock (COSTS& costs, const COORDINATE& coo)
{
	COSTS::iterator it = costs.find( coo );
	if (it != costs.end() && it->second != COST_BLOCKED)
		return;

	it->second = 1;
}



/////////////////////////////////////////////////////////////////////////////////////////////

struct PENALTY
{
	size_t via;
	size_t bend;
};

void read_specs (const std::string& path, const std::string& name, 
				 NETLIST& netlist, COORDINATE& dims, COSTS& costs,
				 PENALTY& penalty)
{
	const boost::filesystem::path p = path ;

	boost::filesystem::path path_netlist = p / (name + ".nl");
	boost::filesystem::path path_grid    = p / (name + ".grid");

	boost::filesystem::ifstream in_netlist ( path_netlist );

	if ( in_netlist )
	{
		size_t nof_nets;
		in_netlist >> nof_nets;

		netlist.reserve(nof_nets);

		for (size_t n = 0; n < nof_nets; n++)
		{
			NETCOORDINATES pcs;
	
			in_netlist >> pcs.netid;

			in_netlist >> pcs.coo1.layer;
			pcs.coo1.layer--;

			in_netlist >> pcs.coo1.x;
			in_netlist >> pcs.coo1.y;

			in_netlist >> pcs.coo2.layer;
			pcs.coo2.layer--;

			in_netlist >> pcs.coo2.x;
			in_netlist >> pcs.coo2.y;

			netlist.push_back( pcs );
		}
	}

	boost::filesystem::ifstream in_grid ( path_grid );

	if ( in_grid )
	{
		size_t size_x, size_y;
		in_grid >> size_x >> size_y >> penalty.bend >> penalty.via;

		dims.layer = 2;
		dims.x = size_x;
		dims.y = size_y;

		for (size_t layer = 0; layer < 2; layer++)
			for (size_t y = 0; y < size_y; y++)
				for (size_t x = 0; x < size_x; x++)
				{
					int cost;
					in_grid >> cost;

					COORDINATE coo;
					coo.layer = layer;
					coo.x = x;
					coo.y = y;
					costs[coo] = cost;
				}

		std::cout << "done" << std::endl;
	}
}

void out_results (std::ofstream& out, const ROUTING& r)
{
	out << r.size() << std::endl;
	BOOST_FOREACH(const ROUTING::value_type& v, r)
	{
		out << v.first << std::endl;

		BOOST_FOREACH(const COORDINATE& coo, v.second)
		{
			out << coo.layer + 1;
			out << " ";
			out << coo.x;
			out << " ";
			out << coo.y;
			out << std::endl;
		}

		out << 0 << std::endl;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

std::set<DIRECTION> allowed_expansion = boost::assign::list_of(N)(S)(E)(W)(U)(D);

bool is_valid (const COORDINATE& dims, const COORDINATE& coo)
{
	return (coo.layer < dims.layer && coo.x < dims.x && coo.y < dims.y );
}

COORDINATE back_trace (DIRECTION d, const COORDINATE& coo )
{
	COORDINATE coo_prev  = coo;

	switch (d)
	{
		case N:
			coo_prev.y--;
			break;

		case S:
			coo_prev.y++;
			break;

		case E:
			coo_prev.x++;
			break;

		case W:
			coo_prev.x--;
			break;

		case U:
			coo_prev.layer--;
			break;

		case D:
			coo_prev.layer++;
			break;
	}

	return coo_prev;
}

bool next_coo (DIRECTION d, const COORDINATE& dims, const COORDINATE& coo, COORDINATE& coo_next )
{
	coo_next = coo;

	switch (d)
	{
		case N:
			if (coo.y + 1 >= dims.y)
				return false;

			coo_next.y++;
			break;

		case S:
			if (coo.y == 0)
				return false;

			coo_next.y--;
			break;

		case E:
			if (coo.x == 0)
				return false;

			coo_next.x--;
			break;

		case W:
			if (coo.x + 1 >= dims.x)
				return false;

			coo_next.x++;
			break;

		case U:
			if (coo.layer + 1 >= dims.layer)
				return false;

			coo_next.layer++;
			break;

		case D:
			if (coo.layer == 0)
				return false;

			coo_next.layer--;
			break;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////

void route_single_route (const COORDINATE& dims, const COSTS& grid, 
						 const NETCOORDINATES& pcs, 
						 const PENALTY& penalty,
						 PATH& path )
{
	std::queue<COORDINATE> qq;
	WAVEFRONT wf; 

	COORDINATE const coo_start = pcs.coo1;
	COORDINATE const coo_end   = pcs.coo2;

	qq.push( coo_start );		// push in start

	WAVE_CELL wc;

	COSTS::const_iterator itC = grid.find( coo_start );
	BOOST_ASSERT(itC != grid.end());

	wc.cost = itC->second;
	wc.pred = NONE;

	wf[coo_start] = wc;

	while ( !qq.empty() )
	{	
		COORDINATE coo_from = qq.front();
		qq.pop();

		WAVEFRONT::const_iterator itW = wf.find( coo_from );
		BOOST_ASSERT (itW != wf.end());

		size_t const costs_so_far = itW->second.cost;

		BOOST_FOREACH(const DIRECTION& d, allowed_expansion)
		{
			COORDINATE coo_next;
			if ( next_coo(d, dims, coo_from, coo_next ) )
			{
				BOOST_ASSERT(is_valid(dims, coo_next));

				if ( !is_blocked( grid, coo_next ) )
				{
					COSTS::const_iterator itCM = grid.find( coo_from );
					BOOST_ASSERT(itCM != grid.end());

					wc.cost = itC->second;

					size_t cost_to_move = itCM->second;
					if ( is_bend(itW->second.pred, d))
						cost_to_move += penalty.bend;

					if ( is_via(d))
						cost_to_move += penalty.via;

					WAVEFRONT::iterator itW = wf.find( coo_next );
				
					if ( itW == wf.end() )
					{
						WAVE_CELL wc;
						wc.cost = costs_so_far + cost_to_move;
						wc.pred = d;

						wf[coo_next] = wc;
						qq.push(coo_next);
					}
					else
					{
						int new_cost = costs_so_far + cost_to_move;
						if (new_cost < itW->second.cost)
						{
							itW->second.cost  = new_cost;
							itW->second.pred  = d;
							qq.push(coo_next);
						}
					}
				}
				else
				{
					//std::cout << coo_next << " blocked " << std::endl;
				}
			}
		}
	}

	BOOST_ASSERT(path.empty());

	if (wf.find( coo_end ) == wf.end())
	{
		std::cout << coo_end << " no route " << std::endl;
		return;
	}

	COORDINATE coo = coo_end;

	while (!(coo == coo_start))
	{
		path.push_back( coo );

		WAVEFRONT::const_iterator itW = wf.find( coo );
		BOOST_ASSERT (itW != wf.end());

		WAVE_CELL wc = itW->second;
		coo = back_trace(wc.pred, coo);

		BOOST_ASSERT( is_valid(dims, coo) );
	}

	path.push_back( coo );
}

/////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char const* argv[])
{
	if (argc < 3)
		return -1;

	NETLIST netlist;
	COORDINATE dims;
	COSTS costs;
	PENALTY penalty;

	std::cout << argv[1] << " -- " << argv[2] << std::endl;

	read_specs (argv[1], argv[2], netlist, dims, costs, penalty);

	ROUTING routing;

	BOOST_FOREACH(const NETCOORDINATES& pcs, netlist)
	{
		std::cout << "routing net " << pcs.netid << "... ";
		PATH path;

		unblock( costs, pcs.coo1 );
		unblock( costs, pcs.coo2 );

		route_single_route( dims, costs, pcs, penalty, path);

		BOOST_FOREACH(const COORDINATE& coo, path)
			block( costs, coo );

		std::reverse(path.begin(), path.end());
		routing[pcs.netid] = path;

		std::cout << "done" << std::endl;
	}


	boost::filesystem::path path_result = (std::string(argv[2]) + ".result");
	boost::filesystem::ofstream out_result ( path_result );
	out_results(out_result, routing);

	return 0;
}

