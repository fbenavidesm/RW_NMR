#ifndef BINARY_IMAGE_OPENER_H
#define BINARY_IMAGE_OPENER_H

#include <amp.h>
#include "math_la/mdefs.h"
#include "binary_image_border_creator.h"
#include "binary_image.h"
#include "binary_image_executor.h"
#include "binary_image_mask_handler.h"

namespace rw
{
	class BinaryImageOpener : public BinaryImageBorderCreator, public BinaryImageMaskHandler
	{
	private:
		int _step;
		int _diameter;
		uint _blockSize;

		/**
		* Set of centers to erode after dilation. This is the set of pore voxels that remained after a dilation
		*/

		set<int> _centersToErodeCorner;

		set<int> _centersToErodeSurface;


		/**
		* Texture array to load the binary image to the GPU
		*/
		concurrency::array<uint, 1>* _gpuArray;
	protected:
		void Open(int rad, BinaryImage::ProgressAdapter* pgdlg);
		void Set_DiamMin_To_2Rad(int _2rad);
		void Dilate(int diam);
		void Update_And_Erode_Pore_Map(int diam);
		void Centers_To_Erode_Surface(vec(int)& centers);
		void Centers_To_Erode_Corner(vec(int)& centers);
	public:
		BinaryImageOpener(BinaryImageBorderCreator& executor);
		~BinaryImageOpener();
		void Set_Diameter(int diam);
		void Execute(BinaryImage::ProgressAdapter* pgdlg = 0);

		/**
		* Solidifies the texture at the position pp.
		* @param vtx GPU Texture
		* @param pp Position to solidify
		* @param size Three dimension size of the texture
		*/
		static uint Accesor_Write_One(concurrency::array<uint, 1>& vtx, const rw::Pos3i& pp, const rw::Pos3i& size) GPUP;

		/**
		* Amplifies the pore at the position pp.
		* @param vtx GPU Texture
		* @param pp Position to amplify
		* @param size Three dimension size of the texture
		*/
		static uint Accesor_Write_Null(concurrency::array<uint, 1>& vtx, const rw::Pos3i& pp, const rw::Pos3i& size) GPUP;

		bool Black_Voxels_Covered() const;

		void Release_GPU();
		void Load_GPU();

		uint Number_Of_Steps() const;
	};

	inline uint BinaryImageOpener::Number_Of_Steps() const
	{
		return(max(50,max(this->Size_3D().x,max(this->Size_3D().y,this->Size_3D().z))/8));
	}

	inline uint BinaryImageOpener::Accesor_Write_One(concurrency::array<uint, 1>& vtx, const rw::Pos3i& pp, const rw::Pos3i& size) GPUP
	{
		uint b = (pp.x >> 2) + (pp.y >> 2)*((size.x >> 2) + 1)
			+ (pp.z >> 2)*((size.x >> 2) + 1)*((size.y >> 2) + 1);
		uint pz = pp.z - ((pp.z >> 2) << 2);
		concurrency::atomic_fetch_or(&vtx[(b<<1) + (pz >> 1)], (0x01 << (pp.x - ((pp.x >> 2) << 2) +
			((pp.y - ((pp.y >> 2) << 2)) << 2) + ((pz & 0x01) << 4))));
		return(1);
	}

	inline uint BinaryImageOpener::Accesor_Write_Null(concurrency::array<uint, 1>& vtx, const rw::Pos3i& pp, const rw::Pos3i& size) GPUP
	{
		uint b = (pp.x >> 2) + (pp.y >> 2)*((size.x >> 2) + 1)
			+ (pp.z >> 2)*((size.x >> 2) + 1)*((size.y >> 2) + 1);
		uint pz = pp.z - ((pp.z >> 2) << 2);
		concurrency::atomic_fetch_and(&vtx[(b<<1) + (pz >> 1)], (~(0x01 << (pp.x - ((pp.x >> 2) << 2) +
			((pp.y - ((pp.y >> 2) << 2)) << 2) + ((pz & 0x01) << 4)))));
		return(1);
	}
}


#endif

