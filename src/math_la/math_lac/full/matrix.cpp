#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>
#include <time.h>

#include "math_la/mdefs.h"
#include "matrix.h"
#include "tbb/parallel_for.h"
#include "tbb/spin_mutex.h"
#include "tbb/parallel_reduce.h"
#include "mkl.h"


namespace math_la
{
	namespace math_lac
	{


		namespace full
		{

			using std::vector;

			Matrix::Matrix()
			{
				this->_rows = 0;
				this->_cols = 0;
				this->_data = 0;
			}

			void Matrix::Free_Memory()
			{
				if (this->_data)
				{
					freeScalar(this->_data);
				}
				this->_data = 0;
			}

			void Matrix::Reserve_Memory()
			{
				if (this->_data)
				{
					freeScalar(this->_data);
				}
				if (this->_rows*this->_cols != 0)
				{
					this->_data = allocScalar(this->_rows*this->_cols);
				}
			}

			void Matrix::Set_Memory()
			{
				if (this->_data)
				{
					freeScalar(this->_data);
				}
				if (this->_rows*this->_cols != 0)
				{
					this->_data = allocScalar(this->_rows*this->_cols);
					memset(this->_data, 0, this->_rows*this->_cols * sizeof(scalar));
				}
			}

			Matrix::Matrix(int rows, int cols)
			{
				this->_data = 0;
				this->_rows = rows;
				this->_cols = cols;
				this->Set_Memory();
			};

			Matrix::~Matrix()
			{
				this->_rows = 0;
				this->_cols = 0;
				this->Free_Memory();
			}

			Matrix Matrix::Identity(int n)
			{
				Matrix r(n,n);
				tbb::parallel_for(tbb::blocked_range<int>(0, n, CHUNK_SIZE), [&r,n](const tbb::blocked_range<int>& b)
				{
					int beg = b.begin();
					int end = b.end();
					for (int i = beg; i < end; ++i)
					{
						for (int j = 0; j < n; ++j)
						{
							r(i, j, 0.0f);
						}
						r(i, i, 1.0f);
					}
				});
				return(r);
			}

			Matrix::Matrix (const Matrix& matrix)
			{
				this->_data = 0;
				this->_rows = matrix._rows;
				this->_cols = matrix._cols;
				this->Reserve_Memory();
				memcpy(this->_data, matrix._data, sizeof(scalar)*this->_rows*this->_cols);
			}

			Matrix& Matrix::operator()(int i, int j, scalar value)
			{
				*(this->_data + i*this->_cols + j) = value;
				return(*this);
			}

			Matrix Matrix::operator()(int i1, int i2, int j1, int j2) const
			{
				if (i2 < i1)
				{
					int f = i1;
					i1 = i2;		
					i2 = f;
				}
				if (j2 < j1)
				{
					int f = j1;
					j1 = j2;		
					j2 = f;
				}
				Matrix r;
				r._rows = i2-i1;
				r._cols = j2-j1;
				r.Reserve_Memory();
				tbb::parallel_for(tbb::blocked_range<int>(0, r._rows, BCHUNK_SIZE), [&r, this, i1, j1](const tbb::blocked_range<int>& b)
				{
					for (int i = b.begin(); i < b.end(); ++i)
					{
						memcpy(r._data + i*r._cols,
							this->_data + (i1 + i)*this->_cols + j1,
							r._cols * sizeof(scalar));
					}
				});	
				return(r);
			}

			Matrix& Matrix::operator()(int i, int j, const Matrix& r, int pib, int pie, int pjb, int pje)
			{
				int Voxel_Length = 0;
				if (pjb > pje)
				{
					Voxel_Length = pjb;
					pjb = pje;
					pje = Voxel_Length;
				}
				if (pib > pie)
				{
					Voxel_Length = pib;
					pib = pie;
					pie = Voxel_Length;
				}
				tbb::parallel_for(tbb::blocked_range<int>(0, pie-pib, BCHUNK_SIZE), 
					[&r, this, i, j, pib, pjb, pie, pje](const tbb::blocked_range<int>& b)
				{
					for (int k = b.begin(); k < b.end(); ++k)
					{		
						memcpy(this->_data + (i + k)*this->_cols + j,
							r._data + (pib + k)*r._cols + pjb, (pje - pjb)*sizeof(scalar));
					}
				});
				return(*this);
			}



