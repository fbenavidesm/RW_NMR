#ifndef BUFFER_OBJ_H
#define BUFFER_OBJ_H

#include <string>
#include <istream>
#include <fstream>
#include <vector>
#include "wx/wx.h"
#include "math_la/mdefs.h"
#include "GL/glew.h"
#include "math_la/math_lac/space/vec3.h"

namespace gl
{

#define  flagPos		1
#define  flagNormal		2
#define  flagTexture2D	4
#define  flagTexture3D	8
#define  flagMAX		512

using std::string;
using math_la::math_lac::space::Vec3;
using std::ifstream;
using std::istream;
using std::vector;


/**
* OpenGL can only render triangles, which are described by this structure
*/
struct Triangle
{
	uint v0;
	uint v1;
	uint v2;
};

/**
* This is an OpenGL class that handles vertex buffers and index buffers. 
* The idea is to facilitate the interface with OpenGL commands and read simple files describing
* meshes. 
*/

class GraphicsBuffer
{
public:

	/**
	* Maximal and minimal values of a surrounding box
	*/
	struct Sizer
	{
		/**
		* Maximal points
		*/
		Vec3 _maxPts;
		/**
		* Minimal points
		*/
		Vec3 _minPts;
	};

	/**
	* A VertexDescriptor contains information about a vertex: coordinates, normal vectors, texture coordinates, etc
	*/
	class VertexDescriptor
	{
	private:
		/**
		* Descriptor values of the vertex
		*/
		vector<float> _values;
	public:
		VertexDescriptor();
		/**
		* Adds an scalar value to the vertex descriptos
		*/
		VertexDescriptor& operator << (float Decay_Step_Value);
		/**
		* @return A scalar descriptor value indexed by i
		* @param i Index of the read value
		*/
		float operator[](uint i) const;
		/**
		* Clears the entire line
		*/
		void Clear();
	};

	/**
	* Stores the coordinates of each mesh vertex. The number of vertices determines what kind of
	* geometric figure is being rendered. The buffer takes charge to interpret this polygon
	* as a set of triangles. 
	*/
	class PolygonDescriptor
	{
	private:
		/** 
		* The list of coordinates of the polygon
		*/
		vector<uint> _values;
	public:
		PolygonDescriptor();
		/**
		* Adds a new coordinate to the polygon
		*/
		PolygonDescriptor& operator << (uint indx);
		/**
		* @return The value of the index, stored at position i
		* @param i The local position of the index in the polygon
		*/
		uint operator[](uint i) const;

		/**
		* Clears the entire polygon set of indices
		*/
		void Clear();
	};

private:
	/**
	* As the GraphicsBuffer is a vertex container, this variables indicate the current position
	* of the stored vertices
	*/
	uint _currentVertexIndex;

	/**
	* As the GraphicsBuffer is a vertex container, this variable indicates the current position
	* of the stored index
	*/
	uint _currentFaceIndex;

	/**
	* TRUE if a 3D texture has been loaded
	*/	
	bool _loadedText3D;

	/**
	* TRUE if a 2D texture has been loaded
	*/
	bool _loadedText2D;

	/**
	* OpenGL index of the 3D texture associated to the mesh
	*/
	uint _idxTextureGL3D;

	/**
	* OpenGL index of the 2D texture associated to the mesh
	*/
	uint _idxTextureGL2D;

	/**
	* CPU memory to store the position information about the vertices
	*/
	float* _positionMem;

	/**
	* CPU memory to store the normal information about the vertices
	*/
	float* _normalMem;

	/**
	* CPU memory to store the texture coordinate information about the vertices
	*/
	float* _texture2DMem;

	/**
	* CPU memory to store the 3D texture coordinate information about the vertices
	*/
	float* _texture3DMem;

	/**
	* List of mesh indices (stored as triangles). These indices are contiguously stored
	* and the size of this memory array is 3N where N is the number of triangles
	*/
	uint*	_triangleIndices;

	/**
	* The set of flags that activate the vertex information
	*/
	uint	_flags;

	/**
	* OpenGL index of the position vertex data
	*/
	GLuint  _idxPositionGL;

	/**
	* OpenGL index of the normal vertex data
	*/
	GLuint  _idxNormalGL;

	/**
	* OpenGL index of the texture coordinates vertex data
	*/
	GLuint  _idxTextCoordsGL2D;

	/**
	* OpenGL index of the 3D texture coordinates vertex data
	*/
	GLuint	_idxTextCoordsGL3D;

	/**
	* OpenGL index of the polygonal mesh
	*/
	GLuint	_idxTriangleGL;

	/**
	* Position of the position data in the vertex array
	*/
	uint	_posArrPos;

	/**
	* Position of the normal data in the vertex array
	*/
	uint	_posArrNorm;

	/**
	* Position of the 2D texture coordinates in the vertex array 
	*/
	uint	_posArrTex2D;

	/**
	* Position of the 3D texture coordinates in the vertex array
	*/

