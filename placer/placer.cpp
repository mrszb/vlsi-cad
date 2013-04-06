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

//////////////////////////////////////////////////////////////////////////////
// check results:
// https://class.coursera.org/vlsicad-001/wiki/view?page=VisualizePlacerResults
///////////////////////////////////////////////////////////////////////

typedef size_t ID_GATE;
typedef size_t ID_NET;

typedef std::set<ID_NET> NETSET;
typedef std::set<ID_GATE> GATESET;
typedef std::map<ID_GATE, NETSET> GATELIST;
typedef std::map<ID_NET, GATESET> NETLIST;

typedef size_t ID_PAD;
struct PAD
{
	ID_NET id_net;
	double x, y;
};

typedef std::map<ID_PAD, PAD> PADLIST;

//////////////////////////////////////////////////////////////////////

int _tmain(int argc, char* argv[])
{
	std::string filename = std::string(argv[1]) ;
	
	if (argc >= 3)
	{
		filename += "/";
		filename += argv[2];
	}

	std::ifstream is( filename );

	if (!is)
		return -1;

	size_t nof_gates, nof_nets;

	is >> nof_gates;
	is >> nof_nets;

	NETLIST gatelist;
	NETLIST netlist;
	PADLIST pads;

	for (size_t i = 0; i < nof_gates; ++i)
	{
		ID_GATE gate;
		is >> gate;

		assert( gate > 0 && gate <= nof_gates);

		size_t nof_nets_for_this_gate;
		is >> nof_nets_for_this_gate;

		for (size_t j = 0; j < nof_nets_for_this_gate; ++j)
		{
			ID_NET net;
			is >> net;

			assert( net > 0 && net <= nof_nets);
			gatelist[gate].insert(net);
			netlist[net].insert(gate);
		}
	}

	size_t nof_pads;
	is >> nof_pads;

	for (size_t i = 0; i < nof_pads; ++i)
	{
		ID_PAD pinid;
		is >> pinid;

		assert( pinid > 0 && pinid <= nof_pads);

		PAD pad;
		is >> pad.id_net;
		is >> pad.x;
		is >> pad.y;

		pads[pinid] = pad;
	}

	std::cout << pads.size() << std::endl;


	///////////////////////////////////////////////////////////////////////////////////////////////

	struct GATE_CONNECTION
	{
		ID_GATE g1, g2;
	};

	std::vector<GATE_CONNECTION> vgateconn;
	
	for (NETLIST::const_iterator itNet = netlist.begin(); itNet != netlist.end(); itNet++)
	{
		const NETSET& nets = itNet->second;

		for (NETSET::const_iterator i = nets.begin(); i != nets.end(); ++i)
			for (NETSET::const_iterator j = nets.begin(); j != nets.end(); ++j)
			{
				GATE_CONNECTION gc;
				gc.g1 = *i;
				gc.g2 = *j;
				if (gc.g1 != gc.g2)
					vgateconn.push_back(gc);
			}
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	std::vector<int> R,C;
	std::vector<double> V;
	R.reserve(vgateconn.size());

	valarray<double> diagonal(0.0, nof_gates);
	valarray<double> xpads(0.0, nof_gates);
	valarray<double> ypads(0.0, nof_gates);

	for (PADLIST::const_iterator itPad = pads.begin(); itPad != pads.end(); itPad++)
	{
		const PAD& pad = itPad->second;
		NETLIST::const_iterator itGates = netlist.find(pad.id_net);
		assert(itGates != netlist.end());

		const NETSET& nets = itGates->second;
		//assert (nets.size() == 1);
		for (NETSET::const_iterator itN = nets.begin(); itN != nets.end(); itN++)
		{
			const ID_GATE& id_gate = *itN;
			diagonal[id_gate - 1] += 1.0;

			xpads[id_gate - 1] += pad.x;
			ypads[id_gate - 1] += pad.y;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////

	for (std::vector<GATE_CONNECTION>::const_iterator itG = vgateconn.begin(); 
			itG != vgateconn.end(); itG++)
	{
		const GATE_CONNECTION& gc = *itG;
		assert (gc.g1 != gc.g2 );

		R.push_back( gc.g1 - 1);
		C.push_back( gc.g2 - 1);
		V.push_back( -1.0 );

		diagonal[gc.g1 -1] += 1.0;
	}


	for (size_t d=0; d < diagonal.size(); d++)
	{
		R.push_back( d );
		C.push_back( d);
		V.push_back( diagonal[d] );
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

	// initialize as [1, 1, 1]
	valarray<double> x(1.0, A.n);
	valarray<double> y(1.0, A.n);

	// solve for x
	cout << "x = " << endl;
	A.solve(xpads, x);
	print_valarray(x);

	// solve for y
	cout << "y = " << endl;
	A.solve(ypads, y);
	print_valarray(y);
	
	if (argc >=3)
	{
		std::string outfilename = "./";
		outfilename += argv[2];
		outfilename += ".placement";

		ofstream out ( outfilename );
		for (size_t g = 0; g < nof_gates; g++)
		{
			out << (g+1);
			out << ' ';
			out << x[g];
			out << ' ';
			out << y[g];
			out << std::endl;
		}
	}

	return 0;
}

