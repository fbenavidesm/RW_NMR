#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "converter.h"

namespace math_la
{
	namespace math_lac
	{

		namespace txt
		{
			int Converter::Convert_To_Int(const string& str)
			{
				return(std::atoi(str.c_str()));
			}

			scalar Converter::Convert_To_Scalar(const string& str)
			{
				return((scalar)std::atof(str.c_str()));
			}

			string Converter::Replace(const string& str, char src, char rep)
			{
				string r = str;
				for (int i = 0; i < r.length(); ++i)
				{
					if (r[i] == src)
					{
						r[i] = rep;
					}
				}
				return(r);
			}

			string Converter::Convert_Int(int i)
			{
				std::stringstream ss;
				ss << i;
				string r = ss.str();
				return(r);
			}

			string Converter::Convert_To_Digit_String(int number, int digits)
			{
				string r = "";
				int p = 1;
				uint zeros = 0;
				for (uint i = 0; i < (uint)digits; ++i)
				{
					p = p * 10;
				}
				if (number > p)
				{
					throw std::runtime_error("Number out of bounds");
				}
				else
				{
					zeros = digits - 1;
					uint p = 10;
					while (p <= (uint)number)
					{
						p = p * 10;
						zeros = zeros - 1;
					}
				}
				for (uint i = 0; i < zeros; ++i)
				{
					r = r + "0";
				}
				r = r + Converter::Convert_Int(number);
				return(r);
			}
		}
	}
}