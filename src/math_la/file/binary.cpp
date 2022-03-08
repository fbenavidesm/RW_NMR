#include "binary.h"

namespace file
{

using std::ifstream;
using std::ofstream;

Binary::Binary(uint flag)
{
	if (flag == WRITE)
	{
		this->_flag = WRITE;
		this->_input = 0;
		this->_output = new ofstream();
	}
	else
	{
		this->_flag = READ;
		this->_output = 0;
		this->_input = new ifstream();
	}
}

Binary::~Binary()
{
	if (this->_output)
	{
		delete this->_output;
	}
	if (this->_input)
	{
		delete this->_input;
	}
}

bool Binary::Open(const string& filename)
{
	if (this->_input)
	{
		ifstream* f = (ifstream*)this->_input;
		f->open(filename.c_str(), std::ios::in | std::ios::binary);
		return(f->is_open());
	}
	if (this->_output)
	{
		ofstream* f = (ofstream*)this->_output;
		f->open(filename.c_str(), std::ios::out | std::ios::binary);
		return(f->is_open());
	}
	return(false);
}

void Binary::Close()
{
	if (this->_input)
	{
		ifstream* f = (ifstream*)this->_input;
		f->close();
	}
	if (this->_output)
	{
		ofstream* f = (ofstream*)this->_output;
		f->close();
	}
}

void Binary::Write(double v)
{
	DoubleChar dv;
	dv.dValue = v;
	this->_output->write(dv.chValue,SIZEDOUBLE);
}

void Binary::Write(float v)
{
	FloatChar fv;
	fv.fValue = v;
	this->_output->write(fv.chValue, SIZEFLOAT);
}

void Binary::Write(int v)
{
	IntChar iv;
	iv.iValue = v;
	this->_output->write(iv.chValue, SIZEINT);
}

void Binary::Write(uint v)
{
	UIntChar uiv;
	uiv.iValue = v;
	this->_output->write(uiv.chValue, SIZEUINT);
}

void Binary::Write(string ss, uint size, char fch)
{
	char* sc = new char[size];
	for (uint i = 0; i < (uint)ss.length(); ++i)
	{
		sc[i] = ss[i];
	}
	for (uint i = (uint)ss.length(); i < size; ++i)
	{
		sc[i] = fch;
	}
	this->_output->write(sc, size);
	delete[]sc;
}

void Binary::Write(char c)
{
	char* sc = new char[1];
	sc[0] = c;
	this->_output->write(sc, 1);
	delete[]sc;
}

void Binary::Write(uchar c)
{
	char* sc = new char[1];
	sc[0] = c;
	this->_output->write(sc, 1);
	delete[]sc;
}


float Binary::Read_Float()
{
	FloatChar fch;
	this->_input->read(fch.chValue, SIZEFLOAT);
	return(fch.fValue);
}

double Binary::Read_Double()
{
	DoubleChar dv;
	this->_input->read(dv.chValue, SIZEDOUBLE);
	return(dv.dValue);
}

char   Binary::Read_Char()
{
	char* r = new char[1];
	this->_input->read(r,1);
	char Voxel_Length = r[0];
	delete[]r;
	return(Voxel_Length);
}

uchar Binary::Read_UChar()
{
	char* r = new char[1];
	this->_input->read(r, 1);
	uchar Voxel_Length = (uchar)r[0];
	delete[]r;
	return(Voxel_Length);
}

string Binary::Read_String(uint size)
{
	string ss;
	char* r = new char[size+1];
	this->_input->read(r, size);
	r[size] = 0;
	ss = string(r);
	delete []r;
	return(ss);
}

int Binary::Read_Int()
{
	IntChar iv;
	this->_input->read(iv.chValue, SIZEINT);
	return(iv.iValue);
}

uint Binary::Read_UInt()
{
	UIntChar uv;
	this->_input->read(uv.chValue, SIZEUINT);
	return(uv.iValue);
}

void Binary::Write(bool t)
{
	BoolChar bc;
	bc.bValue = t;
	this->_output->write(bc.chValue, SIZEBOOL);
}

bool Binary::Read_Bool()
{
	BoolChar bc;
	this->_input->read(bc.chValue, SIZEBOOL);
	return(bc.bValue);
}

}