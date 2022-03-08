#ifndef BINARY_IMAGE_DENOISER_H
#define BINARY_IMAGE_DENOISER_H

#include <amp.h>
#include "binary_image_executor.h"
#include "binary_image_mask_handler.h"

namespace rw
{

	class BinaryImageDenoiser : public BinaryImageExecutor, public BinaryImageMaskHandler
	{
	private:
		int _diameter;
		uint _blockSize;
		vec(int) _centersToErodeCorner;
		vec(int) _centersToErodeSurface;
		vec(int) _cornerBorder;
		vec(int) _surfaceBorder;
		concurrency::array<uint, 1>* _gpuArray;

		void Pick_Centers_To_Erode();
		void Pick_Centers_To_Dilate();
		void Erode();
		void Dilate();
		void Load();
	public:
		BinaryImageDenoiser(BinaryImageExecutor& executor);
		~BinaryImageDenoiser();
		void Set_Diameter(int diam);
		void Execute(BinaryImage::ProgressAdapter* pgdlg = 0);
		uint Number_Of_Steps() const;
	};

	inline uint BinaryImageDenoiser::Number_Of_Steps() const
	{
		return(5);
	}
}


#endif

