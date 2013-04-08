// placer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <assert.h>
#include <valarray>
#include "solver.h"
#include <algorithm>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/bimap.hpp>

int max_depth = 6;
//int max_depth = 3;

//////////////////////////////////////////////////////////////////////////////
// check results:
// https://class.coursera.org/vlsicad-001/wiki/view?page=VisualizePlacerResults
///////////////////////////////////////////////////////////////////////

struct DOUBLEVAL
{
	double dVal;
	DOUBLEVAL (void) { dVal = 0.0; }
	DOUBLEVAL (double val) { dVal = val; }

	operator double () const
	{ return dVal; }

	double operator+(double val) const
	{ return dVal + val; }

	double operator-(double val) const
	{ return dVal - val; }

	DOUBLEVAL& operator+=(double val)
	{ dVal += val; return *this; }

	DOUBLEVAL& operator-=(double val)
	{ dVal -= val; return *this; }

	const DOUBLEVAL& operator=(const DOUBLEVAL& val)
	{ dVal = val.dVal; return *this; }

	const DOUBLEVAL& operator=(double val)
	{ dVal = val; return *this; }
};

struct WEIGHT_2D
{
	DOUBLEVAL x,y;
};

///////////////////////////////////////

struct COORDINATES
{
	double x,y;

	COORDINATES()
	{
		x = y = -1;
	}

	COORDINATES(double xx, double yy)
	{
		x = xx; y = yy;
	}

	friend std::ostream& operator<< (std::ostream&, const COORDINATES&);
};

std::ostream& operator<< ( std::ostream& o , const COORDINATES& c) 
{
	o << '[' << c.x << "," << c.y << "]";
	return o;
}

struct  AREA
{
	COORDINATES xymin, xymax;
	friend std::ostream& operator<< (std::ostream&, const AREA&);
};

std::ostream& operator<< ( std::ostream& o , const AREA& a) 
{
	o << "x,y from ";
	o << "<" << a.xymin.x <<  "," << a.xymax.x << ">";
	o << "<" << a.xymin.y <<  "," << a.xymax.y << ">";
	return o;
}

//////////////////////////////////////////

typedef int ID_NODE;
typedef size_t ID_NET;

typedef std::set<ID_NODE> NODESET;

typedef std::set<ID_NET> NETSET;
typedef std::map<ID_NODE, NETSET> PROBLEMSET;

struct EDGE {
	ID_NODE n1;
	ID_NODE n2;
	double weight;
};

typedef std::map<ID_NODE, COORDINATES> POSITIONS;

typedef std::map<ID_NODE, WEIGHT_2D> NODE_WEIGHTS;
typedef std::map<ID_NODE, DOUBLEVAL> NODE_TO_DOUBLEVAL;

typedef std::vector<NODESET> CONNECTIONS;

///////////////////////////////////////////////////////////////////////

int read_specs ( const boost::filesystem::path& path, 
				PROBLEMSET& netlist, POSITIONS& posit )
{
	boost::filesystem::ifstream is( path );

	if (!is)
		return 0;

	size_t nof_gates, nof_nets;

	is >> nof_gates;
	is >> nof_nets;

	for (size_t i = 0; i < nof_gates; ++i)
	{
		ID_NODE gate;
		is >> gate;

		assert( gate > 0 && gate <= nof_gates);

		size_t nof_nets_for_this_gate;
		is >> nof_nets_for_this_gate;

		for (size_t j = 0; j < nof_nets_for_this_gate; ++j)
		{
			ID_NET net;
			is >> net;

			assert( net > 0 && net <= nof_nets);
			netlist[gate].insert(net);
		}
	}

	size_t nof_pads;
	is >> nof_pads;

	for (size_t i = 0; i < nof_pads; ++i)
	{
		////////////////////////////////////////////////////////////////////

		int pinid;
		is >> pinid;

		assert( pinid > 0 && pinid <= nof_pads);

		int netid;
		is >> netid;

		assert( netid > 0 && netid <= nof_nets);
		netlist[-pinid].insert(netid);

		/////////////////////////////////////////////////////////////////////

		COORDINATES coo;
		is >> coo.x;
		is >> coo.y;

		posit[-pinid] = coo;
	}

	return nof_gates;
}

