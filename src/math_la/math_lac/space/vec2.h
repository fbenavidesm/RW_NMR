#ifndef VEC2_H
#define VEC2_H

#include "math_la/mdefs.h"

namespace math_la
{
	namespace math_lac
	{

		namespace space
		{
			/*
			* Two dimension vector
			*/
			class Vec2
			{
			private:
				friend Vec2 operator * (scalar c, const Vec2& vct);
				scalar _cmpx[3];
			public:
				Vec2();
				/**
				* A 2 dimension vector can be defined through its 2 real entries
				*/
				Vec2(scalar x, scalar y);
				Vec2(const Vec2& vct);
				Vec2& operator = (const Vec2& vct);

				/**
				* Access operator, to set value in one of the two positions
				* @param pos Position to modify
				* @param value Value to set
				*/
				Vec2& operator()(uint pos, scalar value);

				/**
				* Access operator, to read value
				* @param pos Value to read
				* @return Value located in position pos
				*/
				scalar operator()(uint pos) const;
				
				/**
				* @return Vector norm
				*/
				scalar Norm() const;

				/**
				* @return The sum of each element squared
				*/
				scalar Sq_Norm() const;

				/**
				* Normalizes the vector, setting its length equal to 1
				*/
				void Normalize();

				/**
				* Dot product operator. For example:
				* Vec2 a = Vec2(1,0)
				* Vec2 b = Vec2(1,1);
				* scalar d = Vec2::Dot(a,b)
				* d must be 1
				* @return Dot product between two vectors
				*/
				static	scalar Dot(const Vec2& vct1, const Vec2& vct2);

				/**
				* @return TRUE if all entries are smaller than the tolerance
				*/
				bool Is_Zero_Vector() const;

				/**
				* @return Euclidean X vector
				*/
				static	Vec2 Ex();

				/**
				* @return Euclidean Y vector
				*/
				static	Vec2 Ey();

				/**
				* Projects current vector on vector base
				* @param base Vector on which current vector is projected
				* @return Projection
				*/
				Vec2 Project(const Vec2& base) const;

				Vec2 operator + (const Vec2& v) const;
				Vec2 operator - (const Vec2& v) const;
				Vec2 operator * (scalar c) const;
				Vec2 operator - () const;
				bool operator == (const Vec2& v) const;
				bool operator != (const Vec2& v) const;
				Vec2& operator += (const Vec2& v);
				Vec2& operator -= (const Vec2& v);
				Vec2& operator *= (scalar c);
			};

			Vec2 operator * (scalar c, const Vec2& vct);

			inline scalar Dot(const Vec2& vct1, const Vec2& vct2)
			{
				return(Vec2::Dot(vct1, vct2));
			}

			inline Vec2 Vec2::operator - () const
			{
				return((scalar)(-1) * (*this));
			}
		}
	}
}
#endif 
