#include <istream>
#include <fstream>
#include "file.h"

using std::ifstream;

namespace file
{

bool File_Exists(const string& filename)
{
	ifstream f; 
	f.open(filename.c_str(), std::ios::in);
	bool r = false;
	if (f.is_open())
	{
		r = true;
		f.close();
	}
	return(r);
}

}