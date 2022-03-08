#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include "math_la/mdefs.h"

using std::string;

namespace math_la
{
	namespace math_lac
	{
		namespace txt
		{

			class Converter
			{
			public:
				static int Convert_To_Int(const string& str);
				static scalar Convert_To_Scalar(const string& str);
				static string Convert_To_Digit_String(int number, int digits);
				static string Convert_Int(int i);
				static string Replace(const string& str, char src, char rep);
			};
		}
	}
}

#endif