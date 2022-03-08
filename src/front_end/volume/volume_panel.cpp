#include <istream>
#include <iostream>
#include <sstream>
#include <math.h>
#include "volume_panel.h"

void VolumePanel::Observer::Block_Event()
{
	int Voxel_Length = 0;
	while ((this->_painting) && (Voxel_Length < 50))
	{
		Sleep(10);
		++Voxel_Length;
	}
	if (!this->_painting)
	{
		this->_blocked = true;
	}
}

void VolumePanel::Observer::Unblock_Event()
{
	this->_blocked = false;
}


VolumePanel::VolumePanel(wxWindow* Parent, wxWindowID Id, int* attrs, const wxPoint& Position,const wxSize& Size, long Style)
													: WxGLPanel(Parent,Id,attrs,Position,Size,Style)
{
	this->_zcompression = 1.0f;
	this->_cubeMin = math_la::math_lac::space::Vec3(0, 0, 0);
	this->_cubeMax = math_la::math_lac::space::Vec3(1, 1, 1);
	this->_lockRefresh = false;
	this->_mouseDown = false;
	this->_barMaxMoving = false;
	this->_barMinMoving = false;
	this->_barMoving = false;
	this->_barWidth = 4;
	this->_barBorder = 50;
	this->_depthPrecision = 150;
	this->SetBackgroundColour(wxColor(255, 255, 255));
	this->_boxMin = Vec3(-2.0f, -2.0f, -2.0f);
	this->_boxMax = Vec3(-1.0f, -1.0f, -1.0f);
	this->_boxColor = Vec3(0.75f, 1.0f, 0.75f);

	this->Set_Solid_Color(wxColor(255, 225, 105));
	this->_updateEvent = 0;
	this->_barMin = 0.0f;
	this->_barMax = 0.01f;
	this->_controlFactor = 3;
	this->_controlColor = wxColor(150,150,150,255);
	this->_lightControlColor = wxColor(200,200,200,255);
	this->_borderColor = wxColor(150, 150, 150,255);
	this->_revMarker = wxColor(255, 20, 20);
	this->_trackBall.Set_Size(this->GetSize().x, this->GetSize().y);
}

void VolumePanel::Set_Z_Compression(float zcompression)
{
	float z = std::min(1.0f, zcompression);
	z = std::max(0.0f, z);
	this->_zcompression = z; 
}

