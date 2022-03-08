#ifndef BINARY_IMAGE_GROUP_MASK_H
#define BINARY_IMAGE_GROUP_MASK_H

#include "binary_image_mask_handler.h"

namespace rw
{
	class BinaryImageGroupMask : public BinaryImageMaskHandler
	{
	protected:
		void Create_Mask(int rad, vec(rw::Pos3i)& em);
		void Create_Surface_Mask(int diam, vec(rw::Pos3i)& em);
	public:
		BinaryImageGroupMask();
	};
}

#endif
