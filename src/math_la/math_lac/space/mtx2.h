#ifndef MTX2_H
#define MTX2_H

#include "math_la/mdefs.h"
#include "mtx3.h"
#include "vec2.h"


namespace math_la
{
	namespace math_lac
	{

		/**
		* A small set of classes that allow an easy manipulation of vector in two and three dimensions.
		*/
		namespace space
		{
			/**
			* A 2x2 array of numbers. This matrix can be multiplied to the left by a two dimension vector
			*/
			class Mtx2
			{
			private:
				/**
				* A 2x2 matrix can be multiplied by a number on both sides
				*/
				friend Mtx2 operator * (scalar c, const Mtx2& m);

				/**
				A four dimension array is used to store matrix entries
				*/
				scalar _cmpx[4];
			public:
				Mtx2();
				Mtx2(const Mtx2& m);
				Mtx2(const Vec2& row1, const Vec2& row2);
				Mtx2& operator=(const Mtx2& m);

				/**
				* Access operator, setting the enetry located in the row i and column j.
				* @param i Row
				* @param j Column
				* @param value Value to set in the row i and column j
				*/
				Mtx2& operator()(uint i, uint j, scalar value);

				/**
				* Access operator to read the value located in the row i and column j
				* @param i Row
				* @param j Column
				* @return Value located at position (i,j)
				*/
				scalar operator()(uint i, uint j) const;

				/**
				* @return A 2 dimension vector stored on the i'th row
				* @param i Index of the row (it can only be 0 or 1)
				*/
				Vec2 Row(uint i) const;

				/**
				* @return A 2 dimension vector stored on the i'th column
				* @param i Index of the column (it can only be 0 or 1)
				*/
				Vec2 Column(uint i) const;

				/**
				* Sets the i'th row of the matrix with the values stored in the 2 dimension vector given in the parameter row
				* @param i Index of the row
				* @param row Two dimension vector
				* @return A reference to current matrix (so this operator can be composed)
				*/
				Mtx2& Row(uint i, const Vec2& row);

				/**
				* Sets the i'th column of the matrix with the values stored in the 2 dimension vector given in the parameter column
				* @param i Index of the column
				* @param col Two dimension vector
				* @return A reference to current matrix (so this operator can be composed)
				*/
				Mtx2& Column(uint i, const Vec2& col);

				/**
				* Transposes current matrix
				* @return A reference to current modified matrix
				*/
				Mtx2& Transpose();

				/**
				* @return The determinant of the matrix
				*/
				scalar Determinant() const;

				/**
				* @return A new matrix which is the inverse of the current matrix
				*/
				Mtx2 Inverse() const;

				/**
				* @return A new matrix which is the transposed of the current matrix
				*/
				Mtx2 Transposed() const;

				/**
				* A static method that returns a 2x2 identity matrix
				*/
				static Mtx2 Identity();

				/**
				* Modifies the stream, storing the values of the matrix.
				* @param str The float pointer to the array to which matrix values will be stored. It must be created externally.
				*/
				void Stream(double* str) const;

				/**
				* @return The 2x2 rotation matrix with the given angle
				* @param angle Rotation angle
				*/
				static Mtx2 Rotation(scalar angle);

				/**
				* Solves a 2x2 system of equations
				* @param c Constant vector
				*/
				Vec2 Solve(const Vec2& c) const;

				/**
				* Creates an identiy 3x3 matrix and sets its 2x2 submatrix with the current matrix values
				*/
				operator Mtx3();

				Mtx2 operator + (const Mtx2& m) const;
				Mtx2 operator - (const Mtx2& m) const;
				Mtx2 operator * (const Mtx2& m) const;
				Mtx2 operator - () const;
				Mtx2 operator * (scalar c) const;
				bool operator == (const Mtx2& m) const;
				Mtx2& operator += (Mtx2& m);
				Mtx2& operator -= (Mtx2& m);
				Mtx2& operator *= (scalar c);
				Vec2 operator * (const Vec2& v) const;
			};

			Mtx2 operator * (scalar c, const Mtx2& m);

			inline 	Mtx2 Mtx2::operator - () const
			{
				return((scalar)(-1.0) * (*this));
			}

		}
	}
};

#endif 
