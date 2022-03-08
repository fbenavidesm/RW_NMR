#ifndef BINARY_IMAGE_BORDER_CREATOR
#define BINARY_IMAGE_BORDER_CREATOR

#include "math_la/mdefs.h"
#include "binary_image_executor.h"
#include "binary_image.h"

namespace rw
{
	class BinaryImageBorderCreator : public BinaryImageExecutor
	{
	private:
		/*
		*Set of border voxels in the solid phase
		*/
		vec(int)* _cornerBorder;
		vec(int)* _surfaceBorder;

		rw::Pos3i _size;
	public:
		BinaryImageBorderCreator(BinaryImageExecutor& executor);
		BinaryImageBorderCreator(BinaryImageBorderCreator& executor);
		~BinaryImageBorderCreator();
		const vec(int)& Corner_Border() const;
		const vec(int)& Surface_Border() const;
		void Execute(BinaryImage::ProgressAdapter* pgdlg = 0);
	};

	inline const vec(int)& BinaryImageBorderCreator::Corner_Border() const
	{
		return(*this->_cornerBorder);
	}

	inline const vec(int)& BinaryImageBorderCreator::Surface_Border() const
	{
		return(*this->_surfaceBorder);
	}

}


#endif