			Vector Matrix::Row(int i) const
			{
				i = i % this->_rows;
				Vector v(this->_cols);
				memcpy(v._data, this->_data + i*this->_cols, this->_cols * sizeof(scalar));
				return(v);
			}

			Vector Matrix::Column(int i) const
			{
				Vector v(this->_rows);
				tbb::parallel_for(tbb::blocked_range<int>(0, this->_rows, BCHUNK_SIZE), [&v,this,i](const tbb::blocked_range<int>& b)
				{
					int beg = b.begin();
					int end = b.end();
					for (int j = beg; j < end; ++j)
					{
						v._data[j] = (*this)(j, i);
					}
				});
				return(v);
			}

			Matrix& Matrix::operator = (const Matrix& matrix)
			{
				if (this->_cols*this->_rows != matrix._rows*matrix._cols)
				{
					this->Free_Memory();
					this->_cols = matrix._cols;
					this->_rows = matrix._rows;
					this->Reserve_Memory();
				}
				this->_cols = matrix._cols;
				this->_rows = matrix._rows;
				memcpy(this->_data, matrix._data, this->_rows*this->_cols * sizeof(scalar));
				return(*this);
			}

			Matrix& Matrix::operator << (Matrix& ref)
			{
				if (this->_data)
				{
					this->Free_Memory();
				}
				this->_rows = ref._rows;
				this->_cols = ref._cols;
				this->_data = ref._data;
				ref._data = 0;
				ref._cols = 0;
				ref._rows = 0;
				return(*this);
			}

			Matrix Matrix::operator + (const Matrix& m) const
			{
				Matrix r = (*this);
				if ((this->_cols == m._cols)&&(this->_rows == m._rows))
				{
					cblas_daxpy(this->_rows*this->_cols, 1, m._data, 1, r._data, 1);
				}
				return(r);
			}

			Matrix  Matrix::operator - (const Matrix& m) const
			{
				Matrix r = (*this);
				if ((this->_cols == m._cols)&&(this->_rows == m._rows))
				{
					cblas_daxpy(this->_rows*this->_cols, -1, m._data, 1, r._data, 1);
				}
				return(r);
			}


			Matrix  Matrix::operator * (const Matrix& m) const
			{
				Matrix r;
				if (this->_cols == m._rows)
				{
					r._rows = this->_rows;
					r._cols = m._cols;
					r.Reserve_Memory();		
					cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, this->_rows, m._cols, this->_cols, 1,
						this->_data, m._rows, m._data, m._cols, 0, r._data, r._cols);		
				}
				return(r);
			}

			Matrix  Matrix::operator * (scalar c) const
			{
				Matrix r = (*this);
				cblas_dscal(r._cols*r._rows, c, r._data, 1);
				return(r);
			}

			Vector  Matrix::operator * (const Vector& v) const
			{
				Vector r;
				if (this->_cols == v._size)
				{
					r.Set_Size(this->_rows);
					cblas_dgemv(CblasRowMajor, CblasNoTrans, this->_rows, this->_cols,
						1, this->_data, this->_cols, v._data, 1, 0, r._data, 1);
				}
				return(r);
			}

			Matrix& Matrix::operator()(int i, int j, const Matrix& m)
			{
				tbb::parallel_for(tbb::blocked_range<int>(0, m._rows, BCHUNK_SIZE), [this, i, j,&m](const tbb::blocked_range<int>& b)
				{
					int beg = b.begin();
					int end = b.end();
					for (int k = beg; k < end; ++k)
					{
						memcpy(this->_data + (i+k)*this->_cols + j,
							m._data + k*m._cols,
							m._cols * sizeof(scalar));
					}
				});
				return(*this);
			}

			Matrix operator*(scalar c, const Matrix& m)
			{
				return(m.operator*(c));
			}

