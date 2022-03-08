#include "sigmoid.h"

namespace rw
{

Sigmoid::Sigmoid() :RelaxivityDistribution()
{
	this->_slopeScale = 1;
	this->_size = 0;
	this->_K = 0;
	this->_A = 0;
	this->_xi = 0;
	this->_sigmoidSlopeFactor = 0;
}

Sigmoid::~Sigmoid()
{
	if (this->_K)
	{
		delete[]this->_K;
		delete[]this->_A;
		delete[]this->_xi;
		delete[]this->_sigmoidSlopeFactor;
	}
}

void Sigmoid::Set_Slope_Scale(scalar slope)
{
	this->_slopeScale = slope;
}

scalar Sigmoid::Evaluate(scalar xi) const
{
	scalar r = 0;
	for (int k = 0; k < this->_size; ++k)
	{
		r = r + this->_A[k] + (this->_K[k] - this->_A[k]) / (1 + exp(-this->_sigmoidSlopeFactor[k] * (xi - this->_xi[k])));
	}
	return(r);
}

void Sigmoid::Set_Size(int n)
{
	this->_K = new scalar[n];
	this->_A = new scalar[n];
	this->_xi = new scalar[n];
	this->_sigmoidSlopeFactor = new scalar[n];
	this->_size = n;
}

void Sigmoid::Set_Value(int id, scalar value)
{
	int f = id % 4;
	switch (f)
	{
	case 0:
		this->_K[id / 4] = value;
		break;
	case 1:
		this->_A[id / 4] = value;
		break;
	case 2:
		this->_xi[id / 4] = value;
		break;
	default:
		this->_sigmoidSlopeFactor[id / 4] = value * this->_slopeScale;
		break;
	}
}

scalar Sigmoid::K(int id) const
{
	return(this->_K[id]);
}

scalar Sigmoid::A(int id) const
{
	return(this->_A[id]);
}

scalar Sigmoid::XI(int id) const
{
	return(this->_xi[id]);
}

scalar Sigmoid::Slope(int id) const
{
	return(this->_sigmoidSlopeFactor[id]);
}


}