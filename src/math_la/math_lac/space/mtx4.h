#ifndef MTX4_H
#define MTX4_H

#include "math_la/mdefs.h"
#include "vec4.h"

namespace math_la
{
	namespace math_lac
	{

		namespace space
		{

			/**
			* A 4x4 array of numbers. This matrix can be multiplied to the left by a four dimension vector
			*/
			class Mtx4
			{
			private:
				friend Mtx4 operator * (scalar c, const Mtx4& m);
				scalar _cmpx[16];
			public:
				Mtx4();
				Mtx4(const Mtx4& m);
				/**
				* The matrix can be defined through a sequence of four 4'dimension vectors
				*/
				Mtx4(const Vec4& row1,
					const Vec4& row2,
					const Vec4& row3,
					const Vec4& row4);

				Mtx4& operator = (const Mtx4& m);

				/**
				* Access operator, setting the enetry located in the row i and column j.
				* @param i Row
				* @param j Column
				* @param value Value to set in the row i and column j
				*/
				Mtx4& operator()(uint i, uint j, scalar value);

				/**
				* Access operator to read the value located in the row i and column j
				* @param i Row
				* @param j Column
				* @return Value located at position (i,j)
				*/
				scalar operator()(uint i, uint j) const;

				/**
				* @return A 4 dimension vector stored on the i'th row
				* @param i Index of the row (it can only be 0 or 1 or 2 or 3)
				*/
				Vec4 Row(uint i) const;

				/**
				* @return A 4 dimension vector stored on the i'th column
				* @param i Index of the column (it can only be 0 or 1 or 2 or 3)
				*/
				Vec4 Column(uint i) const;

				/**
				* Sets the i'th row of the matrix with the values stored in the 4 dimension vector given in the parameter row
				* @param i Index of the row
				* @param row Three dimension vector
				* @return A reference to current matrix (so this operator can be composed)
				*/
				Mtx4& Row(uint i, const Vec4& row);

				/**
				* Sets the i'th row of the matrix with the values stored in the 4 dimension vector given in the parameter row
				* @param i Index of the row
				* @param row Three dimension vector
				* @return A reference to current matrix (so this operator can be composed)
				*/
				Mtx4& Column(uint i, const Vec4& col);

				/**
				* @return The coefficient stored in the i'th row and the j'th column
				*/
				scalar Coefficient(unsigned int i, unsigned int j) const;

				/**
				* Transposes current matrix
				* @return A reference to current modified matrix
				*/
				Mtx4& Transpose();

				/**
				* @return The determinant of the matrix
				*/
				scalar Determinant() const;

				/**
				* @return A new matrix which is the inverse of the current matrix
				*/
				Mtx4 Inverse() const;

				/**
				* @return A new matrix which is the transposed of the current matrix
				*/
				Mtx4 Transposed() const;

				/**
				* A static method that returns a 4x4 identity matrix
				*/
				static Mtx4 Identity();

				/**
				* Modifies the stream, storing the values of the matrix.
				* @param str The float pointer to the array to which matrix values will be stored. It must be created externally.
				*/
				void Stream(double* str) const;
				void FStream(float* str) const;

				/**
				* Solves a 4x4 system of equations
				* @param c Constant vector
				*/
				Vec4 Solve(const Vec4& c) const;

				Mtx4 operator + (const Mtx4& m) const;
				Mtx4 operator - (const Mtx4& m) const;
				Mtx4 operator - () const;
				Vec4 operator * (const Vec4& v) const;
				Mtx4 operator * (const Mtx4& m) const;
				Mtx4 operator * (scalar c) const;
				bool operator == (const Mtx4& m) const;
				Mtx4& operator += (const Mtx4& m);
				Mtx4& operator -= (const Mtx4& m);
				Mtx4& operator *= (scalar c);
			};

			Mtx4 operator * (scalar c, const Mtx4& m);

			inline 	Mtx4 Mtx4::operator - () const
			{
				return((scalar)(-1.0) * (*this));
			}
		}
	}
}

#endif