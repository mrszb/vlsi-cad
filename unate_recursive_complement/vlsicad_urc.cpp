// vlsicad_urc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <boost/format.hpp>

///////////////////////////////////////////////////

enum CUBEPTS
{
	XT	= 1,
	XF  = 2,
	XX  = 3
};

typedef std::vector<CUBEPTS> cubeline;
typedef std::vector<cubeline> clist;

///////////////////////////////////////////////////

bool has_all_dont_care ( const cubeline& cl )
{
	for ( size_t i = 0; i < cl.size(); i++ )
	{
		if (cl[i] != XX)
			return false;
	}

	return true;
}

bool can_and_cube(const cubeline& cla, const cubeline& clb, cubeline& cl)
{
	cl.clear();

	for ( size_t i = 0; i < cla.size(); i++ )
	{
		if ((cla[i] == XF && clb[i] == XT) || (cla[i] == XT && clb[i] == XF))
			return false;

		else if (cla[i] == XX)
			cl.push_back(clb[i]);

		else 
			cl.push_back(cla[i]);

	}
	return true;
}

std::vector<cubeline> not_cube(const cubeline& cl)
{
	std::vector<cubeline> cubelines;

	for ( size_t i = 0; i < cl.size(); i++ )
	{
		if (cl[i] != XX)
		{
			cubeline clnew;
			for ( size_t j = 0; j < cl.size(); j++ )
			{
				if (i==j)
				{
					if (cl[i] == XT)
						clnew.push_back(XF);
					else
						clnew.push_back(XT);
				}
				else
				 clnew.push_back(XX);
			}

			cubelines.push_back(clnew);
		}
	}

	return cubelines;
}

struct cubeList
{
	size_t _nof_vars;
	clist _l;

	bool can_be_converted (cubeList& result) const;
	bool can_be_simplified(cubeList& result) const;

	bool is_done() const;
	size_t get_nof_vars (void) const { return _nof_vars; };
	bool has_some_dont_care(void) const;

	void read	( std::istream& is );
	void write	( std::ostream& os );
};

typedef std::map<int,cubeList> cubeListMemory;

////////////////////////////////////////////////////////

bool cubeList::has_some_dont_care(void) const
{
	for (size_t i = 0; i < _l.size(); i++)
	{
		if ( has_all_dont_care(_l[i]) )
			return true;
	}

	return false;
}

bool cubeList::can_be_simplified(cubeList& result) const
{
	if ( _l.empty() )
	{
		// Bolean 0:
		return true;
	}
	else if (has_some_dont_care())
	{
		// return 1 [11 11 11 ...11]

		cubeline clres;

		for (size_t i = 0; i < get_nof_vars(); i++)
			clres.push_back(XX);

		result._l.push_back(clres);
		return true;
	}
	else if (_l.size()==1)
	{
		const cubeline cline = _l[0];
		result._l.push_back(cline);
		return true;
	}

	return false;
}


bool cubeList::can_be_converted(cubeList& result) const
{
	if ( _l.empty() )
	{
		// Bolean 0:
		// return 1 [11 11 11 ...11]

		cubeline clres;

		for (size_t i = 0; i < get_nof_vars(); i++)
			clres.push_back(XX);

		result._l.push_back(clres);
		return true;
	}
	else if (has_some_dont_care())
	{
		// Boolean T
		// return empty list
		return true;
	}
	else if (_l.size()==1)
	{
		const cubeline cline = _l[0];
		// De Morgan

		for (size_t i = 0; i < get_nof_vars(); i++)
		{
			cubeline clres;

			switch ( cline[i] )
			{
				case XX:
					break;

				case XT:

					for ( size_t j = 0; j < get_nof_vars(); j++ )
						clres.push_back( (j == i) ?	XF : XX);
					result._l.push_back(clres);
					break;

				case XF:

					for ( size_t j = 0; j < get_nof_vars(); j++ )
						clres.push_back( (j == i) ? XT : XX);
					result._l.push_back(clres);

					break;
			}
		}

		return true;
	}


	return false;
}

