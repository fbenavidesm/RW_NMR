#include <algorithm>
#include <stdexcept>
#include <string.h>
#include "mtx4.h"
#include "mtx3.h"

namespace math_la
{
    namespace math_lac
    {

        namespace space
        {

            using std::size;

            Mtx4 Mtx4::operator + (const Mtx4& m) const
            {
                Mtx4 r;
                for (unsigned int i = 0; i < 4; i++)
                {
                    for (unsigned int j = 0; j < 4; j++)
                    {
                        r(i, j, (*this)(i, j) + m(i, j));
                    }
                }
                return(r);
            }

            Mtx4 Mtx4::operator - (const Mtx4& m) const
            {
                Mtx4 r;
                for (unsigned int i = 0; i < 4; i++)
                {
                    for (unsigned int j = 0; j < 4; j++)
                    {
                        r(i, j, (*this)(i, j) - m(i, j));
                    }
                }
                return(r);
            }

            Mtx4 Mtx4::operator * (scalar c) const
            {
                Mtx4 r;
                for (unsigned int i = 0; i < 4; i++)
                {
                    for (unsigned int j = 0; j < 4; j++)
                    {
                        r(i, j, c * (*this)(i, j));
                    }
                }
                return(r);
            }

            Mtx4 operator * (scalar c, const Mtx4& m)
            {
                Mtx4 r;
                for (unsigned int i = 0; i < 4; i++)
                {
                    for (unsigned int j = 0; j < 4; j++)
                    {
                        r(i, j, c * m(i, j));
                    }
                }
                return(r);
            }


            bool Mtx4::operator == (const Mtx4& m) const
            {
                scalar r = 0;
                for (unsigned int i = 0; i < 4; i++)
                {
                    for (unsigned int j = 0; j < 4; j++)
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

            Mtx4& Mtx4::operator += (const Mtx4& m)
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    for (unsigned int j = 0; j < 4; j++)
                    {
                        (*this)(i, j, (*this)(i, j) + m(i, j));
                    }
                }
                return(*this);
            }

            Mtx4& Mtx4::operator -= (const Mtx4& m)
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    for (unsigned int j = 0; j < 4; j++)
                    {
                        (*this)(i, j, (*this)(i, j) - m(i, j));
                    }
                }
                return(*this);
            }

