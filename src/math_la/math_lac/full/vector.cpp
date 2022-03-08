#include <cstring>
#include <iostream>
#include "math_la/mdefs.h"
#include "vector.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_reduce.h"
#include "mkl.h"

namespace math_la
{
	namespace math_lac
	{


		namespace full
		{

			using std::memset;

			Vector::Vector()
			{
				this->_size = 0;
				this->_data = 0;
			}

			Vector::~Vector()
			{
				this->_size = 0;
				this->Free_Memory();
			}

			Vector::Vector(const Vector& v)
			{
				this->_data = 0;
				this->_size = v._size;
				this->Reserve_Memory();
				memcpy(this->_data, v._data, this->_size * sizeof(scalar));
			}

			void Vector::Reserve_Memory()
			{
				if (this->_data)
				{
					freeScalar(this->_data);
				}
				this->_data = allocScalar(this->_size);
			}

			void Vector::Set_Memory()
			{
				if (this->_data)
				{
					freeScalar(this->_data);
				}
				this->_data = allocScalar(this->_size);
				memset(this->_data, 0, this->_size * sizeof(scalar));
			}

			void Vector::Free_Memory()
			{
				if (this->_data)
				{
					freeScalar(this->_data);
				}
				this->_data = 0;
			}


			Vector::Vector(int size)
			{
				this->_data = 0;
				this->_size = size;
				this->Set_Memory();
			}

			void Vector::Set_Size(int size)
			{
				this->_size = size;
				this->Set_Memory();
			}

			Vector Vector::operator()(int i, int j) const
			{
				Vector v;
				v._size = j - i;
				v.Reserve_Memory();
				memcpy(v._data, this->_data + i, (j - i) * sizeof(scalar));
				return(v);
			}

			Vector Vector::Sub_Vector(int i, int j) const
			{
				Vector v = (*this)(i, j);
				return(v);
			}

			Vector& Vector::operator()(int i, scalar value)
			{
				this->_data[i] = value;
				return (*this);
			}

			Vector operator*(scalar c, const Vector& v)
			{
				return(v * c);
			}

			Vector& Vector::operator = (const Vector& v)
			{
				if (this->_size != v._size)
				{
					this->Free_Memory();
					this->_size = v._size;
					this->Reserve_Memory();
				}
				memcpy(this->_data, v._data, this->_size * sizeof(scalar));
				return(*this);
			}

			Vector& Vector::operator << (Vector& v)
			{
				this->Free_Memory();
				this->_size = v._size;
				this->_data = v._data;
				v._data = 0;
				v._size = 0;
				return(*this);
			}

			Vector  Vector::operator + (const Vector& v) const
			{
				Vector r = (*this);
				if (this->_size == v._size)
				{
					cblas_daxpy(this->_size, 1, v._data, 1, r._data, 1);
				}
				return(r);
			}

			full::Vector Vector::apbv(scalar a, scalar b, const Vector& v) const
			{
				full::Vector r = *this;
				r.apbv(a, b, v);
				return(r);
			}

			void    Vector::apbv(scalar a, scalar b, const Vector& v)
			{
				if (this->_size == v._size)
				{
					cblas_daxpby(this->_size, b, v._data, 1, a, this->_data, 1);
				}
			}

			Vector  Vector::ap(scalar a, const Vector& v) const
			{
				Vector r = (*this);
				if (this->_size == v._size)
				{
					cblas_daxpby(this->_size, 1, v._data, 1, a, r._data, 1);
				}
				return(r);
			}

			Vector  Vector::operator - (const Vector& v) const
			{
				Vector r = (*this);
				if (this->_size == v._size)
				{
					cblas_daxpy(this->_size, -1, v._data, 1, r._data, 1);
				}
				return(r);
			}

			Vector  Vector::am(scalar a, const Vector& v) const
			{
				Vector r = (*this);
				if (this->_size == v._size)
				{
					cblas_daxpby(this->_size, -1, v._data, 1, a, r._data, 1);
				}
				return(r);
			}


