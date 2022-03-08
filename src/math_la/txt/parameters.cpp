#include <fstream>
#include <istream>
#include <string>
#include <vector>
#include "separator.h"
#include "parameters.h"

using std::vector;
using std::string;

namespace math_la
{
	namespace math_lac
	{
		namespace txt
		{

			Params::Params(int n, char** args)
			{
				int nn = n - 1;
				this->_validText = false;
				if ((nn > 1) && (nn % 2 == 0))
				{
					this->_numParams = nn / 2;
					this->_validText = true;
					for (int i = 0; i < this->_numParams; ++i)
					{
						int pni = 2 * i + 1;
						int pi = 2 * i + 2;
						string param = args[pni];
						string value = args[pi];
						this->_parameterMap[param] = value;
					}
				}
			}

			void Params::Load_From_File(const string& fn, char sep)
			{
				std::ifstream file(fn);
				string line;
				while (!file.eof())
				{
					std::getline(file, line);
					if ((!math_la::math_lac::txt::Separator::Find_Key_Word("*", line)) && (math_la::math_lac::txt::Separator::Find_Key_Word(":", line)))
					{
						vector<string> pline = math_la::math_lac::txt::Separator::Separate(line, ":; ");
						this->_parameterMap[pline[0]] = pline[1];
					}
				}
				file.close();
			}

			Params::Params()
			{
				this->_numParams = 0;
				this->_validText = false;
			}

			bool Params::Valid() const
			{
				return(this->_validText);
			}

			bool Params::Exists(const string& pname) const
			{
				map<string, string>::const_iterator i = this->_parameterMap.find(pname);
				if (i != this->_parameterMap.end())
				{
					return(true);
				}
				else
				{
					return(false);
				}
			}

			string Params::operator[](const string& pname) const
			{
				if (this->Exists(pname))
				{
					map<string, string>::const_iterator i = this->_parameterMap.find(pname);
					return(i->second);
				}
				else
				{
					string Voxel_Length = "";
					return(Voxel_Length);
				}
			}
		}
	}
}
