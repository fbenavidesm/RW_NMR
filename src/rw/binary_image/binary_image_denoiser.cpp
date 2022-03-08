#include <tbb/spin_mutex.h>
#include <tbb/parallel_for.h>
#include <algorithm>
#include "binary_image_denoiser.h"
#include "binary_image_opener.h"

namespace rw
{

	const int BLOCK1 = 16384;
	const int BLOCK2 = 8192;
	const int BLOCK3 = 4096;

	BinaryImageDenoiser::BinaryImageDenoiser(BinaryImageExecutor& executor) : BinaryImageExecutor(executor), BinaryImageMaskHandler()
	{
		this->Set_Morphed(false);
		this->Set_Open(false);
		this->Init();
		this->_blockSize = BLOCK1;
		this->_gpuArray = 0;
	};

	void BinaryImageDenoiser::Load()
	{
		if (this->_gpuArray)
		{
			delete this->_gpuArray;
		}
		this->_gpuArray = new concurrency::array<uint, 1>((int)this->Image_Buffer().size(), this->Image_Buffer().begin());
	}

	BinaryImageDenoiser::~BinaryImageDenoiser()
	{
		concurrency::accelerator acc = concurrency::accelerator(concurrency::accelerator::default_accelerator);
		concurrency::accelerator_view aview = acc.get_default_view();
		aview.flush();
		if (this->_gpuArray)
		{
			delete this->_gpuArray;
		}
	}

	void BinaryImageDenoiser::Set_Diameter(int diam)
	{
		this->_diameter = diam;
		if (diam > 16)
		{
			this->_blockSize = BLOCK2;
		}
		if (diam > 32)
		{
			this->_blockSize = BLOCK3;
		}
	}

