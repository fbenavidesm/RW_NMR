#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <string.h>
#include "mtx2.h"

using std::cos;
using std::sin;
using std::size;

namespace math_la
{
    namespace math_lac
    {

        namespace space
        {

            Mtx2 Mtx2::operator + (const Mtx2& m) const
            {
                Mtx2 r;
                for (unsigned int i = 0; i < 2; i++)
                {
                    for (unsigned int j = 0; j < 2; j++)
                    {
                        r(i, j, m(i, j) + (*this)(i, j));
                    }
                }
                return(r);
            }

            Mtx2 Mtx2::operator - (const Mtx2& m) const
            {
                Mtx2 r;
                for (unsigned int i = 0; i < 2; i++)
                {
                    for (unsigned int j = 0; j < 2; j++)
                    {
                        r(i, j, (*this)(i, j) - m(i, j));
                    }
                }
                return(r);
            }

            Mtx2 Mtx2::operator * (scalar c) const
            {
                Mtx2 r;
                for (unsigned int i = 0; i < 2; i++)
                {
                    for (unsigned int j = 0; j < 2; j++)
                    {
                        r(i, j, c * (*this)(i, j));
                    }
                }
                return(r);
            }

            Mtx2 operator * (scalar c, const Mtx2& m)
            {
                Mtx2 r;
                for (unsigned int i = 0; i < 2; i++)
                {
                    for (unsigned int j = 0; j < 2; j++)
                    {
                        r(i, j, c * m(i, j));
                    }
                }
                return(r);
            }


            bool Mtx2::operator == (const Mtx2& m) const
            {
                scalar r = 0;
                for (unsigned int i = 0; i < 2; i++)
                {
                    for (unsigned int j = 0; j < 2; j++)
                    {
                        scalar stabilizer = (m(i, j) > (*this)(i, j) ? m(i, j) : (*this)(i, j));
                        if (stabilizer < 1)
                        {
                            stabilizer = 1.0f;
                        }
                        scalar Voxel_Length = ((*this)(i, j) - m(i, j)) / stabilizer;
                        r = r + Voxel_Length * Voxel_Length;
                    }
                }
                return ((bool)(r < EPSILON));
            }

            Mtx2& Mtx2::operator += (Mtx2& m)
            {
                for (unsigned int i = 0; i < 2; i++)
                {
                    for (unsigned int j = 0; j < 2; j++)
                    {
                        (*this)(i, j, (*this)(i, j) + m(i, j));
                    }
                }
                return(*this);
            }

            Mtx2& Mtx2::operator -= (Mtx2& m)
            {
                for (unsigned int i = 0; i < 2; i++)
                {
                    for (unsigned int j = 0; j < 2; j++)
                    {
                        (*this)(i, j, (*this)(i, j) - m(i, j));
                    }
                }
                return(*this);
            }

            Mtx2& Mtx2::operator *= (scalar c)
            {
                for (unsigned int i = 0; i < 2; i++)
                {
                    for (unsigned int j = 0; j < 2; j++)
                    {
                        (*this)(i, j, c * (*this)(i, j));
                    }
                }
                return(*this);
            }

            Vec2 Mtx2::operator * (const Vec2& v) const
            {
                return(Vec2(
                    (*this)(0, 0) * v(0) + (*this)(0, 1) * v(1),
                    (*this)(1, 0) * v(0) + (*this)(1, 1) * v(1)));
            }

            Mtx2 Mtx2::operator * (const Mtx2& m) const
            {
                Mtx2 r;
                for (unsigned int i = 0; i < 2; ++i)
                {
                    for (unsigned int j = 0; j < 2; ++j)
                    {
                        for (unsigned int k = 0; k < 2; ++k)
                        {
                            r(i, j, r(i, j) + (*this)(i, k) * m(k, j));
                        }
                    }
                }
                return(r);
            }

            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------

            Mtx2::Mtx2()
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    this->_cmpx[i] = 0.0f;
                }
            }

