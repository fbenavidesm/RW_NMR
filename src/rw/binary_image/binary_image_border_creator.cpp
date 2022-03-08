#include <tbb/spin_mutex.h>
#include <tbb/parallel_for.h>
#include "binary_image_border_creator.h"

namespace rw
{

	BinaryImageBorderCreator::BinaryImageBorderCreator(BinaryImageExecutor& executor) : BinaryImageExecutor(executor)
	{
		this->_size.x = this->Image().Width();
		this->_size.y = this->Image().Height();
		this->_size.z = this->Image().Depth();
		this->_cornerBorder = new vec(int);
		this->_surfaceBorder = new vec(int);
	}

	BinaryImageBorderCreator::BinaryImageBorderCreator(BinaryImageBorderCreator& executor) : BinaryImageExecutor(executor)
	{
		this->_size = executor._size;
		this->_cornerBorder = executor._cornerBorder;
		this->_surfaceBorder = executor._surfaceBorder;
		executor._surfaceBorder = 0;
		executor._cornerBorder = 0;
	}

	BinaryImageBorderCreator::~BinaryImageBorderCreator()
	{
		if (this->_cornerBorder)
		{
			delete this->_cornerBorder;
		}
		if (this->_surfaceBorder)
		{
			delete this->_surfaceBorder;
		}
	}


	void BinaryImageBorderCreator::Execute(BinaryImage::ProgressAdapter* pgdlg)
	{
		this->_cornerBorder->clear();
		this->_surfaceBorder->clear();
		this->_cornerBorder->reserve((int)this->Image().Black_Voxels());
		this->_surfaceBorder->reserve((int)this->Image().Black_Voxels());
		int tot = this->_size.x * this->_size.y * this->_size.z;
		tbb::spin_mutex mtx;
		tbb::parallel_for(tbb::blocked_range<int>(0, tot, std::max(tot / 128, BCHUNK_SIZE)),
			[this, &mtx](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				int idc = 0;
				Pos3i pp;
				Pos3i::Int_To_Pos3i(i, this->_size.x, this->_size.y, this->_size.z, pp);
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
						if ((!((ppc.x < 0) || (ppc.x >= this->_size.x)
							|| (ppc.y < 0) || (ppc.y >= this->_size.y)
							|| (ppc.z < 0) || (ppc.z >= this->_size.z))) && (this->Image()(ppc) == 0))
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
							this->_surfaceBorder->push_back(i);
							mtx.unlock();
						}
						else
						{
							mtx.lock();
							this->_cornerBorder->push_back(i);
							mtx.unlock();
						}
					}
				}
			}
		});
		this->Set_Border(true);
	}

}