			Vector  Vector::operator * (scalar c) const
			{
				Vector r = (*this);
				cblas_dscal(r._size, c, r._data, 1);
				return(r);
			}

			scalar Vector::Dot(const Vector& v) const
			{
				scalar r = 0.0f;
				if (this->_size == v._size)
				{
					r = cblas_ddot(this->_size, v._data, 1, this->_data, 1);
				}
				return(r);
			}

			void Vector::Side_Div(const full::Vector& v)
			{
				if (this->Size() == v.Size())
				{
					tbb::parallel_for(tbb::blocked_range<int>(0, this->Size(), BCHUNK_SIZE), [&v, this](const tbb::blocked_range<int>& b)
						{
							int beg = b.begin();
							int end = b.end();
							for (int i = beg; i < end; ++i)
							{
								scalar val = (*this)(i) / v(i);
								(*this)(i, val);
							}
						});
				}
			}

			full::Vector Vector::Cross_Correlation(const full::Vector& y) const
			{
				full::Vector r;
				if (this->_size == y._size)
				{
					r = full::Vector(this->_size);
					tbb::parallel_for(tbb::blocked_range<int>(0, this->_size, BCHUNK_SIZE), [&y, &r, this](const tbb::blocked_range<int>& b)
						{
							int beg = b.begin();
							int end = b.end();
							for (int i = beg; i < end; ++i)
							{
								scalar val = 0;
								for (int m = 0; m < this->_size; ++m)
								{
									int mn = (m + i) % this->_size;
									val = val + (*this)(m) * y(mn);
								}
								r(i, val);
							}
						});
				}
				return(r);
			}

			full::Vector Vector::Sub_Vector(const IndxSet& indx) const
			{
				full::Vector r;
				r.Set_Size((int)indx.size());
				IndxSet::const_iterator i = indx.begin();
				int j = 0;
				while (i != indx.end())
				{
					r(j, (*this)(*i));
					++i;
					++j;
				}
				return(r);
			}

			scalar Vector::V_Min() const
			{
				int i = (int)cblas_idamin(this->_size, this->_data, 1);
				scalar r = this->_data[i];
				return(r);
			}

			scalar Vector::V_Max() const
			{

				int i = (int)cblas_idamax(this->_size, this->_data, 1);
				scalar r = this->_data[i];
				return(r);
			}

			scalar Vector::V_Min(int& k) const
			{
				k = (int)cblas_idamin(this->_size, this->_data, 1);
				scalar r = this->_data[k];
				return(r);
			}

			scalar Vector::V_Max(int& k) const
			{
				k = (int)cblas_idamax(this->_size, this->_data, 1);
				scalar r = this->_data[k];
				return(r);
			}


			scalar Vector::V_Min(const IndxSet& i) const
			{
				IndxSet::const_iterator ii = i.begin();
				scalar r = (*this)(*ii);
				while (ii != i.end())
				{
					if ((*this)(*ii) < r)
					{
						r = (*this)(*ii);
					}
					++ii;
				}
				return(r);
			}

			scalar Vector::V_Max(const IndxSet& i) const
			{
				IndxSet::const_iterator ii = i.begin();
				scalar r = (*this)(*ii);
				while (ii != i.end())
				{
					if ((*this)(*ii) > r)
					{
						r = (*this)(*ii);
					}
					++ii;
				}
				return(r);
			}

			scalar Vector::V_Min(const IndxSet& i, int& k) const
			{
				IndxSet::const_iterator ii = i.begin();
				scalar r = (*this)(*ii);
				k = *ii;
				while (ii != i.end())
				{
					if ((*this)(*ii) < r)
					{
						r = (*this)(*ii);
						k = *ii;
					}
					++ii;
				}
				return(r);
			}

			scalar Vector::V_Max(const IndxSet& i, int& k) const
			{
				IndxSet::const_iterator ii = i.begin();
				scalar r = (*this)(*ii);
				k = *ii;
				while (ii != i.end())
				{
					if ((*this)(*ii) > r)
					{
						r = (*this)(*ii);
						k = *ii;
					}
					++ii;
				}
				return(r);
			}

		}
	}
}
