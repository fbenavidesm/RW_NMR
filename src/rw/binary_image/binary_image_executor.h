#ifndef BINARY_IMAGE_EXECUTOR
#define BINARY_IMAGE_EXECUTOR

#include "binary_image.h"
#include "math_la/file/binary.h"
#include <amp.h>
namespace rw
{
	class BinaryImageExecutor
	{
	private:
		BinaryImage* _image;
		bool _morphed;
		bool _segmented;
		bool _defined;
		bool _border;
		bool _open;
		bool _denoised;
	protected:
		rw::BinaryImage& Image();
		void Save_To_File(file::Binary& file);
		void Load_From_File(file::Binary& file);
		void Set_Defined(bool t);
		void Set_Morphed(bool t);
		void Set_Segmented(bool t);
		void Set_Open(bool t);
		void Set_Border(bool t);
		void Set_Denoised(bool t);
		map<int, rw::BinaryImage::Pore_Voxel>& Pore_Map();
		vec(uint)& Image_Buffer();
		vec(uint)& Processed_Image(int rad);
		void Set_Processed_Image(vec(uint)* buffer, int rad);
		void Rebuild_Pore_Map();
		static uint Accessor_Read(vec(uint)& vtx, const rw::Pos3i& pp, const rw::Pos3i& size);
		void Init();
	public:
		BinaryImageExecutor(rw::BinaryImage* image);
		BinaryImageExecutor(BinaryImageExecutor& executor);
		virtual ~BinaryImageExecutor();
		bool Morphed() const;
		bool Segmented() const;
		bool Defined() const;
		bool Border() const;
		bool Open() const;
		bool Denoised() const;
		virtual void Execute(BinaryImage::ProgressAdapter* pgdlg = 0);
		rw::Pos3i Size_3D() const;
		virtual uint Number_Of_Steps() const;
		int Max_Radius() const;

		static concurrency::array<uint, 1>* Create_GPU_Texture(const rw::BinaryImage& img);

		/**
		* Reads the GPU texture according to the binary image format
		* @param vtx GPU texture
		* @param pp Posiition to read from the texture
		* @param size Three dimension size of the texture
		*/
		static uint Accesor_Read(const concurrency::array<uint, 1>& vtx, const rw::Pos3i& pp, const rw::Pos3i& size) GPUP;

	};

	inline uint BinaryImageExecutor::Accesor_Read(const concurrency::array<uint, 1>& vtx, const rw::Pos3i& pp, const rw::Pos3i& size) GPUP
	{
		uint b = (pp.x >> 2) + (pp.y >> 2)*((size.x >> 2) + 1)
			+ (pp.z >> 2)*((size.x >> 2) + 1)*((size.y >> 2) + 1);
		uint pz = pp.z - ((pp.z >> 2) << 2);
		uint iv = vtx[2 * b + (pz >> 1)];
		return (iv & (0x01 << ((pp.x - ((pp.x >> 2) << 2)) +
			((pp.y - ((pp.y >> 2) << 2)) << 2) + ((pz & 0x01) << 4))));
	}

	inline concurrency::array<uint, 1>* BinaryImageExecutor::Create_GPU_Texture(const rw::BinaryImage& img)
	{		
		concurrency::array<uint, 1>* r = new concurrency::array<uint, 1>((int)img._buffer->size(),img._buffer->begin());
		return(r);
	}

	inline vec(uint)& BinaryImageExecutor::Image_Buffer()
	{
		return(*this->_image->_buffer);
	}

	inline void BinaryImageExecutor::Set_Defined(bool t)
	{
		this->_defined = t;
	}

	inline BinaryImageExecutor::~BinaryImageExecutor()
	{
		this->_image = 0;
	}

	inline void BinaryImageExecutor::Set_Morphed(bool t)
	{
		this->_morphed = t;
	}

	inline void BinaryImageExecutor::Set_Segmented(bool t)
	{
		this->_segmented = t;
	}

	inline void BinaryImageExecutor::Set_Denoised(bool t)
	{
		this->_denoised = t;
	}

