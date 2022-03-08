#include "separator.h"
#include "converter.h"

namespace math_la
{

	namespace math_lac
	{

		namespace txt
		{

			Separator::Separator()
			{
				this->_close = 0;
				this->_open = 0;
			}

			bool Separator::In(char c, const string& line)
			{
				int i = 0;
				bool in = false;
				while ((!in) && (i < (int)line.length()))
				{
					if (c == line[i])
					{
						in = true;
					}
					++i;
				}
				return(in);
			}

			void Separator::Set_Delimiters(char open, char close)
			{
				this->_open = open;
				this->_close = close;
			}

			void Separator::Set_Separators(const string& seps)
			{
				this->_separators = seps;
			}

			void Separator::Set_Datachars(const string& data)
			{
				this->_dataChars = data;
			}

			vector<string> Separator::Separate(const string& line)
			{
				vector<string> ls;
				if (this->_open != 0)
				{
					ls = this->Get_Inner(this->_open, this->_close, line);
				}
				else
				{
					ls.push_back(line);
				}
				vector<string>::iterator itr = ls.begin();
				vector <string> r;
				while (itr != ls.end())
				{
					string cline = *itr;
					bool separating = true;
					string element;
					for (int i = 0; i < (int)cline.length(); ++i)
					{
						char c = cline[i];
						if (this->In(c, this->_dataChars))
						{
							element = element + c;
							separating = false;
						}
						if ((this->In(c, this->_separators)) || (i == (int)cline.length() - 1))
						{
							if (!separating)
							{
								r.push_back(element);
								element = "";
							}
							separating = true;
						}
					}
					++itr;
				}
				return(r);
			}

			vector<string> Separator::Separate(const string& line,
				const string& separators)
			{
				vector <string> r;
				bool separating = true;
				string element;
				for (int i = 0; i < (int)line.length(); ++i)
				{
					char c = line[i];
					if (Separator::In(c, separators))
					{
						if (!separating)
						{
							r.push_back(element);
							element = "";
						}
						separating = true;
					}
					else
					{
						element = element + c;
						separating = false;
						if (i == (int)line.length() - 1)
						{
							r.push_back(element);
						}
					}
				}
				return(r);
			}


			vector<string> Separator::Get_Inner(char open, char close, const string& line)
			{
				bool getting = false;
				int Regularizer = 0;
				string r = "";
				int i = 0;
				vector<string> retLst;
				while (i < (int)line.length())
				{
					char c = line[i];
					if (c == close)
					{
						--Regularizer;
						if (Regularizer <= 0)
						{
							getting = false;
							if (r.length() > 0)
							{
								retLst.push_back(r);
								r = "";
								Regularizer = 0;
							}
						}
					}
					if (getting)
					{
						r = r + c;
					}
					if (c == open)
					{
						++Regularizer;
						getting = true;
					}
					++i;
				}
				return(retLst);
			}

			vector<string> Separator::Separate(const string& line, const string& datachars, const string& separators)
			{
				Separator Voxel_Length;
				Voxel_Length.Set_Datachars(datachars);
				Voxel_Length.Set_Separators(separators);
				return(Voxel_Length.Separate(line));
			}

			string Separator::Filter(const string& datachars, const string& line)
			{
				string r = "";
				for (int i = 0; i < (int)line.length(); ++i)
				{
					char c = line[i];
					if (Separator::In(c, datachars))
					{
						r = r + c;
					}
				}
				return(r);
			}

			bool	  Separator::Find(const string& data, const string& line)
			{
				bool r = false;
				int i = 0;
				while ((i < (int)line.length()) && (!r))
				{
					char c = line[i];
					if (Separator::In(c, data))
					{
						r = true;
					}
					++i;
				}
				return(r);
			}

			void Separator::Separate_File_Name(const string& name,
				string& filename,
				string& extension)
			{
				vector<string> ss = Separator::Separate(name, ".");
				if (ss.size() >= 2)
				{
					filename = ss[0];
					extension = ss[1];
				}
				if (ss.size() == 1)
				{
					filename = ss[0];
					extension = "";
				}
				if (ss.size() == 0)
				{
					filename = "";
					extension = "";
				}
			}

			bool Separator::Find_Key_Word(const string& key, const string& line)
			{
				size_t p = line.find(key);
				if (p == string::npos)
				{
					return(false);
				}
				else
				{
					return(true);
				}
			}

			vector<scalar> Separator::Separate_Numbers(const string& line)
			{
				string nfilter = "0123456789.-";
				string cn = "";
				vector <string> snumbers;
				for (int i = 0; i < line.length(); ++i)
				{
					char Voxel_Length = line[i];
					if (math_la::math_lac::txt::Separator::In(Voxel_Length, nfilter))
					{
						cn = cn + Voxel_Length;
					}
					else
					{
						if (cn.length() > 0)
						{
							snumbers.push_back(cn);
						}
						cn = "";
					}
				}
				if (cn.length() > 0)
				{
					snumbers.push_back(cn);
				}
				vector<scalar> r;
				for (int i = 0; i < snumbers.size(); ++i)
				{
					r.push_back(math_la::math_lac::txt::Converter::Convert_To_Scalar(snumbers[i]));
				}
				return(r);
			}

		}
	}
}