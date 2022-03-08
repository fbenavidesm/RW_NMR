#include <math.h>
#include <stdexcept>
#include "vec4.h"
#include "vec3.h"

namespace math_la
{
    namespace math_lac
    {

        namespace space
        {


            Vec4 Vec4::operator + (const Vec4& v) const
            {
                return(Vec4(v(eX) + (*this)(eX),
                    v(eY) + (*this)(eY),
                    v(eZ) + (*this)(eZ),
                    v(eW) + (*this)(eW)));
            }

            Vec4 Vec4::operator - (const Vec4& v) const
            {
                return(Vec4((*this)(eX) - v(eX),
                    (*this)(eY) - v(eY),
                    (*this)(eZ) - v(eZ),
                    (*this)(eW) - v(eW)));
            }

            Vec4 Vec4::operator * (scalar c) const
            {
                return(Vec4(c * (*this)(eX),
                    c * (*this)(eY),
                    c * (*this)(eZ),
                    c * (*this)(eW)));
            }

            Vec4 operator * (scalar c, const Vec4& v)
            {
                return(Vec4(c * v(eX),
                    c * v(eY),
                    c * v(eZ),
                    c * v(eW)));
            }

            bool Vec4::operator == (const Vec4& v) const
            {
                scalar r = 0;
                scalar stabilizer = (*this)(4) + v(4);
                if (stabilizer < 1)
                {
                    stabilizer = 1.0f;
                }
                for (unsigned int i = 0; i < 4; i++)
                {
                    scalar Voxel_Length = ((*this)(i) - v(i)) / stabilizer;
                    r = r + Voxel_Length * Voxel_Length;
                }
                return(r < EPSILON);
            }

            bool operator != (const Vec4& vct1, const Vec4& vct2)
            {
                return(!(vct1 == vct2));
            }

            Vec4& operator += (Vec4& vct1, const Vec4& vct2)
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    vct1(i, vct1(i) + vct2(i));
                }
                return(vct1);
            }

            Vec4& operator -= (Vec4& vct1, const Vec4& vct2)
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    vct1(i, vct1(i) - vct2(i));
                }
                return(vct1);
            }

            Vec4& operator *= (Vec4& vct, scalar c)
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    vct(i, c * vct(i));
                }
                return(vct);
            }

            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------

            Vec4::Vec4()
            {
                for (unsigned int i = 0; i < 5; i++)
                {
                    this->_cmpx[i] = 0.0f;
                }
            }


            Vec4::Vec4(scalar x, scalar y, scalar z, scalar w)
            {
                this->_cmpx[0] = x;
                this->_cmpx[1] = y;
                this->_cmpx[2] = z;
                this->_cmpx[3] = w;
                this->_cmpx[4] = x * x + y * y + z * z + w * w;
            }

            Vec4::Vec4(const Vec4& vct)
            {
                for (unsigned int i = 0; i < 5; i++)
                {
                    this->_cmpx[i] = vct._cmpx[i];
                }
            }

            Vec4& Vec4::operator()(uint pos, scalar value)
            {
                pos = pos % 4;
                if (!(fabs(this->_cmpx[pos])))
                {
                    this->_cmpx[4] = this->_cmpx[4] - this->_cmpx[pos] * this->_cmpx[pos];
                }
                this->_cmpx[pos] = value;
                this->_cmpx[4] = this->_cmpx[4] + this->_cmpx[pos] * this->_cmpx[pos];
                return(*this);
            }

            scalar Vec4::operator()(uint pos) const
            {
                pos = pos % 4;
                return(this->_cmpx[pos]);
            }

            scalar Vec4::Sq_Norm() const
            {
                return(this->_cmpx[4]);
            }

            scalar Vec4::Norm() const
            {
                return(sqrt(this->_cmpx[4]));
            }

            Vec4& Vec4::operator = (const Vec4& vct)
            {
                for (unsigned int i = 0; i < 5; i++)
                {
                    this->_cmpx[i] = vct._cmpx[i];
                }
                return(*this);
            }

            void Vec4::Normalize()
            {
                if (!this->Is_Zero_Vector())
                {
                    scalar r = this->Norm();
                    for (unsigned int i = 0; i < 4; i++)
                    {
                        this->_cmpx[i] = this->_cmpx[i] / r;
                    }
                    this->_cmpx[4] = 1.0;
                }
                else
                {
                    throw std::runtime_error("Vector entries are 0's");
                }
            }

            bool Vec4::Is_Zero_Vector() const
            {
                return(this->_cmpx[4] == 0);
            }

            scalar Vec4::Dot(const Vec4& vct1, const Vec4& vct2)
            {
                scalar r = 0;
                for (unsigned int i = 0; i < 4; i++)
                {
                    r = r + vct1._cmpx[i] * vct2._cmpx[i];
                }
                return(r);
            }

            Vec4 Vec4::Ex()
            {
                return(Vec4(1, 0, 0, 0));
            }

            Vec4 Vec4::Ey()
            {
                return(Vec4(0, 1, 0, 0));
            }

            Vec4 Vec4::Ez()
            {
                return(Vec4(0, 0, 1, 0));
            }

            Vec4 Vec4::Ew()
            {
                return(Vec4(0, 0, 0, 1));
            }


            Vec4 Vec4::Project(const Vec4& base) const
            {
                if (!base.Is_Zero_Vector())
                {
                    return(((Vec4::Dot((*this), base)) / (base.Sq_Norm())) * base);
                }
                else
                {
                    return(base);
                }
            }
        }
    }
}
