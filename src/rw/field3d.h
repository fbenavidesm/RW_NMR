#ifndef FIELD3D_H
#define FIELD3D_H

#include <amp.h>
#include <amp_math.h>
#include "math_la/mdefs.h"


struct Field3D
{
	scalar x;
	scalar y;
	scalar z;
	Field3D() GPU;
	bool Active() const GPU;
	scalar Exponential_Factor(int dx, int dy, int dz) const GPU;
};

inline Field3D::Field3D() GPU
{
	this->x = 0;
	this->y = 0;
	this->z = 0;
}

inline bool Field3D::Active() const GPU
{
	scalar dd = this->x*this->x + this->y*this->y + this->z*this->z;
	return(dd > 0);
}

inline scalar Field3D::Exponential_Factor(int dx, int dy, int dz) const GPU
{
	scalar sdx = (scalar)dx;
	scalar sdy = (scalar)dy;
	scalar sdz = (scalar)dz;
	sdx = this->x*sdx;
	sdy = this->y*sdy;
	sdz = this->z*sdz;
	scalar dg = concurrency::precise_math::sqrt(sdx*sdx + sdy*sdy + sdz*sdz);
	return((scalar)1-dg);
}

#endif