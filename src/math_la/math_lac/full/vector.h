#ifndef F_VECTOR_H
#define F_VECTOR_H

#include <math.h>
#include <set>
#include "math_la/mdefs.h"


namespace math_la
{
	namespace math_lac
	{
		/**
		* The full namespace defines two classes, Vector and Matrix that do not have any optimization associated to sparsity.
		* Full vectors and matrices are dense and must reserve memory for each one of its entries.
		*/
		namespace full
		{

			using std::set;

			typedef set<int> IndxSet;

			class Matrix;

			/**
			* A full::Vector is a one dimension array of numbers that is not optimized for sparsity.
			* It should be the most common type of vector.
			*/
			class Vector
			{
			private:
				friend Vector operator*(scalar c, const Vector& v);
				friend class Matrix;

				/**
				* The size of the vector, defining its total number of entries
				*/
				int  _size;

				/**
				* Dynamicallly allocated memory to store all vector entries.
				*/
				scalar* _data;

				/**
				* It is called during construction to reserve the memory associated to vector data
				*/
				void Reserve_Memory();

				/**
				* Sets memory to 0
				*/
				void Set_Memory();

				/**
				* Releases reserved memory
				*/
				void Free_Memory();
			public:
				Vector();
				~Vector();
				explicit Vector(int size);
				Vector(const Vector& v);
				void Set_Size(int size);
				scalar operator()(int i) const;
				Vector operator()(int i, int j) const;
				Vector& operator()(int i, scalar value);

				/**
				* A vector defined from entry i to entry j
				*/
				Vector Sub_Vector(int i, int j) const;
				Vector& operator = (const Vector& v);
				Vector& operator << (Vector& v);
				Vector  operator + (const Vector& v) const;
				Vector  operator - (const Vector& v) const;

				/**
				* @return a*this + v
				*/
				Vector  ap(scalar a, const Vector& v) const;

				/**
				* @return a*this - v
				*/
				Vector  am(scalar a, const Vector& v) const;

				/**
				* Applies a*this + b*v to current vector
				*/
				void    apbv(scalar a, scalar b, const Vector& v);

				/**
				* @return a*this + b*v to current vector
				*/
				full::Vector apbv(scalar a, scalar b, const Vector& v) const;

				/**
				* Divides each entry of current vector by the corresponding entries of vector v
				* @param v Vector of divisors.
				*/
				void Side_Div(const full::Vector& v);
				Vector  operator * (scalar c) const;

				/**
				* @return Dot product between current vector and v
				* @param v The other vector
				*/
				scalar Dot(const Vector& v) const;

				/**
				* @return Vector size
				*/
				int Size() const;

				/**
				* @return Vector norm
				*/
				scalar Norm() const;

				/**
				* @return Sub-vector with the indices given in indx
				* @apram indx Set of indices
				*/

				full::Vector Sub_Vector(const IndxSet& indx) const;
				scalar V_Min() const;
				scalar V_Max() const;

				/**
				* @return Minimal vector value
				* @param k Index of the minimal value
				*/
				scalar V_Min(int& k) const;

				/**
				* @return Maximal vector value
				* @param k Index of the maximal value
				*/
				scalar V_Max(int& k) const;

				/**
				* @return Minimal vector value, restricted to indices given in i
				* @param i Indices to which the minimal is restricted
				*/
				scalar V_Min(const IndxSet& i) const;

				/**
				* @return Maximal vector value, restricted to indices i
				* @param i Indices to which the maximal is restricted
				*/
				scalar V_Max(const IndxSet& i) const;

				/**
				* @return Minimal vector value, restricted to indices given in i
				* @param i Indices to which the minimal is restricted
				* @param k Index of the minimal value
				*/
				scalar V_Min(const IndxSet& i, int& k) const;

				/**
				* @return Maximal vector value, restricted to indices given in i
				* @param i Indices to which the maximal is restricted
				* @param k Index of the maximal value
				*/
				scalar V_Max(const IndxSet& i, int& k) const;

				/**
				* @return The convolution between current vector and y
				* @param y The other vector to which current vector is correlated
				*/
				full::Vector Cross_Correlation(const full::Vector& y) const;

			};

			inline int Vector::Size() const
			{
				return(this->_size);
			}

			inline scalar Vector::Norm() const
			{
				return(sqrt(this->Dot(*this)));
			}

			inline scalar Vector::operator()(int i) const
			{
				return (this->_data[i]);
			}


			Vector operator*(scalar c, const Vector& v);

		}
	}
}


#endif