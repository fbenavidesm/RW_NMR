#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include <string.h>
#include "mtx3.h"
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

            Mtx3 Mtx3::operator + (const Mtx3& m) const
            {
                Mtx3 r;
                for (unsigned int i = 0; i < 3; i++)
                {
                    for (unsigned int j = 0; j < 3; j++)
                    {
                        r(i, j, (*this)(i, j) + m(i, j));
                    }
                }
                return(r);
            }

            Mtx3 Mtx3::operator - (const Mtx3& m) const
            {
                Mtx3 r;
                for (unsigned int i = 0; i < 3; i++)
                {
                    for (unsigned int j = 0; j < 3; j++)
                    {
                        r(i, j, (*this)(i, j) - m(i, j));
                    }
                }
                return(r);
            }

            Mtx3 Mtx3::operator * (scalar c) const
            {
                Mtx3 r;
                for (unsigned int i = 0; i < 3; i++)
                {
                    for (unsigned int j = 0; j < 3; j++)
                    {
                        r(i, j, c * (*this)(i, j));
                    }
                }
                return(r);
            }

            Mtx3 operator * (scalar c, const Mtx3& m)
            {
                Mtx3 r;
                for (unsigned int i = 0; i < 3; i++)
                {
                    for (unsigned int j = 0; j < 3; j++)
                    {
                        r(i, j, c * m(i, j));
                    }
                }
                return(r);
            }


            bool Mtx3::operator == (const Mtx3& m) const
            {
                scalar r = 0;
                for (unsigned int i = 0; i < 3; i++)
                {
                    for (unsigned int j = 0; j < 3; j++)
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
                return(r < EPSILON);
            }

            Mtx3& Mtx3::operator += (const Mtx3& m)
            {
                for (unsigned int i = 0; i < 3; i++)
                {
                    for (unsigned int j = 0; j < 3; j++)
                    {
                        (*this)(i, j, (*this)(i, j) + m(i, j));
                    }
                }
                return(*this);
            }

            Mtx3& Mtx3::operator -= (const Mtx3& m)
            {
                for (unsigned int i = 0; i < 3; i++)
                {
                    for (unsigned int j = 0; j < 3; j++)
                    {
                        (*this)(i, j, (*this)(i, j) - m(i, j));
                    }
                }
                return(*this);
            }

            Mtx3& Mtx3::operator *= (scalar c)
            {
                for (unsigned int i = 0; i < 3; i++)
                {
                    for (unsigned int j = 0; j < 3; j++)
                    {
                        (*this)(i, j, c * (*this)(i, j));
                    }
                }
                return(*this);
            }

            Vec3 Mtx3::operator * (const Vec3& v) const
            {
                return(Vec3(
                    (*this)(0, 0) * v(0) + (*this)(0, 1) * v(1) + (*this)(0, 2) * v(2),
                    (*this)(1, 0) * v(0) + (*this)(1, 1) * v(1) + (*this)(1, 2) * v(2),
                    (*this)(2, 0) * v(0) + (*this)(2, 1) * v(1) + (*this)(2, 2) * v(2)));
            }

            Mtx3 Mtx3::operator * (const Mtx3& m) const
            {
                Mtx3 r;
                for (unsigned int i = 0; i < 3; ++i)
                {
                    for (unsigned int j = 0; j < 3; ++j)
                    {
                        for (unsigned int k = 0; k < 3; ++k)
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

            Mtx3::Mtx3()
            {
                for (unsigned int i = 0; i < 9; i++)
                {
                    this->_cmpx[i] = 0.0f;
                }
            }

            Mtx3::Mtx3(const Mtx3& m)
            {
                for (unsigned int i = 0; i < 9; i++)
                {
                    this->_cmpx[i] = m._cmpx[i];
                }
            }

            Mtx3 Mtx3::CrossMtx(const Vec3& v)
            {
                Mtx3 mtx;
                mtx(0, 1, -v(eZ))(0, 2, v(eY));
                mtx(1, 0, v(eZ))(1, 2, -v(eX));
                mtx(2, 0, -v(eY))(0, 1, v(eX));
                return(mtx);
            }

            Mtx3& Mtx3::operator=(const Mtx3& m)
            {
                for (unsigned int i = 0; i < 9; i++)
                {
                    this->_cmpx[i] = m._cmpx[i];
                }
                return(*this);
            }


            Mtx3& Mtx3::operator()(uint i, uint j, scalar value)
            {
                i = i % 3;
                j = j % 3;
                this->_cmpx[3 * i + j] = value;
                return(*this);
            }

            Mtx3::Mtx3(const Vec3& row1,
                const Vec3& row2,
                const Vec3& row3)
            {
                this->_cmpx[0] = row1(eX);
                this->_cmpx[1] = row1(eY);
                this->_cmpx[2] = row1(eZ);

                this->_cmpx[3] = row2(eX);
                this->_cmpx[4] = row2(eY);
                this->_cmpx[5] = row2(eZ);

                this->_cmpx[6] = row3(eX);
                this->_cmpx[7] = row3(eY);
                this->_cmpx[8] = row3(eZ);
            }

            Mtx3 Mtx3::Identity()
            {
                Mtx3 id;
                id(0, 0, 1);
                id(1, 1, 1);
                id(2, 2, 1);
                return(id);
            }

            scalar Mtx3::operator()(uint i, uint j) const
            {
                return(this->_cmpx[3 * i + j]);
            }

            Vec3 Mtx3::Row(uint i) const
            {
                return(Vec3(this->_cmpx[3 * i], this->_cmpx[3 * i + 1], this->_cmpx[3 * i + 2]));
            }

            Vec3 Mtx3::Column(uint i) const
            {
                return(Vec3(this->_cmpx[i], this->_cmpx[3 + i], this->_cmpx[6 + i]));
            }

            Mtx3& Mtx3::Row(uint i, const Vec3& row)
            {
                (*this)(i, 0, row(eX));
                (*this)(i, 1, row(eY));
                (*this)(i, 2, row(eZ));
                return(*this);
            }

            Mtx3& Mtx3::Column(uint i, const Vec3& col)
            {
                (*this)(0, i, col(eX));
                (*this)(1, i, col(eY));
                (*this)(2, i, col(eZ));
                return(*this);
            }

            scalar Mtx3::Coefficient(unsigned int i, unsigned int j) const
            {
                Mtx2 sm;
                uint rr = 0;
                uint cc = 0;
                for (uint k = 0; k < 3; ++k)
                {
                    if (k != i)
                    {
                        cc = 0;
                        for (uint l = 0; l < 3; ++l)
                        {
                            if (l != j)
                            {
                                sm(rr, cc, (*this)(k, l));
                                ++cc;
                            }
                        }
                        ++rr;
                    }
                }
                scalar d = sm.Determinant();
                scalar c = 1;
                if ((i + j) & (0x01))
                {
                    c = -1;
                }
                d = c * d;
                return(d);
            }

            Mtx3& Mtx3::Transpose()
            {
                scalar fNew[9];
                for (unsigned int i = 0; i < 8; i++)
                {
                    fNew[i] = this->_cmpx[(3 * i) % 8];
                }
                fNew[8] = this->_cmpx[8];
                memcpy(this->_cmpx, fNew, 9 * sizeof(scalar));
                return (*this);
            }

            scalar Mtx3::Determinant() const
            {
                scalar d = (*this)(0, 0) * this->Coefficient(0, 0) +
                    (*this)(0, 1) * this->Coefficient(0, 1) +
                    (*this)(0, 2) * this->Coefficient(0, 2);
                return(d);
            }

            Mtx3 Mtx3::Inverse() const
            {
                scalar det = this->Determinant();
                if (det != 0)
                {
                    Mtx3 inv;
                    for (unsigned int i = 0; i < 3; i++)
                    {
                        for (unsigned int j = 0; j < 3; j++)
                        {
                            inv(i, j, this->Coefficient(i, j));
                        }
                    }
                    inv.Transpose();
                    return((1 / det) * inv);
                }
                else
                {
                    throw std::runtime_error("Matrix is not invertible");
                }
            }

            Mtx3 Mtx3::Transposed() const
            {
                Mtx3 copy = (*this);
                copy.Transpose();
                return(copy);
            }

            void Mtx3::Stream(double* str) const
            {
                for (int i = 0; i < 9; ++i)
                {
                    *(str + i) = (double)this->_cmpx[i];
                }
            }

            Mtx3::operator Mtx4()
            {
                Mtx4 r;
                for (unsigned int i = 0; i < 3; ++i)
                {
                    for (unsigned int j = 0; j < 3; ++j)
                    {
                        r(i, j, (*this)(i, j));
                    }
                }
                r(3, 3, 1.0f);
                return(r);
            }

            Mtx3 Mtx3::Rotation(unsigned int axis, scalar angle)
            {
                axis = axis % 3;
                scalar cc = cos(angle);
                scalar ss = sin(angle);
                Mtx3 r;
                if (axis == eX)
                {
                    r(0, 0, 1);
                    r(1, 1, cc);
                    r(1, 2, -ss);
                    r(2, 1, ss);
                    r(2, 2, cc);
                }
                else if (axis == eY)
                {
                    r(0, 0, cc);
                    r(0, 2, ss);
                    r(1, 1, 1);
                    r(2, 0, -ss);
                    r(2, 2, cc);
                }
                else
                {
                    r(0, 0, cc);
                    r(0, 1, -ss);
                    r(1, 0, ss);
                    r(1, 1, cc);
                    r(2, 2, 1);
                }
                return(r);
            }


            Vec3 Mtx3::Solve(const Vec3& c) const
            {
                scalar d = this->Determinant();
                Vec3 r;
                if (d == 0.0f)
                {
                    throw std::runtime_error("System is not solvable");
                }
                else
                {
                    Mtx3 mx = (*this);
                    Mtx3 my = (*this);
                    Mtx3 mz = (*this);
                    mx.Column(eX, c);
                    my.Column(eY, c);
                    mz.Column(eZ, c);
                    r(eX, mx.Determinant() / d)(eY, my.Determinant() / d)(eZ, mz.Determinant() / d);
                }
                return(r);
            }
        }
    }
}




