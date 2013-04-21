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


///////////////////////////////////////////////////////////////////////////////////////////

struct PINCOORDINATE
{
	size_t layer;
	size_t x;
	size_t y;
};

//typedef std::set<PINCOORDINATE> PINCOORDINATES;
typedef std::vector<PINCOORDINATE> PINCOORDINATES;
typedef std::vector<PINCOORDINATES> NETLIST;

//////////////////////////////////////////////////////////////////////////////////////////////

struct ONEROUTE
{
	size_t netid;
	PINCOORDINATES path;
};

typedef std::vector<ONEROUTE> ROUTING;


/////////////////////////////////////////////////////////////////////////////////////////////

void read_specs (const std::string& path, const std::string& name)
{
	const boost::filesystem::path p = path ;

	boost::filesystem::path path_netlist = p / (name + ".nl");
	boost::filesystem::path path_grid    = p / (name + ".grid");

	boost::filesystem::ifstream in_netlist ( path_netlist );

	while ( in_netlist )
	{
		size_t nof_nets;
		in_netlist >> nof_nets;

		NETLIST netlist;
		netlist.reserve(nof_nets);

		for (size_t n = 0; n < nof_nets; n++)
		{
			PINCOORDINATE pcoo1, pcoo2;

			in_netlist >> pcoo1.layer;
			in_netlist >> pcoo1.x;
			in_netlist >> pcoo1.y;

			in_netlist >> pcoo2.layer;
			in_netlist >> pcoo2.x;
			in_netlist >> pcoo2.y;

			PINCOORDINATES pcs;
			pcs.push_back( pcoo1 );
			pcs.push_back( pcoo2 );

			netlist.push_back( pcs );
		}
	}

	// Create a 3D array that is 3 x 4 x 2
	typedef boost::multi_array<int, 3> grid_type;
	typedef grid_type::index index;

	boost::filesystem::ifstream in_grid ( path_grid );

	if ( in_grid )
	{
		size_t size_x, size_y, bend_penalty, via_penalty;
		in_grid >> size_x >> size_y >> bend_penalty >> via_penalty;

		grid_type Grid( boost::extents[2][size_x][size_y] );

		for (size_t layer = 0; layer < 2; layer++)
			for (size_t x = 0; x < size_x; x++)
				for (size_t y = 0; y < size_y; y++)
				{
					int cost;
					in_grid >> cost;
					Grid[layer][x][y] = cost;
				}

		std::cout << "done" << std::endl;

	}
}

void out_results (const ROUTING& r)
{
	BOOST_FOREACH(const ONEROUTE& or, r)
	{

	}
}

//////////////////////////////////////////////////////////////////////////////////////


int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
		return -1;

	read_specs (argv[1], argv[2]);


	return 0;
}

