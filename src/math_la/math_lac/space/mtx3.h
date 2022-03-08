#ifndef MTX3_H
#define MTX3_H

#include "math_la/mdefs.h"
#include "vec3.h"
#include "mtx4.h"

namespace math_la
{
	namespace math_lac
	{

		namespace space
		{
			/**
			* A 3x3 array of numbers. This matrix can be multiplied to the left by a three dimension vector
			*/

			class Mtx3
			{
			private:
				friend Mtx3 operator * (scalar c, const Mtx3& m);
				scalar _cmpx[9];
			public:

				Mtx3();
				Mtx3(const Mtx3& m);
				Mtx3(const Vec3& row1, const Vec3& row2, const Vec3& row3);

				/**
				* @return A cross matrix given by the vector defined in the parameter. This matrix can be multiplied by another vector
				* and the result will be the cross product
				*/
				static Mtx3 CrossMtx(const Vec3& v);

				Mtx3& operator=(const Mtx3& m);

				/**
				* Access operator, setting the enetry located in the row i and column j.
				* @param i Row
				* @param j Column
				* @param value Value to set in the row i and column j
				*/
				Mtx3& operator()(uint i, uint j, scalar value);

				/**
				* Access operator to read the value located in the row i and column j
				* @param i Row
				* @param j Column
				* @return Value located at position (i,j)
				*/
				scalar operator()(uint i, uint j) const;

				/**
				* @return A 3 dimension vector stored on the i'th row
				* @param i Index of the row (it can only be 0 or 1 or 2)
				*/
				Vec3 Row(uint i) const;

				/**
				* @return A 3 dimension vector stored on the i'th column
				* @param i Index of the column (it can only be 0 or 1 or 2)
				*/
				Vec3 Column(uint i) const;

				/**
				* Sets the i'th row of the matrix with the values stored in the 3 dimension vector given in the parameter row
				* @param i Index of the row
				* @param row Three dimension vector
				* @return A reference to current matrix (so this operator can be composed)
				*/
				Mtx3& Row(uint i, const Vec3& row);

				/**
				* Sets the i'th row of the matrix with the values stored in the 3 dimension vector given in the parameter row
				* @param i Index of the row
				* @param row Three dimension vector
				* @return A reference to current matrix (so this operator can be composed)
				*/
				Mtx3& Column(uint i, const Vec3& col);

				/**
				* @return The coefficient stored in the i'th row and the j'th column
				*/
				scalar Coefficient(unsigned int i, unsigned int j) const;

				/**
				* Transposes current matrix
				* @return A reference to current modified matrix
				*/
				Mtx3& Transpose();

				/**
				* @return The determinant of the matrix
				*/
				scalar Determinant() const;

				/**
				* @return A new matrix which is the inverse of the current matrix
				*/
				Mtx3 Inverse() const;

				/**
				* @return A new matrix which is the transposed of the current matrix
				*/
				Mtx3 Transposed() const;

				/**
				* A static method that returns a 3x3 identity matrix
				*/
				static Mtx3 Identity();

				/**
				* Modifies the stream, storing the values of the matrix.
				* @param str The float pointer to the array to which matrix values will be stored. It must be created externally.
				*/
				void Stream(double* str) const;

				/**
				* @return The 3x3 rotation matrix with the given angle around the axis
				* @param angle Rotation angle
				* @param axis Number of axis to which the rotation is obtained
				*/
				static Mtx3 Rotation(unsigned int axis, scalar angle);


				/**
				* Solves a 3x3 system of equations
				* @param c Constant vector
				*/
				Vec3 Solve(const Vec3& c) const;
				operator Mtx4();

				Mtx3 operator + (const Mtx3& m) const;
				Mtx3 operator - (const Mtx3& m) const;
				Mtx3 operator - () const;
				Mtx3 operator * (const Mtx3& m) const;
				Mtx3 operator * (scalar c) const;
				bool operator == (const Mtx3& m) const;
				Mtx3& operator += (const Mtx3& m);
				Mtx3& operator -= (const Mtx3& m);
				Mtx3& operator *= (scalar c);
				Vec3 operator * (const Vec3& v) const;

			};

			Mtx3 operator * (scalar c, const Mtx3& m);
			inline 	Mtx3 Mtx3::operator - () const
			{
				return((scalar)(-1.0) * (*this));
			}
		}
	}
}

#endif 