#include "wxhst3d.h"


WinHst3D::WinHst3D(wxWindow* Parent, wxWindowID Id, const wxPoint& Position,
	const wxSize& Size, long Style) : wxWindow(Parent, Id, Position, Size, Style)
{
	this->_histogramPanel = new WxHst3D(this);
	wxBoxSizer* msizer = new wxBoxSizer(wxVERTICAL);
	msizer->Add(this->_histogramPanel, wxSizerFlags().Expand().Proportion(1));
	this->SetSizer(msizer);
	this->Bind(wxEVT_LEFT_DOWN, &WxHst3D::OnLeftMouseDown, this->_histogramPanel);
	this->Bind(wxEVT_RIGHT_DOWN, &WxHst3D::OnRightMouseDown, this->_histogramPanel);
	this->Bind(wxEVT_LEFT_UP, &WxHst3D::OnLeftMouseUp, this->_histogramPanel);
	this->Bind(wxEVT_LEAVE_WINDOW, &WxHst3D::OnLeftMouseUp, this->_histogramPanel);
	this->Bind(wxEVT_MOTION, &WxHst3D::OnLeftMouseMove, this->_histogramPanel);

}

WinHst3D::~WinHst3D()
{
}

WxHst3D::WxHst3D(wxWindow* Parent, wxWindowID Id, int* attrs, const wxPoint& Position, const wxSize& Size, long Style) :
	WxGLPanel(Parent, Id, attrs, Position, Size, Style)
{
	this->_selectedSeries = -1;
	this->_selectedCategory = -1;
	this->_drawCategorySubtitle = true;
	this->_drawSeriesNames = true;
	this->_drawTitle = true;
	this->_dindent = 0.025f;
	this->_digits = 1;
	this->_charHeight = 14;
	this->_charTitleHeight = 18;
	this->_displayCharList = 2048;
	this->_displayTitleList = 4096;
	this->_textIndent = 32;
	this->_categories = 40;
	this->_charListSet = false;
	this->_stepCategory = 1.0f;
	this->_axisColor = wxColor(50, 50, 50);
	this->_saxisColor = wxColor(180, 180, 180);
	this->_mouseDown = false;
	this->_cameraPosition(eZ, 10);
	this->Define_Projection_Matrix();
	this->_maxCategory = 1.0f;
}

void WxHst3D::OnLeftMouseUp(wxMouseEvent& evt)
{
	this->_mouseDown = false;
}

void WxHst3D::Set_Category_Subtitle(const wxString& subtitle)
{
	this->_categorySubtitle = subtitle;
}

void WxHst3D::OnLeftMouseMove(wxMouseEvent& evt)
{
	if (this->_mouseDown)
	{
		this->_trackball.Set_Position(evt.GetPosition().x, evt.GetPosition().y);
		this->Refresh();
	}	
}

void WxHst3D::Picking_Ray(int u, int v, math_la::math_lac::space::Vec3& direction, math_la::math_lac::space::Vec3& position)
{
	scalar uf = 2.0*((scalar)(u - this->GetSize().x / 2)) / (scalar)this->GetSize().x;
	scalar vf = -2.0*((scalar)(v - this->GetSize().y / 2)) / (scalar)this->GetSize().y;
	math_la::math_lac::space::Mtx3 m = this->_trackball.Rotation_Matrix();
	math_la::math_lac::space::Vec3 nex = m.Column(eX);
	math_la::math_lac::space::Vec3 ney = m.Column(eY);
	math_la::math_lac::space::Vec3 nez = m.Column(eZ);
	position = uf * nex + vf * ney - nez;
	direction = nez;
}