            Mtx2::Mtx2(const Mtx2& m)
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    this->_cmpx[i] = m._cmpx[i];
                }
            }

            Mtx2& Mtx2::operator=(const Mtx2& m)
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    this->_cmpx[i] = m._cmpx[i];
                }
                return(*this);
            }

            Mtx2& Mtx2::operator()(uint i, uint j, scalar value)
            {
                i = i % 2;
                j = j % 2;
                this->_cmpx[2 * i + j] = value;
                return(*this);
            }

            Mtx2::Mtx2(const Vec2& row1,
                const Vec2& row2)
            {
                this->_cmpx[0] = row1(eX);
                this->_cmpx[1] = row1(eY);

                this->_cmpx[2] = row2(eX);
                this->_cmpx[3] = row2(eY);
            }

            Mtx2 Mtx2::Identity()
            {
                Mtx2 id;
                id(0, 0, 1);
                id(1, 1, 1);
                return(id);
            }

            scalar Mtx2::operator()(uint i, uint j) const
            {
                return(this->_cmpx[2 * i + j]);
            }

            Vec2 Mtx2::Row(uint i) const
            {
                return(Vec2(this->_cmpx[2 * i], this->_cmpx[2 * i + 1]));
            }

            Vec2 Mtx2::Column(uint i) const
            {
                return(Vec2(this->_cmpx[i], this->_cmpx[2 + i]));
            }

            Mtx2& Mtx2::Row(uint i, const Vec2& row)
            {
                (*this)(i, 0, row(eX));
                (*this)(i, 1, row(eY));
                return(*this);
            }

            Mtx2& Mtx2::Column(uint i, const Vec2& col)
            {
                (*this)(0, i, col(eX));
                (*this)(1, i, col(eY));
                return(*this);
            }

            Mtx2& Mtx2::Transpose()
            {
                scalar fNew[4];
                for (unsigned int i = 0; i < 3; i++)
                {
                    fNew[i] = this->_cmpx[(2 * i) % 3];
                }
                fNew[3] = this->_cmpx[3];
                memcpy(this->_cmpx, fNew, 4 * sizeof(scalar));
                return (*this);
            }

            scalar Mtx2::Determinant() const
            {
                scalar d = (*this)(0, 0) * (*this)(1, 1) -
                    (*this)(0, 1) * (*this)(1, 0);
                return(d);
            }

            Mtx2 Mtx2::Inverse() const
            {
                scalar det = this->Determinant();
                if (!(fabs(det) < EPSILON))
                {
                    Mtx2 inv;
                    inv(1, 1, (*this)(0, 0));
                    inv(0, 0, (*this)(1, 1));
                    inv(0, 1, -(*this)(0, 1));
                    inv(1, 0, -(*this)(1, 0));
                    return((1 / det) * inv);
                }
                else
                {
                    throw std::runtime_error("Matrix is not invertible");
                }
            }

            Mtx2 Mtx2::Transposed() const
            {
                Mtx2 copy = (*this);
                copy.Transpose();
                return(copy);
            }

            void Mtx2::Stream(double* str) const
            {
                for (int i = 0; i < 4; ++i)
                {
                    *(str + i) = (double)this->_cmpx[i];
                }
            }

            Mtx2::operator Mtx3()
            {
                Mtx3 r;
                for (unsigned int i = 0; i < 2; ++i)
                {
                    for (unsigned int j = 0; j < 2; ++j)
                    {
                        r(i, j, (*this)(i, j));
                    }
                }
                r(2, 2, 1.0f);
                return(r);
            }

            Mtx2 Mtx2::Rotation(scalar angle)
            {
                scalar cs = cos(angle);
                scalar ss = sin(angle);
                return(Mtx2(Vec2(cs, -ss), Vec2(ss, cs)));
            }

            Vec2 Mtx2::Solve(const Vec2& c) const
            {
                Vec2 r;
                scalar d = this->Determinant();
                if (d == 0.0f)
                {
                    throw std::runtime_error("System is not solvable");
                }
                else
                {
                    r(eX, (c(eX) * (*this)(1, 1) - c(eY) * (*this)(0, 1)) / d);
                    r(eY, (c(eY) * (*this)(0, 0) - c(eX) * (*this)(1, 0)) / d);
                }
                return(r);
            }

        }
    }
}