			Matrix	Matrix::Transposed() const
			{
				Matrix nm;
				if ((this->_cols == 1) || (this->_rows == 1))
				{
					nm = (*this);
					nm._cols = this->_rows;
					nm._rows = this->_cols;
				}
				else
				{
					nm._cols = this->_rows;
					nm._rows = this->_cols;
					nm.Reserve_Memory();
					if (nm._rows > nm._cols)
					{
						tbb::parallel_for(tbb::blocked_range<int>(0, nm._rows, BCHUNK_SIZE),
							[this, &nm](const tbb::blocked_range<int>& b)
						{
							for (int i = b.begin(); i < b.end(); ++i)
							{
								for (int j = 0; j < nm._cols; ++j)
								{
									nm(i, j, (*this)(j, i));
								}
							}
						});
					}
					else
					{
						tbb::parallel_for(tbb::blocked_range<int>(0, nm._cols, BCHUNK_SIZE),
							[this, &nm](const tbb::blocked_range<int>& b)
						{
							for (int j = b.begin(); j < b.end(); ++j)
							{
								for (int i = 0; i < nm._rows; ++i)
								{
									nm(i, j, (*this)(j, i));
								}
							}
						});
					}
				}
				return(nm);
			}

			bool Matrix::Symmetric() const
			{
				if (this->Square())
				{
					int i = 0;
					int j = 0;
					bool symmetric = true;
					while ((i < this->_rows)&&(symmetric))
					{
						while ((j < i)&&(symmetric))
						{
							symmetric = ((*this)(i,j) == (*this)(j,i));
							++j;
						}
						++i;
					}
					return(symmetric);
				}
				else
				{
					return(false);
				}
			}

			void Matrix::Exchange_Rows(int i, int j)
			{
				Matrix mi = 1.0f*(*this)(i,i+1,0,this->_cols);
				Matrix mj = 1.0f*(*this)(j,j+1,0,this->_cols);
				(*this)(i,0,mj);
				(*this)(j,0,mi);
			}

			int Matrix::Pivot(int k) const
			{
				int indx = k;	
				scalar pivot_value = 0;
				for (int i = k; i < this->_rows; ++i)
				{
					if (fabs((*this)(i,k)) > pivot_value)
					{
						indx = i;
						pivot_value = fabs((*this)(i,k));
					}
				}
				return(indx);
			}

			void Matrix::To_Stream(ostream& os) const
			{
				int n = (int)os.precision();
				os.precision(5);
				for (int i = 0; i < this->_rows; ++i)
				{
					for (int j = 0; j < this->_cols; ++j)
					{
						os << (*this)(i,j) << "   \t";
					}
					os << "\n";
				}
				os.precision(n);
			}

			scalar Matrix::Norm2() const
			{
				scalar n = 0;
				for (int i = 0; i < this->_rows*this->_cols; ++i)
				{
					int row = i/this->_rows;
					int col = i-row*this->_rows;
					scalar e = (*this)(row,col);
					n = n + e*e;
				}
				return(sqrt(n));
			}

			full::Matrix Matrix::House_Holder(const full::Matrix& x, scalar& beta)
			{
				int n = x.Rows();
				scalar sigma = 0;
				full::Matrix x1n;
				if (n > 1)
				{
					x1n << x(1, n, 0, 1);
					sigma = (x1n.Transposed()*x1n)(0, 0);
				}
				Matrix v(n, 1);
				v(0, 0, 1);
				if (n > 1)
				{
					v(1, 0, x1n);
				}
				if (sigma == 0)
				{
					beta = 0;
				}
				else
				{
					scalar mu = sqrt(x(0, 0)*x(0, 0) + sigma);
					if (x(0, 0) <= 0)
					{
						v(0, 0, x(0, 0) - mu);
					}
					else
					{
						v(0, 0, -sigma / (x(0, 0) + mu));
					}
					beta = 2 * v(0, 0)*v(0, 0) / (sigma + v(0, 0)*v(0, 0));
					v << (1 / v(0, 0))*v;
				}
				return(v);
			}

