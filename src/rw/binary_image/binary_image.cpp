#include <string>
#include <random>
#include "tbb/spin_mutex.h"
#include "tbb/parallel_for.h"
#include "binary_image.h"
#include "binary_image_border_creator.h"
#include "binary_image_opener.h"
#include "binary_image_denoiser.h"
#include "binary_image_clusterer.h"
#include "binary_image_watershed_clusterer.h"

namespace rw
{

	using std::string;

	const map<uint, RGBColor>& BinaryImage::ColorMap::Color_Map() const
	{
		return(*this->_colorMap);
	}

	BinaryImage::ColorMap::ColorMap() 
	{
		this->_colorMap = 0;
	}

	BinaryImage::ColorMap::~ColorMap()
	{
		this->_colorMap = 0;
	}

	BinaryImage::BinaryImage()
	{
		this->_buffer = 0;
		this->_sharedBuffer = false;
		this->_blackVoxels = 0;
		//this->_flags = CITY_BLOCK;
	}

	BinaryImage::BinaryImage(const BinaryImage& img)
	{
		this->_buffer = new vec(uint)(*img._buffer);
		this->_sharedBuffer = false;
		this->_width = img._width;
		this->_height = img._height;
		this->_depth = img._depth;
		this->_blackVoxels = img._blackVoxels;
	}

	BinaryImage::BinaryImage(vec(uint)* buffer, rw::Pos3i size)
	{
		this->_width = size.x;
		this->_height = size.y;
		this->_depth = size.z;
		this->_buffer = buffer;
		this->_sharedBuffer = true;
		this->_blackVoxels = 0;
	}

	void BinaryImage::Init()
	{
		map<int, vec(uint)*>::iterator i = this->_processedImages.begin();
		while (i != this->_processedImages.end())
		{
			delete i->second;
		}
		this->_processedImages.clear();
		this->_state = new BinaryImageExecutor(this);	
		this->_poreMap.clear();
		this->_blackVoxels = 0;
		this->_colorMap.clear();
	}

	BinaryImage BinaryImage::Processed_Image(int key) const
	{
		map<int, vec(uint)*>::const_iterator itr = this->_processedImages.find(key);
		if (itr != this->_processedImages.end())
		{
			vec(uint)* sel_buffer = itr->second;
			rw::Pos3i size;
			size.x = this->_width;
			size.y = this->_height;
			size.z = this->_depth;
			BinaryImage rimg(sel_buffer,size);
			rimg._blackVoxels = 0;
			return(rimg);
		}
		else
		{
			return(BinaryImage());
		}
	}

	int BinaryImage::Length()  const
	{
		return(this->_width*this->_height*this->_depth);
	}

	BinaryImage& BinaryImage::operator=(const BinaryImage& img)
	{
		if (this->_buffer)
		{
			delete this->_buffer;
		}
		if (img._buffer)
		{
			this->_buffer = new vec(uint)(*img._buffer);
		}
		else
		{
			this->_buffer = 0;
		}
		map<int, vec(uint)*>::iterator itr = this->_processedImages.begin();
		while (itr != this->_processedImages.end())
		{
			delete itr->second;
			++itr;
		}
		this->_processedImages.clear();
		this->_sharedBuffer = false;
		this->_width = img._width;
		this->_height = img._height;
		this->_depth = img._depth;
		this->_blackVoxels = img._blackVoxels;
		return(*this);
	}

	BinaryImage::~BinaryImage()
	{
		if ((this->_buffer) && (!this->_sharedBuffer))
		{
			delete this->_buffer;
		}
		map<int, vec(uint)*>::iterator itr = this->_processedImages.begin();
		while (itr != this->_processedImages.end())
		{
			delete itr->second;
			++itr;
		}
		this->_processedImages.clear();
	}

	void BinaryImage::Create(int width, int height, int depth)
	{
		this->_width = width;
		this->_height = height;
		this->_depth = depth;
		int length = ((width >> 2) + 1)*((height >> 2) + 1)*((depth >> 2) + 1);
		if (this->_buffer)
		{
			delete this->_buffer;
		}
		this->_buffer = new vec(uint)(2 * length, 0);
		this->_blackVoxels = 0;
	}

