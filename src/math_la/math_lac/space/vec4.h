#ifndef VEC4_H
#define VEC4_H

#include "vec3.h"
#include "math_la/mdefs.h"

namespace math_la
{
	namespace math_lac
	{

		namespace space
		{

			class Vec4
			{
			private:
				friend Vec4 operator * (scalar c, const Vec4& vct);
				scalar _cmpx[5];
			public:
				/**
				* Transforms a three dimension vector into a 4 dimension vector
				* @param v 2 dimension vector
				* @return A 3 dimension vector. The first three entries are equal to v,  but the thir entry is 1
				*/
				static Vec4 Affine(const Vec3& v);
				Vec4();

				/**
				* A 4 dimension vector can be defined through each one of its entries
				*/
				Vec4(scalar x, scalar y, scalar z, scalar w);
				Vec4(const Vec4& vct);
				Vec4& operator = (const Vec4& vct);
				Vec4& operator()(uint pos, scalar value);
				scalar operator()(uint pos) const;

				/**
				* @return Length of the vector
				*/
				scalar Norm() const;

				/**
				* @return The sum of all squared entries of the vector
				*/
				scalar Sq_Norm() const;

				/**
				* Makes the length of the vector equal to 1
				*/
				void Normalize();

				/**
				* This operation calculates the dot product between two 4 dimension vectors. For example:
				* Vec4 a = Vec4(1,0,0,1)
				* Vec4 b = Vec4(1,1,0,0);
				* scalar d = Vec4::Dot(a,b)
				* d must be 1
				* @return Dot product between two vectors
				*/
				static scalar Dot(const Vec4& vct1, const Vec4& vct2);

				/**
				* @return TRUE if all entries of the vector are less than the tolerance
				*/
				bool Is_Zero_Vector() const;

				/**
				* @return Euclidean X vector
				*/
				static Vec4 Ex();

				/**
				* @return Euclidean Y vector
				*/
				static Vec4 Ey();

				/**
				* @return Euclidean Z vector
				*/
				static Vec4 Ez();

				/**
				* @return Euclidean W vector
				*/
				static Vec4 Ew();

				/**
				* Projects current vector on vector base
				* @param base Vector on which current vector is projected
				*/
				Vec4 Project(const Vec4& base) const;

				Vec4 operator + (const Vec4& v) const;
				Vec4 operator - (const Vec4& v) const;
				Vec4 operator * (scalar c) const;
				Vec4 operator - () const;
				bool operator == (const Vec4& v) const;
			};

			Vec4 operator * (scalar c, const Vec4& vct);

			inline Vec4 Vec4::Affine(const Vec3& v)
			{
				return(Vec4(v(eX), v(eY), v(eZ), 1));
			}

			inline scalar Dot(const Vec4& vct1, const Vec4& vct2)
			{
				return(Vec4::Dot(vct1, vct2));
			}

			inline Vec4 Vec4::operator-()const
			{
				return((scalar)(-1) * (*this));
			}
		}
	}
}
#endif