	uint	_posArrTex3D;

	/**
	* The size of each vertex set of scalar coordinates
	*/
	uint	_vtxCoordSize;

	/**
	* Texture width in voxels
	*/
	uint	_textureWidth;

	/**
	* Texture height in voxels
	*/
	uint	_textureHeight;

	/**
	* Texture depth in voxels
	*/
	uint	_textureDepth;

	/**
	* OpenGL global vertex array
	*/
	GLuint	_idxMeshGL;

	/**
	* Number of vertices in the mesh
	*/
	uint	_vertexSize;

	/**
	* Number of triangles in the mesh
	*/
	uint	_triangleSize;

	/**
	* TRUE if all OpenGL vertex buffers have been loaded
	*/
	bool _loaded;

	/**
	* Loads all vertices from a stream (which can be a file), dimensioning a surrounding box
	*/
	void Load_Vertices_From_Stream(uint nv, istream& file, GraphicsBuffer::Sizer& sizer);

	/**
	* Loads all polygons from a stream (which can be a file). Based on these polygons, the normal
	* vector of each vertex is calculated. Later, this set of normal vectors can be used to 
	* update _normalMem buffer. 
	*/
	void Load_Polygons_From_Stream(uint nf, istream& file,Vec3* normals = 0);

	/**
	* Loads a single polygon from a stream line. The polygon is divided into triangles, and the normal
	* vector is calculated
	*/
	void Load_Polygon_From_Line(const string& line, vector<Triangle>& triangles, Vec3* normals = 0);

public:
	GraphicsBuffer(uint flags = 0);
	~GraphicsBuffer();

	/**
	* Defines the characteristics of the mesh vertices
	*/
	void Set_Mesh_Flags(uint flags);

	/**
	* Defines the number of mesh vertices
	*/
	void Set_Vertex_Size(uint n);

	/**
	* Defines the number of quads in the mesh
	* @param n Number of quads
	*/
	void Set_Quad_Size(uint n);

	/**
	* Defines the number of triangles in the mesh
	* @param n Number of triangles
	*/
	void Set_Triangle_Size(uint n);

	/**
	* Loads all mesh attributes to the graphic device. Optionally, a shader can be specified 
	* @programid GL identifier of the compiled shader
	*/
	void Load_Mesh_To_Graphic_Device(GLint programid = 0);

	/**
	* @return TRUE if mesh has been loaded succesfully to the graphic device
	*/
	bool Loaded() const;

	/**
	* Releases the graphic device from all mesh data
	*/
	void Release_Graphic_Device();

	/**
	* Loads a file following the OFF format
	*/
	GraphicsBuffer::Sizer Load_OFF_File(const string& filename);

	/**
	* Loads an OFF stream format (can be stored in memory)
	*/
	GraphicsBuffer::Sizer Load_OFF_Stream(istream& stream);

	/**
	* Adds a VertexDescriptor to the Mesh
	* @param line Vertex descriptor (generally a line of the OFF file)
	*/
	void Add_Vertex_Line(const GraphicsBuffer::VertexDescriptor& line);

	/**
	* Adds a polygon descriptor to the mesh
	* @param line Polygon descriptor (generally a line of the OFF file)
	*/
	void Add_Polygon_Descriptor(const GraphicsBuffer::PolygonDescriptor& line);

	/**
	* Releases GPU memory associated to the 3D texture
	*/
	void Release_3D_Texture();

	/**
	* Releases GPU memory associated to the 2D texture
	*/
	void Release_2D_Texture();

	/**
	* Activates 3D texture indexed by i. In general, there is only one texture and i = 0
	* @param i Index of the texture. Generally it is 0
	*/
	uint Activate_Texture_3D_GL(uint i = 0);

	/**
	* Activates 2D texture indexed by i. In general, there is only one texture and i = 0
	* @param i Index of the texture. Generally it is 0
	*/
	uint Activate_Texture_2D_GL(uint i = 1);
	
	/**
	* @return TRUE if a 3D texture has been loaded
	*/
	bool Loaded_Texture_3D_GL() const;

	/**
	* @return TRUE if a 2D texture has been loaded
	*/
	bool Loaded_Texture_2D_GL() const;

	/**
	* Flushes all OpenGL commands associated to rendering the mesh
	*/
	void Flush_Buffer();

	/**
	* Clears all memory associated to the Buffer (in GPU and CPU)
	*/
	void Clear_Memory();	

	/**
	* Loads a set of images as a 3D texture
	*/

	uint Load_3D_Texture(const vector<wxImage*>& tex3);

	/**
	* Loads a set of images as a 3D texture
	*/
	uint Load_2D_Texture(wxImage* tex);
};

inline bool GraphicsBuffer::Loaded_Texture_3D_GL() const
{
	return(this->_loadedText3D);
}

inline bool GraphicsBuffer::Loaded_Texture_2D_GL() const
{
	return(this->_loadedText2D);
}


}
#endif