float WxHst3D::Intersect_Box(const math_la::math_lac::space::Vec3& direction, const math_la::math_lac::space::Vec3& position, int s, int k)
{
	float de = 0.02f;
	float yh = this->_seriesValues[s][k]/this->_maxCategory;
	float xmin = -0.5f + (float)k / (float)this->_categories;
	float xmax = -0.5f + (float)(k+1) / (float)this->_categories;
	float zmin = -0.5 + (float)s / (float)this->_seriesValues.size();
	float zmax = -0.5 + (float)(s+1) / (float)this->_seriesValues.size();
	
	math_la::math_lac::space::Vec3 ipos;
	float t = 1e26f;
	float ti = 0.0f;

	ti = (xmin - position(eX)) / direction(eX);
	ipos = ti * direction + position;
	if ((ipos(eZ) >= zmin-de) && (ipos(eZ) <= zmax+de)
		&&(ipos(eY) >= -0.5f-de)&&(ipos(eY) <= yh-0.5f+de)&&(ti < t))
	{
		t = ti;
	}
	ti = (xmax - position(eX)) / direction(eX);
	ipos = ti * direction + position;
	if ((ipos(eZ) >= zmin-de) && (ipos(eZ) <= zmax+de)
		&& (ipos(eY) >= -0.5f-de) && (ipos(eY) <= yh-0.5f+de) && (ti < t))
	{
		t = ti;
	}

	ti = (zmin - position(eZ)) / direction(eZ);
	ipos = ti * direction + position;
	if ((ipos(eX) >= xmin-de) && (ipos(eX) <= xmax+de)
		&& (ipos(eY) >= -0.5f-de) && (ipos(eY) <= yh-0.5f+de) && (ti < t))
	{
		t = ti;
	}

	ti = (zmax - position(eZ)) / direction(eZ);
	ipos = ti * direction + position;
	if ((ipos(eX) >= xmin-de) && (ipos(eX) <= xmax+de)
		&& (ipos(eY) >= -0.5f-de) && (ipos(eY) <= yh-0.5f+de) && (ti < t))
	{
		t = ti;
	}

	ti = (yh-0.5f - position(eY)) / direction(eY);
	ipos = ti * direction + position;
	if ((ipos(eX) >= xmin-de) && (ipos(eX) <= xmax+de)
		&& (ipos(eZ) >= zmin-de) && (ipos(eZ) <= zmax+de) && (ti < t))
	{
		t = ti;
	}
	return(t);
}

void WxHst3D::OnRightMouseDown(wxMouseEvent& evt)
{
	this->_selectedSeries = -1;
	this->_selectedCategory = -1;
	this->_trackball.Set_Size(this->GetSize().x, this->GetSize().y);
	math_la::math_lac::space::Vec3 direction;
	math_la::math_lac::space::Vec3 position;
	this->Picking_Ray(evt.GetPosition().x, evt.GetPosition().y, direction, position);
	float t = 2.0f;
	int selected_series = -1;
	int selected_category = -1;
	for (int s = 0; s < (int)this->_seriesValues.size(); ++s)
	{
		for (int k = 0; k < this->_categories; ++k)
		{
			float ti = this->Intersect_Box(direction, position, s, k);
			if (ti < t)
			{
				selected_series = s;
				selected_category = k;
				t = ti;
			}
		}
	}

	if (selected_series >= 0)
	{
		this->_selectedSeries = selected_series;
	}
	if (selected_category >= 0)
	{
		this->_selectedCategory = selected_category;
	}
	this->Refresh();
}

void WxHst3D::OnLeftMouseDown(wxMouseEvent& evt)
{
	this->_trackball.Set_Size(this->GetSize().x, this->GetSize().y);
	int x = evt.GetPosition().x;
	int y = evt.GetPosition().y;
	if (evt.GetButton() == wxMOUSE_BTN_LEFT)
	{
		this->_trackball.Set_Start_Position(x, y);
		this->_mouseDown = true;
	}
}

void WxHst3D::Define_Projection_Matrix()
{
	this->_trackball.Set_Size(this->GetSize().x, this->GetSize().y);

	scalar pasp = (scalar)this->GetSize().x / (scalar)this->GetSize().y;
	scalar pnear = 0.1;
	scalar pfar = 20;
	scalar ptop = pnear * tan(45 * M_PI / 180);
	scalar pright = ptop * pasp;
	scalar d = this->_cameraPosition(eZ);
	math_la::math_lac::space::Mtx4 pf = math_la::math_lac::space::Mtx4::Identity();
	pf(0, 0, pnear / pright);
	pf(1, 1, pnear / ptop);
	pf(2, 2, (pnear + pfar) / (pfar - pnear));
	pf(2, 3, 2 * pfar * pnear / (pfar - pnear));
	pf(3, 2, 1.0 / d);
	this->_projectionMatrix = pf;
}

void WxHst3D::Set_Max_Weight(float max)
{
	this->_maxCategory = max;
}

void WxHst3D::Add_Series(const wxColor& color, const wxString& name)
{
	this->_seriesNames.push_back(name);
	int r;
	int g;
	int b;
	r = std::max(color.Red() - 16, 0);
	g = std::max(color.Green() - 16, 0);
	b = std::max(color.Blue() - 16, 0);
	wxColor color_up(r,g,b);
	r = std::max(color.Red() - 32, 0);
	g = std::max(color.Green() - 32, 0);
	b = std::max(color.Blue() - 32, 0);
	wxColor color_side(r,g,b);
	Series_Color scolor;
	scolor.front_color = color;
	scolor.side_color = color_side;
	scolor.up_color = color_up;
	this->_seriesColor.push_back(scolor);
	float* seq = new float[this->_categories];
	for (int i = 0; i < this->_categories; ++i)
	{
		seq[i] = 0.0f;
	}
	this->_seriesValues.push_back(seq);
}