void VolumePanel::Setup_Cube()
{
	this->_solidMinVol = Vec3(0.9, 0.9, 0.9);
	this->_solidMaxVol = Vec3(1, 1, 1);
	this->_graphicBuffer.Set_Mesh_Flags(flagPos | flagTexture3D);
	this->_graphicBuffer.Set_Vertex_Size(8);
	this->_graphicBuffer.Set_Quad_Size(6);
	gl::GraphicsBuffer::VertexDescriptor vl;

	Vec3 box = this->_cubeMax - this->_cubeMin;
	scalar nn = std::max(box(eX), std::max(box(eY), box(eZ)));
	box = (1.0 / nn)*box;
	box(eZ, box(eZ) * this->_zcompression);
	this->_cubeVol = box;
	this->_nn = nn;

	vl.Clear();
	this->_solidBoxMin = Vec3(0, 0, 0);
	this->_solidBoxMax = Vec3(1,1,1);

	vl << -box(eX)/2.0 << -box(eY)/2.0 << box(eZ)/2.0 << 0 << 0 << 1.0;
	this->_graphicBuffer.Add_Vertex_Line(vl);

	vl.Clear();
	vl << box(eX) / 2.0 << -box(eY) / 2.0 << box(eZ) / 2.0 << 1.0 << 0 << 1.0;
	this->_graphicBuffer.Add_Vertex_Line(vl);

	vl.Clear();
	vl << box(eX)/2.0 << box(eY)/2.0 << box(eZ)/2.0 << 1.0 << 1.0 << 1.0;
	this->_graphicBuffer.Add_Vertex_Line(vl);

	vl.Clear();
	vl << -box(eX)/2.0 << box(eY)/2.0 << box(eZ)/2.0 << 0 << 1.0 << 1.0;
	this->_graphicBuffer.Add_Vertex_Line(vl);

	//***********

	vl.Clear();
	vl << -box(eX) / 2.0 << -box(eY) / 2.0 << -box(eZ) / 2.0 << 0 << 0 << 0;
	this->_graphicBuffer.Add_Vertex_Line(vl);

	vl.Clear();
	vl << box(eX) / 2.0 << -box(eY) / 2.0 << -box(eZ) / 2.0 << 1.0 << 0 << 0;
	this->_graphicBuffer.Add_Vertex_Line(vl);

	vl.Clear();
	vl << box(eX) / 2.0 << box(eY) / 2.0 << -box(eZ) / 2.0 << 1.0 << 1.0 << 0;
	this->_graphicBuffer.Add_Vertex_Line(vl);

	vl.Clear();
	vl << -box(eX) / 2.0 << box(eY) / 2.0 << -box(eZ) / 2.0 << 0 << 1.0 << 0;
	this->_graphicBuffer.Add_Vertex_Line(vl);


	gl::GraphicsBuffer::PolygonDescriptor il;

	il.Clear();
	il << 4 << 3 << 0 << 1 << 2;
	this->_graphicBuffer.Add_Polygon_Descriptor(il);
	il.Clear();
	il << 4 << 7 << 4 << 5 << 6;
	this->_graphicBuffer.Add_Polygon_Descriptor(il);
	il.Clear();
	il << 4 << 2 << 1 << 5 << 6;
	this->_graphicBuffer.Add_Polygon_Descriptor(il);

	il.Clear();
	il << 4 << 4 << 0 << 3 << 7;
	this->_graphicBuffer.Add_Polygon_Descriptor(il);
	il.Clear();
	il << 4 << 7 << 3 << 2 << 6;
	this->_graphicBuffer.Add_Polygon_Descriptor(il);
	il.Clear();
	il << 4 << 5 << 1 << 0 << 4;
	this->_graphicBuffer.Add_Polygon_Descriptor(il);

	if (!this->_shader.Compiled())
	{
		this->Load_Volume_Shader();
	}
	if (!this->_graphicBuffer.Loaded())
	{
		this->_graphicBuffer.Load_Mesh_To_Graphic_Device();
	}
	this->_cameraPosition = Vec4(0, 0, 4, 1);
	this->Bind(wxEVT_SIZE, &VolumePanel::On_Resize, this);
}

void VolumePanel::Set_Update_Event(VolumePanel::Observer* evt)
{
	this->_updateEvent = evt;
}

VolumePanel::~VolumePanel()
{
	this->Unbind(wxEVT_SIZE, &VolumePanel::On_Resize, this);
	this->Release_3D_Texture();
	this->_graphicBuffer.Release_Graphic_Device();
	this->_shader.Detach();
}

void VolumePanel::Load_Volume_Shader()
{
	this->_shader.Load_File("files\\volumeshader.fsh");
	this->_shader.Compile_Fragment();	
	this->_shader.Attach();
	this->_shader.Execute();
	
	this->_idxModelMatrixGL = glGetUniformLocation(this->_shader.ProgramIDX(), "ortho");
	this->_idxModelMatrixTransposedGL = glGetUniformLocation(this->_shader.ProgramIDX(), "iortho");
	this->_idxCameraPositionGL = glGetUniformLocation(this->_shader.ProgramIDX(), "cam");
	this->_idxBackgroundColorGL = glGetUniformLocation(this->_shader.ProgramIDX(), "back");
	this->_object3DColorGL = glGetUniformLocation(this->_shader.ProgramIDX(), "object_color");
	this->_idxMinVolumeGL = glGetUniformLocation(this->_shader.ProgramIDX(), "min");
	this->_idxMaxVolumeGL = glGetUniformLocation(this->_shader.ProgramIDX(), "max");
	this->_idxMinVolumeColorGL = glGetUniformLocation(this->_shader.ProgramIDX(), "mincolor");
	this->_idxMaxVolumeColorGL = glGetUniformLocation(this->_shader.ProgramIDX(), "maxcolor");
	this->_idxDepthPrecisionGL = glGetUniformLocation(this->_shader.ProgramIDX(), "depthPrecision");
	this->_idxBoxMinGL = glGetUniformLocation(this->_shader.ProgramIDX(), "box_min");
	this->_idxBoxMaxGL = glGetUniformLocation(this->_shader.ProgramIDX(), "box_max");
	this->_idxBoxColorGL = glGetUniformLocation(this->_shader.ProgramIDX(), "box_color");
	this->_idxCubeVol = glGetUniformLocation(this->_shader.ProgramIDX(), "cube_vol");
}

