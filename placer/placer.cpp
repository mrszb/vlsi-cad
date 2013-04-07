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
typedef std::map<ID_NODE, DOUBLEVAL> NODE_TO_DOUBLEVAL;

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

void get_max_rect (const POSITIONS& pos, COORDINATES& coo_min, COORDINATES& coo_max)
{
	if (pos.empty())
		return;

	coo_min = pos.begin()->second;
	coo_max = pos.begin()->second;

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

void filter_by_rect (
	const POSITIONS& pos, 
	const COORDINATES& coo_min, const COORDINATES& coo_max,
	NODESET& inside_nodes)
{

	BOOST_FOREACH(const POSITIONS::value_type& v, pos)
	{
		if (coo_min.x > v.second.x)
			continue;

		if (coo_min.y > v.second.y)
			continue;

		if (coo_max.x < v.second.x)
			continue;

		if (coo_max.y < v.second.y)
			continue;

		inside_nodes.insert(v.first);
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

POSITIONS align_positions (
	// bounding rectangle
	const COORDINATES& bottom_left, 
	const COORDINATES& top_right,

	// position
	const POSITIONS& positions
	)
{
	POSITIONS al_positions;

	//adj_positions.reserve(positions.size());
	BOOST_FOREACH( const POSITIONS::value_type&v, positions)
	{
		COORDINATES newcoo = v.second;
		if (newcoo.x > top_right.x)
			newcoo.x = top_right.x;

		if (newcoo.y > top_right.y)
			newcoo.y = top_right.y;

		if (newcoo.x < bottom_left.x)
			newcoo.x = bottom_left.x;

		if (newcoo.y < bottom_left.y)
			newcoo.y = bottom_left.y;

		al_positions[v.first] = newcoo;
	}

	return al_positions;
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
	std::vector<EDGE> edges;
	std::map<ID_NODE, WEIGHT_2D> weight;
	NODE_TO_DOUBLEVAL diagonal;

	BOOST_FOREACH( const NETLIST::value_type& v, netlist )
	{
		if (v.first == 1092)
		{
			int j;
			j = 2;

		}
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

				diagonal [n1] += 1.0;
			}		
	}

	NODESET will_optimize;
	BOOST_FOREACH( const EDGE& e, edges)
	{
		ID_NODE n1, n2;
		n1 = e.first;
		n2 = e.second;

		// fix mapping
		BOOST_ASSERT ( n1 != n2 );
		BOOST_ASSERT ( n1 > 0 );
		BOOST_ASSERT ( n2 > 0 );

		will_optimize.insert(n1);
		will_optimize.insert(n2);
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
		ID_NODE n1, n2;
		n1 = e.first;
		n2 = e.second;

		IX_MAPPING::left_const_iterator left_iter1 = ixmap.left.find( n1 );
		BOOST_ASSERT (left_iter1 != ixmap.left.end());

		IX_MAPPING::left_const_iterator left_iter2 = ixmap.left.find( n2 );
		BOOST_ASSERT (left_iter2 != ixmap.left.end());

		R.push_back( left_iter1->second );
		C.push_back( left_iter2->second );
		V.push_back( -1.0 );

		diagonal [ n1 ] += 1.0;
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
	POSITIONS fixed_pad_posit;

	const boost::filesystem::path path_specs( filename );
	size_t nof_gates = read_specs( path_specs, netlist, fixed_pad_posit);

	if (nof_gates == 0)
		return -1;

	///////////////////////////////////////////////////////////////////////////////////////////////

	NODESET nodes_to_optimize;
	for (int i = 1; i <= nof_gates; i++)
		nodes_to_optimize.insert(i);

	// test - should not align at all
	COORDINATES coo_min, coo_max;
	get_max_rect( fixed_pad_posit, coo_min, coo_max );
	fixed_pad_posit = align_positions(coo_min, coo_max, fixed_pad_posit);

	POSITIONS posit_full_optimization;
	optimize_placement( nodes_to_optimize, 
		netlist,
		fixed_pad_posit, 
		posit_full_optimization);

	COORDINATES coo_middle;
	get_split_coo ( posit_full_optimization, coo_middle );

	POSITIONS pos_left, pos_right;
	NODESET nodes_left, nodes_right;

	BOOST_FOREACH( const POSITIONS::value_type& v, posit_full_optimization )
	{
		if (v.second.x >= coo_middle.x)
		{
			pos_right.insert(v);
			nodes_right.insert(v.first);
		}
		else
		{
			pos_left.insert(v);
			nodes_left.insert(v.first);
		}
	}

	COORDINATES coo_half_rightleft;
	coo_half_rightleft.x = coo_middle.x;
	coo_half_rightleft.y = coo_max.y;

	POSITIONS fixed_leftoptim = fixed_pad_posit;
	fixed_leftoptim.insert( pos_right.begin(), pos_right.end() );
	fixed_leftoptim = align_positions(coo_min, coo_half_rightleft, fixed_leftoptim);

	POSITIONS fixed_rightoptim = fixed_pad_posit;
	fixed_rightoptim.insert( pos_left.begin(), pos_left.end() );

	coo_half_rightleft.y = coo_min.y;
	fixed_rightoptim = align_positions(coo_half_rightleft, coo_max, fixed_rightoptim);

#if 1
	POSITIONS posit_left_optimization, posit_right_optimization;
	optimize_placement( nodes_left, 
		netlist,
		fixed_leftoptim, 
		posit_left_optimization);

	optimize_placement( nodes_right, 
		netlist,
		fixed_rightoptim, 
		posit_right_optimization);

	/*
	NODESET nodes_all;

	std::transform( 
		posit_1st_optimization.begin(), posit_1st_optimization.end(),
		std::inserter(nodes_all, nodes_all.begin()),
		boost::bind ( &POSITIONS::value_type::first, _1 )
	);

	set_difference(posit_1st_optimization.begin(), posit_1st_optimization.end(), 
		left_nodes.begin(), left_nodes.end(),
		std::back_inserter( right_nodes )
	);*/

	POSITIONS pos_result;
	pos_result.insert( posit_left_optimization.begin(),  posit_left_optimization.end() );
	pos_result.insert( posit_right_optimization.begin(), posit_right_optimization.end() );
#else

	POSITIONS pos_result = posit_full_optimization;
#endif

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