/*
void check_netlist (const NETLIST& netlist)
{
	typedef std::set<ID_NET> NETSET;
	typedef std::map<ID_NODE, NETSET> NODE_TO_NET;
	NODE_TO_NET ntn;

	BOOST_FOREACH( const NETLIST::value_type& v, netlist )
	{
		BOOST_FOREACH( const ID_NODE node, v.second)
		{
			ntn[node].insert(v.first);
		}
	}

	BOOST_FOREACH( const NODE_TO_NET::value_type& v, ntn)
	{
		if (v.second.size() > 1)
		{
			cout << "node " << v.first << " in nets { " ;
			BOOST_FOREACH( ID_NET net, v.second)
			{
				cout << net;
				cout << " ";
			}

			cout << "}" << endl ;
		}
	}
}*/

void write_placement (const boost::filesystem::path& path, const POSITIONS& pos)
{
	boost::filesystem::ofstream out ( path );
	BOOST_FOREACH( const POSITIONS::value_type& v, pos)
	{
		ID_NODE node = v.first;
		if (node < 0)
			continue;

		// BOOST_ASSERT (node > 0 && node <= nof_gates);

		out << (node);
		out << ' ';
		out << v.second.x;
		out << ' ';
		out << v.second.y;
		out << std::endl;
	}
}

void get_max_rect (const POSITIONS& pos, AREA& a)
{
	if (pos.empty())
		return;

	a.xymax = pos.begin()->second;
	a.xymin = pos.begin()->second;

	BOOST_FOREACH(const POSITIONS::value_type& v, pos)
	{
		if (a.xymin.x > v.second.x)
			a.xymin.x = v.second.x;

		if (a.xymin.y > v.second.y)
			a.xymin.y = v.second.y;

		if (a.xymax.x < v.second.x)
			a.xymax.x = v.second.x;

		if (a.xymax.y < v.second.y)
			a.xymax.y = v.second.y;
	}
}

struct NODE_COO {
	ID_NODE id;
	COORDINATES coo;
};

bool lower_left ( const NODE_COO& a, const NODE_COO& b )
{
	if (a.coo.x != b.coo.x)
		return a.coo.x < b.coo.x;

	return a.coo.y < b.coo.y;
}

bool lower_bottom ( const NODE_COO& a, const NODE_COO& b )
{
	if (a.coo.y != b.coo.y)
		return a.coo.y < b.coo.y;

	return a.coo.x < b.coo.x;
}

void get_split_coo ( const POSITIONS& pos, COORDINATES& coo_middle )
{
	std::vector<NODE_COO> vnc, vnc_lr, vnc_bt;
	vnc.reserve(pos.size());

	BOOST_ASSERT ( !pos.empty() );

	BOOST_FOREACH( const POSITIONS::value_type&v, pos)
	{
		NODE_COO nc;
		nc.id  = v.first;
		nc.coo = v.second;

		vnc.push_back(nc);
	}

	size_t const midix = vnc.size() / 2;

	vnc_lr = vnc;
	stable_sort (vnc_lr.begin(), vnc_lr.end(), lower_left);
	coo_middle.x = vnc_lr[ midix ].coo.x;
	
	vnc_bt = vnc;
	stable_sort (vnc_bt.begin(), vnc_bt.end(), lower_bottom);
	coo_middle.y = vnc_bt[ midix ].coo.y;
}

enum DIR {TOP, BOTTOM, LEFT, RIGHT};

POSITIONS align_positions (
	DIR direction,
	// bounding rectangle
	double xycutoff,
	double xynew,

	// position
	const POSITIONS& positions
	)
{
	POSITIONS al_positions;

	//adj_positions.reserve(positions.size());
	BOOST_FOREACH( const POSITIONS::value_type&v, positions)
	{
		bool is_pad = v.first < 0;
		COORDINATES newcoo = v.second;

		switch (direction)
		{
			case LEFT:
				if (newcoo.x > xynew)
					newcoo.x = xynew;
				break;

			case RIGHT:

				if (newcoo.x < xynew)
					newcoo.x = xynew;
				break;

			case BOTTOM:
				if (newcoo.y > xynew)
					newcoo.y = xynew;
				break;

			case TOP:

				if (newcoo.y < xynew)
					newcoo.y = xynew;
				break;

		}
		al_positions[v.first] = newcoo;
	}

	return al_positions;
}

