#include <tbb/parallel_for.h>
#include <tbb/spin_mutex.h>
#include "binary_image_clusterer.h"

namespace rw
{

	BinaryImageClusterer::BinaryImageClusterer(BinaryImageExecutor& executor) : BinaryImageExecutor(executor), BinaryImageGroupMask()
	{
		this->_step = 0;
	}

	void BinaryImageClusterer::Set_Rad_Min_To_Max()
	{
		rw::Pos3i size3d = this->Size_3D();
		int length = size3d.x*size3d.y*size3d.z;
		tbb::parallel_for(tbb::blocked_range<int>(0, length, BCHUNK_SIZE),
			[this, size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, pp);
				if (this->Image()(pp) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						ii->second.dist_min = ii->second.diam_max;
					}
				}
			}
		});
	}

	void BinaryImageClusterer::Recluster(int rad, BinaryImage::ProgressAdapter* pgdlg)
	{
		map<int, int> fusion_map;
		rw::Pos3i size3d = this->Size_3D();
		int length = size3d.x*size3d.y*size3d.z;
		vec(uint)* buffer_ptr = 0;
		buffer_ptr = &this->Image_Buffer();
		vec(uint)& buffer = *buffer_ptr;
		BinaryImage bimg(&buffer, size3d);
		int s_max = min(size3d.z, min(size3d.y, size3d.z));
		tbb::spin_mutex mtx;
		const vec(rw::Pos3i) mask = this->Corner_Mask(3);
		int max_fusion = 1;
		if (pgdlg)
		{
			pgdlg->Update(this->_step, "Checking voxels neighborhood");
		}
		tbb::parallel_for(tbb::blocked_range<int>(0, length, length/32),
			[this, &bimg, &mtx, mask, size3d,rad,&max_fusion, &fusion_map](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i ppc;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, ppc);
				if (bimg(ppc) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						int* updates = new int[mask.size()];
						int toupd = 0;
						int cluster = -1;
						int rad_max = rad;
						map<int, int> cluster_table;
						for (int k = 0; k < (int)mask.size(); ++k)
						{
							updates[k] = -1;
							rw::Pos3i pp = mask[k];
							pp.x = pp.x + ppc.x;
							pp.y = pp.y + ppc.y;
							pp.z = pp.z + ppc.z;
							if (!((pp.x < 0) || (pp.x >= size3d.x)
								|| (pp.y < 0) || (pp.y >= size3d.y)
								|| (pp.z < 0) || (pp.z >= size3d.z)))
							{
								int idp = rw::Pos3i::Pos3i_To_Int(pp, size3d.x, size3d.y, size3d.z);
								map<int, BinaryImage::Pore_Voxel>::iterator iidp = this->Pore_Map().find(idp);
								if (iidp != this->Pore_Map().end())
								{
									int l_rad = iidp->second.diam_max;
									if (l_rad > rad)
									{
										map<int, int>::iterator ict = cluster_table.find(iidp->second.cluster);
										if (ict == cluster_table.end())
										{
											int x = iidp->second.diam_max;
											cluster_table.insert(std::pair<int, int>(iidp->second.cluster, x));
										}
									}
									updates[k] = idp;
									++toupd;
								}
							}
						}
						if ((int)cluster_table.size() > max_fusion)
						{
							mtx.lock();
							max_fusion = (int)cluster_table.size();
							mtx.unlock();
						}
						map<int, int>::iterator ict = cluster_table.begin();
						while (ict != cluster_table.end())
						{
							if (ict->second > rad_max)
							{
								cluster = ict->first;
								rad_max = ict->second;
							}
							++ict;
						}
						if (cluster_table.size() == 2)
						{
							map<int, int>::iterator ift = cluster_table.begin();
							map<int, int>::iterator ist = ift;
							++ist;
							if (ift->second == ist->second)
							{
								mtx.lock();
								fusion_map[ist->first] = ift->first;
								mtx.unlock();
							}
						}

						if ((cluster >= 0) && (toupd > 0))
						{
							for (int k = 0; k < (int)mask.size(); ++k)
							{
								if (updates[k] >= 0)
								{
									map<int, BinaryImage::Pore_Voxel>::iterator uitr = this->Pore_Map().find(updates[k]);
									if ((uitr != this->Pore_Map().end())&&(uitr->second.diam_max > 0))
									{
										mtx.lock();
										uitr->second.cluster = cluster;
										uitr->second.diam_max = -rad_max;
										mtx.unlock();
									}
								}
							}
						}
						delete[]updates;
					}
				}
			}
		});
		for (int k = 0; k < max_fusion; ++k)
		{
			if (pgdlg)
			{
				pgdlg->Update(this->_step, "Fusioning same size clusters. Step: " + std::to_string(k+1));
			}
			this->Apply_Fusion_Map(fusion_map);
		}
	}

	void BinaryImageClusterer::Extend_Diameter(int diam, BinaryImage::ProgressAdapter* pgdlg)
	{
		rw::Pos3i size3d = this->Size_3D();
		int length = size3d.x*size3d.y*size3d.z;
		vec(uint)* buffer_ptr = 0;
		if (diam > 3)
		{
			buffer_ptr = &this->Processed_Image(diam);
		}
		else
		{
			buffer_ptr = &this->Image_Buffer();
		}
		vec(uint)& buffer = *buffer_ptr;
		BinaryImage bimg(&buffer, size3d);
		int s_max = min(size3d.z, min(size3d.y, size3d.z));
		tbb::spin_mutex mtx;
		this->Add_Mask(diam);
		const vec(rw::Pos3i)& mask = this->Corner_Mask(diam);
		if (pgdlg)
		{
			pgdlg->Update(this->_step, std::string("Extending spheres of diameter ") + std::to_string(diam));
		}
		tbb::parallel_for(tbb::blocked_range<int>(0, length, length/32),
			[this, &bimg, &mtx, diam, mask, size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i ppc;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, ppc);
				if (bimg(ppc) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						int* updates = new int[mask.size()];
						int toupd = 0;
						int col = 0;
						int cluster = -1;
						int diam_max = diam;
						map<int, int2> cluster_table;
						for (int k = 0; k < (int)mask.size(); ++k)
						{
							updates[k] = -1;
							rw::Pos3i pp = mask[k];
							pp.x = pp.x + ppc.x;
							pp.y = pp.y + ppc.y;
							pp.z = pp.z + ppc.z;
							if (!((pp.x < 0) || (pp.x >= size3d.x)
								|| (pp.y < 0) || (pp.y >= size3d.y)
								|| (pp.z < 0) || (pp.z >= size3d.z)))
							{
								int idp = rw::Pos3i::Pos3i_To_Int(pp, size3d.x, size3d.y, size3d.z);
								map<int, BinaryImage::Pore_Voxel>::iterator iidp = this->Pore_Map().find(idp);
								if (iidp != this->Pore_Map().end())
								{
									int l_diam = iidp->second.diam_max;
									if (l_diam > diam)
									{
										map<int, int2>::iterator ict = cluster_table.find(iidp->second.cluster);
										if (ict != cluster_table.end())
										{
											ict->second.x = iidp->second.diam_max;
											ict->second.y = ict->second.y + 1;
										}
										else
										{
											int2 ne;
											ne.x = iidp->second.diam_max;
											ne.y = 1;
											cluster_table.insert(std::pair<int, int2>(iidp->second.cluster, ne));
										}
									}
									updates[k] = idp;
									++toupd;
								}
							}
						}
						map<int, int2>::iterator ict = cluster_table.begin();
						while (ict != cluster_table.end())
						{
							if (ict->second.y > col)
							{
								col = ict->second.y;
								cluster = ict->first;
								diam_max = ict->second.x;
							}
							else if ((ict->second.y == col) && (col > 0))
							{
								rw::Pos3i pc;
								rw::Pos3i pn;
								Pos3i::Int_To_Pos3i(cluster, size3d.x, size3d.y, size3d.z, pc);
								Pos3i::Int_To_Pos3i(ict->first, size3d.x, size3d.y, size3d.z, pn);
								if (ppc.Square_Distance(pn) < ppc.Square_Distance(pc))
								{
									cluster = ict->first;
									diam_max = ict->second.x;
								}
							}
							++ict;
						}
						if ((cluster >= 0) && (toupd > 0))
						{
							for (int k = 0; k < (int)mask.size(); ++k)
							{
								if (updates[k] >= 0)
								{
									map<int, BinaryImage::Pore_Voxel>::iterator uitr = this->Pore_Map().find(updates[k]);
									if ((uitr != this->Pore_Map().end()) && (abs(uitr->second.diam_max) != diam_max))
									{
										mtx.lock();
										uitr->second.cluster = cluster;
										uitr->second.diam_max = -diam_max;
										mtx.unlock();
									}
								}
							}
						}
						delete[]updates;
					}
				}
			}
		});
		tbb::parallel_for(tbb::blocked_range<int>(0, length, BCHUNK_SIZE),
			[this, size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, pp);
				if (this->Image()(pp) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						if (ii->second.diam_max < 0)
						{
							ii->second.diam_max = abs(ii->second.diam_max);
						}
					}
				}
			}
		});
	}

	
	void BinaryImageClusterer::Apply_Fusion_Map(const map<int, int>& fusion)
	{
		rw::Pos3i size3d = this->Size_3D();
		int length = size3d.x*size3d.y*size3d.z;
		tbb::spin_mutex mtx;
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)length, max((int)length / 128, BCHUNK_SIZE)),
			[this, &fusion, &mtx,size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, pp);
				if (this->Image()(pp) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						int idcenter = ii->second.cluster;
						map<int, int>::const_iterator itr = fusion.find(idcenter);
						if (itr != fusion.end())
						{
							int newcenter = itr->second;
							mtx.lock();
							ii->second.cluster = newcenter;
							mtx.unlock();
						}
					}
				}
			}
		});
	}

	void BinaryImageClusterer::Apply_Fusion_Map(const map<int, int>& fusion, int diam)
	{
		rw::Pos3i size3d = this->Size_3D();
		int length = size3d.x*size3d.y*size3d.z;
		tbb::spin_mutex mtx;
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)length, max((int)length / 128, BCHUNK_SIZE)),
			[this, &fusion, &mtx, diam,size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, pp);
				if (this->Image()(pp) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if ((ii != this->Pore_Map().end()) && (ii->second.diam_max == diam))
					{
						int idcenter = ii->second.cluster;
						map<int, int>::const_iterator itr = fusion.find(idcenter);
						if (itr != fusion.end())
						{
							int newcenter = itr->second;
							mtx.lock();
							ii->second.cluster = newcenter;
							mtx.unlock();
						}
					}
				}
			}
		});
	}

	void BinaryImageClusterer::Set_Box(rw::Box3D& box, const rw::Pos3i& center, int diam) const
	{
		int dr = (int)(((float)diam)*1.41f);

		box._eA.x = center.x - dr;
		box._eA.y = center.y - dr;
		box._eA.z = center.z - dr;

		box._eB.x = center.x + dr;
		box._eB.y = center.y + dr;
		box._eB.z = center.z + dr;
	}


	void BinaryImageClusterer::Extend_Box(rw::Box3D& box, const rw::Pos3i& center, int diam) const
	{
		int dr = (int)(((float)diam)*1.41f);

		box._eA.x = min(center.x - dr, box._eA.x);
		box._eA.y = min(center.y - dr, box._eA.y);
		box._eA.z = min(center.z - dr, box._eA.z);

		box._eB.x = max(center.x + dr, box._eB.x);
		box._eB.y = max(center.y + dr, box._eB.y);
		box._eB.z = max(center.z + dr, box._eB.z);

	}

	int BinaryImageClusterer::Square_Cluster_Distance(int rad) const
	{
		return (4*rad*rad);
	}


	void BinaryImageClusterer::Group_Clusters(int diam, BinaryImage::ProgressAdapter* pgdlg)
	{
		rw::Pos3i size3d = this->Size_3D();
		int length = size3d.x*size3d.y*size3d.z;
		BinaryImage bimg(&this->Image_Buffer(),size3d);
		set<int> centers;
		tbb::spin_mutex mtx;
		if (pgdlg)
		{
			pgdlg->Update(this->_step, std::string("Picking centers from morphology of diameter ") + std::to_string(diam));
		}
		tbb::parallel_for(tbb::blocked_range<int>(0, length, BCHUNK_SIZE),
			[this, &bimg, &mtx, &centers, diam, size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, pp);
				if (bimg(pp) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						if (ii->second.diam_max == diam)
						{
							if (centers.find(ii->second.cluster) == centers.end())
							{
								mtx.lock();
								centers.insert(ii->second.cluster);
								mtx.unlock();
							}
						}
					}
				}
			}
		});
		vec(int) v_centers(centers.begin(), centers.end());
		map<int, int> fusion_map;
		vector<set<int>> kdtree;
		vector<Box3D> boxes;
		kdtree.reserve(centers.size());
		boxes.reserve(centers.size());
		int size = 0;
		if (pgdlg)
		{
			pgdlg->Update(this->_step, std::string("Detecting overlapping centers from spheres of diameter ") + std::to_string(diam));
		}

		tbb::parallel_for(tbb::blocked_range<int>(0, (int)v_centers.size(), max((int)v_centers.size() / 128, BCHUNK_SIZE)),
			[this, diam, &kdtree, &mtx, &boxes, &v_centers, size3d,&size](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				Pos3i scenter;
				int id = v_centers[i];
				Pos3i::Int_To_Pos3i(id, size3d.x, size3d.y, size3d.z, scenter);
				bool merged = false;
				for (int k = 0; k < size; ++k)
				{
					Box3D& box = boxes[k];
					if (box.In(scenter))
					{
						mtx.lock();
						this->Extend_Box(box, scenter, diam);
						kdtree[k].insert(id);
						merged = true;
						k = (int)kdtree.size();
						mtx.unlock();
					}
				}
				if (!merged)
				{
					mtx.lock();
					set<int> nset;
					nset.insert(id);
					kdtree.push_back(nset);
					Box3D box;
					this->Set_Box(box, scenter, diam);
					boxes.push_back(box);
					size = (int)kdtree.size();
					mtx.unlock();
				}
			}
		});

		if (pgdlg)
		{
			pgdlg->Update(this->_step, std::string("Fusioning overlapping centers from spheres of diameter ") + std::to_string(diam));
		}

		int dstc = this->Square_Cluster_Distance(diam);

		tbb::parallel_for(tbb::blocked_range<int>(0, (int)kdtree.size(), BCHUNK_SIZE),
			[this, diam, &kdtree, &fusion_map, &mtx,size3d,dstc](const tbb::blocked_range<int>& b)
		{
			for (int k = b.begin(); k < b.end(); ++k)
			{
				set<int>::iterator ii = kdtree[k].begin();
				while (ii != kdtree[k].end())
				{
					Pos3i ip;
					Pos3i::Int_To_Pos3i(*ii, size3d.x, size3d.y, size3d.z, ip);
					set<int>::iterator jj = ii;
					++jj;
					while (jj != kdtree[k].end())
					{
						Pos3i jp;
						Pos3i::Int_To_Pos3i(*jj, size3d.x, size3d.y, size3d.z, jp);
						if (ip.Square_Distance(jp) < dstc)
						{
							map<int, int>::iterator fmi = fusion_map.find((*ii));
							if (fmi == fusion_map.end())
							{
								mtx.lock();
								fusion_map.insert(std::pair<int, int>(*jj, *ii));
								mtx.unlock();
							}
							else
							{
								mtx.lock();
								int vox = fmi->second;
								fusion_map.insert(std::pair<int, int>(*jj, vox));
								mtx.unlock();
							}
						}
						++jj;
					}
					++ii;
				}
			}
		});
		this->Apply_Fusion_Map(fusion_map, diam);
	}

	void BinaryImageClusterer::Set_Positive_Distances()
	{
		rw::Pos3i size3d = this->Size_3D();
		int length = size3d.x*size3d.y*size3d.z;
		tbb::parallel_for(tbb::blocked_range<int>(0, length, BCHUNK_SIZE),
			[this,size3d](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, size3d.x, size3d.y, size3d.z, pp);
				if (this->Image()(pp) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::iterator ii = this->Pore_Map().find(i);
					if (ii != this->Pore_Map().end())
					{
						ii->second.diam_max = abs(ii->second.diam_max);
						ii->second.dist_min = abs(ii->second.dist_min);
					}
				}
			}
		});
	}

	void BinaryImageClusterer::Expand_Diameter(BinaryImage::ProgressAdapter* pgdlg)
	{
		this->_step = 0;
		int rad = this->Max_Radius();
		this->Set_Rad_Min_To_Max();
		while (rad > 0)
		{
			if (pgdlg)
			{
				pgdlg->Update(this->_step,string("Extending radius ") + std::to_string(rad));
			}
			this->Extend_Diameter(rad,pgdlg);
			++this->_step;
			if (pgdlg)
			{
				pgdlg->Update(this->_step,string("Grouping radius ") + std::to_string(rad));
			}
			++this->_step;
			this->Group_Clusters(rad,pgdlg);
			--rad;
			--rad;
		}
		++this->_step;
		if (pgdlg)
		{
			pgdlg->Update(this->_step,string("Updating pore map"));
		}
		this->Recluster(1,pgdlg);
		this->Set_Positive_Distances();
	}

	void BinaryImageClusterer::Execute(BinaryImage::ProgressAdapter* pgdlg)
	{
		if (pgdlg)
		{
			pgdlg->Set_Range(this->Number_Of_Steps());
		}
		this->Expand_Diameter(pgdlg);
	}
}