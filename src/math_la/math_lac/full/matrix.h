#ifndef F_MATRIX
#define F_MATRIX

#include <ostream>
#include <set>
#include "vector.h"
#include "math_la/mdefs.h"

/**
* math_la stands for Mathematics and linear algebra classes. It contains Matrix and Vector classes, optimized for different uses
* and also cointains heuristics and optimization methods
*/
namespace math_la
{
	/**
	* math_lac is a mathematical set of classes that are executed in CPU. 
	*/
	namespace math_lac
	{
		/**
		* Full matrices and vectors, without sparsity optimization
		*/
		namespace full
		{
			using std::ostream;

			/**
			* A mxn dimension matrix
			*/
			class Matrix
			{
			private:
				/**
				* Number of rows
				*/
				int	_rows;
				/**
				* Number of columns
				*/
				int	_cols;

				/**
				* A pointer to all data contained in the matrix. The dimension of this array is mxn rows. It is one dimension. That is, in order to acces the element located
				* in the i'th row and the j'th column, index must be i*n+j.
				*/
				scalar* _data;

				/**
				* Reseerves memory associated to the array size
				*/
				void Reserve_Memory();

				/**
				* Sets memory to 0
				*/
				void Set_Memory();

				/**
				* Deletes all memory associated to matrix data
				*/
				void Free_Memory();
			public:
				friend Matrix operator*(scalar c, const Matrix& m);

				static Matrix Identity(int n);

				explicit Matrix(int rows, int cols);
				Matrix();
				~Matrix();
				Matrix(const Matrix& matrix);
				scalar operator()(int i, int j) const;
				Matrix& operator()(int i, int j, scalar value);
				Matrix& operator()(int i, int j, const Matrix& m);
				Matrix operator()(int i1, int i2, int j1, int j2) const;
				Matrix& operator()(int i, int j, const Matrix& r, int pib, int pie, int pjb, int pje);

				/**
				* @return The vector in the i'th row
				* @param i Index of the row
				*/
				Vector Row(int i) const;

				/**
				* @return The vector in the i'th column
				* @param i Index of the column
				*/
				Vector Column(int i) const;

				/**
				* Fills matrix column with the vector data
				* @param v Columnd vector
				* @param col Index of the column
				*/
				void Set_Column(const full::Vector& v, int col);

				/**
				* Fills matrix row with the vector data
				* @param v Row vector
				* @param row Index of the row
				*/
				void Set_Row(const full::Vector& v, int row);
				scalar Norm() const;

				Matrix& operator = (const Matrix& matrix);
				Matrix  operator + (const Matrix& m) const;
				Matrix  operator - (const Matrix& m) const;
				Matrix  operator * (const Matrix& m) const;
				Matrix  operator * (scalar c) const;
				Vector  operator * (const Vector& v) const;
				Matrix& operator << (Matrix& ref);

				Matrix	Transposed() const;

				/**
				* @return TRUE if matrix is square (number of rows is equal to the number of columns)
				*/
				bool Square() const;

				/**
				* @return TRUE if matrix is symmetric
				*/
				bool Symmetric() const;

				/**
				* @return Number of rows
				*/
				int Rows() const;

				/**
				* @return Number of columns
				*/
				int Columns() const;

				/**
				* Exchange row i with row j. This is an elementary Gaussian operation
				*/
				void Exchange_Rows(int i, int j);

				/**
				* @return The largest absolute value of all entries in column k
				* @param k Index of the column
				*/
				int Pivot(int k) const;

				/**
				* Fills the stream with all matrix entries
				* @param os Output stream
				*/
				void To_Stream(ostream& os) const;

				/**
				* @return The summ of all squares of matrix entries
				*/
				scalar Norm2() const;

				/**
				* Executes QR factorization. Result matrices are returned in Q and R. 
				* @param Q Matrix obtained after factorization
				* @param R Matrix obtained after factorization
				* @param update This parameter is generally true when the exact value of matrix R is required. This matrix contains residual data
				* after the numerical process, so it must be cleaned with zeros. If this parameter is FALSE, the matrix will not be cleaned
				*/
				void QR_Fact(full::Matrix& Q, full::Matrix& R, bool update = true) const;