void WxHst3D::Set_Value(int id, int category, float value)
{
	id = id % (int)this->_seriesValues.size();
	category = category % (int)this->_categories;
	this->_seriesValues[id][category] = value;
}

void WxHst3D::Draw_Box(	int id, int category, float y, float dx, float dz, float sq,
						float3 f1color, float3 f2color, float3 f3color)
{
	float c = (float)category;
	float cz = (float)id;
	glBegin(GL_QUADS);
		glColor3f(f1color.x, f1color.y, f1color.z);
		glVertex3f(-sq+c*dx+dx,-sq,-sq+cz*dz);
		glVertex3f(-sq+c*dx + dx, -sq+y, -sq+cz*dz);
		glVertex3f(-sq+c*dx + dx, -sq + y, -sq+cz*dz+dz);
		glVertex3f(-sq+c*dx + dx, -sq, -sq+cz*dz + dz);

		glVertex3f(-sq + c * dx, -sq, -sq + cz * dz);
		glVertex3f(-sq + c * dx, -sq + y, -sq + cz * dz);
		glVertex3f(-sq + c * dx, -sq + y, -sq + cz * dz + dz);
		glVertex3f(-sq + c * dx, -sq, -sq + cz * dz + dz);

		glColor3f(f2color.x, f2color.y, f2color.z);
		glVertex3f(-sq+c*dx, -sq, -sq+cz*dz);
		glVertex3f(-sq+c*dx + dx, -sq, -sq+cz*dz);
		glVertex3f(-sq+c*dx + dx, -sq + y, -sq+cz*dz);
		glVertex3f(-sq+c*dx, -sq+y, -sq+cz*dz);

		glVertex3f(-sq + c * dx, -sq, -sq + cz * dz + dz);
		glVertex3f(-sq + c * dx + dx, -sq, -sq + cz * dz + dz);
		glVertex3f(-sq + c * dx + dx, -sq + y, -sq + cz * dz + dz);
		glVertex3f(-sq + c * dx, -sq + y, -sq + cz * dz + dz);


		glColor3f(f3color.x, f3color.y, f3color.z);
		glVertex3f(-sq+c*dx, -sq+y, -sq+cz*dz);
		glVertex3f(-sq+c*dx + dx, -sq+y, -sq+cz*dz);
		glVertex3f(-sq+c*dx + dx, -sq + y, -sq+dz+cz*dz);
		glVertex3f(-sq+c*dx, -sq + y, -sq+dz+cz*dz);			   
	glEnd();
}