			void Matrix::QR_Fact(full::Matrix& Q, full::Matrix& R, bool update) const
			{
				Matrix A = *this;
				scalar* tau = allocScalar(std::min(A._rows,A._cols));
				LAPACKE_dgeqrf(LAPACK_ROW_MAJOR,A._rows,A._cols,A._data,A._cols,tau);
				if (!update)
				{
					Q = A;
					LAPACKE_dorgqr(LAPACK_ROW_MAJOR, Q._rows, Q._cols, Q._cols, Q._data, Q._cols, tau);
				}
				else
				{
					Q << full::Matrix::Identity(A._rows);
					Q(0, 0, A);
					LAPACKE_dorgqr(LAPACK_ROW_MAJOR, Q._rows, Q._cols, A._cols, Q._data, Q._cols, tau);
				}
				freeScalar(tau);
				R << A;
				if (update)
				{
					tbb::parallel_for(tbb::blocked_range<int>(0, R._rows, BCHUNK_SIZE), [&R](const tbb::blocked_range<int>& b)
					{
						for (int i = b.begin(); i < b.end(); ++i)
						{
							if (i < R._cols)
							{
								for (int j = 0; j < i; ++j)
								{
									R(i, j, 0.0f);
								}
							}
							else
							{
								for (int j = 0; j < R._cols; ++j)
								{
									R(i, j, 0.0f);
								}
							}
						}
					});
				}
			}

			void Matrix::Cholesky(full::Matrix& L, bool update) const
			{
				Matrix r = *this;
				int n = r.Rows();
				for (int k = 0; k < n; ++k)
				{
					if (r(k, k) > 0)
					{
						r(k, k, sqrt(r(k, k)));
						if (k < n - 1)
						{
							full::Matrix rs;
							rs << (1.0f / r(k, k))*r(k + 1, n, k, k + 1);
							r(k + 1, k, rs);
							for (int j = k + 1; j < n; ++j)
							{
								full::Matrix ps;					
								ps << r(j, n, j, j + 1) - r(j, k)*r(j, n, k, k + 1);
								r(j, j, ps);
							}
						}
					}
				}
				if (update)
				{
					for (int i = 0; i < r.Rows(); ++i)
					{
						for (int j = i + 1; j < r.Columns(); ++j)
						{
							r(i, j, 0.0f);
						}
					}
				}
				L = r;
			}

			full::Vector Matrix::Solve_QR(const full::Vector& c) const
			{
				Matrix Q;
				Matrix Rr;
				this->QR_Fact(Q, Rr,false);
				Matrix R;
				if (Rr.Rows() != Rr.Columns())
				{
					R << Rr(0, Rr.Columns(), 0, Rr.Columns());
				}
				else
				{
					R << Rr;
				}
				full::Vector r;
				r << Matrix::Solve_Upper_Triangular(R, (Q.Transposed()*c));
				return(r);
			}

			full::Vector Matrix::Solve_Cholesky(const full::Vector& c) const
			{
				Matrix L;
				this->Cholesky(L,false);
				full::Vector y = Matrix::Solve_Lower_Triangular(L, c);
				y = Matrix::Solve_Upper_Triangular(L.Transposed(), y);
				return(y);
			}

			full::Vector Matrix::Solve_Upper_Triangular(const full::Matrix& triangular, const full::Vector& c)
			{
				full::Vector m = c;
				int n = triangular.Rows();
				if (n > 1)
				{
					scalar* packet = allocScalar(n*(n + 1) / 2);
					tbb::parallel_for(tbb::blocked_range<int>(0, n, BCHUNK_SIZE), [&triangular, packet, n](const tbb::blocked_range<int>& b)
					{
						for (int i = b.begin(); i < b.end(); ++i)
						{
							for (int j = i; j < n; ++j)
							{
								packet[i*n+j-i*(i+1)/2] = triangular(i, j);
							}
						}
					});
					cblas_dtpsv(CblasRowMajor, CblasUpper, CblasNoTrans, CblasNonUnit, n,
						packet, m._data, 1);
					freeScalar(packet);
					return(m);
				}
				else
				{
					m(0, c(0) / triangular(0, 0));
				}
				return(m);
			}

