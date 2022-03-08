#include <cmath>
#include "trackball.h"
#include "math_la/math_lac/space/mtx4.h"
#include "math_la/math_lac/space/vec4.h"

namespace gl
{

    using math_la::math_lac::space::Mtx4;
    using math_la::math_lac::space::Vec4;



    TrackBall::TrackBall(unsigned int width, unsigned int height)
    {
        this->_width = (float)width;
        this->_height = (float)height;
	    this->_radius = .5f;
	    this->_rotationMatrix(0,0,1);
        this->_rotationMatrix(1,1,1);
        this->_rotationMatrix(2,2,1);
        this->_basisChangeMatrix(0,0,1);
        this->_basisChangeMatrix(1,1,1);
        this->_basisChangeMatrix(2,2,1);
	    for (unsigned int i = 0; i < 3; ++i)
	    {
		    for (unsigned int j = 0; j < 3; ++j)
		    {
			    this->_transformationMatrixGL(i, j, this->_rotationMatrix(i, j));
		    }
	    }
	    this->_transformationMatrixGL(3, 3, 1);
    }

    void TrackBall::Set_Size(unsigned int width, unsigned int height)
    {
	    this->_width = (float)width;
	    this->_height = (float)height;
    }

    void TrackBall::Set_Start_Position(int X, int Y)
    {
        scalar u = (scalar)X;
        scalar v = (scalar)Y;
        u = (u-this->_width/2)/(this->_width);
        v = -(v-this->_height/2)/(this->_height);
        scalar z = this->_radius - u*u - v*v;
        if (z < 0)
        {
            z = 0;
        }
        z = sqrt(z);

        this->_mouse3DPos(eX,u);
        this->_mouse3DPos(eY,v);
        this->_mouse3DPos(eZ,z);
    }


    void TrackBall::Set_Position(int X, int Y)
    {
        scalar u = (scalar)X;
        scalar v = (scalar)Y;
        u = (u-this->_width/2.0f)/(this->_width);
        v = -(v-this->_height/2.0f)/(this->_height);

        scalar z = this->_radius - u*u - v*v;
        if (z < 0)
        {
            z = 0;
        }
        z = sqrt(z);
        Vec3 np;
        np(eX,u);
        np(eY,v);
        np(eZ,z);

	
	    Vec3 bp = this->_rotationMatrix*this->_mouse3DPos;
	    this->_mouse3DPos = np;
        np = this->_rotationMatrix*np;
	    np.Normalize();
	    bp.Normalize();
	    scalar cs = Vec3::Dot(np, bp);
	    if (cs > 1)
	    {
		    cs = 1;
	    }
	    scalar angle = acos(cs);
	    scalar ss = sin(angle);
	    cs = cos(angle);
	    ss = sin(angle);

	    Mtx3 rot;
	    rot(0, 0, 1);
	    rot(1, 1, cs);
	    rot(1, 2, ss);
	    rot(2, 1, -ss);
	    rot(2, 2, cs);

        if (!np.Is_Zero_Vector())
        {
            np.Normalize();

            Vec3 ex;
		    ex = Vec3::Cross(np, bp);

            if (!ex.Is_Zero_Vector())
            {
                ex.Normalize();

                Vec3 ez = np;
                if (!ez.Is_Zero_Vector())
                {
                    ez.Normalize();
                    Vec3 ey = Vec3::Cross(ez,ex);
                    if (!ey.Is_Zero_Vector())
                    {
                        ey.Normalize();
                        this->_basisChangeMatrix.Column(0,ex);
                        this->_basisChangeMatrix.Column(1,ey);
                        this->_basisChangeMatrix.Column(2,ez);
                        Mtx3 m = this->_basisChangeMatrix*rot*this->_basisChangeMatrix.Transposed();
					    this->_rotationMatrix = m*this->_rotationMatrix;
					    for (unsigned int i = 0; i < 3; ++i)
					    {
						    for (unsigned int j = 0; j < 3; ++j)
						    {
							    this->_transformationMatrixGL(i, j, this->_rotationMatrix(i,j));
						    }
					    }
					    this->_transformationMatrixGL(3, 3, 1);
                    }
                }
            }
        }
    }

    Mtx3 TrackBall::Rotation_Matrix() const
    {
	    Mtx3 r;
	    for (unsigned int i = 0; i < 3; ++i)
	    {
		    for (unsigned int j = 0; j < 3; ++j)
		    {
			    r(i, j, this->_transformationMatrixGL(i, j));
		    }
	    }
	    return(r);
    }

    Mtx4 TrackBall::Affine_Matrix() const
    {
	    return(this->_transformationMatrixGL);
    }

    void TrackBall::Set_Radius(scalar radius)
    {
        this->_radius = (float)radius;
    }

}