	void BinaryImageDenoiser::Pick_Centers_To_Dilate()
	{
		rw::Pos3i size3d = this->Size_3D();
		this->_cornerBorder.clear();
		this->_surfaceBorder.clear();
		int black_size = this->Image().Black_Voxels();
		this->_cornerBorder.reserve((int)black_size);
		this->_surfaceBorder.reserve((int)black_size);
		int tot = size3d.x*size3d.y*size3d.z;
		tbb::spin_mutex mtx;
		tbb::parallel_for(tbb::blocked_range<int>(0, tot, std::max(tot / 128, BCHUNK_SIZE)),
			[this, &mtx, size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				int idc = 0;
				Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, pp);
				uint v = this->Image()(pp);
				if (v > 0)
				{
					int pborder = 0;
					char surround[27];
					for (int dxy = 0; dxy < 27; ++dxy)
					{
						surround[dxy] = 1;
						int dz = (dxy / 9) - 1;
						int dy = (dxy / 3) % 3 - 1;
						int dx = dxy % 3 - 1;
						Pos3i ppc = pp;
						ppc.x = ppc.x + dx;
						ppc.y = ppc.y + dy;
						ppc.z = ppc.z + dz;
						if ((!((ppc.x < 0) || (ppc.x >= size3d.x)
							|| (ppc.y < 0) || (ppc.y >= size3d.y)
							|| (ppc.z < 0) || (ppc.z >= size3d.z))) && (this->Image()(ppc) == 0))
						{
							surround[dxy] = 0;
							++pborder;
						}
					}
					if (pborder > 0)
					{
						if (((surround[22] > 0) && (surround[4] > 0)) ||
							((surround[12] > 0) && (surround[14] > 0)) ||
							((surround[16] > 0) && (surround[10] > 0)))
						{
							mtx.lock();
							this->_surfaceBorder.push_back(i);
							mtx.unlock();
						}
						else
						{
							mtx.lock();
							this->_cornerBorder.push_back(i);
							mtx.unlock();
						}
					}
				}
			}
		});
		this->Set_Border(true);
	}

	void BinaryImageDenoiser::Pick_Centers_To_Erode()
	{
		rw::Pos3i size3d = this->Size_3D();
		int diam = this->_diameter;
		tbb::spin_mutex mtx;
		this->_centersToErodeCorner.clear();
		this->_centersToErodeSurface.clear();
		int tot = size3d.x*size3d.y*size3d.z;
		vec(uint)& dilated = this->Image_Buffer();
		tbb::parallel_for(tbb::blocked_range<int>(0, tot, std::max(tot / 128, BCHUNK_SIZE)),
			[this, &mtx, diam, &dilated, size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, pp);
				uint cc = BinaryImageExecutor::Accessor_Read(dilated, pp, size3d);
				if (cc == 0)
				{
					int pborder = 0;
					char surround[27];
					for (int dxy = 0; dxy < 27; ++dxy)
					{
						surround[dxy] = 0;
						int dz = (dxy / 9) - 1;
						int dy = (dxy / 3) % 3 - 1;
						int dx = dxy % 3 - 1;
						Pos3i ppc = pp;
						ppc.x = ppc.x + dx;
						ppc.y = ppc.y + dy;
						ppc.z = ppc.z + dz;
						if ((!((ppc.x < 0) || (ppc.x >= size3d.x)
							|| (ppc.y < 0) || (ppc.y >= size3d.y)
							|| (ppc.z < 0) || (ppc.z >= size3d.z))) && (this->Image()(ppc) > 0))
						{
							surround[dxy] = 1;
							++pborder;
						}
					}
					if (pborder > 0)
					{
						if (((surround[22] == 0) && (surround[4] == 0)) ||
							((surround[12] == 0) && (surround[14] == 0)) ||
							((surround[16] == 0) && (surround[10] == 0)))
						{
							mtx.lock();
							this->_centersToErodeSurface.push_back(i);
							mtx.unlock();
						}
						else
						{
							mtx.lock();
							this->_centersToErodeCorner.push_back(i);
							mtx.unlock();
						}
					}
				}
			}
		});
	}

	void BinaryImageDenoiser::Erode()
	{
		int diam = this->_diameter;
		concurrency::accelerator acc = concurrency::accelerator(concurrency::accelerator::default_accelerator);
		concurrency::accelerator_view aview = acc.get_default_view();
		rw::Pos3i size3d = this->Size_3D();

		concurrency::array<uint, 1>& a = *this->_gpuArray;
		const vec(rw::Pos3i)& smask = this->Surface_Mask(diam);
		concurrency::array<rw::Pos3i, 1> asmask((int)smask.size(), smask.begin());

		concurrency::array<int, 1> scenters((int)this->_centersToErodeSurface.size(), this->_centersToErodeSurface.begin(), this->_centersToErodeSurface.end());
		for (int k = 0; k < std::max((int)(this->_centersToErodeSurface.size() / this->_blockSize + 1), (int)1); ++k)
		{
			int begin = k * this->_blockSize;
			int size = this->_blockSize;
			if (begin + size > (int)this->_centersToErodeSurface.size())
			{
				size = (int)this->_centersToErodeSurface.size() - begin;
			}
			if (size > 0)
			{
				concurrency::array_view<int, 1> centers = scenters.section(begin, size);
				parallel_for_each(centers.extent, [size3d, &a, &asmask, centers](concurrency::index<1> idx) restrict(amp)
				{
					int center = centers[idx];
					rw::Pos3i pcenter;
					Pos3i::Int_To_Pos3i(center, size3d.x, size3d.y, size3d.z, pcenter);
					for (int i = 0; i < (int)asmask.extent.size(); ++i)
					{
						rw::Pos3i mcenter = asmask[i];
						mcenter.x = pcenter.x + mcenter.x;
						mcenter.y = pcenter.y + mcenter.y;
						mcenter.z = pcenter.z + mcenter.z;
						if (!((mcenter.x < 0) || (mcenter.x >= size3d.x)
							|| (mcenter.y < 0) || (mcenter.y >= size3d.y)
							|| (mcenter.z < 0) || (mcenter.z >= size3d.z)))
						{
							int idc = Pos3i::Pos3i_To_Int(mcenter, size3d.x, size3d.y, size3d.z);
							if (BinaryImageOpener::Accesor_Read(a, mcenter, size3d) > 0)
							{
								BinaryImageOpener::Accesor_Write_Null(a, mcenter, size3d);
							}
						}
					}
				});
				aview.wait();
			}
		}

		const vec(rw::Pos3i)& mask = this->Corner_Mask(diam);
		concurrency::array<rw::Pos3i, 1> amask((int)mask.size(), mask.begin());

		concurrency::array<int, 1> ccenters((int)this->_centersToErodeCorner.size(), this->_centersToErodeCorner.begin(), this->_centersToErodeCorner.end());
		for (int k = 0; k < std::max((int)(this->_centersToErodeCorner.size() / this->_blockSize + 1), (int)1); ++k)
		{
			int begin = k * this->_blockSize;
			int size = this->_blockSize;
			if (begin + size > (int)this->_centersToErodeCorner.size())
			{
				size = (int)this->_centersToErodeCorner.size() - begin;
			}
			if (size > 0)
			{
				concurrency::array_view<int, 1> centers = ccenters.section(begin, size);
				parallel_for_each(centers.extent, [size3d, &a, &amask, centers](concurrency::index<1> idx) restrict(amp)
				{
					int center = centers[idx];
					rw::Pos3i pcenter;
					Pos3i::Int_To_Pos3i(center, size3d.x, size3d.y, size3d.z, pcenter);
					for (int i = 0; i < (int)amask.extent.size(); ++i)
					{
						rw::Pos3i mcenter = amask[i];
						mcenter.x = pcenter.x + mcenter.x;
						mcenter.y = pcenter.y + mcenter.y;
						mcenter.z = pcenter.z + mcenter.z;
						if (!((mcenter.x < 0) || (mcenter.x >= size3d.x)
							|| (mcenter.y < 0) || (mcenter.y >= size3d.y)
							|| (mcenter.z < 0) || (mcenter.z >= size3d.z)))
						{
							int idc = Pos3i::Pos3i_To_Int(mcenter, size3d.x, size3d.y, size3d.z);
							if (BinaryImageOpener::Accesor_Read(a, mcenter, size3d) > 0)
							{
								BinaryImageOpener::Accesor_Write_Null(a, mcenter, size3d);
							}
						}
					}
				});
				aview.wait();
			}
		}
		concurrency::copy(a, this->Image_Buffer().begin());
	}

	void BinaryImageDenoiser::Dilate()
	{
		int diam = this->_diameter;
		concurrency::accelerator acc = concurrency::accelerator(concurrency::accelerator::default_accelerator);
		concurrency::accelerator_view aview = acc.get_default_view();
		rw::Pos3i size3d = this->Size_3D();
		if (diam > 0)
		{
			concurrency::array<uint, 1>& a = *this->_gpuArray;
			const vec(int)& surface_border = this->_surfaceBorder;
			if (surface_border.size() > 0)
			{
				const vec(rw::Pos3i)& smask = this->Surface_Mask(diam);
				const concurrency::array<rw::Pos3i, 1> asmask((int)smask.size(), smask.begin(), smask.end());
				concurrency::array<int, 1> sborder((int)surface_border.size(), surface_border.begin(), surface_border.end());
				for (int k = 0; k < std::max((int)(surface_border.size() / this->_blockSize + 1), (int)1); ++k)
				{
					int begin = k * this->_blockSize;
					int size = this->_blockSize;
					if (begin + size > (int)surface_border.size())
					{
						size = (int)surface_border.size() - begin;
					}
					if (size > 0)
					{
						concurrency::array_view<int, 1> aborder = sborder.section(begin, size);
						parallel_for_each(aborder.extent, [size3d, &a, &asmask, aborder](concurrency::index<1> idx) restrict(amp)
						{
							uint center = aborder[idx];
							rw::Pos3i pcenter;
							Pos3i::Int_To_Pos3i(center, size3d.x, size3d.y, size3d.z, pcenter);
							for (uint i = 0; i < (uint)asmask.extent.size(); ++i)
							{
								rw::Pos3i mcenter = asmask[i];
								mcenter = pcenter + mcenter;
								if ((!((mcenter.x < 0) || (mcenter.x >= size3d.x)
									|| (mcenter.y < 0) || (mcenter.y >= size3d.y)
									|| (mcenter.z < 0) || (mcenter.z >= size3d.z)))
									&& (BinaryImageOpener::Accesor_Read(a, mcenter, size3d) == 0))
								{
									BinaryImageOpener::Accesor_Write_One(a, mcenter, size3d);
								}
							}
						});
						aview.wait();
					}
				}
			}
			const vec(int)& corner_border = this->_cornerBorder;
			if (corner_border.size() > 0)
			{
				const vec(rw::Pos3i)& mask = this->Corner_Mask(diam);
				const concurrency::array<rw::Pos3i, 1> amask((int)mask.size(), mask.begin(), mask.end());
				concurrency::array<int, 1> border((int)corner_border.size(), corner_border.begin(), corner_border.end());
				for (int k = 0; k < std::max((int)(corner_border.size() / this->_blockSize + 1), 1); ++k)
				{
					int begin = k * this->_blockSize;
					int size = this->_blockSize;
					if (begin + size > (int)corner_border.size())
					{
						size = (int)corner_border.size() - begin;
					}
					if (size > 0)
					{
						concurrency::array_view<int, 1> aborder = border.section(begin, size);
						rw::Pos3i size3d = this->Size_3D();
						parallel_for_each(aborder.extent, [size3d, &a, &amask, aborder](concurrency::index<1> idx) restrict(amp)
						{
							uint center = aborder[idx];
							rw::Pos3i pcenter;
							Pos3i::Int_To_Pos3i(center, size3d.x, size3d.y, size3d.z, pcenter);
							for (uint i = 0; i < (uint)amask.extent.size(); ++i)
							{
								rw::Pos3i mcenter = amask[i];
								mcenter = pcenter + mcenter;
								if ((!((mcenter.x < 0) || (mcenter.x >= size3d.x)
									|| (mcenter.y < 0) || (mcenter.y >= size3d.y)
									|| (mcenter.z < 0) || (mcenter.z >= size3d.z)))
									&& (BinaryImageOpener::Accesor_Read(a, mcenter, size3d) == 0))
								{
									BinaryImageOpener::Accesor_Write_One(a, mcenter, size3d);
								}
							}
						});
						aview.wait();
					}
				}
			}
			concurrency::copy(a, this->Image_Buffer().begin());
		}
	}

	void BinaryImageDenoiser::Execute(BinaryImage::ProgressAdapter* pgdlg)
	{
		if (pgdlg)
		{
			pgdlg->Set_Range(this->Number_Of_Steps());
			pgdlg->Update(1, "Picking centers to erode");
		}
		int diam = this->_diameter;
		this->Add_Mask(diam);
		this->Add_Surface_Mask(diam);
		this->Init();
		this->Pick_Centers_To_Erode();
		if (pgdlg)
		{
			pgdlg->Update(2, "Eroding");
		}
		this->Load();
		this->Erode();
		if (pgdlg)
		{
			pgdlg->Update(3, "Picking centers to dilate");
		}
		this->_centersToErodeCorner.clear();
		this->_centersToErodeSurface.clear();
		this->Pick_Centers_To_Dilate();
		if (pgdlg)
		{
			pgdlg->Update(4, "Dilating");
		}
		this->Load();
		this->Dilate();
	}
}