			full::Vector Matrix::Solve_Lower_Triangular(const full::Matrix& triangular, const full::Vector& c)
			{
				full::Vector m = c;
				int n = triangular.Rows();
				if (n > 1)
				{
					scalar* packet = allocScalar(n*(n + 1) / 2);
					tbb::parallel_for(tbb::blocked_range<int>(0, n, BCHUNK_SIZE), [&triangular, packet, n](const tbb::blocked_range<int>& b)
					{
						for (int i = b.begin(); i < b.end(); ++i)
						{
							for (int j = 0; j <= i; ++j)
							{
								packet[j+i*(i+1)/2] = triangular(i, j);
							}
						}
					});
					cblas_dtpsv(CblasRowMajor, CblasLower, CblasNoTrans, CblasNonUnit, n,
						packet, m._data, 1);
					freeScalar(packet);
					return(m);
				}
				else
				{
					m(0, c(0) / triangular(0, 0));
				}
				return(m);
			}

			full::Matrix Matrix::Sub_Columns(const IndxSet& indx) const
			{
				full::Matrix r(this->Rows(), (int)indx.size());
				IndxSet::const_iterator i = indx.begin();
				int j = 0;
				while (i != indx.end())
				{
					int k = *i;
					for (int val = 0; val < this->Rows(); ++val)
					{
						r(val, j, (*this)(val, k));
					}
					++i;
					++j;
				}
				return(r);
			}

			full::Matrix Matrix::Sub_Rows(const IndxSet& indx) const
			{
				full::Matrix r((int)indx.size(),this->Columns());
				IndxSet::const_iterator i = indx.begin();
				int j = 0;
				while (i != indx.end())
				{
					int k = *i;
					for (int Voxel_Length = 0; Voxel_Length < this->Columns(); ++Voxel_Length)
					{
						r(j, Voxel_Length, (*this)(k, Voxel_Length));
					}
					++i;
					++j;
				}
				return(r);
			}

			full::Vector Matrix::NNLS_Iterative(const full::Matrix& M, const full::Vector& d)
			{
				full::Matrix Mt;
				Mt << M.Transposed();
				full::Matrix H = Mt*M;
				full::Vector f = ((scalar)(-1))*Mt*d;
				full::Vector u = f;
				int n = u.Size();
				full::Vector xkp(n);
				full::Vector xk(n);
				bool stop = false;
				int its = 0;
				while (!stop)
				{
					++its;
					int k;
					for (k = 0; k < n; ++k)
					{
						scalar Voxel_Length = std::max((scalar)0,xk(k) - u(k) / H(k, k));
						xkp(k, Voxel_Length);
						if (xkp(k) != xk(k))
						{
							u = u + (xkp(k) - xk(k))*H.Column(k);
						}
						xk(k,xkp(k));
					}
					if (its > n*100)
					{
						stop = true;
					}
				}
				return(xkp);
			}

			void rot(const scalar f, const scalar g, scalar& cs, scalar& sn, scalar& r)
			{
				if (fabs(f) < EPSILON)
				{
					cs = 0;
					sn = 1;
					r = g;
				}
				else if (fabs(f) >= fabs(g))
				{
					scalar t = g / f;
					scalar tt = sqrt(1 + t*t);
					cs = 1 / tt;
					sn = t*cs;
					r = f*tt;
				}
				else
				{
					scalar t = f / g;
					scalar tt = sqrt(1 + t*t);
					sn = 1 / tt;
					cs = t*sn;
					r = g*tt;
				}
			}

			void Matrix::Bidiagonal(full::Matrix& U, full::Matrix& V, full::Matrix& B) const
			{
				full::Matrix A = *this;
				scalar v = (*this)(0, 0);
				A(0, 0, v+1);
				A(0, 0, v);
				int m = A.Rows();
				int n = A.Columns();
				int p = n;
				U << Matrix::Identity(m);
				V << Matrix::Identity(n);
				if (m == n)
				{
					--p;
				}
				for (int j = 0; j < p; ++j)
				{
					full::Matrix HA;
					full::Matrix HU;
					full::Matrix HV;
					scalar b = 0;
					full::Matrix v;
					v << Matrix::House_Holder(A(j, m, j, j + 1), b);
					if (b != 0)
					{
						full::Matrix Ajmjn;
						full::Matrix Ujm0m;
						full::Matrix HA;
						full::Matrix HU;
						Ajmjn << A(j, m, j, n);
						HA << Ajmjn - (b*v)*(v.Transposed()*Ajmjn);
						A(j, j, HA);
						Ujm0m << U(j, m, 0, m);
						HU << Ujm0m - (b*v)*(v.Transposed()*Ujm0m);
						U(j, 0, HU);
					}
					if (j < n - 1)
					{
						v << Matrix::House_Holder(A(j, j + 1, j + 1, n).Transposed(), b);
						if (b != 0)
						{
							full::Matrix HV;
							full::Matrix HA;
							full::Matrix Vjnjn;
							Vjnjn << V(j + 1, n, j + 1, n);
							HV << Vjnjn - (Vjnjn*(b*v))*v.Transposed();
							V(j + 1, j + 1, HV);
							full::Matrix Ajmjn;
							Ajmjn = A(j, m, j + 1, n);
							HA << Ajmjn - (Ajmjn*(b*v))*v.Transposed();
							A(j, j + 1, HA);
						}
					}
				}
				B << A;
			}


