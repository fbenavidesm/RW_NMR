#ifndef VEC3_H
#define VEC3_H

#include "math_la/mdefs.h"
#include "vec2.h"

namespace math_la
{
	namespace math_lac
	{

		namespace space
		{
			/**
			* Three dimension vector
			*/
			class Vec3
			{
			private:
				friend Vec3 operator * (scalar c, const Vec3& vct);
				scalar _cmpx[4];
			public:
				/**
				* Transforms a two dimension vector into a 3 dimension vector
				* @param v 2 dimension vector
				* @return A 3 dimension vector. The first two entries are equal to v,  but the thir entry is 1
				*/
				static Vec3 Affine(const Vec2& v);
				Vec3();

				/**
				* A three dimension vector can be defined through its three different entries
				*/
				Vec3(scalar x, scalar y, scalar z);
				Vec3(const Vec3& vct);
				Vec3& operator = (const Vec3& vct);
				Vec3& operator()(uint pos, scalar value);
				scalar operator()(uint pos) const;

				/**
				* @return The length of the vector
				*/
				scalar Norm() const;

				/**
				* @return The sum of all squared entries of the vector
				*/
				scalar Sq_Norm() const;

				/**
				* Makes current vector length equal to 1
				*/
				void Normalize();

				/**
				* This operation calculates the dot product between two 3 dimension vectors. For example:
 				* Vec3 a = Vec3(1,0,0)
				* Vec3 b = Vec3(1,1,0);
				* scalar d = Vec3::Dot(a,b)
				* d must be 1
				* @return Dot product between two vectors				
				*/
				static scalar Dot(const Vec3& vct1, const Vec3& vct2);

				/**
				* Cross product only makes sense for 3 dimension vectors. For example:
				* Vec3 a = Vec3(1,0,0)
				* Vec3 b = Vec3(0,1,0);
				* Vec3 d = Vec2::Dot(a,b)
				* d must be equal to Vec3(0,0,1)
				* @return Cross vector between a and b				
				*/
				static Vec3 Cross(const Vec3& vct1, const Vec3& vct2);

				/**
				* @return TRUE if all vector entries are less than the tolerance
				*/
				bool Is_Zero_Vector() const;

				/**
				* @return Euclidean X vector
				*/
				static Vec3 Ex();

				/**
				* @return Euclidean Y vector
				*/
				static Vec3 Ey();

				/**
				* @return Euclidean Z vector
				*/
				static Vec3 Ez();

				/**
				* Projects current vector on vector base
				* @param base Vector on which current vector is projected
				*/
				Vec3 Project(const Vec3& base) const;

				Vec3 operator + (const Vec3& v) const;
				Vec3 operator - (const Vec3& v) const;
				Vec3 operator * (scalar c) const;
				Vec3 operator - () const;
				bool operator == (const Vec3& v) const;
				bool operator != (const Vec3& v) const;
				Vec3& operator += (const Vec3& v);
				Vec3& operator -= (const Vec3& v);
				Vec3& operator *= (scalar c);
			};

			Vec3 operator * (scalar c, const Vec3& vct);

			inline scalar Dot(const Vec3& vct1, const Vec3& vct2)
			{
				return(Vec3::Dot(vct1, vct2));
			}

			inline Vec3 Vec3::Affine(const Vec2& v)
			{
				return(Vec3(v(eX), v(eY), 1));
			}
			inline Vec3 Vec3::operator - () const
			{
				return((scalar)(-1) * (*this));
			}
		}
	}
}

#endif