POSITIONS attached_pads_only(const POSITIONS& p)
{
	return p;
}

void optimize_placement( 
	// input:
	const NODESET& need_optimize,
	const CONNECTIONS& connected,

	// position
	const POSITIONS& fixed_positions,
	
	// output
	POSITIONS& optimized_positions )
{
	std::vector<EDGE> edges;
	std::map<ID_NODE, WEIGHT_2D> weight;
	NODE_TO_DOUBLEVAL diagonal;

	BOOST_FOREACH( const CONNECTIONS::value_type& v, connected )
	{
		const NODESET& nodes = v;

		NODESET nodes_optim, nodes_fixed;
		BOOST_FOREACH( ID_NODE const node, nodes )
		{
			if ( need_optimize.find( node ) != need_optimize.end() )
			{
				// never optimize pads
				BOOST_ASSERT( node > 0 );
				nodes_optim.insert( node );
			}
			else
				nodes_fixed.insert( node );
		}

		BOOST_FOREACH(ID_NODE const n1, nodes_optim)
			BOOST_FOREACH(ID_NODE const n2, nodes_optim)
				if (n1 != n2)
				{
					EDGE e;
					e.n1 = n1;
					e.n2 = n2;

					double const clique_weight = 1.0 / (nodes_optim.size() -1);
					e.weight = clique_weight;
					//e.weight = 1.0;

					edges.push_back( e );
					diagonal [ n1 ] += e.weight;
				}

		BOOST_FOREACH(ID_NODE const n1, nodes_optim)
			BOOST_FOREACH(ID_NODE const n2, nodes_fixed)
			{
				// BOOST_ASSERT(nodes_fixed.size() == 1);
				POSITIONS::const_iterator itWeights = fixed_positions.find( n2 );
				BOOST_ASSERT( itWeights != fixed_positions.end() );
				const COORDINATES& coo = itWeights->second;

				double const clique_weight = 1.0 / (nodes_optim.size() + nodes_fixed.size() -1);

				WEIGHT_2D& w = weight[n1];
				w.x += (clique_weight * coo.x);
				w.y += (clique_weight * coo.y);

				diagonal [n1] += clique_weight;
			}		
	}

	NODESET will_optimize;
	BOOST_FOREACH( const EDGE& e, edges)
	{
		// fix mapping
		BOOST_ASSERT ( e.n1 != e.n2 );
		BOOST_ASSERT ( e.n1 > 0 );
		BOOST_ASSERT ( e.n2 > 0 );

		will_optimize.insert(e.n1);
		will_optimize.insert(e.n2);
	}

	BOOST_FOREACH( const NODE_TO_DOUBLEVAL::value_type& v, diagonal)
	{
		BOOST_ASSERT ( v.first > 0 );
		will_optimize.insert(v.first);
	}

	BOOST_ASSERT( will_optimize == need_optimize );
	const size_t nof_gates = will_optimize.size();

	typedef boost::bimap< ID_NODE, size_t > IX_MAPPING;
	IX_MAPPING ixmap;

	size_t ix = 0;
	BOOST_FOREACH( ID_NODE node, will_optimize)
		ixmap.insert (IX_MAPPING::value_type (node, ix++));

	std::vector<int> R,C;
	std::vector<double> V;
	R.reserve( edges.size() + nof_gates);

	BOOST_FOREACH( const EDGE& e, edges)
	{
		IX_MAPPING::left_const_iterator left_iter1 = ixmap.left.find( e.n1 );
		BOOST_ASSERT (left_iter1 != ixmap.left.end());

		IX_MAPPING::left_const_iterator left_iter2 = ixmap.left.find( e.n2 );
		BOOST_ASSERT (left_iter2 != ixmap.left.end());

		R.push_back( left_iter1->second );
		C.push_back( left_iter2->second );
		V.push_back( -e.weight );
	}

	BOOST_FOREACH ( const NODE_TO_DOUBLEVAL::value_type& v, diagonal)
	{
		IX_MAPPING::left_const_iterator left_iter = ixmap.left.find( v.first );
		BOOST_ASSERT (left_iter != ixmap.left.end());
		size_t ix = left_iter->second;

		R.push_back( ix );
		C.push_back( ix );
		V.push_back( v.second );
	}

	valarray<double> xpads(0.0, nof_gates);
	valarray<double> ypads(0.0, nof_gates);

	BOOST_FOREACH(const NODE_WEIGHTS::value_type& v, weight)
	{
		IX_MAPPING::left_const_iterator left_iter = ixmap.left.find( v.first );
		BOOST_ASSERT (left_iter != ixmap.left.end());
		size_t ix = left_iter->second;

		xpads[ix] = v.second.x;
		ypads[ix] = v.second.y;
	}

	///////////////////////////////////////////////////
	coo_matrix A;

	A.n = nof_gates;
	A.nnz = R.size();

	A.row.resize(A.nnz);
	A.col.resize(A.nnz);
	A.dat.resize(A.nnz);

	A.row = valarray<int>(&R.front(), A.nnz);
	A.col = valarray<int>(&C.front(), A.nnz);
	A.dat = valarray<double>(&V.front(), A.nnz);

	// initialize as [1, ... 1]
	valarray<double> x(1.0, A.n);
	valarray<double> y(1.0, A.n);

	// solve for x
	//cout << "x = " << endl;
	A.solve(xpads, x);
	//print_valarray(x);

	// solve for y
	//cout << "y = " << endl;
	A.solve(ypads, y);
	//print_valarray(y);

	for ( size_t d=0; d < xpads.size(); d++ )
	{
		COORDINATES coo;
		coo.x = x[d];
		coo.y = y[d];

		IX_MAPPING::right_const_iterator right_iter = ixmap.right.find( d );
		BOOST_ASSERT (right_iter != ixmap.right.end());
		size_t ix = right_iter->second;

		optimized_positions[ix] = coo;
	}
}

