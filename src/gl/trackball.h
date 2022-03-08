#ifndef TRACKBALL_H
#define TRACKBALL_H

#include "math_la/math_lac/space/mtx3.h"
#include "math_la/math_lac/space/mtx4.h"
#include "math_la/math_lac/space/vec3.h"

namespace gl
{

	using math_la::math_lac::space::Mtx3;
	using math_la::math_lac::space::Vec3;
	using math_la::math_lac::space::Mtx4;


	/**
	* A TrackBall allows the user to intuitively manipulate OpenGL 3D objects with Mouse movements.
	* The projected position of the mouse is transformed to a rotation matrix that can be applied in the OpenGL rendering pipeline
	*/
	class TrackBall
	{
	private:
		/**
		* The width of the window to which the 3D object is being rendered. Determines Mouse horizontal precision
		*/
		float _width;

		/**
		* The height of the window to which the 3D object is being rendered. Determines Mouse vertical precision
		*/
		float _height;

		/**
		* The distance to which the object is being rendered. Determines the depth precision of the trackball
		*/
		float _radius;

		/**
		* Current 3D position of the mouse
		*/
		Vec3 _mouse3DPos;

		/**
		* When the user moves the 3D objects, the 3D orthogonal axis: X,Y and Z are moved. Every new rotation must be applied according
		* to this new basis, and this matrix stores the axis change of basis to which the rotation takes effect. It is calculated
		* with each mouse movement. 
		*/
		Mtx3 _basisChangeMatrix;

		/**
		* The rotation mattrix that is applied to the 3D object, in orthoghonal coordinates
		*/
		Mtx3 _rotationMatrix;
	
		/**
		* The OpenGL affine transformation that is applied to the object
		*/
		Mtx4 _transformationMatrixGL;
	public:
		TrackBall(unsigned int width = 0, unsigned int height = 0);

		/**
		* Defines the window size of the trackball. It is generally updated when the window is resized
		*/
		void Set_Size(unsigned int width, unsigned int height);

		/**
		* Defines the TrackBall start position for reference. This method is usually called when the user clicks on the
		* trackball window
		*/
		void Set_Start_Position(int X, int Y);

		/**
		* Defines the distance to whihc the trackball is being moved. Smaller values provide a more precise TrackBall. 
		*/
		void Set_Radius(scalar radius);

		/**
		* Defines the current position of the trackball, typically when the mouse is moving and the trackball is active
		* (for example, when the button mouse is clicked)
		*/
		void Set_Position(int X, int Y);

		/**
		* The affine transformation that is applied to OpenGL rendering pipeline
		*/
		Mtx4 Affine_Matrix() const;

		/**
		* The rotation matrix that is applied to the 3D object
		*/
		Mtx3 Rotation_Matrix() const;
	};

}

#endif 