void VolumePanel::Define_Projection_Matrix()
{
	this->_trackBall.Set_Size(this->GetSize().x, this->GetSize().y);
	glViewport(0, 0, this->GetSize().x, this->GetSize().y);
	glMatrixMode(GL_PROJECTION);

	scalar pasp = (scalar)this->GetSize().x / (scalar)this->GetSize().y;
	scalar pnear = 0.1;
	scalar pfar = 10;
	scalar ptop = pnear*tan(45 * M_PI / 180);
	scalar pright = ptop*pasp;
	scalar d = this->_cameraPosition(eZ);
	Mtx4 pf = Mtx4::Identity();
	pf(0, 0, pnear/pright);
	pf(1, 1, pnear/ptop);
	pf(2, 2, (pnear + pfar) / (pfar - pnear));
	pf(2, 3, 2 * pfar * pnear / (pfar - pnear));
	pf(3, 2, 1.0/d);

	this->_projectionMatrix = pf;
	double stream[16];
	this->_projectionMatrix.Stream(stream);
	glLoadTransposeMatrixd((GLdouble*)stream);
}


void VolumePanel::OnLeftMouseDown(wxMouseEvent& evt)
{
	int x = evt.GetPosition().x;
	int y = evt.GetPosition().y;
	if (x < this->GetSize().x - this->_barWidth-this->_barBorder)
	{
		if (evt.GetButton() == wxMOUSE_BTN_LEFT)
		{
			this->_trackBall.Set_Start_Position(x, y);
			this->_mouseDown = true;
		}
	}
	else
	{
		int rad = std::max(this->_barWidth / 2,2);
		int mrad = this->_controlFactor * rad;
		int barlength = this->GetSize().y - 2 * this->_barBorder;
		int lx = 0;
		int ux = 0;
		int ly = 0;
		int uy = 0;
		this->_currentBarPosition = 0;
		if ((y <= this->GetSize().y - this->_barBorder + 8*mrad) && (y >= this->_barBorder-8*mrad))
		{
			lx = this->GetSize().x - this->_barBorder + 2 * mrad-1;
			ux = this->GetSize().x - this->_barBorder + 2 * mrad + 3 * rad+1;
			ly = this->GetSize().y - this->_barBorder - (int)((this->_barMax + this->_barMin)*(float)barlength*0.5f) - 12*rad-1;
			uy = this->GetSize().y - this->_barBorder - (int)((this->_barMax + this->_barMin)*(float)barlength*0.5f) + 12*rad+1; 
			if ((x >= lx)&&(x <= ux)&&(y >= ly)&&(y <= uy))
			{
				this->_barMoving = true;
				this->_currentBarPosition = this->GetSize().y - this->_barBorder - (int)((this->_barMax + this->_barMin)*(float)barlength*0.5f) - y;
			}
			lx = this->GetSize().x - this->_barBorder - mrad;
			ux = this->GetSize().x - this->_barBorder + mrad;
			ly = this->GetSize().y - this->_barBorder - (int)((this->_barMin)*(float)barlength) - mrad;
			uy = this->GetSize().y - this->_barBorder - (int)((this->_barMin)*(float)barlength) + 2*mrad;
			if ((x >= lx) && (x <= ux) && (y >= ly) && (y <= uy))
			{
				this->_barMinMoving = true;
				this->_currentBarPosition = this->GetSize().y - this->_barBorder - (int)((this->_barMin)*(float)barlength) - y;
			}
			ly = this->GetSize().y - this->_barBorder - (int)((this->_barMax)*(float)barlength) - mrad;
			uy = this->GetSize().y - this->_barBorder - (int)((this->_barMax)*(float)barlength) + 2*mrad;
			if ((x >= lx) && (x <= ux) && (y >= ly) && (y <= uy))
			{
				this->_barMaxMoving = true;
				this->_barMinMoving = false;
				this->_currentBarPosition = this->GetSize().y - this->_barBorder - (int)((this->_barMax)*(float)barlength) - y;
			}
		}
	}
}

