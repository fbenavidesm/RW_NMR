#include "hat.h"

namespace rw
{

scalar Hat::Evaluate(scalar xi) const
{
	scalar r = 0;
	for (int k = 0; k < this->_size; ++k)
	{
		if ((xi >= this->_min[k]) && (xi <= this->_max[k]))
		{
			r = r + this->_K[k];
		}
	}
	if (r == 0)
	{
		r = this->_ground;
	}
	return(r);
}

void Hat::Set_Size(int n)
{
	this->_size = n;
	this->_max = new scalar[n];
	this->_min = new scalar[n];
	this->_K = new scalar[n];
}

void Hat::Set_Value(int id, scalar Decay_Step_Value)
{
	int f = id % 3;
	switch (f)
	{
	case 0:
		this->_K[id / 3] = Decay_Step_Value;
		break;
	case 1:
		this->_min[id / 3] = Decay_Step_Value;
		break;
	default:
		this->_max[id / 3] = Decay_Step_Value;
		break;
	}
}

void Hat::Set_Ground(scalar gr)
{
	this->_ground = gr;
}

Hat::~Hat()
{
	delete []this->_max;
	delete []this->_min; 
	delete []this->_K;
}

scalar Hat::K(int id) const
{
	return(this->_K[id]);
}

scalar Hat::X_Min(int id) const
{
	return(this->_min[id]);
}

scalar Hat::X_Max(int id) const
{
	return(this->_max[id]);
}

scalar Hat::Ground() const
{
	return(this->_ground);
}


}