            Mtx4& Mtx4::operator *= (scalar c)
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    for (unsigned int j = 0; j < 4; j++)
                    {
                        (*this)(i, j, c * (*this)(i, j));
                    }
                }
                return(*this);
            }

            Vec4 Mtx4::operator * (const Vec4& v) const
            {
                Vec4 rv;
                for (uint i = 0; i < 4; ++i)
                {
                    scalar r = 0.0f;
                    for (uint j = 0; j < 4; ++j)
                    {
                        r = r + v(j) * (*this)(i, j);
                    }
                    rv(i, r);
                }
                return(rv);
            }

            Mtx4 Mtx4::operator * (const Mtx4& m) const
            {
                Mtx4 rm;
                for (uint i = 0; i < 4; ++i)
                {
                    for (uint j = 0; j < 4; ++j)
                    {
                        for (uint k = 0; k < 4; ++k)
                        {
                            rm(i, j, rm(i, j) + (*this)(i, k) * m(k, j));
                        }
                    }
                }
                return(rm);
            }


            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------

            Mtx4::Mtx4()
            {
                for (unsigned int i = 0; i < 16; i++)
                {
                    this->_cmpx[i] = 0.0f;
                }
            }

            Mtx4::Mtx4(const Mtx4& m)
            {
                for (unsigned int i = 0; i < 16; i++)
                {
                    this->_cmpx[i] = m._cmpx[i];
                }
            }

            Mtx4& Mtx4::operator = (const Mtx4& m)
            {
                for (unsigned int i = 0; i < 16; i++)
                {
                    this->_cmpx[i] = m._cmpx[i];
                }
                return(*this);
            }


            Mtx4& Mtx4::operator()(uint i, uint j, scalar value)
            {
                i = i % 4;
                j = j % 4;
                this->_cmpx[4 * i + j] = value;
                return(*this);
            }

            Mtx4::Mtx4(const Vec4& row1,
                const Vec4& row2,
                const Vec4& row3,
                const Vec4& row4)
            {
                this->_cmpx[0] = row1(eX);
                this->_cmpx[1] = row1(eY);
                this->_cmpx[2] = row1(eZ);
                this->_cmpx[3] = row1(eW);

                this->_cmpx[4] = row2(eX);
                this->_cmpx[5] = row2(eY);
                this->_cmpx[6] = row2(eZ);
                this->_cmpx[7] = row2(eW);

                this->_cmpx[8] = row3(eX);
                this->_cmpx[9] = row3(eY);
                this->_cmpx[10] = row3(eZ);
                this->_cmpx[11] = row3(eW);

                this->_cmpx[12] = row4(eX);
                this->_cmpx[13] = row4(eY);
                this->_cmpx[14] = row4(eZ);
                this->_cmpx[15] = row4(eW);
            }

            Mtx4 Mtx4::Identity()
            {
                Mtx4 id;
                id(0, 0, 1);
                id(1, 1, 1);
                id(2, 2, 1);
                id(3, 3, 1);
                return(id);
            }

            scalar Mtx4::operator()(uint i, uint j) const
            {
                return(this->_cmpx[4 * i + j]);
            }

            Vec4 Mtx4::Row(uint i) const
            {
                return(Vec4(
                    this->_cmpx[4 * i],
                    this->_cmpx[4 * i + 1],
                    this->_cmpx[4 * i + 2],
                    this->_cmpx[4 * i + 3])
                    );
            }

            Vec4 Mtx4::Column(uint i) const
            {
                return(Vec4(
                    this->_cmpx[i],
                    this->_cmpx[4 + i],
                    this->_cmpx[8 + i],
                    this->_cmpx[12 + i]
                ));
            }

            Mtx4& Mtx4::Row(uint i, const Vec4& row)
            {
                (*this)(i, 0, row(eX));
                (*this)(i, 1, row(eY));
                (*this)(i, 2, row(eZ));
                (*this)(i, 3, row(eW));
                return(*this);
            }

            Mtx4& Mtx4::Column(uint i, const Vec4& col)
            {
                (*this)(0, i, col(eX));
                (*this)(1, i, col(eY));
                (*this)(2, i, col(eZ));
                (*this)(3, i, col(eW));
                return(*this);
            }

            scalar Mtx4::Coefficient(unsigned int i, unsigned int j) const
            {
                Mtx3 m;
                i = i % 4;
                j = j % 4;
                unsigned int cr = 0;
                unsigned int cc = 0;
                scalar c = 1;
                for (uint k = 0; k < 4; k++)
                {
                    if (k != i)
                    {
                        cc = 0;
                        for (uint l = 0; l < 4; l++)
                        {
                            if (l != j)
                            {
                                m(cr, cc, (*this)(k, l));
                                ++cc;
                            }
                        }
                        ++cr;
                    }
                }
                if ((i + j) & 0x01)
                {
                    c = -1;
                }
                scalar d = c * m.Determinant();
                return(d);
            }

            Mtx4& Mtx4::Transpose()
            {
                scalar fNew[16];
                for (unsigned int i = 0; i < 15; i++)
                {
                    fNew[i] = this->_cmpx[(4 * i) % 15];
                }
                fNew[15] = this->_cmpx[15];
                memcpy(this->_cmpx, fNew, 16 * sizeof(scalar));
                return (*this);
            }

            scalar Mtx4::Determinant() const
            {
                scalar d = (*this)(0, 0) * this->Coefficient(0, 0) +
                    (*this)(0, 1) * this->Coefficient(0, 1) +
                    (*this)(0, 2) * this->Coefficient(0, 2) +
                    (*this)(0, 3) * this->Coefficient(0, 3);
                return(d);
            }

            Mtx4 Mtx4::Inverse() const
            {
                scalar det = this->Determinant();
                if (det != 0)
                {
                    Mtx4 inv;
                    for (unsigned int i = 0; i < 4; i++)
                    {
                        for (unsigned int j = 0; j < 4; j++)
                        {
                            inv(i, j, (*this).Coefficient(i, j));
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

            Mtx4 Mtx4::Transposed() const
            {
                Mtx4 copy(*this);
                copy.Transpose();
                return(copy);
            }

            void Mtx4::Stream(double* str) const
            {
                for (int i = 0; i < 16; ++i)
                {
                    *(str + i) = (double)this->_cmpx[i];
                }
            }

            void Mtx4::FStream(float* str) const
            {
                for (int i = 0; i < 16; ++i)
                {
                    float s = (float)this->_cmpx[i];
                    int si = (int)(s * 1000.0f);
                    s = (float)si / (1000.0f);
                    *(str + i) = s;
                }
            }

            Vec4 Mtx4::Solve(const Vec4& c) const
            {
                scalar d = this->Determinant();
                Vec4 r;
                if (d == 0.0f)
                {
                    throw std::runtime_error("System is not solvable");
                }
                else
                {
                    Mtx4 m0 = (*this);
                    Mtx4 m1 = (*this);
                    Mtx4 m2 = (*this);
                    Mtx4 m3 = (*this);
                    m0.Column(0, c);
                    m1.Column(1, c);
                    m2.Column(2, c);
                    m3.Column(3, c);
                    r(0, m0.Determinant() / d);
                    r(1, m1.Determinant() / d);
                    r(2, m2.Determinant() / d);
                    r(3, m3.Determinant() / d);
                }
                return(r);
            }
        }
    }

}