void VolumePanel::OnLeftMouseUp(wxMouseEvent& evt)
{
	this->_mouseDown = false;
	this->_barMinMoving = false;
	this->_barMaxMoving = false;
	this->_barMoving = false;
}

void VolumePanel::OnLeftMouseMove(wxMouseEvent& evt)
{
	float barlength = (float)(this->GetSize().y - 2*this->_barBorder);
	float rad = std::max((float)this->_barWidth / 2, 1.0f);
	if (this->_mouseDown)
	{
		this->_trackBall.Set_Position(evt.GetPosition().x, evt.GetPosition().y);
	}
	if (this->_barMoving)
	{
		float diff = this->_barMax - this->_barMin;
		int x = evt.GetPosition().x;
		int y = evt.GetPosition().y;
		float yf = ((float)this->GetSize().y - (float)this->_barBorder - (float)y) / barlength - (float)this->_currentBarPosition/barlength;
		
		this->_barMax = std::min(yf+diff/2.0f, 1.0f);
		this->_barMin = std::max(this->_barMax - diff, 0.0f);
		this->_barMin = std::max(0.0f, this->_barMin);
		this->_barMax = this->_barMin + diff;
	}
	if (this->_barMinMoving)
	{	
		float diff = this->_barMax - this->_barMin;
		int y = evt.GetPosition().y;
		float yf = ((float)this->GetSize().y - (float)this->_barBorder - (float)y) / barlength - (float)this->_currentBarPosition / barlength;
		this->_barMin = std::max(yf, 0.0f);
		this->_barMin = std::min(this->_barMax - 0.01f, this->_barMin);
	}
	else if (this->_barMaxMoving)
	{
		float diff = this->_barMax - this->_barMin;
		int y = evt.GetPosition().y;
		float yf = ((float)this->GetSize().y - (float)this->_barBorder - (float)y) / barlength - (float)this->_currentBarPosition / barlength;
		this->_barMax = std::min(yf, 1.0f);
		this->_barMax = std::max(this->_barMin + 0.01f, this->_barMax);
	}		
	if ((this->_barMoving) || (this->_barMaxMoving) || (this->_barMinMoving))
	{
		if (this->_updateEvent)
		{
			this->_updateEvent->On_Update_Bar(this->_barMin, this->_barMax);
		}
	}
	this->_solidBoxMin = Vec3(0, 0,this->_barMin);
	this->_solidBoxMax = Vec3(1,1, this->_barMax);
	this->Refresh();
}

