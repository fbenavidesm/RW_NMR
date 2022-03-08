#include <stdexcept>
#include <cmath>
#include "vec2.h"


using std::sqrt;

namespace math_la
{
    namespace math_lac
    {

        namespace space
        {


            Vec2 Vec2::operator + (const Vec2& v) const
            {
                return(Vec2(v(eX) + (*this)(eX),
                    v(eY) + (*this)(eY)));
            }


            Vec2 Vec2::operator - (const Vec2& v) const
            {
                return(Vec2((*this)(eX) - v(eX),
                    (*this)(eY) - v(eY)));
            }

            Vec2 Vec2::operator * (scalar c) const
            {
                return(Vec2(c * (*this)(eX),
                    c * (*this)(eY)));
            }

            Vec2 operator * (scalar c, const Vec2& v)
            {
                return(Vec2(c * v(eX),
                    c * v(eY)));
            }

            bool Vec2::operator == (const Vec2& v) const
            {
                scalar r = 0;
                scalar stabilizer = (*this)(2) + v(2);
                if (stabilizer < 1)
                {
                    stabilizer = 1.0f;
                }
                for (unsigned int i = 0; i < 2; i++)
                {
                    scalar Voxel_Length = ((*this)(i) - v(i)) / stabilizer;
                    r = r + Voxel_Length * Voxel_Length;
                }
                return(r < EPSILON);
            }

            bool Vec2::operator != (const Vec2& v) const
            {
                return(!((*this) == v));
            }

            Vec2& Vec2::operator += (const Vec2& v)
            {
                for (unsigned int i = 0; i < 2; i++)
                {
                    (*this)(i, (*this)(i) + v(i));
                }
                return(*this);
            }

            Vec2& Vec2::operator -= (const Vec2& v)
            {
                for (unsigned int i = 0; i < 2; i++)
                {
                    (*this)(i, (*this)(i) - v(i));
                }
                return(*this);
            }

            Vec2& Vec2::operator *= (scalar c)
            {
                for (unsigned int i = 0; i < 2; i++)
                {
                    (*this)(i, c * (*this)(i));
                }
                return(*this);
            }

            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------
            //---------------------------------------------------------------------------


            Vec2::Vec2()
            {
                for (unsigned int i = 0; i < 3; i++)
                {
                    this->_cmpx[i] = 0.0f;
                }
            }


            Vec2::Vec2(scalar x, scalar y)
            {
                this->_cmpx[0] = x;
                this->_cmpx[1] = y;
                this->_cmpx[2] = x * x + y * y;
            }

            Vec2& Vec2::operator =(const Vec2& vct)
            {
                for (unsigned int i = 0; i < 3; ++i)
                {
                    this->_cmpx[i] = vct._cmpx[i];
                }
                return(*this);
            }

            Vec2::Vec2(const Vec2& vct)
            {
                for (unsigned int i = 0; i < 3; i++)
                {
                    this->_cmpx[i] = vct._cmpx[i];
                }
            }

            Vec2& Vec2::operator()(uint pos, scalar value)
            {
                pos = pos % 2;
                if (!(fabs(this->_cmpx[pos]) < EPSILON))
                {
                    this->_cmpx[2] = this->_cmpx[2] - this->_cmpx[pos] * this->_cmpx[pos];
                }
                this->_cmpx[pos] = value;
                this->_cmpx[2] = this->_cmpx[2] + this->_cmpx[pos] * this->_cmpx[pos];
                return(*this);
            }

            scalar Vec2::operator()(uint pos) const
            {
                pos = pos % 2;
                return(this->_cmpx[pos]);
            }

            scalar Vec2::Sq_Norm() const
            {
                return(this->_cmpx[2]);
            }

            scalar Vec2::Norm() const
            {
                return(sqrt(this->_cmpx[2]));
            }

            void Vec2::Normalize()
            {
                if (!this->Is_Zero_Vector())
                {
                    scalar r = this->Norm();
                    for (unsigned int i = 0; i < 2; i++)
                    {
                        this->_cmpx[i] = this->_cmpx[i] / r;
                    }
                    this->_cmpx[2] = 1.0;
                }
                else
                {
                    throw std::runtime_error("Vector entries are 0's");
                }
            }

            bool Vec2::Is_Zero_Vector() const
            {
                return(this->_cmpx[2] == 0);
            }

            scalar Vec2::Dot(const Vec2& vct1, const Vec2& vct2)
            {
                scalar r = 0;
                for (unsigned int i = 0; i < 2; i++)
                {
                    r = r + vct1._cmpx[i] * vct2._cmpx[i];
                }
                return(r);
            }

            Vec2 Vec2::Ex()
            {
                return(Vec2(1, 0));
            }

            Vec2 Vec2::Ey()
            {
                return(Vec2(0, 1));
            }

            Vec2 Vec2::Project(const Vec2& base) const
            {
                if (!base.Is_Zero_Vector())
                {
                    return(((Vec2::Dot((*this), base)) / (base.Sq_Norm())) * base);
                }
                else
                {
                    return(base);
                }
            }
        }
    }

}