bool cubeList::is_done() const
{
	return (_l.size()<=1);
}

void cubeList::read(std::istream& is)
{
	is >> _nof_vars;

	int lines;
	is >> lines;


	_l.reserve(lines);

	/////////////////////////////////////

	while (lines--> 0)
	{
		cubeline ln;
		ln.reserve( get_nof_vars() );

		for (size_t i = 0; i < get_nof_vars(); ++i)
			ln.push_back(XX);

		size_t cnt;
		is >> cnt;

		for (size_t i = 0; i < cnt; ++i)
		{
			int var;
			is >> var;

			if (var < 0)
				ln[-var-1] = XF;
			else
				ln[var-1] = XT;
		}

		_l.push_back(ln);
	}
}

void cubeList::write (std::ostream& os)
{
	os << get_nof_vars() << std::endl;
	os << _l.size() << std::endl;

	for (size_t cubeno = 0; cubeno < _l.size(); ++cubeno)
	{
		const cubeline& cl = _l[ cubeno ];

		std::vector<int> carevar;

		for (size_t i = 0; i < get_nof_vars(); ++i)
		{
			switch (cl[i])
			{

				case XT:
					carevar.push_back(i+1);
					break;

				case XF:
					carevar.push_back(-(i+1));
					break;

				case XX:
					break;

				default:
					break;
			}
		}

		os << carevar.size();
		os << " ";

		for (size_t i = 0; i < carevar.size(); ++i)
		{
			os << carevar[i];
			os << " ";
		}

		os << std::endl;
	}

}

/////////////////////////////////////////////////////////////////////////////

struct binate {
	size_t t;
	size_t f;

	binate ()
	{
		t = f = 0;
	}

	///////////////////////////////////////////

	bool is_binate (void) const
	{
		return ( t != 0 && f != 0 );
	}

	size_t get_binates() const
	{
		return (is_binate()) ? t+f : 0;
	}

	size_t get_unate() const
	{
		return !is_binate() ? t+f : 0;
	}

	int get_tminc (void) const
	{
		return (t>f) ? t-f : f-t;
	}
};

size_t find_split_index (const cubeList& cl )
{
	std::vector<binate> bincnt;
	bincnt.resize( cl.get_nof_vars() );

	for ( clist::const_iterator it = cl._l.begin();
		it != cl._l.end(); ++it
		)
	{
		const cubeline& cline = *it;
		for (size_t ix = 0; ix < bincnt.size(); ++ix)
		{
			if (cline[ix] == XT)
				bincnt[ix].t++;

			else if (cline[ix] == XF)
				bincnt[ix].f++;

			//else if (cline[ix] == NOX)
			//	bincnt[ix].n++;
		}
	}

	std::vector<size_t> most_binate;
	size_t max_binatness = 1;

	std::vector<size_t> most_unate;
	size_t max_unatness = 1;

	for (size_t ix = 0; ix < bincnt.size(); ++ix)
	{
		const binate& b = bincnt[ix];
		size_t mb = b.get_binates();
		size_t mu = b.get_unate();

		if (mb > max_binatness)
		{
			most_binate.clear();
			max_binatness = mb;
		}

		if (mb == max_binatness)
			most_binate.push_back(ix);

		if (mu > max_unatness)
		{
			most_unate.clear();
			max_unatness = mu;
		}

		if (mu == max_unatness)
			most_unate.push_back(ix);
	}

	if (!most_binate.empty())
	{
		std::vector<size_t> min_tc;
		int min_tc_val = max_binatness+3;

		for (size_t i = 0; i < most_binate.size(); ++i)
		{
			size_t j = most_binate[i];
			int tmic = bincnt[j].get_tminc();
			if (tmic < min_tc_val)
			{
				min_tc.clear();
				min_tc_val = tmic;
			}

			if (min_tc_val == tmic)
				min_tc.push_back(j);
		}

		return *min_tc.begin();
	}
	else
	{
		return *most_unate.begin();
	}

	return 0;
}