void VolumePanel::On_Update()
{
	this->_locker.Lock();
	float dd = 0.005f;
	bool redraw = false;
	if (!this->_lockRefresh)
	{
		if (this->_graphicBuffer.Loaded_Texture_3D_GL())
		{
			if (this->_updateEvent)
			{
				this->_updateEvent->_painting = true;
			}
			glUseProgram(0);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glClear(GL_DEPTH_BUFFER_BIT);
			double stream[16];
			glMatrixMode(GL_PROJECTION);
			this->_projectionMatrix.Stream(stream);
			glLoadMatrixd((GLdouble*)stream);

			glMatrixMode(GL_MODELVIEW);
			Mtx4 mv = this->_trackBall.Affine_Matrix();
			mv.Stream(stream);
			glLoadMatrixd((GLdouble*)stream);

			glColor3f((float)this->_controlColor.Red() / 255.0f, (float)this->_controlColor.Green() / 255.0f, (float)this->_controlColor.Blue() / 255.0f);
			
			Vec3 box = 0.5*this->_cubeVol;
			
			glBegin(GL_LINES);
			glVertex3f(-box(eX), -box(eY), -box(eZ));
			glVertex3f(box(eX), -box(eY), -box(eZ));

			glVertex3f(box(eX), -box(eY), -box(eZ));
			glVertex3f(box(eX), box(eY), -box(eZ));

			glVertex3f(box(eX), box(eY), -box(eZ));
			glVertex3f(-box(eX), box(eY), -box(eZ));

			glVertex3f(-box(eX), box(eY), -box(eZ));
			glVertex3f(-box(eX), -box(eY), -box(eZ));

			glVertex3f(-box(eX), -box(eY), box(eZ));
			glVertex3f(box(eX), -box(eY), box(eZ));

			glVertex3f(box(eX), -box(eY), box(eZ));
			glVertex3f(box(eX), box(eY), box(eZ));

			glVertex3f(box(eX), box(eY), box(eZ));
			glVertex3f(-box(eX), box(eY), box(eZ));

			glVertex3f(-box(eX), box(eY), box(eZ));
			glVertex3f(-box(eX), -box(eY), box(eZ));

			glVertex3f(-box(eX), -box(eY), -box(eZ));
			glVertex3f(-box(eX), -box(eY), box(eZ));
			glVertex3f(-box(eX), box(eY), -box(eZ));
			glVertex3f(-box(eX), box(eY), box(eZ));

			glVertex3f(box(eX), -box(eY), -box(eZ));
			glVertex3f(box(eX), -box(eY), box(eZ));
			glVertex3f(box(eX), box(eY), -box(eZ));
			glVertex3f(box(eX), box(eY), box(eZ));
			glEnd();

			glColor3f((float)this->_revMarker.Red() / 255.0f, (float)this->_revMarker.Green() / 255.0f, (float)this->_revMarker.Blue() / 255.0f);
			Vec3 sboxmin = this->_solidBoxMin;
			Vec3 sboxmax = this->_solidBoxMax;
			sboxmin(eX, sboxmin(eX)*this->_cubeVol(eX))(eY,sboxmin(eY)*this->_cubeVol(eY))(eZ, sboxmin(eZ)*this->_cubeVol(eZ));
			sboxmax(eX, sboxmax(eX)*this->_cubeVol(eX))(eY, sboxmax(eY)*this->_cubeVol(eY))(eZ, sboxmax(eZ)*this->_cubeVol(eZ));

			glBegin(GL_LINES);
			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmin(eZ) -box(eZ) - dd);
			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmin(eZ) - box(eZ) - dd);

			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmin(eZ) - box(eZ) - dd);
			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmin(eZ) - box(eZ) - dd);

			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmin(eZ) - box(eZ) - dd);
			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmin(eZ) - box(eZ) - dd);

			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmin(eZ) - box(eZ) - dd);
			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmin(eZ) - box(eZ) - dd);


			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmax(eZ) - box(eZ) + dd);
			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmax(eZ) - box(eZ) + dd);

			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmax(eZ) - box(eZ) + dd);
			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmax(eZ) - box(eZ) + dd);

			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmax(eZ) - box(eZ) + dd);
			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmax(eZ) - box(eZ) + dd);

			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmax(eZ) - box(eZ) + dd);
			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmax(eZ) - box(eZ) + dd);


			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmin(eZ) - box(eZ) - dd);
			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmax(eZ) - box(eZ) + dd);

			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmin(eZ) - box(eZ) - dd);
			glVertex3f((float)sboxmin(eX) - box(eX) - dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmax(eZ) - box(eZ) + dd);

			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmin(eZ) - box(eZ) - dd);
			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmin(eY) - box(eY) - dd, (float)sboxmax(eZ) - box(eZ) + dd);

			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmin(eZ) - box(eZ) - dd);
			glVertex3f((float)sboxmax(eX) - box(eX) + dd, (float)sboxmax(eY) - box(eY) + dd, (float)sboxmax(eZ) - box(eZ) + dd);

			glEnd();

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glUseProgram(this->_shader.ProgramIDX());
			float fstream[16];
			glUniform3f(this->_idxCameraPositionGL, (float)this->_cameraPosition(0), 
				(float)this->_cameraPosition(1), (float)this->_cameraPosition(2));
			mv.FStream(fstream);
			glUniformMatrix4fv(this->_idxModelMatrixGL, 1, GL_FALSE, (GLfloat*)fstream);
			mv.Transposed().FStream(fstream);
			glUniformMatrix4fv(this->_idxModelMatrixTransposedGL, 1, GL_FALSE, (GLfloat*)fstream);
			wxColor back(245, 245, 255);
			glUniform3f(this->_idxBackgroundColorGL, (GLfloat)back.Green() / (GLfloat)255,
				(GLfloat)back.Red() / (GLfloat)255,
					(GLfloat)back.Blue() / (GLfloat)255);
			glUniform3f(this->_object3DColorGL, (GLfloat)this->_objectColor(eX),
				(GLfloat)this->_objectColor(eY),
					(GLfloat)this->_objectColor(eZ));
			glUniform3f(this->_idxMinVolumeGL, (GLfloat)this->_solidBoxMin(eX),
				(GLfloat)this->_solidBoxMin(eY),
					(GLfloat)this->_solidBoxMin(eZ));
			glUniform3f(this->_idxMaxVolumeGL, (GLfloat)this->_solidBoxMax(eX),
				(GLfloat)this->_solidBoxMax(eY),
					(GLfloat)this->_solidBoxMax(eZ));
			glUniform3f(this->_idxMinVolumeColorGL, (GLfloat)this->_solidMinVol(eX),
				(GLfloat)this->_solidMinVol(eY),
					(GLfloat)this->_solidMinVol(eZ));
			glUniform3f(this->_idxMaxVolumeColorGL, 
				(GLfloat)this->_solidMaxVol(eX),
					(GLfloat)this->_solidMaxVol(eY), (GLfloat)this->_solidMaxVol(eZ));
			glUniform3f(this->_idxBoxMinGL,	(GLfloat)this->_boxMin(eX),
				(GLfloat)this->_boxMin(eY),
					(GLfloat)this->_boxMin(eZ));
			glUniform3f(this->_idxBoxMaxGL, (GLfloat)this->_boxMax(eX),
				(GLfloat)this->_boxMax(eY),
					(GLfloat)this->_boxMax(eZ));
			glUniform3f(this->_idxBoxColorGL, (GLfloat)this->_boxColor(eX),
				(GLfloat)this->_boxColor(eY),
					(GLfloat)this->_boxColor(eZ));

			glUniform3f(this->_idxCubeVol,(GLfloat)this->_cubeVol(eX),
				(GLfloat)this->_cubeVol(eY),
				(GLfloat)this->_cubeVol(eZ));
			glUniform1i(this->_idxDepthPrecisionGL, this->_depthPrecision);

			this->_graphicBuffer.Flush_Buffer();
			glUseProgram(0);		
			glDisable(GL_BLEND);
			glColor3f(this->_boxColor(eX), this->_boxColor(eY), this->_boxColor(eZ));
			glClear(GL_DEPTH_BUFFER_BIT);
			glBegin(GL_LINE_LOOP);
			Vec3 mx = this->_boxMax;
			Vec3 ix = this->_boxMin;
			mx(eX, mx(eX)*this->_cubeVol(eX))(eY, mx(eY)*this->_cubeVol(eY))(eZ, mx(eZ)*this->_cubeVol(eZ));
			ix(eX, ix(eX)*this->_cubeVol(eX))(eY, ix(eY)*this->_cubeVol(eY))(eZ, ix(eZ)*this->_cubeVol(eZ));
			mx = mx - box;
			ix = ix - box;
			glVertex3f(mx(eX), mx(eY), mx(eZ));
			glVertex3f(mx(eX), mx(eY), ix(eZ));
			glVertex3f(mx(eX), ix(eY), ix(eZ));
			glVertex3f(mx(eX), ix(eY), mx(eZ));
			glVertex3f(mx(eX), mx(eY), mx(eZ));
			glVertex3f(ix(eX), mx(eY), mx(eZ));
			glVertex3f(ix(eX), ix(eY), mx(eZ));
			glVertex3f(ix(eX), ix(eY), ix(eZ));
			glVertex3f(ix(eX), mx(eY), ix(eZ));
			glVertex3f(mx(eX), mx(eY), ix(eZ));
			glVertex3f(mx(eX), ix(eY), ix(eZ));
			glVertex3f(ix(eX), ix(eY), ix(eZ));
			glVertex3f(ix(eX), mx(eY), ix(eZ));
			glVertex3f(ix(eX), mx(eY), mx(eZ));
			glVertex3f(mx(eX), mx(eY), mx(eZ));
			glVertex3f(mx(eX), ix(eY), mx(eZ));
			glVertex3f(ix(eX), ix(eY), mx(eZ));
			glVertex3f(ix(eX), mx(eY), mx(eZ));

			glEnd();


			if (this->_updateEvent)
			{
				if (!this->_updateEvent->_blocked)
				{
					this->_updateEvent->On_Update();
				}
			}
			glClear(GL_DEPTH_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);
			if (this->_updateEvent)
			{
				this->_updateEvent->_painting = false;
			}
		}
		else
		{
			this->Load_Texture_To_Graphic_Device();
			redraw = true;
		}
		this->Draw_Bar();
	}
	this->_locker.Unlock();
	if (redraw)
	{
		if (this->_graphicBuffer.Loaded_Texture_3D_GL())
		{
			this->Refresh();
		}
	}
}

