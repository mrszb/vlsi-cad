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
};

typedef int ID_NODE;
typedef size_t ID_NET;

typedef std::set<ID_NODE> NODESET;
typedef std::map<ID_NET, NODESET> NETLIST;
typedef std::pair<ID_NODE, ID_NODE> EDGE;
typedef std::map<ID_NODE, COORDINATES> POSITIONS;

typedef std::map<ID_NODE, WEIGHT_2D> NODE_WEIGHTS;

////////////////////////////////////////////////////////////////////////

int read_specs ( const boost::filesystem::path& path, NETLIST& netlist, POSITIONS& posit )
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
			netlist[net].insert(gate);
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
		netlist[netid].insert(-pinid);

		/////////////////////////////////////////////////////////////////////

		COORDINATES coo;
		is >> coo.x;
		is >> coo.y;

		posit[-pinid] = coo;
	}

	return nof_gates;
}

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

void get_rect (const POSITIONS& pos, COORDINATES& coo_min, COORDINATES& coo_max)
{
	if (pos.empty())
		return;

	coo_min = pos.begin()->second;
	BOOST_FOREACH(const POSITIONS::value_type& v, pos)
	{
		if (coo_min.x > v.second.x)
			coo_min.x = v.second.x;

		if (coo_min.y > v.second.y)
			coo_min.y = v.second.y;

		if (coo_max.x < v.second.x)
			coo_max.x = v.second.x;

		if (coo_max.y < v.second.y)
			coo_max.y = v.second.y;
	}
}

void move_positions (
	// bounding rectangle
	const COORDINATES& bottom_left, 
	const COORDINATES& top_right,

	// position
	const POSITIONS& positions,

	// adjusted position
	POSITIONS& adj_positions
	)
{
	//adj_positions.reserve(positions.size());
	BOOST_FOREACH( const POSITIONS::value_type&v, positions)
	{
		COORDINATES coo = v.second;
		adj_positions[v.first] = coo;
	}

}


void optimize_placement( 
	// input:
	const NODESET& need_optimize,
	const NETLIST& netlist,

	// position
	const POSITIONS& fixed_positions,
	
	// output
	POSITIONS& optimized_positions )
{

	NODESET will_optimize = need_optimize;

	std::vector<EDGE> edges;
	std::map<ID_NODE, WEIGHT_2D> weight;
	std::map<ID_NODE, DOUBLEVAL> diagonal;

	BOOST_FOREACH( const NETLIST::value_type& v, netlist )
	{
		const NODESET& nodes = v.second;

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
					edges.push_back( EDGE(n1,n2) );

		BOOST_FOREACH(ID_NODE const n1, nodes_optim)
			BOOST_FOREACH(ID_NODE const n2, nodes_fixed)
			{
				POSITIONS::const_iterator itWeights = fixed_positions.find( n2 );
				BOOST_ASSERT( itWeights != fixed_positions.end() );
				const COORDINATES& coo = itWeights->second;

				WEIGHT_2D& w = weight[n1];
				w.x += coo.x;
				w.y += coo.y;

				diagonal [n1 - 1] += 1.0;
			}		
	}

	const size_t nof_gates = will_optimize.size();

	std::vector<int> R,C;
	std::vector<double> V;
	R.reserve( edges.size() + nof_gates);

	BOOST_FOREACH( const EDGE& e, edges)
	{
		ID_NODE n1, n2;
		n1 = e.first;
		n2 = e.second;

		// fix mapping
		BOOST_ASSERT ( n1 != n2 );
		BOOST_ASSERT ( n1 > 0 );
		BOOST_ASSERT ( n2 > 0 );

		R.push_back( n1 - 1);
		C.push_back( n2 - 1);
		V.push_back( -1.0 );

		diagonal [n1 - 1] += 1.0;
	}

	for (size_t d=0; d < diagonal.size(); d++)
	{
		R.push_back( d );
		C.push_back( d);
		V.push_back( diagonal[d] );
	}

	valarray<double> xpads(0.0, nof_gates);
	valarray<double> ypads(0.0, nof_gates);

	BOOST_FOREACH(const NODE_WEIGHTS::value_type& w, weight)
	{
		int wno = w.first -1;
		xpads[wno] = w.second.x;
		ypads[wno] = w.second.y;
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

		optimized_positions[d+1] = coo;
	}
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

	//////////////////////////////////////////////////////////////////////////////////////////////

	NETLIST netlist; 
	POSITIONS posit;

	const boost::filesystem::path path_specs( filename );
	size_t nof_gates = read_specs( path_specs, netlist, posit);

	if (nof_gates == 0)
		return -1;

	///////////////////////////////////////////////////////////////////////////////////////////////

	NODESET nodes_to_optimize;
	for (int i = 1; i <= nof_gates; i++)
		nodes_to_optimize.insert(i);

	COORDINATES coo_min, coo_max;
	get_rect( posit, coo_min, coo_max );

	POSITIONS fixed_posit;
	move_positions(coo_min, coo_max, posit, fixed_posit);

	POSITIONS new_posit;
	optimize_placement( nodes_to_optimize, 
		netlist,
		fixed_posit, 
		new_posit);

	if (argc >= 3)
	{
		std::string outfilename = "./";
		outfilename += argv[2];
		outfilename += ".placement";

		ofstream out ( outfilename );
		BOOST_FOREACH( const POSITIONS::value_type&v, new_posit)
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

