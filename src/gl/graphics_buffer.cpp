#include "graphics_buffer.h"
#include "math_la\txt\separator.h"

namespace gl
{

using math_la::math_lac::txt::Separator;
using math_la::math_lac::space::Vec3;

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

GraphicsBuffer::VertexDescriptor::VertexDescriptor()
{
	this->_values.reserve(512);
}

GraphicsBuffer::VertexDescriptor& GraphicsBuffer::VertexDescriptor::operator << (float Decay_Step_Value)
{
	this->_values.push_back(Decay_Step_Value);
	return(*this);
}

float GraphicsBuffer::VertexDescriptor::operator[](uint i) const
{
	return(this->_values[i]);
}

void GraphicsBuffer::VertexDescriptor::Clear()
{
	this->_values.clear();
	this->_values.reserve(512);
}


GraphicsBuffer::PolygonDescriptor::PolygonDescriptor()
{
	this->_values.reserve(512);
}

GraphicsBuffer::PolygonDescriptor& GraphicsBuffer::PolygonDescriptor::operator << (uint Decay_Step_Value)
{
	this->_values.push_back(Decay_Step_Value);
	return(*this);
}

uint GraphicsBuffer::PolygonDescriptor::operator[](uint i) const
{
	return(this->_values[i]);
}

void GraphicsBuffer::PolygonDescriptor::Clear()
{
	this->_values.clear();
	this->_values.reserve(512);
}



GraphicsBuffer::GraphicsBuffer(uint flags)
{
	this->_flags = flags;
	this->_normalMem = 0;
	this->_positionMem = 0;
	this->_texture2DMem = 0;
	this->_triangleIndices = 0;
	this->_idxNormalGL = 0;
	this->_idxPositionGL = 0;
	this->_idxTextCoordsGL2D = 0;
	this->_vertexSize = 0;
	this->_triangleSize = 0;
	this->_idxTriangleGL = 0;
	this->_currentVertexIndex = 0;
	this->_currentFaceIndex = 0;
	this->_idxTextureGL3D = 0;
	this->_idxTextureGL2D = 0;
	this->_loadedText2D = false;
	this->_loadedText3D = false;
	this->_texture2DMem = 0;
	this->_texture3DMem = 0;	
	this->_textureHeight = 0;
	this->_textureWidth = 0;
	this->_textureDepth = 0;
	this->_loaded = false;
}

void GraphicsBuffer::Set_Mesh_Flags(uint flags)
{
	this->_flags = flags;
	uint cf = 1;
	uint lim = flagMAX;
	uint cpos = 0;
	while (cf < lim)
	{
		if ((cf&flags) ==  flagPos)		
		{
			this->_posArrPos = cpos;
			cpos = cpos + 3;
		}
		if ((cf&flags) == flagNormal)
		{
			this->_posArrNorm = cpos;
			cpos = cpos + 3;
		}
		if ((cf&flags) == flagTexture2D)
		{
			this->_posArrTex2D = cpos;
			cpos = cpos + 2;
		}
		if ((cf&flags) == flagTexture3D)
		{
			this->_posArrTex3D = cpos;
			cpos = cpos + 3;
		}
		cf = cf * 2;
	}
	this->_vtxCoordSize = cpos;
}

void GraphicsBuffer::Add_Vertex_Line(const GraphicsBuffer::VertexDescriptor& line)
{
	if (this->_flags & flagPos)
	{
		for (uint k = 0; k < 3; ++k)
		{
			this->_positionMem[3 * this->_currentVertexIndex+k] = (float)line[this->_posArrPos+k];
		}
	}
	if (this->_flags & flagTexture3D)
	{
		for (uint k = 0; k < 3; ++k)
		{
			this->_texture3DMem[3 * this->_currentVertexIndex+k] = (float)line[this->_posArrTex3D+k];
		}
	}
	this->_currentVertexIndex++;
}

void GraphicsBuffer::Add_Polygon_Descriptor(const GraphicsBuffer::PolygonDescriptor& line)
{
	vector<Triangle> triangles;
	uint size = line[0];
	uint r = 0;
	vector<uint> v;
	for (uint k = 1; k < size+1; ++k)
	{
		v.push_back(line[k]);
	}
	while (r < size - 1)
	{
		Vec3 n;
		Triangle t;
		t.v0 = (uint)v[r%size];
		t.v1 = (uint)v[(r + 1) % size];
		if (r == 0)
		{
			t.v2 = (uint)v[(r + 2) % size];
			r = r + 2;
		}
		else
		{
			t.v2 = (uint)v[0];
			r = r + 1;
		}
		triangles.push_back(t);
	}
	for (uint k = 0; k < triangles.size(); ++k)
	{
		uint Voxel_Length = k + this->_currentFaceIndex;
		this->_triangleIndices[3 * Voxel_Length] = triangles[k].v0;
		this->_triangleIndices[3 * Voxel_Length + 1] = triangles[k].v1;
		this->_triangleIndices[3 * Voxel_Length + 2] = triangles[k].v2;
	}
	this->_currentFaceIndex = this->_currentFaceIndex + (uint)triangles.size();
}


GraphicsBuffer::~GraphicsBuffer()
{
	this->Clear_Memory();
	if (this->_loadedText3D)
	{
		glDeleteTextures(1, &this->_idxTextureGL3D);
		this->_loadedText3D = false;
	}
	if (this->_loadedText2D)
	{
		glDeleteTextures(1, &this->_idxTextureGL2D);
		this->_loadedText2D = false;
	}
}

void GraphicsBuffer::Set_Vertex_Size(uint n)
{
	this->_vertexSize = n;
	uint Voxel_Length = 0;
	Voxel_Length = this->_flags & flagPos;
	if (Voxel_Length)
	{
		if (this->_positionMem)
		{
			delete[]this->_positionMem;
		}
		this->_positionMem = new float[3*n];
	}
	Voxel_Length = this->_flags & flagNormal;
	if (Voxel_Length)
	{
		if (this->_normalMem)
		{
			delete[]this->_normalMem;
		}
		this->_normalMem = new float[3*n];
	}
	Voxel_Length = this->_flags & flagTexture2D;
	if (Voxel_Length)
	{
		if (this->_texture2DMem)
		{
			delete[]this->_texture2DMem;
		}
		this->_texture2DMem = new float[2*n];
	}
	Voxel_Length = this->_flags & flagTexture3D;
	if (Voxel_Length)
	{
		if (this->_texture3DMem)
		{
			delete[]this->_texture3DMem;
		}
		this->_texture3DMem = new float[3 * n];
	}
}

void GraphicsBuffer::Set_Quad_Size(uint n)
{
	this->_triangleSize = 2 * n;
	if (this->_triangleIndices)
	{
		delete[]this->_triangleIndices;
	}
	this->_triangleIndices = new uint[6* n];
}

void GraphicsBuffer::Set_Triangle_Size(uint n)
{
	if (this->_triangleIndices)
	{
		delete[]this->_triangleIndices;
	}
	this->_triangleIndices = new uint[3 * n];
	this->_triangleSize = n;
}

bool GraphicsBuffer::Loaded() const
{
	return(this->_loaded);
}

void GraphicsBuffer::Load_Mesh_To_Graphic_Device(GLint programid)
{
	this->_loaded = true;
	uint current_flag = 0;
	current_flag = this->_flags & flagPos;
	glGenVertexArrays(1, &this->_idxMeshGL);
	glBindVertexArray(this->_idxMeshGL);
	
	if (current_flag)
	{
		glGenBuffers(1, &this->_idxPositionGL);
		glBindBuffer(GL_ARRAY_BUFFER, this->_idxPositionGL);
		glBufferData(GL_ARRAY_BUFFER, this->_vertexSize * 3 * sizeof(float), (GLvoid*)this->_positionMem, GL_STATIC_DRAW);
		glVertexPointer(3, GL_FLOAT, 0, 0);
	}
	current_flag = this->_flags & flagNormal;
	if (current_flag)
	{
		glGenBuffers(1, &this->_idxNormalGL);
		glBindBuffer(GL_ARRAY_BUFFER, this->_idxNormalGL);
		glBufferData(GL_ARRAY_BUFFER, this->_vertexSize * 3 * sizeof(float), (GLvoid*)this->_normalMem, GL_STATIC_DRAW);
		glNormalPointer(GL_FLOAT, 0, 0);
	}
	current_flag = this->_flags & flagTexture2D;
	if (current_flag)
	{
		glGenBuffers(1, &this->_idxTextCoordsGL2D);
		glBindBuffer(GL_ARRAY_BUFFER, this->_idxTextCoordsGL2D);
		glBufferData(GL_ARRAY_BUFFER, this->_vertexSize * 2 * sizeof(float), (GLvoid*)this->_texture2DMem, GL_STATIC_DRAW);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
	}
	current_flag = this->_flags & flagTexture3D;
	if (current_flag)
	{
		glGenBuffers(1, &this->_idxTextCoordsGL3D);
		glBindBuffer(GL_ARRAY_BUFFER, this->_idxTextCoordsGL3D);
		glBufferData(GL_ARRAY_BUFFER, this->_vertexSize * 3 * sizeof(float), (GLvoid*)this->_texture3DMem, GL_STATIC_DRAW);
		glTexCoordPointer(3, GL_FLOAT, 0, 0);
	}
	if (this->_triangleIndices)
	{
		glGenBuffers(1, &this->_idxTriangleGL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_idxTriangleGL);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->_triangleSize * 3 * sizeof(uint), (GLvoid*)this->_triangleIndices, GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GraphicsBuffer::Release_Graphic_Device()
{
	if (this->_loaded)
	{
		uint Voxel_Length = 0;
		Voxel_Length = this->_flags & flagPos;
		glDeleteVertexArrays(1, &this->_idxMeshGL);
		if (Voxel_Length)
		{
			glDeleteBuffers(1, &this->_idxPositionGL);
		}
		Voxel_Length = this->_flags & flagNormal;
		if (Voxel_Length)
		{
			glDeleteBuffers(1, &this->_idxNormalGL);
		}
		Voxel_Length = this->_flags & flagTexture2D;
		if (Voxel_Length)
		{
			glDeleteBuffers(1, &this->_idxTextCoordsGL2D);
		}
		Voxel_Length = this->_flags & flagTexture3D;
		if (Voxel_Length)
		{
			glDeleteBuffers(1, &this->_idxTextCoordsGL3D);
		}
		if (this->_triangleIndices)
		{
			glDeleteBuffers(1, &this->_idxTriangleGL);
		}
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		this->_loaded = false;
	}
}

uint GraphicsBuffer::Activate_Texture_3D_GL(uint i)
{
	glBindVertexArray(this->_idxMeshGL);
	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(GL_TEXTURE_3D, this->_idxTextureGL3D);
	glBindVertexArray(0);
	return(this->_idxTextureGL3D);
}

uint GraphicsBuffer::Activate_Texture_2D_GL(uint i)
{
	glBindVertexArray(this->_idxMeshGL);
	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(GL_TEXTURE_1D, this->_idxTextureGL2D);
	glBindVertexArray(0);
	return(this->_idxTextureGL2D);
}


uint GraphicsBuffer::Load_3D_Texture(const vector<wxImage*>& tex3)
{
	if (tex3.size() > 0)
	{
		wxImage* f = tex3[0];
		uint width = f->GetSize().x;
		uint height = f->GetSize().y;
		uint depth = (uint)tex3.size();
		this->_textureHeight = height;
		this->_textureWidth = width;
		this->_textureDepth = depth;
		GLubyte* text = new GLubyte[width*depth*height * 4];
		for (uint w = 0; w < depth; ++w)
		{
			wxImage* img = tex3[w];
			for (uint x = 0; x < width; ++x)
			{
				for (uint y = 0; y < height; ++y)
				{
					GLubyte r = (GLubyte)img->GetRed(x, y);
					GLubyte g = (GLubyte)img->GetGreen(x,y);
					GLubyte b = (GLubyte)img->GetBlue(x, y);
					uint sy = height - y - 1;
					text[4 * (height*width*w + width*sy + x) + 0] = r;
					text[4 * (height*width*w + width*sy + x) + 1] = g;
					text[4 * (height*width*w + width*sy + x) + 2] = b;
					text[4 * (height*width*w + width*sy + x) + 3] = 0;
				}
			}
		}
		this->_loadedText3D = true;
		glGenTextures(1, &this->_idxTextureGL3D);
		glBindTexture(GL_TEXTURE_3D, this->_idxTextureGL3D);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)text);
		delete []text;
	}
	return(this->_idxTextureGL3D);
}

void GraphicsBuffer::Release_3D_Texture()
{
	if (this->_loadedText3D)
	{
		this->_loadedText3D = false;
		glDeleteTextures(1, &this->_idxTextureGL3D);
	}
}

uint GraphicsBuffer::Load_2D_Texture(wxImage* tex)
{
	uint width = tex->GetSize().x;
	uint height = tex->GetSize().y;
	this->_textureWidth = width;
	this->_textureHeight = height;
	unsigned char* text = new unsigned char[width*3*height];
	for (uint x = 0; x < width; ++x)
	{
		for (uint y = 0; y < height; ++y)
		{
			unsigned char r = (unsigned char)tex->GetRed(x, y);
			unsigned char g = (unsigned char)tex->GetGreen(x, y);
			unsigned char b = (unsigned char)tex->GetBlue(x,y);
			unsigned char a = (unsigned char)tex->GetAlpha(x,y);
			text[3*(y*width + x) + 0] = r;
			text[3*(y*width + x) + 1] = g;
			text[3*(y*width + x) + 2] = b;
		}
	}
	this->_loadedText2D = true;
	glGenTextures(1, &this->_idxTextureGL2D);
	glBindTexture(GL_TEXTURE_2D, this->_idxTextureGL2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height,0, GL_RGB, GL_UNSIGNED_BYTE, text);
	delete []text;
	return(this->_idxTextureGL2D);
}

void GraphicsBuffer::Release_2D_Texture()
{
	glDeleteTextures(1, &this->_idxTextureGL2D);
}


void GraphicsBuffer::Load_Polygon_From_Line(const string& line, vector<Triangle>& triangles, Vec3* normals)
{
	vector<scalar> vvf = Separator::Separate_Numbers(line);
	if (vvf.size() >= 4)
	{
		uint r = 0;		
		uint size = (uint)vvf[0];		
		vector<scalar>::iterator nit = vvf.begin();
		++nit;
		vector<scalar> v(nit, vvf.end());
		while (r < size-1)
		{
			Vec3 n;
			Triangle t;
			t.v0 = (uint)v[r%size];
			t.v1 = (uint)v[(r + 1)%size];
			if (r == 0)
			{
				t.v2 = (uint)v[(r + 2) % size];
				r = r + 2;
				if (normals)
				{
					Vec3 v2 = Vec3((scalar)this->_positionMem[3 * t.v1],
						(scalar)this->_positionMem[3 * t.v1 + 1], (scalar)this->_positionMem[3 * t.v1 + 2]);
					Vec3 v1 = Vec3((scalar)this->_positionMem[3 * t.v2],
						(scalar)this->_positionMem[3 * t.v2 + 1], (scalar)this->_positionMem[3 * t.v2 + 2]);
					Vec3 v0 = Vec3((scalar)this->_positionMem[3 * t.v0],
						(scalar)this->_positionMem[3 * t.v0 + 1], (scalar)this->_positionMem[3 * t.v0 + 2]);
					Vec3 d1 = v2 - v0;
					Vec3 d2 = v1 - v0;
					n = Vec3::Cross(d2, d1);
					if (n.Norm() != 0)
					{
						n.Normalize();
						normals[t.v0] = normals[t.v0] + n;
						normals[t.v1] = normals[t.v1] + n;
						normals[t.v2] = normals[t.v2] + n;
					}
				}
			}
			else
			{
				t.v2 = (uint)v[0];
				r = r + 1;
				if (normals)
				{
					Vec3 v2 = Vec3((scalar)this->_positionMem[3 * t.v1],
						(scalar)this->_positionMem[3 * t.v1 + 1], (scalar)this->_positionMem[3 * t.v1 + 2]);
					Vec3 v1 = Vec3((scalar)this->_positionMem[3 * t.v2],
						(scalar)this->_positionMem[3 * t.v2 + 1], (scalar)this->_positionMem[3 * t.v2 + 2]);
					Vec3 v0 = Vec3((scalar)this->_positionMem[3 * t.v0],
						(scalar)this->_positionMem[3 * t.v0 + 1], (scalar)this->_positionMem[3 * t.v0 + 2]);
					Vec3 d1 = v2 - v0;
					Vec3 d2 = v1 - v0;
					n = Vec3::Cross(d2, d1);
					if (n.Norm() != 0)
					{
						n.Normalize();
						normals[t.v1] = normals[t.v1] + n;
					}
				}
			}
			triangles.push_back(t);
		}
	}
}

void GraphicsBuffer::Load_Vertices_From_Stream(uint nv, istream& file, GraphicsBuffer::Sizer& sizer)
{
	bool end = false;
	this->Set_Vertex_Size(nv);
	while ((!file.eof()) && (!end))
	{
		uint cv = 0;
		while ((cv < nv) && (!file.eof()))
		{
			string line = "";
			bool vv = false;
			while ((!vv) && (!file.eof()))
			{
				std::getline(file, line);
				vector<scalar> vvx = Separator::Separate_Numbers(line);
				if (vvx.size() >= 3)
				{
					if (vvx[0] < sizer._minPts(eX))
					{
						sizer._minPts(eX, vvx[0]);
					}
					if (vvx[1] < sizer._minPts(eY))
					{
						sizer._minPts(eY, vvx[1]);
					}
					if (vvx[2] < sizer._minPts(eZ))
					{
						sizer._minPts(eZ, vvx[2]);
					}

					if (vvx[0] > sizer._maxPts(eX))
					{
						sizer._maxPts(eX, vvx[0]);
					}
					if (vvx[1] > sizer._maxPts(eY))
					{
						sizer._maxPts(eY, vvx[1]);
					}
					if (vvx[2] > sizer._maxPts(eZ))
					{
						sizer._maxPts(eZ, vvx[2]);
					}

					this->_positionMem[3 * cv] = (float)vvx[0];
					this->_positionMem[3 * cv + 1] = (float)vvx[1];
					this->_positionMem[3 * cv + 2] = (float)vvx[2];
					vv = true;
					++cv;
					if (cv == nv)
					{
						end = true;
					}
				}
			}
		}
	}
}

void GraphicsBuffer::Load_Polygons_From_Stream(uint nf, istream& file, Vec3* normals)
{
	uint cf = 0;
	vector<Triangle> triangles;
	while ((cf < nf) && (!file.eof()))
	{
		string line = "";
		std::getline(file, line);
		this->Load_Polygon_From_Line(line, triangles,normals);
		++cf;
	}

	this->Set_Triangle_Size((uint)triangles.size());
	for (uint k = 0; k < triangles.size(); ++k)
	{
		this->_triangleIndices[3 * k] = triangles[k].v0;
		this->_triangleIndices[3 * k + 1] = triangles[k].v1;
		this->_triangleIndices[3 * k + 2] = triangles[k].v2;
	}
}

GraphicsBuffer::Sizer GraphicsBuffer::Load_OFF_File(const string& filename)
{
	ifstream file(filename);
	GraphicsBuffer::Sizer Voxel_Length = this->Load_OFF_Stream(file);
	file.close();
	return(Voxel_Length);
}

GraphicsBuffer::Sizer GraphicsBuffer::Load_OFF_Stream(istream& file)
{	
	this->Clear_Memory();
	GraphicsBuffer::Sizer sizer;
	this->Set_Mesh_Flags(3);
	Vec3* normals = 0;
	int nv = 0;
	int nf = 0;
	bool h = false;
	while ((!h) && (!file.eof()))
	{
		string line = "";
		std::getline(file, line);
		vector<scalar> header = Separator::Separate_Numbers(line);
		if (header.size() >= 2)
		{
			h = true;
			nv = (int)header[0];
			nf = (int)header[1];
		}
	}
	normals = new Vec3[nv];
	this->Load_Vertices_From_Stream(nv, file, sizer);
	this->Load_Polygons_From_Stream(nf, file, normals);
	if (normals)
	{
		for (int j = 0; j < (int)this->_vertexSize; ++j)
		{
			Vec3 n = normals[j];
			if (normals[j].Norm() != 0)
			{
				n.Normalize();
			}
			this->_normalMem[3 * j] = (float)n(eX);
			this->_normalMem[3 * j + 1] = (float)n(eY);
			this->_normalMem[3 * j + 2] = (float)n(eZ);
		}
	}
	delete[]normals;
	return(sizer);
}

void GraphicsBuffer::Flush_Buffer()
{
	if (this->_triangleSize > 0)
	{
		glBindVertexArray(this->_idxMeshGL);
		uint flag_pos = 0;
		flag_pos = this->_flags & flagPos;
		if (flag_pos)
		{		
			glEnableClientState(GL_VERTEX_ARRAY);
		}
		flag_pos = this->_flags & flagNormal;
		if (flag_pos)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
		}
		flag_pos = this->_flags & flagTexture2D;
		if ((flag_pos)&&(this->_loadedText2D))
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		flag_pos = this->_flags & flagTexture3D;
		if ((flag_pos)&&(this->_loadedText3D))
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glDrawElements(GL_TRIANGLES, this->_triangleSize * 3, GL_UNSIGNED_INT, 0);
		flag_pos = this->_flags & flagPos;
		if (flag_pos)
		{
			glDisableClientState(GL_VERTEX_ARRAY);
		}
		flag_pos = this->_flags & flagNormal;
		if (flag_pos)
		{
			glDisableClientState(GL_NORMAL_ARRAY);
		}
		flag_pos = this->_flags & flagTexture2D;
		if (flag_pos)
		{
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		flag_pos = this->_flags & flagTexture3D;
		if (flag_pos)
		{
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glBindVertexArray(0);
	}
}

void GraphicsBuffer::Clear_Memory()
{
	if (this->_loaded)
	{
		if (this->_idxPositionGL > 0)
		{
			glDeleteBuffers(1, &this->_idxPositionGL);
			delete[]this->_positionMem;
			this->_positionMem = 0;
			this->_vertexSize = 0;
		}
		if (this->_idxNormalGL)
		{
			glDeleteBuffers(1, &this->_idxNormalGL);
			delete[]this->_normalMem;
			this->_normalMem = 0;
		}
		if (this->_idxTextCoordsGL2D)
		{
			glDeleteBuffers(1, &this->_idxTextCoordsGL2D);
			delete[]this->_texture2DMem;
			this->_texture2DMem = 0;
		}
		if (this->_idxTextCoordsGL3D)
		{
			glDeleteBuffers(1, &this->_idxTextCoordsGL3D);
			delete[]this->_texture3DMem;
			this->_texture3DMem = 0;
		}
		if (this->_idxTriangleGL)
		{
			glDeleteBuffers(1, &this->_idxTriangleGL);
			delete[]this->_triangleIndices;
			this->_triangleIndices = 0;
			this->_triangleSize = 0;
		}
		glDeleteBuffers(1, &this->_idxMeshGL);
	}
	else
	{
		if (this->_positionMem)
		{
			delete[]this->_positionMem;
			this->_positionMem = 0;
		}
		if (this->_normalMem)
		{
			delete[]this->_normalMem;
			this->_normalMem = 0;
		}
		if (this->_texture2DMem)
		{
			delete[]this->_texture2DMem;
			this->_texture2DMem = 0;
		}
		if (this->_texture3DMem)
		{
			for (int k = 0; k < 18; ++k)
			{
				float tt = this->_texture3DMem[k];
				int y = 0;
				++y;
			}
			delete[]this->_texture3DMem;
			this->_texture3DMem = 0;
		}
		if (this->_triangleIndices)
		{
			delete[]this->_triangleIndices;
			this->_triangleIndices = 0;
			this->_triangleSize = 0;
		}
	}
}

}