	void BinaryImage::Add_Layer(const BinaryImage::ImageAdapter& img, int depth)
	{
		tbb::spin_mutex* mtx = new tbb::spin_mutex[4];
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)img.Height(), BCHUNK_SIZE), [this, mtx, &img, depth](const tbb::blocked_range<int>& b)
		{
			for (int y = b.begin(); y < b.end(); ++y)
			{
				for (int x = 0; x < (int)img.Width(); ++x)
				{
					rw::Pos3i pos;
					pos.x = x;
					pos.y = y;
					pos.z = depth;
					if (img(pos.x, pos.y).Red() > 0)
					{
						mtx[x & 0x03].lock();
						(*this)(pos, 255);
						mtx[x & 0x03].unlock();
					}
					else
					{
						mtx[x & 0x03].lock();
						(*this)(pos, 0);
						mtx[x & 0x03].unlock();
					}
				}
			}
		});
	}

	void BinaryImage::Clear(int depth)
	{
		tbb::spin_mutex* mtx = new tbb::spin_mutex[32];
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)this->_height, BCHUNK_SIZE), [this, mtx, depth](const tbb::blocked_range<int>& b)
		{
			for (int y = b.begin(); y < b.end(); ++y)
			{
				for (int x = 0; x < (int)this->_width; ++x)
				{
					rw::Pos3i pos;
					pos.x = x;
					pos.y = y;
					pos.z = depth;
					if ((*this)(pos) > 0)
					{
						mtx[y % 32].lock();
						(*this)(pos, 0);
						mtx[y % 32].unlock();
					}
				}
			}
		});
	}

	void BinaryImage::Create_Border()
	{
		if (this->_state->Defined())
		{
			rw::BinaryImageBorderCreator* bc = new BinaryImageBorderCreator(*this->_state);
			delete this->_state;
			this->_state = bc;
			this->_state->Execute();
		}
	}

	uint BinaryImage::operator()(const rw::Pos3i& pos, int v)
	{
		uint b = (pos.x >> 2) + (pos.y >> 2)*((this->_width >> 2) + 1)
			+ (pos.z >> 2)*((this->_width >> 2) + 1)*((this->_height >> 2) + 1);
		uint pz = pos.z - ((pos.z >> 2) << 2);
		uint i = 2 * b + (pz >> 1);
		uint rr = 0;
		if (v > 0)
		{
			(*this->_buffer)[i] = (*this->_buffer)[i]
				| (0x01 << (pos.x - ((pos.x >> 2) << 2) +
				((pos.y - ((pos.y >> 2) << 2)) << 2) + ((pz & 0x01) << 4)));
			rr = 1;
		}
		else
		{
			(*this->_buffer)[i] = (*this->_buffer)[i] &
				(~(0x01 << ((pos.x - ((pos.x >> 2) << 2)) +
				(((pos.y - ((pos.y >> 2) << 2)) << 2) + ((pz & 0x01) << 4)))));
		}
		return(rr);
	}

	uint BinaryImage::operator()(const rw::Pos3i& pos) const
	{
		uint b = (pos.x >> 2) + (pos.y >> 2)*((this->_width >> 2) + 1)
			+ (pos.z >> 2)*((this->_width >> 2) + 1)*((this->_height >> 2) + 1);
		uint pz = pos.z - ((pos.z >> 2) << 2);
		return ((*this->_buffer)[2 * b + (pz >> 1)] & (0x01 << (pos.x - ((pos.x >> 2) << 2) +
			((pos.y - ((pos.y >> 2) << 2)) << 2) + ((pz & 0x01) << 4))));
	}


	void BinaryImage::Open(BinaryImage::ProgressAdapter* pgdlg)
	{
		map<int, vec(uint)*>::iterator itr = this->_processedImages.begin();
		while (itr != this->_processedImages.end())
		{
			delete itr->second;
			++itr;
		}
		this->_processedImages.clear();
		this->_poreMap.clear();
		int diam = 1;
		if (pgdlg)
		{
			pgdlg->Update(string("Finding pore surfaces"));
		}
		this->Create_Border();
		BinaryImageBorderCreator* bc = static_cast<BinaryImageBorderCreator*>(this->_state);
		BinaryImageOpener* oc = new BinaryImageOpener(*bc);
		delete bc;
		this->_state = oc;
		bool r = true;
		int it = 0;
		while ((diam == 1)||((!oc->Black_Voxels_Covered()) && (it < 100)))
		{
			oc->Set_Diameter(diam);
			oc->Execute(pgdlg);
			++it;
			++diam;
			++diam;
		}
		oc->Release_GPU();
	}

		
	void BinaryImage::Clear_Null_Processed_Images()
	{
		set<int> to_erase;
		map<int, vec(uint)*>::iterator itr = this->_processedImages.begin();
		while (itr != this->_processedImages.end())
		{
			if (!itr->second)
			{
				to_erase.insert(itr->first);
			}
			++itr;
		}
		set<int>::iterator ii = to_erase.begin();
		while (ii != to_erase.end())
		{
			this->_processedImages.erase(*ii);
			++ii;
		}
	}

	void BinaryImage::Save_File(const string& filename) const
	{
		uint black_voxels = this->Black_Voxels();
		file::Binary file(WRITE);
		file.Open(filename);
		file.Write((uchar)0);
		file.Write((int)this->_width);
		file.Write((int)this->_height);
		file.Write((int)this->_depth);
		file.Write((uint)black_voxels);
		file.Write((uint)this->_buffer->size() / 2);
		for (int k = 0; k < this->_buffer->size(); ++k)
		{
			file.Write((uint)(*this->_buffer)[k]);
		}
		if (this->_processedImages.size() > 0)
		{
			file.Write((uint)this->_processedImages.size());
			map<int, vec(uint)*>::const_iterator itr = this->_processedImages.begin();
			while (itr != this->_processedImages.end())
			{
				file.Write((int)itr->first);
				vec(uint)* proc = itr->second;
				for (int k = 0; k < proc->size(); ++k)
				{
					file.Write((uint)(*proc)[k]);
				}
				++itr;
			}
		}
		else
		{
			file.Write((uint)0);
		}
		if (this->_poreMap.size() > 0)
		{
			file.Write((uint)this->_poreMap.size());
			map<int, Pore_Voxel>::const_iterator c_itr = this->_poreMap.begin();
			while (c_itr != this->_poreMap.end())
			{
				file.Write((int)c_itr->first);
				file.Write((char)c_itr->second.dist_min);
				file.Write((char)c_itr->second.diam_max);
				file.Write((int)c_itr->second.cluster);
				++c_itr;
			}
		}
		else
		{
			file.Write((uint)0);
		}
		file.Close();
	}

	void BinaryImage::Load_File(const string& filename, BinaryImage::ProgressAdapter* pgdlg)
	{
		pgdlg->Set_Range(4);
		file::Binary file(READ);
		file.Open(string(filename.c_str()));
		this->_poreMap.clear();
		file.Read_UChar();
		this->_width = file.Read_Int();
		this->_height = file.Read_Int();
		this->_depth = file.Read_Int();
		this->_blackVoxels = file.Read_UInt();
		uint length = file.Read_UInt();
		if (length == ((this->_width >> 2) + 1)*((this->_height >> 2) + 1)*((this->_depth >> 2) + 1))
		{
			if (pgdlg)
			{
				pgdlg->Update(1, "Loading image");
			}
			this->_buffer = new vec(uint)(2 * length, 0);
			for (int k = 0; k < this->_buffer->size(); ++k)
			{
				(*this->_buffer)[k] = file.Read_UInt();
			}
			uint sproc = file.Read_UInt();
			if (sproc > 0)
			{
				if (pgdlg)
				{
					pgdlg->Update(2, "Loading processed images");
				}

				for (uint k = 0; k < sproc; ++k)
				{
					int key = file.Read_Int();
					vec(uint)* proc = new vec(uint)(2 * length, 0);
					for (uint s = 0; s < 2 * length; ++s)
					{
						(*proc)[s] = file.Read_UInt();
					}
					this->_processedImages[key] = proc;
				}
			}
		}
		if (pgdlg)
		{
			pgdlg->Update(3, "Loading pore map");
		}
		uint map_size = file.Read_UInt();
		if (map_size > 0)
		{
			for (uint k = 0; k < map_size; ++k)
			{
				int idx = file.Read_Int();
				char rad_min = file.Read_Char();
				char rad_max = file.Read_Char();
				Pore_Voxel r;
				r.dist_min = rad_min;
				r.diam_max = rad_max;
				r.cluster = file.Read_Int();
				this->_poreMap[idx] = r;
			}
		}
		file.Close();
		this->_state = new BinaryImageExecutor(this);		
		this->_state = new BinaryImageExecutor(this);		
	}

	void BinaryImage::Get_File_Image_Size(int& x, int& y, int& z, int& black, const string& filename)
	{
		file::Binary file(READ);
		file.Open(string(filename.c_str()));
		file.Read_Char();
		x = file.Read_Int();
		y = file.Read_Int();
		z = file.Read_Int();
		black = (int)file.Read_UInt();
		file.Close();
	}

	void BinaryImage::Layer(BinaryImage::ImageAdapter& img, int id, rw::RGBColor pore_color, rw::RGBColor solid_color) const
	{
		img.Reserve_Memory(this->_width, this->_height);
		tbb::spin_mutex mtx;
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)img.Height(), SCHUNK_SIZE),
			[this, &mtx, &img, id, pore_color, solid_color](const tbb::blocked_range<int>& b)
		{
			for (int y = b.begin(); y < b.end(); ++y)
			{
				for (int x = 0; x < (int)img.Width(); ++x)
				{
					rw::Pos3i pp;
					pp.x = x;
					pp.y = y;
					pp.z = id;
					uint v = (*this)(pp);
					if (v == 0)
					{
						int f_r = min((int)(y*pore_color.Red() / (img.Height()) + pore_color.Red()), 255);
						int f_g = min((int)(y*pore_color.Green() / (img.Height()) + pore_color.Green()), 255);
						int f_b = min((int)(y*pore_color.Blue() / (img.Height()) + pore_color.Blue()), 255);
						RGBColor c((uchar)f_r, (uchar)f_g, (uchar)f_b);
						img(pp.x, pp.y, c);
					}
					else
					{
						int f_r = min((int)((img.Height() - y)*solid_color.Red() / (10 * img.Height()) + solid_color.Red()), 255);
						int f_g = min((int)((img.Height() - y)*solid_color.Green() / (10 * img.Height()) + solid_color.Green()), 255);
						int f_b = min((int)((img.Height() - y)*solid_color.Blue() / (10 * img.Height()) + solid_color.Blue()), 255);
						RGBColor c((uchar)f_r, (uchar)f_g, (uchar)f_b);
						img(pp.x, pp.y, c);
					}
				}
			}
		});
	}

	BinaryImage BinaryImage::Invert() const
	{
		BinaryImage rimg;
		rimg._buffer = new vec(uint)(this->_buffer->size());
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)this->_buffer->size(), BCHUNK_SIZE), [this, &rimg](const tbb::blocked_range<int>& b)
		{
			uint mask = 0xFFFFFFFF;
			for (int i = b.begin(); i < b.end(); ++i)
			{
				(*rimg._buffer)[i] = (*this->_buffer)[i] ^ mask;
			}
		});
		rimg._width = this->_width;
		rimg._height = this->_height;
		rimg._depth = this->_depth;
		rimg._blackVoxels = rimg._width*rimg._height*rimg._depth - this->Black_Voxels();
		return(rimg);
	}

	void BinaryImage::Maximal_Spheres(vector<int>& radius) const
	{
		radius.clear();
		map<int, vec(uint)*>::const_iterator itr = this->_processedImages.begin();
		while (itr != this->_processedImages.end())
		{
			radius.push_back(itr->first);
		}
	}

	void BinaryImage::Build_Pore_Map()
	{
		this->_poreMap.clear();
		int length = this->_width*this->_height*this->_depth;
		tbb::spin_mutex mtx;
		tbb::parallel_for(tbb::blocked_range<int>(0, length, BCHUNK_SIZE), [this, &mtx](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, this->_width, this->_height, this->_depth, pp);
				if ((*this)(pp) == 0)
				{
					mtx.lock();
					Pore_Voxel r;
					r.cluster = -1;
					r.dist_min = 1;
					r.diam_max = 1;
					this->_poreMap.insert(std::pair<int, Pore_Voxel>(i, r));
					mtx.unlock();
				}
			}
		});
	}

	map<uint, RGBColor> BinaryImage::Build_Diameter_Color_Map()
	{
		map<uint, RGBColor> rmap;
		map<int, vec(uint)*>::const_reverse_iterator ritr = this->_processedImages.rbegin();
		int diam = (int)ritr->first + 1;
		int AR = 80;
		int AG = 80;
		int AB = 255;

		int BR = 255;
		int BG = 50;
		int BB = 50;

		int ik = 0;
		int ick = (int)this->_processedImages.size() / 2 + 1;
		bool sr = true;
		while (ritr != this->_processedImages.rend())
		{
			int i = ritr->first;
			int b = 0;
			int r = 0;
			int g = 0;
			if (sr)
			{
				b = (BB * (i + 1 - diam / 2) + AB * (diam - i - 1)) / (diam / 2);
				r = (BR * (i + 1 - diam / 2) + AR * (diam - i - 1)) / (diam / 2);
				g = (BG * (i + 1 - diam / 2) + AG * (diam - i - 1)) / (diam / 2);
			}
			else
			{
				b = (BB * (i-1) + AB * (diam/2 - i+1)) / (diam / 2);
				r = (BR * (i-1) + AR * (diam/2 - i+1)) / (diam / 2);
				g = (BG * (i-1) + AG * (diam/2 - i+1)) / (diam / 2);
			}

			b = min((int)255, b);
			b = max((int)0, b);
			g = min((int)255, g);
			g = max((int)0, g);
			r = min((int)255, r);
			r = max((int)0, r);
			RGBColor color((uchar)r, (uchar)g, (uchar)b);
			rmap[i] = color;
			++ritr;
			++ik;
			if (ik == ick)
			{
				BR = AR;
				BG = AG;
				BB = AB;
				AR = 255;
				AG = 255;
				AB = 25;
				sr = false;
			}
		}
		this->_colorMap = rmap;
		return(rmap);
	}

	void BinaryImage::Pore_Size_Distribution_Layer(BinaryImage::ImageAdapter& img, int id, bool maximal, const map<uint, RGBColor>& rad_color_map, RGBColor solid_color)
	{
		img.Reserve_Memory(this->_width, this->_height);
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)img.Height(), SCHUNK_SIZE),
			[this, &img, id, &rad_color_map, solid_color, maximal](const tbb::blocked_range<int>& b)
		{
			for (int y = b.begin(); y < b.end(); ++y)
			{
				for (int x = 0; x < (int)img.Width(); ++x)
				{
					rw::Pos3i pp;
					pp.x = x;
					pp.y = y;
					pp.z = id;
					uint v = (*this)(pp);
					if (v > 0)
					{
						img(pp.x, pp.y, solid_color);
					}
					else
					{
						int idx = rw::Pos3i::Pos3i_To_Int(pp, this->_width, this->_height, this->_depth);
						map<int, Pore_Voxel>::const_iterator ritr = this->_poreMap.find(idx);
						if (ritr != this->_poreMap.end())
						{
							int v = (int)ritr->second.diam_max;
							if (!maximal)
							{
								v = (int)ritr->second.dist_min;
							}
							RGBColor rad_color = rad_color_map.find(v)->second;
							if ((ritr->second.diam_max == ritr->second.dist_min)&&(ritr->second.dist_min > 1))
							{
							//	rad_color = RGBColor(255, 0, 0);
							}
							img(pp.x, pp.y, rad_color);
						}
						else
						{
							int f_r = min((int)(y*solid_color.Red() / (img.Height()) + solid_color.Red()), 255);
							int f_g = min((int)(y*solid_color.Green() / (img.Height()) + solid_color.Green()), 255);
							int f_b = min((int)(y*solid_color.Blue() / (img.Height()) + solid_color.Blue()), 255);
							RGBColor color((uchar)f_r, (uchar)f_g, (uchar)f_b);
							img(pp.x, pp.y, color);
						}
					}
				}
			}
		});
	}

	void BinaryImage::Pore_Size_Distribution_Layer(BinaryImage::ImageAdapter& img, int id, bool maximal)
	{
		return(this->Pore_Size_Distribution_Layer(img, id, maximal, this->Build_Diameter_Color_Map()));
	}

	int BinaryImage::Depth() const
	{
		return(this->_depth);
	}

	int BinaryImage::Width() const
	{
		return(this->_width);
	}

	int BinaryImage::Height() const
	{
		return(this->_height);
	}

	uint BinaryImage::Black_Voxels() const
	{
		if (this->_blackVoxels == 0)
		{
			uint black_voxels = 0;
			int length = this->_width*this->_height*this->_depth;
			tbb::spin_mutex mtx;
			tbb::parallel_for(tbb::blocked_range<int>(0, (int)length, max((int)length / 128, BCHUNK_SIZE)),
				[this, &mtx, &black_voxels](const tbb::blocked_range<int>& b)
			{
				for (int i = b.begin(); i < b.end(); ++i)
				{
					rw::Pos3i pp;
					Pos3i::Int_To_Pos3i(i, this->_width, this->_height, this->_depth, pp);
					uint v = (*this)(pp);
					if (v == 0)
					{
						mtx.lock();
						++black_voxels;
						mtx.unlock();
					}
				}
			});
			return(black_voxels);
		}
		else
		{
			return(this->_blackVoxels);
		}
	}

	bool BinaryImage::Opened() const
	{
		return((int)this->_processedImages.size() > 0);
	}

	void BinaryImage::Count_Spheres(map<int, Freq_Rad>& distribution) const
	{
		distribution.clear();
		int length = this->_width*this->_height*this->_depth;
		tbb::spin_mutex mtx;
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)length, max((int)length / 128, BCHUNK_SIZE)),
			[this, &mtx, &distribution](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, this->_width, this->_height, this->_depth, pp);
				if ((*this)(pp) == 0)
				{
					Freq_Rad fr;
					fr.freq_rad_max = 0;
					fr.freq_rad_min = 0;
					fr.freq_rad_eq = 0;
					map<int, Pore_Voxel>::const_iterator itr = this->_poreMap.find(i);
					Pore_Voxel r = itr->second;
					mtx.lock();
					map<int, Freq_Rad>::iterator ii = distribution.find(r.diam_max);
					if (ii == distribution.end())
					{
						fr.freq_rad_max = 1.0f;
						distribution[r.diam_max] = fr;
					}
					else
					{
						ii->second.freq_rad_max = ii->second.freq_rad_max + 1.0f;
					}
					ii = distribution.find(r.dist_min);
					if (ii == distribution.end())
					{
						fr.freq_rad_min = 1.0f;
						distribution[r.dist_min] = fr;
					}
					else
					{
						ii->second.freq_rad_min = ii->second.freq_rad_min + 1.0f;
					}
					if (r.diam_max == r.dist_min)
					{
						ii->second.freq_rad_eq = ii->second.freq_rad_eq + 1.0f;
					}
					mtx.unlock();
				}
			}
		});
		float den = (float)this->_blackVoxels;
		map<int, Freq_Rad>::iterator fi = distribution.begin();
		while (fi != distribution.end())
		{
			fi->second.freq_rad_max = fi->second.freq_rad_max / den;
			fi->second.freq_rad_min = fi->second.freq_rad_min / den;
			fi->second.freq_rad_eq = fi->second.freq_rad_eq / den;
			++fi;
		}
	}

	void BinaryImage::Cluster_Pores(BinaryImage::ProgressAdapter* pgdlg)
	{
		BinaryImageClusterer* bc = new BinaryImageClusterer(*this->_state);
		//BinaryImageWaterShedClusterer* bc = new BinaryImageWaterShedClusterer(*this->_state);
		delete this->_state;
		this->_state = bc;
		bc->Execute(pgdlg);
		this->Cluster_PSD_File("Cluster.csv",1,100);
	}


	rw::BinaryImage BinaryImage::Sub(int bx, int by, int bz, int dx, int dy, int dz) const
	{
		rw::BinaryImage r;
		r.Create(dx, dy, dz);
		tbb::spin_mutex mtx[8];
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)dz, max(dz / 128, BCHUNK_SIZE)),
			[this, bx, by, bz, dx, dy, dz, &mtx, &r](const tbb::blocked_range<int>& b)
		{
			for (int z = b.begin(); z < b.end(); ++z)
			{
				for (int x = 0; x < dx; ++x)
				{
					for (int y = 0; y < dy; ++y)
					{
						rw::Pos3i pp;
						pp.x = bx + x;
						pp.y = by + y;
						pp.z = bz + z;
						int v = (*this)(pp);
						if (v > 0)
						{
							rw::Pos3i pm;
							pm.x = x;
							pm.y = y;
							pm.z = z;
							mtx[z % 8].lock();
							r(pm, v);
							mtx[z % 8].unlock();
						}
					}
				}
			}
		});
		return(r);
	}

	void BinaryImage::Denoise(int diam, BinaryImage::ProgressAdapter* pgdlg)
	{
		if (!this->_state)
		{
			this->_state = new BinaryImageExecutor(this);
		}
		BinaryImageDenoiser* denoiser = new BinaryImageDenoiser(*this->_state);
		denoiser->Set_Diameter(diam);
		delete this->_state;
		this->_state = denoiser;
		denoiser->Execute(pgdlg);
	}

	void BinaryImage::Create_Pore_Color_Map(map<uint, RGBColor>& color_map)
	{
		std::mt19937 rnd;
		rnd.seed((uint)time(0));
		color_map.clear();
		tbb::spin_mutex mtx;
		int length = this->_width*this->_height*this->_depth;
		auto Pick_Random = [&rnd]()
		{
			std::uniform_real_distribution<float> urd;
			float s = urd(rnd);
			uint rs = (uint)(100.0f * s);
			return(rs + 100);
		};
		tbb::parallel_for(tbb::blocked_range<uint>(0, (uint)length, max(length / 128, BCHUNK_SIZE)),
			[this, &mtx, &color_map, &rnd, &Pick_Random](const tbb::blocked_range<uint>& b)
		{
			for (uint i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, this->_width, this->_height, this->_depth, pp);
				if ((*this)(pp) == 0)
				{
					map<int, BinaryImage::Pore_Voxel>::const_iterator sitr =
						this->_poreMap.find(i);
					if (sitr != this->_poreMap.end())
					{
						map<uint, RGBColor>::iterator itr = color_map.find(sitr->second.cluster);
						if (itr == color_map.end())
						{
							mtx.lock();
							RGBColor sel_color((uchar)Pick_Random(),
								(uchar)Pick_Random(), (uchar)Pick_Random());
							color_map[sitr->second.cluster] = sel_color;
							mtx.unlock();
						}
					}
				}
			}
		});
	}

	void BinaryImage::Pore_Segmentation_Layer(ImageAdapter& img, int id, RGBColor solid_color)
	{
		if (this->_colorMap.size() == 0)
		{
			this->Create_Pore_Color_Map(this->_colorMap);
		}
		img.Reserve_Memory(this->_width, this->_height);
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)img.Height(), SCHUNK_SIZE),
			[this, &img, id, solid_color](const tbb::blocked_range<int>& b)
		{
			for (int y = b.begin(); y < b.end(); ++y)
			{
				for (int x = 0; x < (int)img.Width(); ++x)
				{
					rw::Pos3i pp;
					pp.x = x;
					pp.y = y;
					pp.z = id;
					uint v = (*this)(pp);
					if (v > 0)
					{
						img(pp.x, pp.y, solid_color);
					}
					else
					{
						int idx = rw::Pos3i::Pos3i_To_Int(pp, this->_width, this->_height, this->_depth);
						map<int, BinaryImage::Pore_Voxel>::const_iterator sitr =
							this->_poreMap.find(idx);
						if (sitr != this->_poreMap.end())
						{
							RGBColor color;
							map<uint, RGBColor>::const_iterator itr = this->_colorMap.find(sitr->second.cluster);
							if (itr != this->_colorMap.end())
							{
								color = itr->second;
							}
							if (sitr->second.diam_max == sitr->second.dist_min)
							{
								//color = RGBColor(255, 0, 0);
							}
							img(x, y, color);
						}
					}
				}
			}
		});
	}

	void BinaryImage::Create_Pore_S_Structure(map<int, float2>& structure, const vec(int)& border) const
	{
		vec(rw::Pos3i) mask;
		rw::Pos3i scp;
		scp.x = -1;
		scp.y = 0;
		scp.z = 0;
		mask.push_back(scp);
		scp.x = 1;
		scp.y = 0;
		scp.z = 0;
		mask.push_back(scp);
		scp.x = 0;
		scp.y = -1;
		scp.z = 0;
		mask.push_back(scp);
		scp.x = 0;
		scp.y = 1;
		scp.z = 0;
		mask.push_back(scp);
		scp.x = 0;
		scp.y = 0;
		scp.z = -1;
		mask.push_back(scp);
		scp.x = 0;
		scp.y = 0;
		scp.z = 1;
		mask.push_back(scp);

		tbb::spin_mutex mtx;
		float s_factor = 1.0f;
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)border.size(), BCHUNK_SIZE),
			[this, &mtx, &structure, s_factor, &border, mask](const tbb::blocked_range<int>& b)
		{
			for (int k = b.begin(); k < b.end(); ++k)
			{
				int idpos = border[k];
				rw::Pos3i c_pos;
				rw::Pos3i::Int_To_Pos3i(idpos, this->Width(), this->Height(), this->Depth(), c_pos);
				for (int i = 0; i < mask.size(); ++i)
				{
					rw::Pos3i position_to_check = c_pos + mask[i];
					int id_position_to_check =
						rw::Pos3i::Pos3i_To_Int(position_to_check, this->Width(), this->Height(), this->Depth());
					map<int, rw::BinaryImage::Pore_Voxel>::const_iterator itr = this->_poreMap.find(id_position_to_check);
					if (itr != this->_poreMap.end())
					{
						const rw::BinaryImage::Pore_Voxel& pv = itr->second;
						map<int, float2>::iterator map_itr = structure.find(pv.cluster);
						if (map_itr == structure.end())
						{
							float2 ss;
							ss.y = s_factor;
							ss.x = 0.0f;
							mtx.lock();
							structure.insert(std::pair<int, float2>(pv.cluster, ss));
							mtx.unlock();
						}
						else
						{
							mtx.lock();
							map_itr->second.y = map_itr->second.y + s_factor;
							mtx.unlock();
						}
					}
				}
			}
		});
	}

	void BinaryImage::Create_Pore_VS_Structure(map<int, float2>& structure) const
	{
	
		structure.clear();
		rw::BinaryImageBorderCreator* bc = new BinaryImageBorderCreator(*this->_state);
		bc->Execute();
		const vec(int)& border = bc->Corner_Border();
		this->Create_Pore_S_Structure(structure, border);
		const vec(int)& s_border = bc->Surface_Border();
		this->Create_Pore_S_Structure(structure, s_border);

		int length = this->_width*this->_height*this->_depth;
		tbb::spin_mutex mtx;
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)length, max((int)length / 128, BCHUNK_SIZE)),
			[this, &mtx,&structure](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(i, this->_width, this->_height, this->_depth, pp);
				if ((*this)(pp) == 0)
				{
					map<int,rw::BinaryImage::Pore_Voxel>::const_iterator itr = this->_poreMap.find(i);
					if (itr != this->_poreMap.end())
					{
						const rw::BinaryImage::Pore_Voxel& pv = itr->second;
						map<int, float2>::iterator structure_itr = structure.find(pv.cluster);
						if (structure_itr != structure.end())
						{
							mtx.lock();
							structure_itr->second.x = structure_itr->second.x + 1.0f;
							mtx.unlock();
						}
					}
				}
			}
		});
	}

	void BinaryImage::Create_VS_Distribution(map<float, float>& histogram) const
	{
		map<int, float2> clustered_VS;
		this->Create_Pore_VS_Structure(clustered_VS);
		map<int, float2>::const_iterator itr = clustered_VS.begin();
		float vt = 0.0f;
		while (itr != clustered_VS.end())
		{
			float2 vs = itr->second;
			float vsr = (3.0f*vs.x) / (2.0f*vs.y);
			float r = 3 * vsr;
			vt = vt + vs.x;
			map<float, float>::iterator h_itr = histogram.upper_bound(r);
			if (h_itr != histogram.begin())
			{
				--h_itr;
				h_itr->second = h_itr->second + vs.x;
			}
			if (h_itr == histogram.end())
			{
				map<float, float>::reverse_iterator rh_itr = histogram.rbegin();
				rh_itr->second = rh_itr->second + vs.x;
			}
			++itr;
		}

		map<float, float>::iterator h_itr = histogram.begin();
		while (h_itr != histogram.end())
		{
			h_itr->second = h_itr->second / vt;
			++h_itr;
		}
	}
	
	void BinaryImage::Cluster_PSD_File(const string& filename, float step, float max_val) const
	{
		map<float, float> psd;
		float b = 0.0f;
		while (b < max_val)
		{
			psd[b] = 0.0f;
			b = b + step;
		}
		this->Create_VS_Distribution(psd);
		std::ofstream file(filename);
		file << "Radius,V\n";
		map<float, float>::iterator i = psd.begin();
		while (i != psd.end())
		{
			file << i->first << "," << i->second << "\n";
			++i;
		}
		file.close();		
	}
}
