#include <algorithm>
#include <iostream>
#include <tbb/parallel_for.h>
#include <tbb/spin_mutex.h>
#include "binary_image_opener.h"

namespace rw
{

	const int BLOCK1 = 65536;
	const int BLOCK2 = 32768;
	const int BLOCK3 = 16384;

	BinaryImageOpener::BinaryImageOpener(rw::BinaryImageBorderCreator& executor) : BinaryImageBorderCreator(executor), BinaryImageMaskHandler()
	{
		this->_diameter = 0;
		this->_blockSize = BLOCK1;		
		this->_gpuArray = new concurrency::array<uint, 1>((int)this->Image_Buffer().size(), this->Image_Buffer().begin());	
		this->_step = 0;
	}

	void BinaryImageOpener::Release_GPU()
	{
		if (this->_gpuArray)
		{
			delete this->_gpuArray;
			this->_gpuArray = 0;
		}
	}

	void BinaryImageOpener::Load_GPU()
	{
		if (!this->_gpuArray)
		{
			this->_gpuArray = new concurrency::array<uint, 1>((int)this->Image_Buffer().size(), this->Image_Buffer().begin());
		}
	}

	void BinaryImageOpener::Centers_To_Erode_Surface(vec(int)& centers)
	{
		centers.clear();
		centers.assign(this->_centersToErodeSurface.begin(), this->_centersToErodeSurface.end());
	}

	void BinaryImageOpener::Centers_To_Erode_Corner(vec(int)& centers)
	{
		centers.clear();
		centers.assign(this->_centersToErodeCorner.begin(), this->_centersToErodeCorner.end());
	}

	BinaryImageOpener::~BinaryImageOpener()
	{
		if (this->_gpuArray)
		{
			delete this->_gpuArray;
			this->_gpuArray = 0;
		}
		concurrency::accelerator acc = concurrency::accelerator(concurrency::accelerator::default_accelerator);
		concurrency::accelerator_view aview = acc.get_default_view();
		aview.flush();
	}
	
	void BinaryImageOpener::Set_Diameter(int diam)
	{
		this->_diameter = diam;
		if (diam > 32)
		{
			this->_blockSize = BLOCK2;
		}
		if (diam > 56)
		{
			this->_blockSize = BLOCK3;
		}
	}