void WxHst3D::Draw_Series(float dx, float dz, float sq)
{
	for (int k = 0; k < (int)this->_seriesValues.size(); ++k)
	{
		if (k == this->_selectedSeries)
		{
			glLineWidth(3.0f);
			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		}
		else
		{
			glLineWidth(1.0f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		wxColor series_color;
		series_color = this->_seriesColor[k].front_color;
		float3 f1_color((float)series_color.Red() / 255.0f, (float)series_color.Green() / 255.0f, (float)series_color.Blue() / 255.0f);
		series_color = this->_seriesColor[k].side_color;
		float3 f2_color((float)series_color.Red() / 255.0f, (float)series_color.Green() / 255.0f, (float)series_color.Blue() / 255.0f);
		series_color = this->_seriesColor[k].up_color;
		float3 f3_color((float)series_color.Red() / 255.0f, (float)series_color.Green() / 255.0f, (float)series_color.Blue() / 255.0f);
		this->Draw_One_Series(k, dx, dz, sq,f1_color,f2_color,f3_color);
		glLineWidth(1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void WxHst3D::Draw_One_Series(int id, float dx, float dz, float sq, 
							  float3 f1color, float3 f2color, float3 f3color)
{
	float* vals = this->_seriesValues[id];
	for (int k = 0; k < this->_categories; ++k)
	{
		float y = vals[k];
		this->Draw_Box(id, k, y/this->_maxCategory, dx, dz, sq,f1color,f2color,f3color);
	}
}

void WxHst3D::Draw_Title()
{
	glListBase(this->_displayTitleList);
	glColor3f(0, 0, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float charx = (float)this->_charTitleHeight / (1.0f*(float)this->GetSize().x);
	float y = 0.9f;
	float x = 0.0f;
	float z = 0.5f;
	glRasterPos3f(x-charx*(float)this->_graphTitle.Length()/2.0f,y,z);
	glCallLists(this->_graphTitle.Length(), GL_UNSIGNED_BYTE, this->_graphTitle);
}

void WxHst3D::Draw_Subtitle()
{
	glListBase(this->_displayCharList);
	glColor3f(0, 0, 0);
	float charx = (float)this->_charHeight / (2.0f*(float)this->GetSize().x);
	float y = -0.65f;
	float x = 0.0f;
	float z = -0.75f;
	glRasterPos3f(x - charx * (float)this->_categorySubtitle.Length() / 2.0f, y, z);
	glCallLists(this->_categorySubtitle.Length(), GL_UNSIGNED_BYTE, this->_categorySubtitle);

}


void WxHst3D::Set_Graph_Title(const wxString& title)
{
	this->_graphTitle = title;
}

void WxHst3D::Draw_Series_Names()
{
	glListBase(this->_displayCharList);
	float chary = 32.0f/(float)this->GetSize().y;
	float charx = 32.0f/(float)this->GetSize().x;
	float ds = 8.0f / (float)this->GetSize().y;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBegin(GL_QUADS);
	float ybeg = 0.8f;
	float ykbeg = ybeg;
	float xbeg = 0.4f;
	float zbeg = -0.5f;
	for (int k = 0; k < this->_seriesValues.size(); ++k)
	{
		wxColor series_color = this->_seriesColor[k].front_color;
		float3 f1_color((float)series_color.Red() / 255.0f, 
			(float)series_color.Green() / 255.0f, (float)series_color.Blue() / 255.0f);
		glColor3f(f1_color.x, f1_color.y, f1_color.z);
		glVertex3f(xbeg,ykbeg-(float)k*chary,zbeg);
		glVertex3f(xbeg+charx,ykbeg-(float)k*chary, zbeg);
		glVertex3f(xbeg+charx, ykbeg-(float)k*chary-chary, zbeg);
		glVertex3f(xbeg, ykbeg-(float)k*chary-chary,zbeg);
		ykbeg = ykbeg - ds;
	}
	glEnd();
	ykbeg = ybeg;
	glColor3f((float)this->_axisColor.Red() / 255.0f, (float)this->_axisColor.Green() / 255.0f,
		(float)this->_axisColor.Blue() / 255.0f);
	for (int k = 0; k < this->_seriesValues.size(); ++k)
	{
		wxString& txt = this->_seriesNames[k];
		glRasterPos3f(xbeg + 2.0f*charx, ykbeg - (float)k*chary-chary/1.35f,zbeg);
		glCallLists(txt.Length(), GL_UNSIGNED_BYTE, txt);
		ykbeg = ykbeg - ds;
	}
}


void WxHst3D::Draw_Grid()
{
	int nnsize = std::min(this->GetSize().x, this->GetSize().y);
	float sep = (2.0f*(float)this->_textIndent)/(float)nnsize;
	float chary = 2.0f*(float)this->_charHeight / (float)nnsize;
	float charx = (float)this->_charHeight / (2.0f*(float)nnsize);
	float sq = 0.5f;
	float dz = 1.0f;

	if (this->_seriesValues.size() > 0)
	{
		dz = dz / (float)this->_seriesValues.size();
	}
	glColor3f((float)this->_saxisColor.Red() / 255.0f, (float)this->_saxisColor.Green() / 255.0f,
		(float)this->_saxisColor.Blue() / 255.0f);

	float dx = 1.0f / (float)this->_categories;
	float sx = sep + dx;
	glBegin(GL_LINES);
	for (int k = 0; k < this->_categories+1; ++k)
	{
		if (sx > sep)
		{
			sx = 0;
			float px = (float)k*dx;
			glVertex3f(-sq + px, -sq, sq);
			glVertex3f(-sq + px, -sq, -sq - this->_dindent);

			glVertex3f(-sq + px, -sq, -sq - this->_dindent);
			glVertex3f(-sq + px, -sq - this->_dindent, -sq - this->_dindent);

			glVertex3f(-sq + px, sq, sq);
			glVertex3f(-sq + px, -sq, sq);
		}
		sx = sx + dx;
	}
	glEnd();
	sx = sep+dx;
	glListBase(this->_displayCharList);
	glColor3f((float)this->_axisColor.Red() / 255.0f, (float)this->_axisColor.Green() / 255.0f,
		(float)this->_axisColor.Blue() / 255.0f);
	for (int k = 0; k < this->_categories+1; ++k)
	{
		if (sx > sep)
		{
			sx = 0;
			wxString ss = wxString::FromDouble((float)k*this->_stepCategory, this->_digits);
			glRasterPos3f(-sq + k*dx-(float)ss.Length()*charx/2.0f, -sq-chary, -sq-4.0f*charx-this->_dindent);
			glCallLists(ss.Length(), GL_UNSIGNED_BYTE,ss);
		}
		sx = sx + dx;
	}
	float factor = 1.0f/pow(10, (float)(this->_digits));
	float sy = sep + factor;
	float ity = 0.0f;
	while (ity < this->_maxCategory + factor)
	{
		if (sy > sep/3.0f)
		{
			sy = 0.0f;
			wxString ss = wxString::FromDouble((double)ity, this->_digits);
			glRasterPos3f(-sq-3.0f*chary/2.0f,-sq + ity/this->_maxCategory-chary/3.0f, -sq-4.0f*charx-this->_dindent);
			glCallLists(ss.Length(), GL_UNSIGNED_BYTE, ss);
		}
		ity = ity + factor;
		sy = sy + factor;
	}
	if (this->_drawCategorySubtitle)
	{
		this->Draw_Subtitle();
	}
	sy = sep + factor;
	ity = 0.0f;
	glColor3f((float)this->_saxisColor.Red() / 255.0f, (float)this->_saxisColor.Green() / 255.0f,
		(float)this->_saxisColor.Blue() / 255.0f);
	glBegin(GL_LINES);
	while (ity < this->_maxCategory + factor)
	{
		float yy = ity / this->_maxCategory;
		if (sy > sep)
		{
			glVertex3f(-sq, -sq + yy, -sq - this->_dindent);
			glVertex3f(-sq-this->_dindent, -sq + yy, -sq - this->_dindent);

			glVertex3f(-sq, -sq + yy, -sq-this->_dindent);
			glVertex3f(-sq, -sq + yy,  sq);

			glVertex3f(-sq, -sq + yy,  sq);
			glVertex3f(sq,  -sq + yy,  sq);
		}
		ity = ity + factor;
		sy = sy + factor;
	}
	glEnd();

	glColor3f((float)this->_axisColor.Red() / 255.0f, (float)this->_axisColor.Green() / 255.0f,
		(float)this->_axisColor.Blue() / 255.0f);
	glBegin(GL_LINE_STRIP);
		glVertex3f(sq, -sq, sq);
		glVertex3f(sq, -sq, -sq);
		glVertex3f(-sq, -sq, -sq);
		glVertex3f(-sq, -sq, sq);

		glVertex3f(sq, -sq, sq);
		glVertex3f(sq, sq, sq);
		glVertex3f(-sq, sq, sq);
		glVertex3f(-sq, -sq, sq);

		glVertex3f(-sq, -sq, -sq);
		glVertex3f(-sq, sq, -sq);
		glVertex3f(-sq, sq, sq);
		glVertex3f(-sq, -sq, sq);
	glEnd();
	if (this->_seriesValues.size() > 0)
	{
		this->Draw_Series(dx, dz, sq);
		if (this->_drawSeriesNames)
		{
			this->Draw_Series_Names();
		}
	}
	if (this->_drawTitle)
	{
		this->Draw_Title();
	}
}

void WxHst3D::Set_Categories_Size(int categories_size)
{
	this->_categories = categories_size;
}

void WxHst3D::Set_Char_List()
{
	this->_hdc = this->GetHDC();
	HFONT font = CreateFont(this->_charHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
	SelectObject(this->_hdc, font);
	wglUseFontBitmaps(this->_hdc, 0, 255, this->_displayCharList);

	font = CreateFont(this->_charTitleHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Arial Black"));
	SelectObject(this->_hdc, font);
	wglUseFontBitmaps(this->_hdc, 0, 255, this->_displayTitleList);

	this->_charListSet = true;
}

void WxHst3D::On_Update()
{
	if (!this->_charListSet)
	{
		this->Set_Char_List();
	}
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	double stream[16];
	glMatrixMode(GL_PROJECTION);
	this->_projectionMatrix.Stream(stream);
	glLoadTransposeMatrixd((GLdouble*)stream);

	glMatrixMode(GL_MODELVIEW);
	math_la::math_lac::space::Mtx4 mv = this->_trackball.Affine_Matrix();
	mv.Stream(stream);
	glLoadMatrixd((GLdouble*)stream);
	glClear(GL_DEPTH_BUFFER_BIT);
	this->Draw_Grid();
	
}
