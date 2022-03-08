#ifndef SIGMOID_H
#define SIGMOID_H

#include "relaxivity_distribution.h"

namespace rw
{

class Sigmoid : public rw::RelaxivityDistribution
{
private:
	scalar* _K;
	scalar* _A;
	scalar* _xi;
	scalar* _sigmoidSlopeFactor;
	scalar _slopeScale;
	int _size;
public:
	Sigmoid();
	void Set_Size(int n);
	void Set_Value(int id, scalar Decay_Step_Value);
	void Set_Slope_Scale(scalar slope);
	scalar K(int id) const;
	scalar A(int id) const;
	scalar XI(int id) const;
	scalar Slope(int id) const;
	scalar Evaluate(scalar xi) const;
	int Size() const;
	~Sigmoid();
};

inline int Sigmoid::Size() const
{
	return(this->_size);
}

}

#endif