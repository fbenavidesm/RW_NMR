#include <math.h>
#include <stdexcept>
#include "vec3.h"
#include "vec4.h"

namespace math_la
{
    namespace math_lac
    {

        namespace space
        {


            Vec3 Vec3::operator + (const Vec3& v) const
            {
                return(Vec3((*this)(eX) + v(eX),
                    (*this)(eY) + v(eY),
                    (*this)(eZ) + v(eZ)));
            }


            Vec3 Vec3::operator - (const Vec3& v) const
            {
                return(Vec3((*this)(eX) - v(eX),
                    (*this)(eY) - v(eY),
                    (*this)(eZ) - v(eZ)));
            }

            Vec3 Vec3::operator * (scalar c) const
            {
                return(Vec3(c * (*this)(eX),
                    c * (*this)(eY),
                    c * (*this)(eZ)));
            }

            Vec3 operator * (scalar c, const Vec3& v)
            {
                return(Vec3(c * v(eX),
                    c * v(eY),
                    c * v(eZ)));
            }

            bool Vec3::operator == (const Vec3& v) const
            {
                scalar r = 0;
                scalar stabilizer = v(3) + (*this)(3);
                if (stabilizer < 1)
                {
                    stabilizer = 1.0f;
                }
                for (unsigned int i = 0; i < 3; i++)
                {
                    scalar Voxel_Length = (v(i) - (*this)(i)) / stabilizer;
                    r = r + Voxel_Length * Voxel_Length;
                }
                return(r < EPSILON);
            }

            bool Vec3::operator != (const Vec3& v) const
            {
                return(!((*this) == v));
            }

            Vec3& Vec3::operator += (const Vec3& v)
            {
                for (unsigned int i = 0; i < 3; i++)
                {
                    (*this)(i, (*this)(i) + v(i));
                }
                return(*this);
            }

            Vec3& Vec3::operator -= (const Vec3& v)
            {
                for (unsigned int i = 0; i < 3; i++)
                {
                    (*this)(i, (*this)(i) - v(i));
                }
                return(*this);
            }

            Vec3& Vec3::operator *= (scalar c)
            {
                for (unsigned int i = 0; i < 3; i++)
                {
                    (*this)(i, c * (*this)(i));
                }
                return(*this);
            }

            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------


            Vec3::Vec3()
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    this->_cmpx[i] = 0.0f;
                }
            }

            Vec3::Vec3(scalar x, scalar y, scalar z)
            {
                this->_cmpx[0] = x;
                this->_cmpx[1] = y;
                this->_cmpx[2] = z;
                this->_cmpx[3] = x * x + y * y + z * z;
            }

            Vec3& Vec3::operator =(const Vec3& vct)
            {
                for (unsigned int i = 0; i < 4; ++i)
                {
                    this->_cmpx[i] = vct._cmpx[i];
                }
                return(*this);
            }

            Vec3::Vec3(const Vec3& vct)
            {
                for (unsigned int i = 0; i < 4; i++)
                {
                    this->_cmpx[i] = vct._cmpx[i];
                }
            }

            Vec3& Vec3::operator()(uint pos, scalar value)
            {
                pos = pos % 3;
                if (!(fabs(this->_cmpx[pos]) < EPSILON))
                {
                    this->_cmpx[3] = this->_cmpx[3] - this->_cmpx[pos] * this->_cmpx[pos];
                }
                this->_cmpx[pos] = value;
                this->_cmpx[3] = this->_cmpx[3] + this->_cmpx[pos] * this->_cmpx[pos];
                return(*this);
            }

            scalar Vec3::operator()(uint pos) const
            {
                pos = pos % 3;
                return(this->_cmpx[pos]);
            }

            scalar Vec3::Sq_Norm() const
            {
                return(this->_cmpx[3]);
            }

            scalar Vec3::Norm() const
            {
                return((scalar)sqrt(this->_cmpx[3]));
            }

            void Vec3::Normalize()
            {
                if (!this->Is_Zero_Vector())
                {
                    scalar r = this->Norm();
                    for (unsigned int i = 0; i < 3; i++)
                    {
                        this->_cmpx[i] = this->_cmpx[i] / r;
                    }
                    this->_cmpx[3] = 1.0;
                }
                else
                {
                    throw std::runtime_error("Vector entries are 0's");
                }
            }

            bool Vec3::Is_Zero_Vector() const
            {
                return(this->_cmpx[3] == 0);
            }

            scalar Vec3::Dot(const Vec3& vct1, const Vec3& vct2)
            {
                scalar r = 0;
                for (unsigned int i = 0; i < 3; i++)
                {
                    r = r + vct1._cmpx[i] * vct2._cmpx[i];
                }
                return(r);
            }

            Vec3 Vec3::Cross(const Vec3& vct1, const Vec3& vct2)
            {
                scalar x = 0;
                scalar y = 0;
                scalar z = 0;
                crossR3(vct1(eX), vct1(eY), vct1(eZ), vct2(eX), vct2(eY), vct2(eZ), x, y, z);
                return(Vec3(x, y, z));
            }

            Vec3 Vec3::Ex()
            {
                return(Vec3(1, 0, 0));
            }

            Vec3 Vec3::Ey()
            {
                return(Vec3(0, 1, 0));
            }

            Vec3 Vec3::Ez()
            {
                return(Vec3(0, 0, 1));
            }

            Vec3 Vec3::Project(const Vec3& base) const
            {
                if (!base.Is_Zero_Vector())
                {
                    return(((Vec3::Dot((*this), base)) / (base.Sq_Norm())) * base);
                }
                else
                {
                    return(base);
                }
            }
        }
    }
}
