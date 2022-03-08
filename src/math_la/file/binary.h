#ifndef FILE_BINARY_H
#define FILE_BINARY_H

#include <string>
#include <ostream>
#include <istream>
#include <fstream>

#include "math_la\mdefs.h"

namespace file
{

using std::ostream;
using std::istream;
using std::string;

#define READ  4
#define WRITE 8

	/**
	* This class stores and loads binary file. These files store integers, doubles and floats
	* in binary format, as a sequence of bits in a file. 
	*/
	class Binary
	{
	private:
		uint _flag;
		ostream* _output;
		istream* _input;
	public:
		Binary(uint flag);
		bool Open(const string& filename);
		void Close();
		void Write(double v);
		void Write(float v);
		void Write(int v);
		void Write(uint v);
		void Write(string ss, uint size, char fch = 0);
		void Write(char c);
		void Write(uchar c);
		void Write(bool t);
		float Read_Float();
		double Read_Double();
		char   Read_Char();
		string Read_String(uint size);
		int Read_Int();
		uint Read_UInt();
		bool Read_Bool();
		uchar Read_UChar();
		~Binary();

	};

}

#endif