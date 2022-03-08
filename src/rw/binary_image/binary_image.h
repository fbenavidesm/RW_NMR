#ifndef BINARY_IMAGE_H
#define BINARY_IMAGE_H

#include <map>
#include <set>
#include <string>
#include "rw/binary_image/pos3i.h"
#include "math_la/mdefs.h"
#include "math_la/file/binary.h"
#include "rgb_color.h"


#define EXPANDED 1 
#define SPHERES 2
#define CITY_BLOCK 4

namespace rw
{
	class BinaryImageExecutor;

	using std::map;
	using std::set;
	using std::string;

	/**
	* An encapsulated method to store large binary images, using only one bit per pixel. 
	* For example, a 1000x1000x1000 image will require only 120 MB to be
	* stored, which faciliates handling these large textures even in a modest GPU. 
	* This class can also handle fast morphologic operations such as Dilation 
	* and Erosion by spherical structuring elements. The opening stores a sequence
	* of opened images which can be useful to estimate a pore size distribution. 
	*/
	class BinaryImage
	{
	public:

		class ColorMap
		{
		private:
			friend class BinaryImage;
			const map<uint, RGBColor>* _colorMap;
		protected:
			const map<uint, RGBColor>& Color_Map() const;
		public:
			ColorMap();
			virtual ~ColorMap();
		};


		/**
		* This class implements an adapter to other image formats present in other windows systems.
		* The idea is to provide a generic interfase for many image formats present in other GUI's 
		* 
		*/

		class ImageAdapter
		{
		private:
			/**
			* Image width
			*/
			uint _width;
			/**
			* Image height
			*/
			uint _height;
		protected:
			/**
			* Defines image size, for external use, without reserving memory
			*/
			void Set_Size(uint width, uint height);
		public:
			ImageAdapter();
			ImageAdapter(uint width, uint height);
			uint Width() const;
			uint Height() const;
			/**
			* Dimensions image size, reserving memory
			* @param width Image width
			* @param height Image height
			*/
			virtual void Reserve_Memory(uint width, uint height) = 0;
			virtual RGBColor operator()(uint x, uint y) const = 0;	
			virtual void operator()(uint x, uint y, const RGBColor& color) = 0;
			virtual ~ImageAdapter() {};
		};

		/**
		* A class to bridge to a progress dialog for several windows systems. 
		*/
		class ProgressAdapter
		{
		public:
			/**
			* Updates progress bar 
			* @param step Progress bar position
			* @param msg Message
			*/
			virtual void Update(uint step, const string& msg) = 0;

			/**
			* Updates progress bar
			* @param msg Message
			*/
			virtual void Update(const string& msg) = 0;

			virtual void Set_Range(uint N) = 0;
		};

		/**
		* This structure characterizes every pore voxel
		*/
		struct Pore_Voxel
		{
			/**
			* Maximal sphere cluster to which the voxel belongs
			*/
			int cluster;
			/**
			* The radius of the maximal ball that contains the voxel
			*/
			char diam_max;
			/**
			* The distance from the voxel to the nearest wall
			*/
			char dist_min;	
		};

		/**
		* This structure is used to handle pore size distributions
		*/
		struct Freq_Rad
		{
			/**
			* Frequency of maximal radius for every voxel
			*/
			float freq_rad_max;

			/**
			* Frequency of minimal radius for every voxel
			*/
			float freq_rad_min;

			/**
			* Frequency of voxels where the maximal and minimal radius are equal
			*/
			float freq_rad_eq;
		};

		/**
		* An element of gauss distributions sumatory. Each element characterizes a gaussian distribution
		* with an avarage value, a standard deviation and a weight on the sumatory
		*/
		struct Gauss
		{
			/**
			* Number of elements (weight)
			*/
			float size;

			/**
			* Average value
			*/
			float avg;

			/**
			* Standard deviation
			*/
			float dev;
		};
	private:
		friend class BinaryImageExecutor;

		BinaryImageExecutor* _state;
		/**
		* A color map that is used to segment the pore space, to distinguish different pore clusters
		*/
		map<uint, RGBColor> _colorMap;

		/**
		* Texture width
		*/
		int _width;

		/**
		* Texture height
		*/
		int _height;

		/**
		* Texture depth
		*/
		int _depth;