cubeList cofactor(const cubeList& cl, size_t x, bool positive)
{
	cubeList res = cl;

	for ( clist::iterator it = res._l.begin();
		it != res._l.end();
		)
	{
		cubeline& cline = *it;
		if ( (cline[x] == XT && positive ) || (cline[x] == XF && !positive) )
		{
			cline[x] = XX;
		}
		else if (cline[x] != XX)
		{
			it = res._l.erase(it);
			continue;
		}
		it++;
	}

	return res;
}

cubeList or (const cubeList& cl1, const cubeList& cl2)
{
	cubeList res = cl1;

	for (size_t i = 0; i < cl2._l.size(); ++i)
	{
		const cubeline& cl = cl2._l[i];
		res._l.push_back(cl);
	}

	return res;
}

////////////////////////////////////////////////////////////////////////

cubeList and(const cubeList& cl, size_t x, bool positive)
{
	cubeList res = cl;

	for (size_t i = 0; i < res._l.size(); ++i)
	{
		cubeline& clline = res._l[i];

		clline[x] = positive ? XT : XF;
	}

	return res;
}

cubeList Complement (const cubeList& F)
{
	cubeList res;
	res._nof_vars = F.get_nof_vars();

	if (F.can_be_converted(res))
		return res;
	else
	{
		size_t x = find_split_index(F);

		cubeList P = Complement(cofactor(F,x, true));
		cubeList N = Complement(cofactor(F,x, false));

		P = and( P, x, true);
		N = and( N, x, false);

		return or (P,N);
	}

	return res;
}

cubeList NComplement (const cubeList& F)
{
	cubeList res;
	res._nof_vars = F.get_nof_vars();

	if (F.can_be_simplified(res))
		return res;
	else
	{
		size_t x = find_split_index(F);

		cubeList P = NComplement(cofactor(F,x, true));
		cubeList N = NComplement(cofactor(F,x, false));

		P = and( P, x, true);
		N = and( N, x, false);

		return or (P,N);

	}

	return res;
}


cubeList and (const cubeList& cl1, const cubeList& cl2)
{
	cubeList res;
	res._nof_vars = cl1.get_nof_vars();

	for (size_t i = 0; i < cl1._l.size(); ++i)
	{
		for (size_t j = 0; j < cl2._l.size(); ++j)
		{
			const cubeline& cla = cl1._l[i];
			const cubeline& clb = cl2._l[j];

			cubeline clres;
			if (can_and_cube(cla, clb, clres))
				res._l.push_back(clres);
		}
	}
	res = NComplement(res);
	return res;
}

cubeList not (const cubeList& cl)
{
	cubeList res;
	res =  Complement(cl);
	return res;
}

////////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		return -1;
	}

	std::string name = argv[1];
	std::ifstream in(name);

	if (name.find(_T("cmd")) == std::string::npos)
	{
		cubeList cl;
		cl.read(in);

		cubeList clres = Complement(cl);
		//cl.write (std::cout);
		clres.write (std::cout);
	}
	else
	{
		cubeListMemory memory;
		std::string line;
		while (std::getline(in, line ))
		{
			std::cout << line << std::endl;

			std::stringstream iss (line);

			char command;
			if (!(iss >> command)) 
				{ break; } // error

			switch (command)
			{

				case 'q':
					break;

				case 'r':
					{
						int n;
						iss >> n;
						std::string filename = str (boost::format("%d.pcn") % n);
						std::ifstream in(filename);

						cubeList cl;
						cl.read(in);
						memory[n] = cl;
					}
					break;

				case 'p':
					{
						int n;
						iss >> n;
						std::string filename = str (boost::format("%d.pcn") % n);
						std::ofstream out(filename);
						memory[n].write(out);
					}
					break;


				case '!':
					{
						int k, n;
						iss >> k >> n;
						memory[k] = not (memory[n]);
					}
					break;


				case '+':
					{
						int k, n, m;
						iss >> k >> n >> m;
						memory[k] = or (memory[n], memory[m]);
					}

					break;


				case '&':
					{
						int k, n, m;
						iss >> k >> n >> m;
						memory[k] = and (memory[n], memory[m]);
					}
					break;


			}

		}
	}
	return 0;
}

