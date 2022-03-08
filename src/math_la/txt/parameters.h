#ifndef PARAMS_H
#define PARAMS_H

#include <map>
#include <string>
#include "math_la/mdefs.h"

using std::map;
using std::string;

namespace math_la
{
	namespace math_lac
	{
		namespace txt
		{
			class Params
			{
			private:
				int				    _numParams;
				map<string, string>	_parameterMap;
				bool				_validText;
			public:
				Params(int n, char** args);
				Params();
				void Load_From_File(const string& fn, char sep = ':');
				bool Valid() const;
				bool Exists(const string& pname) const;
				string operator[](const string& pname) const;
			};
		}
	}
}

#endif