	inline void BinaryImageExecutor::Execute(BinaryImage::ProgressAdapter* pgdlg)
	{
		this->_defined = true;
	}

	inline BinaryImageExecutor::BinaryImageExecutor(rw::BinaryImage* image)
	{
		this->_denoised = false;
		this->_open = false;
		this->_border = false;
		this->_image = image;
		this->_morphed = false;
		this->_segmented = false;
		if ((this->_image->_buffer) && (this->_image->_buffer->size() > 0))
		{
			this->_defined = true;
		}
		else
		{
			this->_defined = false;
		}
	}

	inline map<int, rw::BinaryImage::Pore_Voxel>& BinaryImageExecutor::Pore_Map()
	{
		return(this->_image->_poreMap);
	}

	inline BinaryImageExecutor::BinaryImageExecutor(BinaryImageExecutor& executor)
	{
		this->_image = executor._image;
		this->_defined = executor._defined;
		this->_border = executor._border;
		this->_denoised = executor._denoised;
		this->_morphed = executor._morphed;
		this->_open = executor._open;
		this->_segmented = executor._segmented;
	}

	inline uint BinaryImageExecutor::Number_Of_Steps() const
	{
		return(1);
	}

	inline rw::BinaryImage& BinaryImageExecutor::Image() 
	{
		return(*this->_image);
	}

	inline void BinaryImageExecutor::Save_To_File(file::Binary& file)
	{
		file.Write((bool)this->Defined());
		file.Write((bool)this->Morphed());
		file.Write((bool)this->Segmented());
	}

	inline void BinaryImageExecutor::Load_From_File(file::Binary& file)
	{
		this->_defined = file.Read_Bool();
		this->_morphed = file.Read_Bool();
		this->_segmented = file.Read_Bool();
	}

	inline bool BinaryImageExecutor::Morphed() const
	{
		return(this->_morphed);
	}

	inline bool BinaryImageExecutor::Segmented() const
	{
		return(this->_segmented);
	}

	inline bool BinaryImageExecutor::Defined() const
	{
		return(this->_defined);
	}

	inline bool BinaryImageExecutor::Border() const
	{
		return(this->_border);
	}

	inline bool BinaryImageExecutor::Open() const
	{
		return(this->_open);
	}

	inline void BinaryImageExecutor::Set_Open(bool t)
	{
		this->_open = t;
	}

	inline void BinaryImageExecutor::Set_Border(bool t)
	{
		this->_border = t;
	}

	inline bool BinaryImageExecutor::Denoised() const
	{
		return(this->_denoised);
	}

	inline void BinaryImageExecutor::Rebuild_Pore_Map()
	{
		this->_image->Build_Pore_Map();
	}

	inline vec(uint)& BinaryImageExecutor::Processed_Image(int rad)
	{
		return(*this->_image->_processedImages[rad]);
	}

	inline rw::Pos3i BinaryImageExecutor::Size_3D() const
	{
		rw::Pos3i s;
		s.x = this->_image->_width;
		s.y = this->_image->_height;
		s.z = this->_image->_depth;
		return(s);
	}

	inline void BinaryImageExecutor::Init()
	{
		this->_image->Init();
	}

	inline void BinaryImageExecutor::Set_Processed_Image(vec(uint)* buffer, int rad)
	{
		map<int, vec(uint)*>::iterator itr = this->_image->_processedImages.find(rad);
		if (itr != this->_image->_processedImages.end())
		{
			delete itr->second;
			this->_image->_processedImages.erase(rad);
		}
		this->_image->_processedImages[rad] = buffer;
	}

	inline uint BinaryImageExecutor::Accessor_Read(vec(uint)& vtx, const rw::Pos3i& pp, const rw::Pos3i& size) 
	{
		return(rw::BinaryImage::Accesor_Read(vtx, pp, size));
	}

	inline int BinaryImageExecutor::Max_Radius() const
	{
		int rad = this->_image->_processedImages.rbegin()->first;
		return(rad);
	}
}

#endif