	void BinaryImageOpener::Set_DiamMin_To_2Rad(int _2rad)
	{
		vec(uint)& buffer = this->Processed_Image(_2rad);
		rw::Pos3i size = this->Size_3D();
		BinaryImage bimg(&buffer,size);

		int rad = _2rad / 2;
		int length = size.x*size.y*size.z;
		tbb::parallel_for(tbb::blocked_range<int>(0, length, BCHUNK_SIZE),
			[this, rad, &bimg, size](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size.x, size.y, size.z, pp);
				if (bimg(pp) == 0)
				{
					map<int, rw::BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						ii->second.dist_min = (char)rad;
					}
				}
			}
		});
	}

	

	void BinaryImageOpener::Dilate(int diam)
	{
		concurrency::accelerator acc = concurrency::accelerator(concurrency::accelerator::default_accelerator);
		concurrency::accelerator_view aview = acc.get_default_view();
		rw::Pos3i size3d = this->Size_3D();
		this->Add_Mask(diam);
		this->Add_Surface_Mask(diam);
		vec(uint)* dilatedTexture = new vec(uint)((int)this->Image_Buffer().size(), 0);
		if (diam > 0)
		{
			concurrency::array<uint, 1>& a = *this->_gpuArray;

			if (this->Surface_Border().size() > 0)
			{
				vec(rw::Pos3i) smask = this->Surface_Mask(diam);
				const concurrency::array<rw::Pos3i, 1> asmask((int)smask.size(), smask.begin(), smask.end());				
				const vec(int)& surface_border = this->Surface_Border();
				concurrency::array<int, 1> sborder((int)surface_border.size(), surface_border.begin() , surface_border.end());
				for (int k = 0; k < max((int)(surface_border.size() / this->_blockSize + 1), (int)1); ++k)
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
								mcenter.x = pcenter.x + mcenter.x;
								mcenter.y = pcenter.y + mcenter.y;
								mcenter.z = pcenter.z + mcenter.z;
								if ((!((mcenter.x < 0) || (mcenter.x >= size3d.x)
									|| (mcenter.y < 0) || (mcenter.y >= size3d.y)
									|| (mcenter.z < 0) || (mcenter.z >= size3d.z)))
									&& (BinaryImageExecutor::Accesor_Read(a, mcenter, size3d) == 0))
								{
									BinaryImageOpener::Accesor_Write_One(a, mcenter, size3d);
								}
							}
						});
						aview.wait();
					}
				}
			}
			if (this->Corner_Border().size() > 0)
			{
				vec(rw::Pos3i) mask = this->Corner_Mask(diam);
				const concurrency::array<rw::Pos3i, 1> amask((int)mask.size(), mask.begin(), mask.end());
				const vec(int)& corner_border = this->Corner_Border();
				concurrency::array<int, 1> border((int)corner_border.size(), corner_border.begin(), corner_border.end());
				for (int k = 0; k < max((int)(corner_border.size() / this->_blockSize + 1), 1); ++k)
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
								mcenter.x = pcenter.x + mcenter.x;
								mcenter.y = pcenter.y + mcenter.y;
								mcenter.z = pcenter.z + mcenter.z;
								if ((!((mcenter.x < 0) || (mcenter.x >= size3d.x)
									|| (mcenter.y < 0) || (mcenter.y >= size3d.y)
									|| (mcenter.z < 0) || (mcenter.z >= size3d.z)))
									&& (BinaryImageExecutor::Accesor_Read(a, mcenter, size3d) == 0))
								{
									BinaryImageOpener::Accesor_Write_One(a, mcenter, size3d);
								}
							}
						});
						aview.wait();
					}
				}
			}
			concurrency::copy(a, dilatedTexture->begin());
		}
		tbb::spin_mutex mtx;
		this->_centersToErodeCorner.clear();
		this->_centersToErodeSurface.clear();
		int tot = size3d.x*size3d.y*size3d.z;		
		vec(uint)& dilated = *dilatedTexture;
		tbb::parallel_for(tbb::blocked_range<int>(0, tot, max(tot / 128, BCHUNK_SIZE)),
			[this, &mtx, diam, &dilated, size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y , size3d.z, pp);
				uint cc = BinaryImageExecutor::Accessor_Read(dilated, pp, size3d);
				if (cc == 0)
				{
					bool corner = true;
					rw::Pos3i ppx1 = pp;
					ppx1.x = ppx1.x - 1;
					rw::Pos3i ppx2 = pp;
					ppx2.x = ppx2.x + 1;

					rw::Pos3i ppy1 = pp;
					ppy1.y = ppy1.y - 1;
					rw::Pos3i ppy2 = pp;
					ppy2.y = ppy2.y + 1;

					rw::Pos3i ppz1 = pp;
					ppz1.z = ppz1.z - 1;
					rw::Pos3i ppz2 = pp;
					ppz2.z = ppz2.z + 1;

					if ((ppx1.x >= 0) && (ppx2.x < size3d.x)
						&& (ppy1.y >= 0) && (ppy2.y < size3d.y)
						&& (ppz1.z >= 0) && (ppz2.z < size3d.z))
					{
						uint cx1 = rw::BinaryImageExecutor::Accessor_Read(dilated, ppx1, size3d);
						uint cx2 = rw::BinaryImageExecutor::Accessor_Read(dilated, ppx2, size3d);
						uint cy1 = rw::BinaryImageExecutor::Accessor_Read(dilated, ppy1, size3d);
						uint cy2 = rw::BinaryImageExecutor::Accessor_Read(dilated, ppy2, size3d);
						uint cz1 = rw::BinaryImageExecutor::Accessor_Read(dilated, ppz1, size3d);
						uint cz2 = rw::BinaryImageExecutor::Accessor_Read(dilated, ppz2, size3d);
						if (((cx1 == 0) && (cx2 == 0))
							|| ((cy1 == 0) && (cy2 == 0))
							|| ((cz1 == 0) && (cz2 == 0)))
						{
							mtx.lock();
							this->_centersToErodeSurface.insert(i);
							mtx.unlock();
							corner = false;
						}
					}
					if (corner)
					{
						mtx.lock();
						this->_centersToErodeCorner.insert(i);
						mtx.unlock();
					}
				}
			}
		});
		if (this->Black_Voxels_Covered())
		{
			delete dilatedTexture;
		}
		else
		{
			this->Set_Processed_Image(dilatedTexture, diam);
		}
	}

	void BinaryImageOpener::Update_And_Erode_Pore_Map(int diam)
	{
		this->Add_Mask(diam);
		this->Add_Surface_Mask(diam);
		rw::Pos3i size3d = this->Size_3D();
		if (this->_centersToErodeSurface.size() > 0)
		{
			const vec(rw::Pos3i)& mask = this->Surface_Mask(diam);
			vec(int) centers(this->_centersToErodeSurface.begin(), this->_centersToErodeSurface.end());
			tbb::spin_mutex mtx;
			tbb::parallel_for(tbb::blocked_range<int>(0, (int)centers.size(), DCHUNK_SIZE),
				[this, diam, &mtx, &centers, mask, size3d](const tbb::blocked_range<int>& b)
			{
				for (int i = b.begin(); i < b.end(); ++i)
				{
					int ii = centers[i];
					rw::Pos3i ppc;
					rw::Pos3i::Int_To_Pos3i(ii, size3d.x, size3d.y, size3d.z, ppc);
					for (int k = 0; k < mask.size(); ++k)
					{
						rw::Pos3i pp = mask[k];
						pp.x = ppc.x + pp.x;
						pp.y = ppc.y + pp.y;
						pp.z = ppc.z + pp.z;
						if (!((pp.x < 0) || (pp.x >= size3d.x)
							|| (pp.y < 0) || (pp.y >= size3d.y)
							|| (pp.z < 0) || (pp.z >= size3d.z)))
						{
							int idp = rw::Pos3i::Pos3i_To_Int(pp, size3d.x, size3d.y, size3d.z);
							map<int, BinaryImage::Pore_Voxel>::iterator pidp = this->Pore_Map().find(idp);
							if ((pidp != this->Pore_Map().end()) && (pidp->second.diam_max < diam))
							{
								mtx.lock();
								pidp->second.diam_max = diam;
								pidp->second.cluster = ii;
								mtx.unlock();
							}
						}
					}
				}
			});
		}

		if (this->_centersToErodeCorner.size() > 0)
		{
			const vec(rw::Pos3i)& mask = this->Corner_Mask(diam);
			vec(int) centers(this->_centersToErodeCorner.begin(), this->_centersToErodeCorner.end());
			tbb::spin_mutex mtx;
			tbb::parallel_for(tbb::blocked_range<int>(0, (int)centers.size(), DCHUNK_SIZE),
				[this, diam, &mtx, &centers, mask, size3d](const tbb::blocked_range<int>& b)
			{
				for (int i = b.begin(); i < b.end(); ++i)
				{
					int ii = centers[i];
					rw::Pos3i ppc;
					rw::Pos3i::Int_To_Pos3i(ii, size3d.x, size3d.y, size3d.z, ppc);
					for (int k = 0; k < mask.size(); ++k)
					{
						rw::Pos3i pp = mask[k];
						pp.x = ppc.x + pp.x;
						pp.y = ppc.y + pp.y;
						pp.z = ppc.z + pp.z;
						if (!((pp.x < 0) || (pp.x >= size3d.x)
							|| (pp.y < 0) || (pp.y >= size3d.y)
							|| (pp.z < 0) || (pp.z >= size3d.z)))
						{
							int idp = rw::Pos3i::Pos3i_To_Int(pp, size3d.x, size3d.y, size3d.z);
							map<int, BinaryImage::Pore_Voxel>::iterator pidp = this->Pore_Map().find(idp);
							if ((pidp != this->Pore_Map().end()) && (pidp->second.diam_max < diam))
							{
								mtx.lock();
								pidp->second.diam_max = diam;
								pidp->second.cluster = ii;
								mtx.unlock();
							}
						}
					}
				}
			});
		}
	}

	void BinaryImageOpener::Open(int diam, BinaryImage::ProgressAdapter* pgdlg)
	{
		if (pgdlg)
		{
			std::string msg = std::string("Dilating by diameter ") + std::to_string(this->_diameter);
			pgdlg->Update(this->_step, msg);
		}
		this->Dilate(diam);
		if (!this->Black_Voxels_Covered())
		{
			this->Set_DiamMin_To_2Rad(diam);
			if (pgdlg)
			{
				std::string msg = std::string("Eroding by diameter ") + std::to_string(this->_diameter);
				pgdlg->Update(this->_step, msg);
			}
			this->Update_And_Erode_Pore_Map(diam);
		}
	}

	bool BinaryImageOpener::Black_Voxels_Covered() const
	{
		return(this->_centersToErodeCorner.empty() && this->_centersToErodeSurface.empty());
	}

	void BinaryImageOpener::Execute(BinaryImage::ProgressAdapter* pgdlg)
	{
		if (((pgdlg)&&(this->_step == 0))||(this->_step >= (int)this->Number_Of_Steps()))
		{			
			pgdlg->Set_Range(this->Number_Of_Steps()+this->_step);
		}
		this->_step = this->_step + 1;
		if (this->Pore_Map().size() == 0)
		{
			if (pgdlg)
			{
				pgdlg->Update(this->_step, "Building pore map");
			}
			this->Rebuild_Pore_Map();
		}
		if (pgdlg)
		{
			std::string msg = std::string("Opening by radius ") + std::to_string(this->_diameter);
			pgdlg->Update(this->_step, msg);
		}
		this->Open(this->_diameter, pgdlg);
	}


}