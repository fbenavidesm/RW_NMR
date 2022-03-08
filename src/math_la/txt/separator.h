#ifndef SEPARATOR_H
#define SEPARATOR_H

#include "math_la/mdefs.h"
#include <string>
#include <vector>


namespace math_la
{
	namespace math_lac
	{
		namespace txt
		{
			using std::string;
			using std::vector;

			class Separator
			{
			private:
				string _separators;
				string _dataChars;
				char   _open;
				char   _close;
			public:
				static bool In(char c, const string& line);

				static vector<string> Get_Inner(char open, char close, const string& line);

				static string Filter(const string& datachars, const string& line);

				static bool	  Find(const string& data, const string& line);

				static vector<string> Separate(const string& line,
					const string& datachars,
					const string& separators);

				static vector<string> Separate(const string& line,
					const string& separators);

				static void Separate_File_Name(const string& name,
					string& filename,
					string& extension);

				Separator();
				void Set_Separators(const string& seps);
				void Set_Datachars(const string& data);
				void Set_Delimiters(char open, char close);
				vector<string> Separate(const string& line);
				static bool Find_Key_Word(const string& key, const string& line);
				static vector<scalar> Separate_Numbers(const string& line);
			};
		}
	}

}

#endif