POSITIONS split_optimize_merge (
	std::string depth,
	const AREA& a,
	const NODESET& need_optimize,
	const CONNECTIONS& connections,
	const POSITIONS& fixed_positions
)
{
	cout << depth << " ";
	bool horizontal = (depth.length() % 2) == 0;

	// test - should not align at all
	// AREA a;
	// get_max_rect( fixed_positions, a );

	cout << a << " -> ";

	POSITIONS posit_full_optimization;
	optimize_placement( need_optimize, 
		connections,
		fixed_positions, 
		posit_full_optimization);

	COORDINATES coo_qcenter;
	get_split_coo ( posit_full_optimization, coo_qcenter );

	cout << coo_qcenter;

	if (depth.length() >= max_depth)
	{
		cout << " DONE" << std::endl;
		return posit_full_optimization;
	}

	cout << " split" << std::endl;

	POSITIONS pos_A, pos_B;
	NODESET nodes_A, nodes_B;

	BOOST_FOREACH( const POSITIONS::value_type& v, posit_full_optimization )
	{
		if (horizontal)
		{
			if (v.second.x >= coo_qcenter.x)
			{
				pos_B.insert(v);
				nodes_B.insert(v.first);
			}
			else
			{
				pos_A.insert(v);
				nodes_A.insert(v.first);
			}
		}
		else
		{
			if (v.second.y >= coo_qcenter.y)
			{
				pos_B.insert(v);
				nodes_B.insert(v.first);
			}
			else
			{
				pos_A.insert(v);
				nodes_A.insert(v.first);
			}
		}
	}

	double const newmiddle_x = (a.xymax.x + a.xymin.x) / 2.0;
	double const newmiddle_y = (a.xymax.y + a.xymin.y) / 2.0;

	AREA areaA, areaB;
	areaA = areaB = a;

	POSITIONS fixed_optimA = attached_pads_only(fixed_positions);
	fixed_optimA.insert( pos_B.begin(), pos_B.end() );

	POSITIONS fixed_optimB = attached_pads_only(fixed_positions);
	fixed_optimB.insert( pos_A.begin(), pos_A.end() );

	if (horizontal)
	{
		fixed_optimA = align_positions(LEFT,  coo_qcenter.x, newmiddle_x, fixed_optimA);
		fixed_optimB = align_positions(RIGHT, coo_qcenter.x, newmiddle_x, fixed_optimB);

		areaA.xymax.x = areaB.xymin.x = newmiddle_x;
	}
	else
	{
		fixed_optimA = align_positions(BOTTOM, coo_qcenter.y, newmiddle_y, fixed_optimA);
		fixed_optimB = align_positions(TOP,    coo_qcenter.y, newmiddle_y, fixed_optimB);

		areaA.xymax.y = areaB.xymin.y = newmiddle_y;
	}

	POSITIONS posit_B_optimization = split_optimize_merge( 
		horizontal ? depth + "R" : depth + "T",
		areaB,
		nodes_B, 
		connections,
		fixed_optimB
		);

	POSITIONS posit_A_optimization = split_optimize_merge(
		horizontal ? depth + "L" : depth + "B",
		areaA,
		nodes_A, 
		connections,
		fixed_optimA 
	);

	// merge results
	POSITIONS pos_result;
	pos_result.insert( posit_A_optimization.begin(), posit_A_optimization.end() );
	pos_result.insert( posit_B_optimization.begin(), posit_B_optimization.end() );

	return pos_result;
}