void VolumePanel::Set_Box_Color(const wxColor& color)
{
	float r = (float)color.Red()/255.0f;
	float g = (float)color.Green() / 255.0f;
	float b = (float)color.Blue() / 255.0f;
	this->_boxColor = Vec3(r, g, b);
	this->Refresh();
}

void VolumePanel::Set_Mark_Box(float xmin, float ymin, float zmin,
	float xmax, float ymax, float zmax)
{
	this->_boxMin = Vec3(xmin, ymin, zmin);
	this->_boxMax = Vec3(xmax, ymax, zmax);
	this->Refresh();
}


void VolumePanel::Draw_Circle(float radius, float x, float y, float scx, float scy, 
	const wxColor& border_color, const wxColor& color, int res)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0, this->GetSize().x, this->GetSize().y, 0, -1, 1);

	float PI = 3.14f;
	glColor3f((float)color.Red() / 255.0f, 
		(float)color.Green() / 255.0f, (float)color.Blue() / 255.0f);
	glLineWidth(1);
	glTranslatef(x+radius*scx, y+radius*scy,0.0f);
	glScalef(scx, scy, 1.0f);
	float r = (float)radius;
	glBegin(GL_TRIANGLES);
	for (int k = 0; k < res; ++k)
	{
		glVertex2f(0, 0);
		float x = r * cos(2 * PI*(float)k / (float)res);
		float y = r * sin(2 * PI*(float)k / (float)res);
		glVertex2f(x, y);
		x = r * cos(2 * PI*(float)(k+1) / (float)res);
		y = r * sin(2 * PI*(float)(k+1) / (float)res);
		glVertex2f(x, y);
	}
	glEnd();

	glColor3f((float)border_color.Red() / 255.0f, 
		(float)border_color.Green() / 255.0f, (float)border_color.Blue() / 255.0f);
	glBegin(GL_LINE_LOOP);
	for (int k = 0; k < res; ++k)
	{
		float x = r * cos(2 * PI*(float)k / (float)res);
		float y = r * sin(2 * PI*(float)k / (float)res);
		glVertex2f(x, y);
	}
	glEnd();
}