		/**
		* The number of black voxels inside the texture. This is used to calculate porosity
		*/
		uint _blackVoxels;

		/**
		* It is true if the image buffer is shared, so its associated memory is not released on destructor
		*/
		bool _sharedBuffer; 

		/**
		* Buffer of the image
		*/
		vec(uint)* _buffer;

		/**
		* This is the vector of processed images (opened images). These images are stored after each dilation/erosion in order to preserve
		* peculairities of the pore space shapes. 
		*/
		map<int, vec(uint)*> _processedImages;


		/**
		* Pore map, characterizing every pore voxel with a Binary::Pore_Voxel structure
		*/
		map<int, Pore_Voxel> _poreMap;

	
		/**
		* Creates a pore map for the black voxels in the image
		*/
		void Build_Pore_Map();


		/**
		* Counts the spheres frequency, by counting the classified pore voxels
		*/
		void Count_Spheres(map<int,Freq_Rad>& distribution) const;


		/**
		* Clears last processed image, the image that has not been eroded because there are no remaining voxels
		* after the dilation
		*/
		void Clear_Null_Processed_Images();

		/**
		* Creates internal data structures associated to the border
		*/
		void Create_Border();

		/**
		* Reads the GPU texture according to the binary image format
		* @param vtx GPU texture
		* @param pp Posiition to read from the texture
		* @param size Three dimension size of the texture
		*/
		static uint Accesor_Read(const vec(uint)& vtx, const rw::Pos3i& pp, const rw::Pos3i& size);

		void Init();
		void Create_Pore_VS_Structure(map<int, float2>& structure) const;
		void Create_Pore_S_Structure(map<int, float2>& structure,const vec(int)& border) const;
		void Create_VS_Distribution(map<float, float>& histogram) const;
		void Create_Pore_Color_Map(map<uint, RGBColor>& color_map);
	public:
		BinaryImage();
		~BinaryImage();
		BinaryImage(const BinaryImage& img);
		/**
		* Creates a binary image using a buffer pointer. The buffer is shared between both images
		*/
		BinaryImage(vec(uint)* buffer, rw::Pos3i size);

	
		BinaryImage& operator=(const BinaryImage& img);

		/**
		* Create image buffers, with the adequate size in each dimension
		*/
		void Create(int width, int height, int depth);

		/**
		* Adds a layer to the 3D structure
		* @param img Image layer
		* @param depth Position of the layer
		*/
		void Add_Layer(const BinaryImage::ImageAdapter& img, int depth);

		/**
		* Clears a layer of the image, setting all its values to white.
		* @param depth Layer to be cleaned
		*/
		void Clear(int depth);

		/**
		* Applies the opening operator.
		* @param pgdlg Progress dialog
		*/
		void Open(BinaryImage::ProgressAdapter* pdlg = 0);

		/**
		* @return TRUE if the image has been opened
		*/
		bool Opened() const;

		uint operator()(const rw::Pos3i& pos) const;
		uint operator()(const rw::Pos3i& pos, int v);

		/**
		* Memory size of the texture
		*/
		int Length() const;

		/**
		* Saves the 3D texture to a file
		* @param filename File name
		*/
		void Save_File(const string& filename) const;

		/**
		* Loads the 3D texture from a file
		* @param filename File name
		*/
		void Load_File(const string& filename, BinaryImage::ProgressAdapter* pgdlg = 0);

		/**
		* Gets basic characteristics of the image file, without loading all information. This is useful to dsiplay
		* the binary image basic properties.
		* @param x Width
		* @param y Height
		* @param z Depth
		* @param black Black voxels
		* @param filename File to load
		*/
		static void Get_File_Image_Size(int& x, int& y, int& z, int& black, const string& filename);

		/**
		* @return An wxWidgets image corresponding to a layer of the 3D texture.
		* @param id Layer
		* @param pore_color Color of the pore voxels
		* @param solid_color Color of the solid voxels
		* @param img Image brdige to set the colors of the returned layer
		*/
		void Layer(BinaryImage::ImageAdapter& img,int id, RGBColor pore_color = RGBColor(0,0,0), RGBColor solid_color = RGBColor(255,255,255)) const;

		/**
		* @return A processed image indexed by key. This is the image where the opening operator corresponding to radius key
		* has beel applied
		* @param key Index of the image
		*/
		BinaryImage Processed_Image(int key) const;