			full::Vector Matrix::NNLS(const full::Matrix& M, const full::Vector& d)
			{
				full::Matrix A;
				full::Vector y;
				A = M;
				y = d;

				int n = A.Rows();
				int m = A.Columns();
				IndxSet P;
				IndxSet Z;
				int i;
				for (i = 0; i < m; ++i)
				{
					Z.insert(i);
				}
				full::Vector x(m);
				full::Vector w;

				full::Matrix B;
				full::Matrix At;
				At << A.Transposed();
				B << At*A;
				full::Vector yb;
				yb << At*y;

				int j;

				IndxSet::const_iterator ip;
				bool elp = true;
				int its = 0;
				int maxits = std::max(n, m);
				maxits = std::min(maxits, MAX_ITERATIONS);
				while ((elp) && (its < maxits))
				{
					++its;
					w << At*(y - A*x);
					if ((Z.size() > 0) && (w.V_Max(Z, j) > EPSILON))
					{
						Z.erase(j);
						P.insert(j);
						bool ilp = true;
						while (ilp)
						{
							full::Matrix Bp;
							Bp << B.Sub_Columns(P).Sub_Rows(P);
							full::Vector sp;
							sp << Bp.Solve_QR(yb.Sub_Vector(P));
							if (sp.V_Min() <= 0)
							{
								i = 0;
								ip = P.begin();
								scalar Regularizer = (scalar)1e32;
								while (ip != P.end())
								{
									scalar d = x(*ip) / (x(*ip) - sp(i));
									if ((d < Regularizer) && (sp(i) < 0))
									{
										Regularizer = d;
									}
									++ip;
									++i;
								}
								ip = P.begin();
								i = 0;
								IndxSet NS;
								while (ip != P.end())
								{
									x((*ip), x(*ip) + Regularizer*(sp(i) - x(*ip)));
									scalar vc = x(*ip);
									if (vc > 0)
									{
										NS.insert(*ip);
									}
									else
									{
										x(*ip, (scalar)0);
										Z.insert(*ip);
									}
									++ip;
									++i;
								}
								P = NS;
							}
							else
							{
								ip = P.begin();
								i = 0;
								while (ip != P.end())
								{
									x(*ip, sp(i));
									++ip;
									++i;
								}
								ilp = false;
							}
						}
					}
					else
					{
						elp = false;
					}
				}
				tbb::parallel_for(tbb::blocked_range<int>(0, x.Size(), BCHUNK_SIZE),
					[&x](const tbb::blocked_range<int>& b)
				{
					for (int i = b.begin(); i < b.end(); ++i)
					{
						if (x(i) < EPSILON)
						{
							x(i, (scalar)0);
						}
					}
				});
				return(x);
			}

			void Matrix::Column_Gram_Schmidth()
			{
				for (int i = 0; i < this->Columns(); ++i)
				{
					full::Vector v = this->Column(i);
					for (int j = 0; j < i; ++j)
					{
						v.apbv(1, -this->Column(j).Dot(v), this->Column(j));
					}
					scalar nrm = v.Norm();		
					this->Set_Column((1 / nrm)*v, i);
				}
			}