				/**
				* Executes Cholesky factorization. Result matrix is returned in L. This method does not check if the matrix is symmetric
				* @param L Matrix obtained after factorization
				* @param update This parameter is generally true when the exact value of matrix R is required. This matrix contains residual data
				* after the numerical process, so it must be cleaned with zeros. If this parameter is FALSE, the matrix will not be cleaned
				*/
				void Cholesky(full::Matrix& L, bool update = true) const;

				/**
				* Returns a new matrix with the columns specified in indx. 
				* @param indx Set of indices incorporated in the returned matrix
				* @return A matrix with the columns indexed by indx and the same number of rows of the current matrix
				*/
				full::Matrix Sub_Columns(const IndxSet& indx) const;

				/**
				* Returns a new matrix with the rows specified in indx.
				* @param indx Set of indices incorporated in the returned matrix
				* @return A matrix with the rows indexed by indx and the same number of columns of the current matrix
				*/
				full::Matrix Sub_Rows(const IndxSet& indx) const;

				/**
				* Solves a linear systems with a vector of constants "c". It uses QR factorization
				* @param c Vector of constants
				* @return Solution vector of the system
				*/
				full::Vector Solve_QR(const full::Vector& c) const;

				/**
				* Solves a linear systems with a vector of constants "c". It uses Cholesky factorization
				* @param c Vector of constants
				* @return Solution vector of the system
				*/
				full::Vector Solve_Cholesky(const full::Vector& c) const;

				/**
				* Solves upper triangular system (it ignores lower triangular values)
				* @param c Vector of constants
				* @return Solution vector of the system
				*/
				static full::Vector Solve_Upper_Triangular(const full::Matrix& triangular, const full::Vector& c);

				/**
				* Solves lower triangular system (it ignores upper triangular values)
				* @param c Vector of constants
				* @return Solution vector of the system
				*/
				static full::Vector Solve_Lower_Triangular(const full::Matrix& triangular, const full::Vector& c);

				/**
				* Applies HouseHolder decomposition, providing a mx1 matrix
				* @param x Matrix to decompose
				* @param beta Scalar of HouseHolder decomposition
				* @return HouseHolder vector
				*/
				static full::Matrix House_Holder(const full::Matrix& x, scalar& beta);

				/**
				* Applies non negative least squares using iterative methods.
				* This method minimizes the norm of M*x-d (the number of rows of M must be equal to the size of d
				* @param M Matrix of coefficients
				* @param d Vector of constants
				* @return Solution vector
				*/
				static full::Vector NNLS_Iterative(const full::Matrix& M, const full::Vector& d);

				/**
				* Applies non negative least squares using QR methods
				* This method minimizes the norm of M*x-d (the number of rows of M must be equal to the size of d
				* @param M Matrix of coefficients
				* @param d Vector of constants
				* @return Solution vector
				*/
				static full::Vector NNLS(const full::Matrix& A, const full::Vector& y);

				/**
				* Applies SVD factorization. The factorization is returned in parameters U, V and B. B is the diagonal matrix
				* and U is the left multiplier while V is the right multiplier. The values are returned as parameter's references
				*/
				void SVD_Factor(full::Matrix& U, full::Matrix& V, full::Matrix& B) const;

				/**
				* Applies SVD factorization using Zero-Shift method. The factorization is returned in parameters U, V and B. B is the diagonal matrix
				* and U is the left multiplier while V is the right multiplier. The values are returned as parameter's references
				*/
				void SVD_Zero_Shift(full::Matrix& U, full::Matrix& V, full::Matrix& B) const;

				/**
				* Applies Bidiagonal factorization. The factorization is returned in parameters U, V and B. B is the bidiagonal matrix
				* and U is the left multiplier while V is the right multiplier. The values are returned as parameter's references
				* This is necessary for SVD decomposition
				*/
				void Bidiagonal(full::Matrix& U, full::Matrix& V, full::Matrix& B) const;

				/**
				* Applies Gram-Schmidth normalization to column vectors of the matrix
				*/
				void Column_Gram_Schmidth();
			};

			inline bool Matrix::Square() const
			{
				return(this->_rows == this->_cols);
			}

			inline int Matrix::Rows() const
			{
				return(this->_rows);
			}

			inline int Matrix::Columns() const
			{
				return(this->_cols);
			}

			inline scalar Matrix::operator()(int i, int j) const
			{
				return *(this->_data + i * this->_cols + j);
			}
		}
	}
}

#endif