//////////////////////////////////////////////////////////////////////

int _tmain(int argc, char* argv[])
{
	std::string filename = std::string(argv[1]) ;
	
	if (argc >= 3)
	{
		filename += "/";
		filename += argv[2];
	}

	if (argc >=4)
	{
		max_depth = atoi(argv[3]);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////

	PROBLEMSET netlist; 
	POSITIONS fixed_pad_posit;

	const boost::filesystem::path path_specs( filename );
	size_t nof_gates = read_specs( path_specs, netlist, fixed_pad_posit);

	if (nof_gates == 0)
		return -1;

	// find loops
	CONNECTIONS connected;
	NODESET nodes;

	typedef std::map<ID_NET, NODESET> NET_TO_NODE;
	NET_TO_NODE ntn;

	BOOST_FOREACH( const PROBLEMSET::value_type& v, netlist )
	{
		nodes.insert(v.first);
		BOOST_FOREACH(ID_NET net, v.second)
			ntn[ net ].insert( v.first );
	}

	BOOST_FOREACH( const NET_TO_NODE::value_type& v, ntn )
		connected.push_back(v.second);

	/*
	BOOST_FOREACH (ID_NODE const node, nodes)
	{
		size_t t = 0;
		NODESET nodes_merged;

		CONNECTIONS::iterator itFirst = connected.end();
		for ( CONNECTIONS::iterator itConn = connected.begin(); itConn != connected.end(); )
		{
			const NODESET& nodes = *itConn;
			if (nodes.find(node) != nodes.end())
			{
				// first occurrence
				if ( itFirst == connected.end() )
				{
					itFirst = itConn;
					t = 1;
				}
				else
				{
					t++;
					const NODESET& duplicate_nodes = *itConn;
					nodes_merged.insert(duplicate_nodes.begin(), duplicate_nodes.end());
					itConn = connected.erase(itConn);
					continue;
				}
			}

			itConn++;
		}

		if ( t != 1)
			cout << "node " << node << " shown " << t << "times" << endl;

		if ( !nodes_merged.empty() )
		{
			BOOST_ASSERT(itFirst != connected.end());
			itFirst->insert(nodes_merged.begin(), nodes_merged.end());
		}
	}*/

	///////////////////////////////////////////////////////////////////////////////////////////////
	//check_netlist( netlist );

	NODESET nodes_to_optimize;
	for (int i = 1; i <= nof_gates; i++)
		nodes_to_optimize.insert(i);

	AREA a;
	get_max_rect( fixed_pad_posit, a );
	cout << "pads create the area of " << a << endl;

	POSITIONS pos_result = split_optimize_merge(
		"", a, nodes_to_optimize,
		connected, fixed_pad_posit);

	if (argc >= 3)
	{
		std::string outfilename = "./";
		outfilename += argv[2];
		outfilename += ".placement";

		ofstream out ( outfilename );
		BOOST_FOREACH( const POSITIONS::value_type&v, pos_result)
		{
			const COORDINATES& coo = v.second;
		
			out << (v.first);
			out << ' ';
			out << coo.x;
			out << ' ';
			out << coo.y;
			out << std::endl;
		}
	}

	return 0;
}