			void Matrix::Set_Column(const full::Vector& v, int col)
			{
				if (this->_rows == v.Size())
				{
					tbb::parallel_for(tbb::blocked_range<int>(0, v.Size(), BCHUNK_SIZE), [&v, this, col](const tbb::blocked_range<int>& b)
					{
						for (int i = b.begin(); i != b.end(); ++i)
						{
							(*this)(i, col, v(i));
						}
					});
				}
			}

			void Matrix::Set_Row(const full::Vector& v, int row)
			{
				if (this->_cols == v.Size())
				{
					tbb::parallel_for(tbb::blocked_range<int>(0, v.Size(), CHUNK_SIZE), [&v, this, row](const tbb::blocked_range<int>& b)
					{
						for (int i = b.begin(); i != b.end(); ++i)
						{
							(*this)(row, i, v(i));
						}
					});
				}
			}

			scalar Matrix::Norm() const
			{
				scalar* Voxel_Length = allocScalar(this->Rows());
				tbb::parallel_for(tbb::blocked_range<int>(0, this->Rows(), CHUNK_SIZE), [Voxel_Length, this](const tbb::blocked_range<int>& b)
				{
					for (int i = b.begin(); i != b.end(); ++i)
					{
						Voxel_Length[i] = 0;
						for (int j = 0; j < this->Columns(); ++j)
						{
							scalar d = fabs((*this)(i, j));
							if (d > Voxel_Length[i])
							{
								Voxel_Length[i] = d;
							}
						}
					}
				});
				scalar sl = 0;
				for (int i = 0; i < this->Rows(); ++i)
				{
					scalar d = fabs(Voxel_Length[i]);
					if (d > sl)
					{
						sl = Voxel_Length[i];
					}
				}
				freeScalar(Voxel_Length);
				return(sl);
			}

			void Exchange(full::Matrix& G, full::Matrix& V, vector<scalar>& norms, int i, int j)
			{
				scalar tmp = norms[i];
				norms[i] = norms[j];
				norms[j] = tmp;
				full::Vector tmv = G.Column(i);
				G.Set_Column(G.Column(j), i);
				G.Set_Column(tmv, j);
				tmv = V.Column(i);
				V.Set_Column(V.Column(j), i);
				V.Set_Column(tmv, j);
			}

			void Matrix::SVD_Zero_Shift(full::Matrix& U, full::Matrix& V, full::Matrix& B) const
			{
				int m = this->Rows();
				int n = this->Columns();
				full::Matrix U1;
				full::Matrix V1;
				full::Matrix A;
				full::Matrix Q;
				if (m > 5 * n / 3)
				{
					full::Matrix R;
					this->QR_Fact(Q, R);
					R << R(0, n, 0, n);
					R.Bidiagonal(U1, V1, A);
					A << A(0, n, 0, n);
				}
				else
				{
					this->Bidiagonal(U1, V1, A);
					A << A(0, n, 0, n);
				}
				V = full::Matrix::Identity(n);
				U = full::Matrix::Identity(n);

				scalar oldcs = 0;
				scalar oldsn = 0;
				scalar f = 0;
				scalar g = 0;
				scalar cs = 0;
				scalar sn = 0;
				scalar r = 0;
				scalar h = 0;
				bool val = true;
				int its = 0;
				while ((val) && (its < m))
				{
					++its;
					oldcs = 1;
					f = A(0, 0);
					g = A(0, 1);
					for (int i = 0; i < n-1; ++i)
					{
						rot(f, g, cs, sn, r);
						full::Vector v1 = V.Column(i);
						full::Vector v2 = V.Column(i + 1);
						V.Set_Column(((const full::Vector&)v1).apbv(cs,sn,v2), i);
						V.Set_Column(((const full::Vector&)v1).apbv(-sn,cs,v2), i+1);
						if (i > 0)
						{
							A(i - 1, i, oldsn*r);
						}
						f = oldcs*r;
						g = A(i + 1, i + 1)*sn;
						h = A(i + 1, i + 1)*cs;
						rot(f, g, cs, sn, r);
						v1 = U.Column(i);
						v2 = U.Column(i + 1);
						U.Set_Column(((const full::Vector&)v1).apbv(cs,sn,v2), i);
						U.Set_Column(((const full::Vector&)v1).apbv(-sn, cs, v2), i + 1);
						A(i, i, r);
						f = h;
						if (i < n - 2)
						{
							g = A(i + 1, i + 2);
						}
						else
						{
							g = 0;
						}
						oldcs = cs;
						oldsn = sn;
					}
					A(n - 2, n - 1, h*sn);
					A(n - 1, n - 1, h*cs);
					scalar u = A(0, 0);
					val = false;
					scalar b = A(0,1);
					if (b < EPSILON)
					{
						A(0, 1, 0);
					}
					else
					{
						val = true;
					}
				}
				B << full::Matrix(m, n);
				B(0, 0, A);
				if (U1.Rows() > n)
				{
					full::Matrix EU;
					full::Matrix U1T;
					U1T << U1.Transposed();
					EU << U1T(0, U1T.Rows(), 0, U.Rows())*U;
					U << U1T(0, 0, EU);
				}
				else
				{
					U << U1.Transposed()*U;
				}
				if (Q.Rows() > U.Rows())
				{
					full::Matrix EU;
					EU << Q(0,Q.Rows(),0,U.Rows());
					EU << EU*U;
					U << Q(0, 0, EU);
				}
				V << V1.Transposed()*V;
			}

