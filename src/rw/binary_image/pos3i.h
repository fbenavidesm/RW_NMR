#ifndef POSITION3D_H
#define POSITION3D_H

#include <amp.h>
#include <amp_math.h>
#include "math_la/mdefs.h"

namespace rw
{

/**
* Position of the walker given by its 3D discrete coordinates in the texture
*/
	struct Pos3i
	{
		/**
		* X position
		*/
		int x;

		/**
		* Y position
		*/
		int y;

		/**
		* Z position
		*/
		int z;

		/**
		* The constructor sets all positions to zero
		*/
		Pos3i() GPU;

		Pos3i(const Pos3i& position) GPU;

		Pos3i& operator=(const Pos3i& position) GPU;

		/**
		* Square_Distance between two discrete points. It is given as a scalar, using Euclidean norm
		*/
		int Square_Distance(const Pos3i& pos) const GPU;

		/**
		* Ordering method for positions, in the usual order
		*/
		bool operator<(const Pos3i& pos) const GPU;
		bool operator==(const Pos3i& pos) const GPU;
		Pos3i operator+(const Pos3i& pos) const GPU;
		Pos3i operator-(const Pos3i& pos) const GPU;
		static void Int_To_Pos3i(int id, int width, int height, int depth, rw::Pos3i& pos) GPU;
		static int  Pos3i_To_Int(const rw::Pos3i& pos, int width, int height, int depth) GPU;
	};


	inline bool Pos3i::operator<(const Pos3i& pos) const GPU
	{
		int d1 = this->x + this->y + this->z;
		int d2 = pos.x + pos.y + pos.z;	
		if (d1 == d2)
		{
			d1 = this->y+this->z;
			d2 = pos.y+pos.z;
			if (d1 == d2)
			{
				d1 = this->z;
				d2 = pos.z;
				return(d1 < d2);
			}
			else
			{
				return(d1 < d2);
			}
		}
		else
		{
			return(d1 < d2);
		}
	}

	inline bool Pos3i::operator==(const Pos3i& pos) const GPU
	{
		return ((this->x == pos.x) && (this->y == pos.y) && (this->z == pos.z));
	}

	inline Pos3i::Pos3i() GPU
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	inline Pos3i::Pos3i(const Pos3i& position) GPU
	{
		this->x = position.x;
		this->y = position.y;
		this->z = position.z;
	}

	inline Pos3i& Pos3i::operator=(const Pos3i& position) GPU
	{
		this->x = position.x;
		this->y = position.y;
		this->z = position.z;
		return(*this);
	}

	inline int Pos3i::Square_Distance(const Pos3i& pos) const GPU
	{
		int dx = this->x - pos.x;
		int dy = this->y - pos.y;
		int dz = this->z - pos.z;
		int length = dx*dx + dy*dy + dz*dz;
		return(length);
	}

	inline void Pos3i::Int_To_Pos3i(int id, int width, int height, int depth, rw::Pos3i& pos) GPU
	{
		int wh = width*height;
		pos.z = (id / wh);
		pos.y = ((id - pos.z*wh) / width);
		pos.x = (id - pos.z*wh - pos.y*width);
	}

	inline int Pos3i::Pos3i_To_Int(const rw::Pos3i& pos, int width, int height, int depth) GPU
	{
		int r = pos.z*width*height + pos.y*width + pos.x;
		return(r);
	}

	inline Pos3i Pos3i::operator+(const Pos3i& pos) const GPU
	{
		Pos3i r;
		r.x = this->x + pos.x;
		r.y = this->y + pos.y;
		r.z = this->z + pos.z;
		return(r);
	}

	inline Pos3i Pos3i::operator-(const Pos3i& pos) const GPU
	{
		Pos3i r;
		r.x = this->x - pos.x;
		r.y = this->y - pos.y;
		r.z = this->z - pos.z;
		return(r);
	}

}


#endif