void VolumePanel::Draw_Bar()
{
	float px;
	float py;

	float rad = std::max((float)this->_barWidth / 2, 1.0f);
	float barlength = (float)(this->GetSize().y - 2 * this->_barBorder);

	px = (float)this->GetSize().x - (float)this->_barBorder - rad;
	py = (float)this->GetSize().y / 2 - rad * barlength / (float)this->_barWidth;

	this->Draw_Circle(rad, px, py, 1.0f, barlength / ((float)this->_barWidth),
		this->_controlColor, this->_controlColor);

	float mrad = ((float)this->_controlFactor) * rad;

	px = (float)this->GetSize().x - (float)this->_barBorder - mrad;
	py = (float)this->GetSize().y - this->_barBorder - barlength * this->_barMin;

	this->Draw_Circle(mrad, px, py, 1.0f, 1.0f,
		this->_borderColor, this->_controlColor);

	px = (float)this->GetSize().x - (float)this->_barBorder - mrad;
	py = (float)this->GetSize().y - this->_barBorder - barlength * this->_barMax;

	this->Draw_Circle(mrad, px, py, 1.0f, 1.0f,
		this->_borderColor, this->_lightControlColor);

	px = (float)this->GetSize().x - (float)this->_barBorder + 2 * mrad + rad;
	py = (float)this->GetSize().y - this->_barBorder - (this->_barMax + this->_barMin)*0.5f*barlength - 4 * rad;

	this->Draw_Circle(rad, px, py, 1, 8, this->_borderColor, this->_controlColor);
}
	