			void Matrix::SVD_Factor(full::Matrix& U, full::Matrix& V, full::Matrix& B) const
			{
				full::Matrix G;
				full::Matrix Q;
				this->QR_Fact(Q, G);

				G = G(0, G.Columns(), 0, G.Columns());

				scalar nrm = G.Norm();
				scalar e = 1;
				int its = 0;
				V = full::Matrix::Identity(G.Columns());
				while ((fabs(e) > EPSILON) && (its < 100*G.Columns()))
				{
					++its;
					e = 0;
					for (int i = 0; i < G.Columns(); ++i)
					{
						full::Vector gi = G.Column(i);
						scalar a = gi.Dot(gi);
						for (int j = i + 1; j < G.Columns(); ++j)
						{
							full::Vector gj = G.Column(j);
							scalar b = gj.Dot(gj);
							scalar c = gj.Dot(gi);
							if (fabs(c) > EPSILON)
							{
								scalar rho = 0;
								scalar t = 0;
								rho = (b - a) / (2 * c);
								t = 1 / (fabs(rho) + sqrt(1 + rho*rho));

								if (rho < 0)
								{
									t = -t;
								}
								scalar cs = 1 / (sqrt(1 + t*t));
								scalar sn = cs*t;

								full::Vector tmp = G.Column(i);
								full::Vector tmpj = G.Column(j);
								G.Set_Column(cs*tmp - sn*tmpj, i);
								G.Set_Column(sn*tmp + cs*tmpj, j);

								tmp = V.Column(i);
								tmpj = V.Column(j);
								V.Set_Column(cs*tmp - sn*tmpj, i);
								V.Set_Column(sn*tmp + cs*tmpj, j);
								e = e + fabs(c) / sqrt(a*b);
							}
						}
					}
					e = e / nrm;
				}
				B = full::Matrix(this->Rows(), this->Columns());
				vector<scalar> norms;
				for (int i = 0; i < this->Columns(); ++i)
				{
					norms.push_back(0);
					norms[i] = G.Column(i).Norm();
				}
				for (int i = 0; i < norms.size(); ++i)
				{
					for (int j = 0; j < norms.size(); ++j)
					{
						if (norms[j] < norms[i])
						{
							Exchange(G, V, norms, i, j);
						}
					}
				}
				for (int i = 0; i < this->Columns(); ++i)
				{
					B(i, i, norms[i]);
					G.Set_Column((1 / B(i, i))*G.Column(i), i);
				}
				U = full::Matrix::Identity(G.Rows());
				if (G.Columns() > U.Columns())
				{
					U(0, 0, G(0, U.Rows(), 0, U.Rows()));
				}
				else
				{
					U(0, 0, G);
				}
	
				full::Matrix EU = full::Matrix::Identity(Q.Rows());
				EU(0, 0, U);
				U = Q*EU;
				full::Matrix EB(this->Rows(), this->Columns());
				EB(0, 0, B);
				B = EB;	
			}	
		}
	}
}
