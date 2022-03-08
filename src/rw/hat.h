#ifndef HAT_H
#define HAT_H

#include "relaxivity_distribution.h"

namespace rw
{

class Hat : public rw::RelaxivityDistribution
{
private:
	scalar* _K;
	scalar* _min;
	scalar* _max;
	scalar _ground;
	int _size;
public:
	void Set_Size(int n);
	void Set_Value(int id, scalar Decay_Step_Value);
	void Set_Ground(scalar gr);
	scalar K(int id) const;
	scalar X_Min(int id) const;
	scalar X_Max(int id) const;
	scalar Ground() const;
	scalar Evaluate(scalar xi) const;
	int Size() const;
	~Hat();
};

inline int Hat::Size() const
{
	return(this->_size);
}

}

#endif