void VolumePanel::Set_Solid_Color(wxColor Sim_Color)
{
	this->_objectColor(eX, (float)Sim_Color.Red() / (float)255);
	this->_objectColor(eY, (float)Sim_Color.Green() / (float)255);
	this->_objectColor(eZ, (float)Sim_Color.Blue() / (float)255);
}

void VolumePanel::On_Resize(wxSizeEvent& evt)
{
	this->WxGLPanel::On_Resize(evt);
	if ((evt.GetSize().x > 0) && (evt.GetSize().y > 0))
	{
		this->Define_Projection_Matrix();
		this->_trackBall.Set_Size(this->GetSize().x, this->GetSize().y);
		this->_voxelFactor = (float)1.0;
		if (this->_imageTextureList.size() > 0)
		{
			wxImage* fi = this->_imageTextureList[0];
			this->_voxelFactor = (float)(3*this->GetSize().y) / (float)(2.0*fi->GetSize().y);
			if (this->_voxelFactor < 1.0f)
			{
				this->_voxelFactor = 1.0f;
			}
		}
		this->Refresh();
	}
}


void VolumePanel::AddImage(const wxImage& img)
{
	this->_locker.Lock();
	this->_lockRefresh = true;
	this->_locker.Unlock();
	wxImage* nimg = new wxImage(img);
	this->_imageTextureList.push_back(nimg);
}


void VolumePanel::Load_Texture_To_Graphic_Device()
{
	if (this->_imageTextureList.size() > 0)
	{
		this->Set_GL_Context();	
		this->_cubeMin = Vec3(0,0,0);
		this->_cubeMax = Vec3(this->_imageTextureList[0]->GetSize().x, this->_imageTextureList[0]->GetSize().y,
			this->_imageTextureList.size());
		GLint id = GL_INVALID_OPERATION;
		id = glGetUniformLocation(this->_shader.ProgramIDX(), "SamplerDataVolume");
		glBindSampler(0, id);

		this->_graphicBuffer.Load_3D_Texture(this->_imageTextureList);
		this->Setup_Cube();
		this->_graphicBuffer.Activate_Texture_3D_GL();

		wxImage* fi = this->_imageTextureList[0];
		this->_correctionFactor(eX, (float)1.0 / (float)fi->GetSize().x);
		this->_correctionFactor(eY, (float)1.0 / (float)fi->GetSize().y);
		this->_correctionFactor(eZ, (float)1.0 / (float)this->_imageTextureList.size());
		this->Define_Projection_Matrix();
	}
}

void VolumePanel::Load_3D_Texture()
{
	this->_lockRefresh = true;
	this->Load_Texture_To_Graphic_Device();
	this->_lockRefresh = false;
}

void VolumePanel::Release_3D_Texture()
{
	this->_locker.Lock();
	this->_graphicBuffer.Release_3D_Texture();
	for (int i = 0; i < this->_imageTextureList.size(); ++i)
	{
		wxImage* img = this->_imageTextureList[i];
		delete img;
	}
	this->_imageTextureList.clear();
	this->_graphicBuffer.Release_Graphic_Device();
	//this->_shader.Detach();
	this->_locker.Unlock();
}

void VolumePanel::Set_Min_Vol(const Vec3& vmin)
{
	this->_solidBoxMin = vmin;
	this->Refresh();
}

void VolumePanel::Set_Max_Vol(const Vec3& vmax)
{
	this->_solidBoxMax(eX, vmax(eX));
	this->_solidBoxMax(eY, vmax(eY));
	this->_solidBoxMax(eZ, vmax(eZ));
	this->Refresh();
}

void VolumePanel::Unlock_Refresh()
{
	this->_locker.Lock();
	this->_lockRefresh = false;
	this->_locker.Unlock();
}

void VolumePanel::Lock_Refresh()
{
	this->_locker.Lock();
	this->_lockRefresh = true;
	this->_locker.Unlock();
}

void VolumePanel::Set_Bar_Border(float bordermin, float bordermax)
{
	this->_barMin = 0;
	this->_barMax = 1;
	if (this->_updateEvent)
	{
		this->_updateEvent->On_Update_Bar(this->_barMin, this->_barMax);
	}
}

