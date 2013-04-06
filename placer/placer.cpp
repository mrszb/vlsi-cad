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

///////////////////////////////////////////////////////////////////////

typedef size_t ID_GATE;
typedef size_t ID_NET;
typedef std::map<ID_GATE, std::set<ID_NET> > NETLIST;

typedef size_t ID_PAD;
struct PAD
{
	ID_NET id_net;
	double x, y;
};

typedef std::map<ID_PAD, PAD> PADLIST;

//////////////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
	std::ifstream is(argv[1]);

	if (!is)
		return -1;

	size_t nof_gates, nof_nets;

	is >> nof_gates;
	is >> nof_nets;

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
			netlist[gate].insert(net);
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

	coo_matrix A;
	int R[]    = {0, 0, 1, 1, 1, 2, 2};
	int C[]    = {0, 1, 0, 1, 2, 1, 2};
	double V[] = {4.0, -1.0, -1.0,  4.0, -1.0, -1.0, 4.0};
	A.n = 3;
	A.nnz = sizeof(R) / sizeof(int);
	A.row.resize(A.nnz);
	A.col.resize(A.nnz);
	A.dat.resize(A.nnz);

	A.row = valarray<int>(R, A.nnz);
	A.col = valarray<int>(C, A.nnz);
	A.dat = valarray<double>(V, A.nnz);

	// initialize as [1, 1, 1]
	valarray<double> x(1.0, A.n);
	valarray<double> b(A.n);
	A.matvec(x, b); // b = Ax

	cout << "b should equal [3,2,3]" << endl;
	print_valarray(b);

	// solve for x
	cout << "x = " << endl;
	A.solve(b, x);
	print_valarray(x);

	return 0;
}