		/**
		* Inverts the solid and the porous phases
		*/
		BinaryImage Invert() const;

		/**
		* Populates the vectors with the radius of the maximal spheres filing the pore space
		* @param radius Vector of radius
		*/
		void Maximal_Spheres(vector<int>& radius) const;

		/**
		* @return A wxImage with the pore space segmented. 
		* @param id Layer
		* @param maximal The voxels are colored according to the maximal radius
		* @param rad_color_map The colors of the different radius characterizing the pore space
		* @param solid_color Color of the solid phase
		* @param img Image bridge to set
		*/
		void Pore_Size_Distribution_Layer(ImageAdapter& img, int id, bool maximal, const map<uint,RGBColor>& rad_color_map, 
														RGBColor solid_color = RGBColor(255,255,255));

		/**
		* @return A wxImage with the pore space segmented. A default pore map color is used
		* @param id Layer
		* @param maximal The voxels are colored according to the maximal radius
		* @param img Image bridge to set
		*/
		void Pore_Size_Distribution_Layer(ImageAdapter& img, int id, bool maximal = true);


		/**
		* Creates an image that displays a segmentation layer of the pore space
		* @param img Image to display
		* @param id Depth of the 3D texture to pick
		*/
		void Pore_Segmentation_Layer(ImageAdapter& img, int id, RGBColor solid_color = RGBColor(255, 255, 255));

		/**
		* Creates a defualt color map
		* @return Color map
		*/
		map<uint, RGBColor> Build_Diameter_Color_Map();

		/**
		* Applies an erosion, followed by a dilation in the 3D image.
		* @param Radius of the inverse opening operator
		*/
		void Denoise(int diam, BinaryImage::ProgressAdapter* pgdlg = 0);

		int Depth() const;
		int Width() const;
		int Height() const;

		/**
		*@return The number of black voxels inside the matrix
		*/
		uint Black_Voxels() const;

		/**
		* @return The sub-image  limited by the indices
		* @param bx Origin in the x-axis
		* @param by Origin in the y-axis
		* @param bz Origin in the z-axis
		* @param dx Width of the section
		* @param dy Height of the section
		* @param dz Depth of the section
		*/
		rw::BinaryImage Sub(int bx, int by, int bz, int dx, int dy, int dz) const;


		void Cluster_PSD_File(const string& filename, float step, float max_val) const;

		/**
		* Clusters maximal spheres voxel classification into pores. This method requires a previous maximal
		* sphere classification. The voxels are clustered according to how refining spheres fill the
		* pore space
		* @param pgdlg Progress dialog
		*/
		void Cluster_Pores(BinaryImage::ProgressAdapter* pgdlg);

		void Populate_Color_Map(BinaryImage::ColorMap& cmp) const;
	};

	
	inline void BinaryImage::Populate_Color_Map(BinaryImage::ColorMap& cmp) const
	{
		cmp._colorMap = &this->_colorMap;
	}

	inline uint BinaryImage::Accesor_Read(const vec(uint)& vtx, const rw::Pos3i& pp, const rw::Pos3i& size)
	{
		uint b = (pp.x >> 2) + (pp.y >> 2)*((size.x >> 2) + 1)
			+ (pp.z >> 2)*((size.x >> 2) + 1)*((size.y >> 2) + 1);
		uint pz = pp.z - ((pp.z >> 2) << 2);
		uint iv = vtx[2 * b + (pz >> 1)];
		return (iv & (0x01 << ((pp.x - ((pp.x >> 2) << 2)) +
			((pp.y - ((pp.y >> 2) << 2)) << 2) + ((pz & 0x01) << 4))));
	}


	inline BinaryImage::ImageAdapter::ImageAdapter()
	{
		this->_width = 0;
		this->_height = 0;
	}

	inline BinaryImage::ImageAdapter::ImageAdapter(uint width, uint height)
	{
		this->_width = width;
		this->_height = height;
	}

	inline void BinaryImage::ImageAdapter::Set_Size(uint width, uint height)
	{
		this->_width = width;
		this->_height = height;
	}

	inline uint BinaryImage::ImageAdapter::Width() const
	{
		return(this->_width);
	}

	inline uint BinaryImage::ImageAdapter::Height() const
	{
		return(this->_height);
	}
}


#endif