#include <tbb/parallel_for.h>
#include <tbb/spin_mutex.h>
#include "binary_image_watershed_clusterer.h"

namespace rw
{

	BinaryImageWaterShedClusterer::BinaryImageWaterShedClusterer(BinaryImageExecutor& executor) : BinaryImageExecutor(executor)	
	{

	}

	vec(rw::Pos3i) BinaryImageWaterShedClusterer::Neigborhood(int rad)
	{
		vec(rw::Pos3i) r;
		for (int dz = -rad; dz <= rad; ++dz)
		{
			for (int dy = -rad; dy <= rad; ++dy)
			{
				for (int dx = -rad; dx <= rad; ++dx)
				{
					if (!((dx == 0) && (dy == 0) && (dz == 0)))
					{
						rw::Pos3i p;
						p.x = dx;
						p.y = dy;
						p.z = dz;
						r.push_back(p);
					}
				}
			}
		}
		return(r);
	}

	void BinaryImageWaterShedClusterer::Median()
	{
		rw::Pos3i size3d = this->Size_3D();
		tbb::spin_mutex mtx;
		set<int> indexes;
		vec(rw::Pos3i) nhd = this->Neigborhood(1);
		int length = size3d.x*size3d.y*size3d.z;
		tbb::parallel_for(tbb::blocked_range<int>(0, length, BCHUNK_SIZE),
			[this, size3d, &indexes, &mtx, nhd](const tbb::blocked_range<int>& b)
		{
			for (int id = b.begin(); id < b.end(); ++id)
			{
				int i = id;
				map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
				if (ii != this->Pore_Map().end())
				{
					rw::Pos3i root_pos;
					rw::Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, root_pos);
					int avg = 0;
					for (int k = 0; k < nhd.size(); ++k)
					{
						rw::Pos3i n_pos = nhd[k];
						n_pos = n_pos + root_pos;
						int idn_pos = rw::Pos3i::Pos3i_To_Int(n_pos, size3d.x, size3d.y, size3d.z);
						map<int, BinaryImage::Pore_Voxel>::iterator idn_itr = this->Pore_Map().find(idn_pos);
						if (idn_itr != this->Pore_Map().end())
						{
							avg = avg + idn_itr->second.dist_min;
						}
					}
					avg = avg / (int)nhd.size() + 1;
					ii->second.dist_min = (char)avg;
				}
			}
		});
	}

	void BinaryImageWaterShedClusterer::Cluster(const vec(int)& input, vec(int)& output, int rad, BinaryImage::ProgressAdapter* pgdlg)
	{
		if (pgdlg)
		{
			pgdlg->Update(this->_step,"Wathershedding step " + std::to_string(this->_step));
		}

		rw::Pos3i size3d = this->Size_3D();
		tbb::spin_mutex mtx;
		set<int> indexes;
		vec(rw::Pos3i) nhd = this->Neigborhood(rad);
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)input.size(), 1000*BCHUNK_SIZE),
			[this, size3d, &indexes, &mtx,nhd,&input](const tbb::blocked_range<int>& b)
		{
			for (int id = b.begin(); id < b.end(); ++id)
			{
				int i = input[id];
				map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
				int current_rad = ii->second.dist_min;
				rw::Pos3i current_pos;
				rw::Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, current_pos);
				BinaryImage::Pore_Voxel* root = &(ii->second);
				bool clustered = false;
				int cluster = -1;
				rw::Pos3i root_pos = current_pos;
				while ((root) && (!clustered))
				{
					root = 0;
					for (int k = 0; k < nhd.size(); ++k)
					{
						rw::Pos3i n_pos = nhd[k];
						n_pos = n_pos + root_pos;
						int idn_pos = rw::Pos3i::Pos3i_To_Int(n_pos, size3d.x, size3d.y, size3d.z);
						map<int, BinaryImage::Pore_Voxel>::iterator idn_itr = this->Pore_Map().find(idn_pos);
						if (idn_itr != this->Pore_Map().end())
						{
							if (idn_itr->second.dist_min > current_rad)
							{
								current_rad = idn_itr->second.dist_min;
								root = &(idn_itr->second);
								current_pos = n_pos;
								clustered = false;
							}
							if (idn_itr->second.cluster >= 0)
							{
								if (current_rad <= idn_itr->second.dist_min)
								{
									current_rad = idn_itr->second.dist_min;
									clustered = true;
									cluster = idn_itr->second.cluster;
								}
							}
						}
					}		
					root_pos = current_pos;
				}
				if (clustered)
				{
					ii->second.dist_min = current_rad;
					ii->second.cluster = -cluster;
				}
				else
				{
					mtx.lock();
					indexes.insert(i);
					mtx.unlock();
				}
			}
		});
		this->Update_Clusters();
		output.assign(indexes.begin(), indexes.end());
		++this->_step;
	}

	void BinaryImageWaterShedClusterer::Update_Clusters()
	{
		rw::Pos3i size3d = this->Size_3D();
		int length = size3d.x*size3d.y*size3d.z;

		tbb::parallel_for(tbb::blocked_range<int>(0, length, BCHUNK_SIZE),
			[this, size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i cpos;
				rw::Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, cpos);
				if (this->Image()(cpos) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						BinaryImage::Pore_Voxel& pv = ii->second;
						if ((pv.diam_max == pv.dist_min) && (pv.dist_min > 1))
						{
							pv.cluster = abs(pv.cluster);
						}
					}
				}
			}
		});
	}

	void BinaryImageWaterShedClusterer::Init_Clusters(vec(int)& points, BinaryImage::ProgressAdapter* pgdlg)
	{
		if (pgdlg)
		{
			pgdlg->Update("Init clusters");
		}
		rw::Pos3i size3d = this->Size_3D();
		int length = size3d.x*size3d.y*size3d.z;
		tbb::spin_mutex mtx;
		set<int> indexes;		
		
		tbb::parallel_for(tbb::blocked_range<int>(0, length, BCHUNK_SIZE),
			[this, size3d,&indexes,&mtx](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i cpos;
				rw::Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, cpos);
				if (this->Image()(cpos) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						BinaryImage::Pore_Voxel& pv = ii->second;
						if ((pv.diam_max == pv.dist_min)&&(pv.dist_min > 1))
						{
							pv.cluster = i;							
						}
						else
						{
							pv.cluster = -1;
							mtx.lock();
							indexes.insert(i);
							mtx.unlock();
						}
					}
				}
			}
		});
		points.assign(indexes.begin(), indexes.end());
	}

	void BinaryImageWaterShedClusterer::Execute(BinaryImage::ProgressAdapter* pgdlg)
	{
		if (pgdlg)
		{
			pgdlg->Set_Range(10*this->Max_Radius());
		}
		this->_step = 0;
		vec(int)* input = new vec(int)();
		vec(int)* output = new vec(int)();
		this->Median();
		this->Init_Clusters(*input,pgdlg);
		int k = 0;
		int rad = 1;
		int size = 0;
		while ((input->size() > 0)&&(k < 50))
		{
			if (pgdlg)
			{
				pgdlg->Update(this->_step,string("Clustering")+std::to_string(input->size())+string(" points"));
			}

			this->Cluster(*input,*output,rad,pgdlg);			
			if ((int)output->size() != size)
			{
				size = (int)output->size();
			}
			else
			{
				++rad;
			}
			delete input;
			input = output;
			output = new vec(int);
			++k;
		}
		delete input;
		delete output;
	}

}