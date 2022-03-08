#include <algorithm>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <wx/graphics.h>
#include <wx/dcgraph.h>
#include <wx/dcsvg.h>
#include "wx_plotter.h"
#include "tbb/parallel_for.h"

using std::size;

WxPlotter::Pair::Pair()
{
	this->x = (float)0;
	this->y = (float)0;
}

WxPlotter::Pair::Pair(float x, float y)
{
	this->x = x;
	this->y = y;
}

float WxPlotter::Pair::Distance(const WxPlotter::Pair& p1, const WxPlotter::Pair& p2)
{
	return(fabs(p1.x - p2.x) + (fabs(p1.y - p2.y)));
}


bool WxPlotter::Pair::operator < (const WxPlotter::Pair& pt) const
{
	if (this->x == pt.x)
	{
		return(this->y < pt.y);
	}
	else
	{
		return(this->x < pt.x);
	}
}


WxPlotter::WxPlotter(wxWindow* Parent, wxWindowID Id, const wxPoint& Position,
	const wxSize& Size, long Style) : wxControl(Parent, Id, Position, Size, Style)
{
	this->SetBackgroundStyle(wxBG_STYLE_PAINT);
	this->SetBackgroundColour(wxColor(255, 255, 255));
	this->_enableHorizontalLegend = false;
	this->_editPlot = true;
	this->_blockedPlot = false;
	this->_changeIntervalYEvt = 0;
	this->_dynamicIntervalColor = wxColor(255, 255, 0);
	this->_intervalMarkerYMin = 1e18f;
	this->_intervalMarkerYMax = -1e18f;
	this->_intervalMarkerXMin = 1e18f;
	this->_intervalMarkerXMax = -1e18f;
	this->_hasTitles = false;
	this->_xTitleBorder = 80;
	this->_yTitleBorder = 60;
	this->_pixelSpacing = 50;
	this->_selectedCurve = -1;
	this->_markerDistance = 16;
	this->SetDoubleBuffered(true);
	this->_linearScale = true;
	this->_numberOfCurves = 0;
	this->Bind(wxEVT_SIZE, &WxPlotter::On_Resize, this);
	this->Bind(wxEVT_PAINT, &WxPlotter::On_Paint, this);
	this->Bind(wxEVT_ERASE_BACKGROUND, &WxPlotter::On_Erase_Background, this);
	this->Bind(wxEVT_MOTION, &WxPlotter::On_Mouse_Move_Event, this);
	this->Bind(wxEVT_LEFT_DOWN, &WxPlotter::Mouse_Button_Down, this);
	this->Bind(wxEVT_LEFT_UP, &WxPlotter::On_Mouse_Leave_Event, this);
	this->_mainAxisColor = wxColour(50, 50, 50);
	this->_secondaryAxisColor = wxColour(180, 180, 180);
	this->_borderColor = wxColour(80, 80, 80);
	this->_zeroColor = wxColour(250, 50, 50);
	this->_backgroundDefined = false;
	this->_labelsEnabled = true;
	srand(time(0));
	for (int i = 0; i < 9; ++i)
	{
		this->_log10Values[i] = (float)log10((double)(i + 2));
	}
	this->_drawMousePosition = true;
	this->_showingVerticalInterval = false;
	this->_showingHorizontalInterval = false;
	this->_precisionDigits = 4;
	this->_selectingMinimalXMarker = false;
	this->_selectingMinimalYMarker = false;
	this->_selectingMaximalXMarker = false;
	this->_selectingMaximalYMarker = false;
}

void WxPlotter::Block()
{
	this->_blockedPlot = true;
}

void WxPlotter::UnBlock()
{
	this->_blockedPlot = false;
}


void WxPlotter::On_Mouse_Leave_Event(wxMouseEvent& evt)
{
	this->_selectingMinimalYMarker = false;
	this->_selectingMaximalYMarker = false;
}

void WxPlotter::Set_Change_Interval_Y_Event(WxPlotter::ChangeIntervalYEvt* evt)
{
	this->_changeIntervalYEvt = evt;
}

void WxPlotter::Set_Change_Interval_X_Event(WxPlotter::ChangeIntervalXEvt* evt)
{
	this->_changeIntervalXEvt = evt;
}

void WxPlotter::Define_Y_Interval_Markers(float ymin, float ymax)
{
	this->_intervalMarkerYMin = ymin;
	this->_intervalMarkerYMax = ymax;
	if ((this->_changeIntervalYEvt))
	{
		this->_changeIntervalYEvt->On_Update_Y_Value(this->_intervalMarkerYMin, this->_intervalMarkerYMax);
	}
}

float WxPlotter::Ymin_Marker_Interval() const
{
	return(this->_intervalMarkerYMin);
}

float WxPlotter::Ymax_Marker_Interval() const
{
	return(this->_intervalMarkerYMax);
}


void WxPlotter::Set_Background_Bitmap(const wxBitmap& bmp)
{
	this->_backgroundBitmap = bmp;
	this->_backgroundDefined = true;
}

int WxPlotter::Number_Of_Curves() const
{
	return(this->_numberOfCurves);
}

void WxPlotter::On_Resize(wxSizeEvent& evt)
{
	if (!this->_blockedPlot)
	{
		this->Reset();
	}
}

int WxPlotter::Add_Curves(int n)
{
	int i = this->_numberOfCurves;
	this->_numberOfCurves = this->_numberOfCurves + n;
	for (int k = this->_curves.size(); k < this->_numberOfCurves; ++k)
	{
		set<Pair> curve;
		set<wxPoint,WxPlotter::wxPoint_Comparer> plotcurve;
		this->_curves.push_back(curve);
		this->_discreteCurves.push_back(plotcurve);
		this->_curveColors.push_back(this->Pick_Curve_Color());
	}
	return(i);
}

void WxPlotter::Add_Curve_Point(int id,const WxPlotter::Pair& pt)
{
	int n = this->_curves[id].size();
	id = id % this->_numberOfCurves;
	this->_curves[id].insert(pt);
	if (this->_curves[id].size() > n)
	{
		if (pt.x < this->_xmin)
		{
			this->_xmin = pt.x;
		}
		if (pt.x > this->_xmax)
		{
			this->_xmax = pt.x;
		}
		if (pt.y < this->_ymin)
		{
			this->_ymin = pt.y;
		}
		if (pt.y > this->_ymax)
		{
			this->_ymax = pt.y;
		}
	}
}

wxColor WxPlotter::Pick_Curve_Color() const
{
	wxColor col;
	int s[3];
	s[0]= rand() % 125;
	s[1] = rand() % 125;
	s[2] = rand() % 125;
	int s4 = rand() % 3;
	s[s4] = s[s4] + 50;
	col = wxColor(55 + s[0], 55 + s[1], 55 + s[2]);
	return(col);
}

void WxPlotter::Add_Curve_Point(int id,float x, float y)
{
	WxPlotter::Pair p;
	p.x = x;
	p.y = y;
	this->Add_Curve_Point(id,p);
	if (this->_curveColors.size() <= id)
	{
		while (this->_curveColors.size() <= id)
		{
			this->_curveColors.push_back(this->Pick_Curve_Color());
		}
	}
}

void WxPlotter::Set_Interval_Limits(float xmin, float xmax, float ymin, float ymax)
{
	this->_xmin = xmin;
	this->_xmax = xmax;
	this->_ymax = ymax;
	this->_ymin = ymin;
	this->Adjust(this->GetSize().x,this->GetSize().y);
}


void WxPlotter::Adjust(int wx, int wy)
{
	if (this->_linearScale)
	{
		if (wx - 2*this->_xTitleBorder <= 0)
		{
			this->_xPixConverter = 1e5;
		}
		else
		{
			this->_xPixConverter = (this->_xmax - this->_xmin) / (float)(wx-2*this->_xTitleBorder);
		}
	}
	else
	{
		float s;
		int si;
		s = log10(this->_xmin);
		si = (int)s;
		if (fabs(s - (float)si) > 0)
		{
			--si;
		}
		this->_xmin = pow(10, si);

		s = log10(this->_xmax);
		si = (int)s;
		if (fabs(s - (float)si) > 0)
		{
			++si;
		}
		this->_xmax = pow(10, si);
		
		if (wx -2 * this->_xTitleBorder <= 0)
		{
			this->_xPixConverter = 1e5;
		}
		else
		{
			this->_xPixConverter = (log10(this->_xmax) - log10(this->_xmin)) / (float)(wx-2*this->_xTitleBorder);
		}
	}
	if (this->_xPixConverter == 0)
	{
		this->_xPixConverter = 1;
	}
	if (wy - 2 * this->_yTitleBorder <= 0)
	{
		this->_yPixConverter = 1e5;
	}
	else
	{
		this->_yPixConverter = (this->_ymax - this->_ymin) / (float)(wy-2*this->_yTitleBorder);
	}
	if (this->_yPixConverter == 0)
	{
		this->_yPixConverter = 1;
	}
	float s = fabs(this->_xmax - this->_xmin);
	int div = (wx - 2 * this->_xTitleBorder)/ this->_pixelSpacing;
	if (div % 2 == 1)
	{
		++div;
	}
	if (div <= 0)
	{
		div = 2;
	}
	this->_axisStepSizeX = s / div;
	if (this->_axisStepSizeX == 0)
	{
		this->_axisStepSizeX = s;
	}
	
	s = fabs(this->_ymax - this->_ymin);
	div = (wy-2*this->_yTitleBorder) / this->_pixelSpacing;
	if (div % 2 == 1)
	{
		++div;
	}
	if (div <= 0)
	{
		div = 2;
	}
	this->_axisStepSizeY = s / div;
	if (this->_axisStepSizeY == 0)
	{
		this->_axisStepSizeY = s;
	}
	if (this->_curves.size() > 0)
	{
		if (this->_linearScale)
		{
			tbb::parallel_for(tbb::blocked_range<int>(0, this->_curves.size(), std::max((int)(this->_curves.size() / 16),1)),
				[this, wy, wx](const tbb::blocked_range<int>& b)
			{
				for (int k = b.begin(); k < b.end(); ++k)
				{
					this->_discreteCurves[k].clear();
					const set<WxPlotter::Pair>& curve = this->_curves[k];
					set<WxPlotter::Pair>::const_iterator ci = curve.begin();
					while (ci != curve.end())
					{
						int x = (int)((ci->x - this->_xmin) / (this->_xPixConverter));
						int y = (int)((ci->y - this->_ymin) / (this->_yPixConverter));
						wxPoint p;
						p.x = this->_xTitleBorder + x;
						p.y = wy - this->_yTitleBorder - y;
						this->_discreteCurves[k].insert(p);
						++ci;
					}
				}
			});
		}
		else
		{
			tbb::parallel_for(tbb::blocked_range<int>(0, this->_curves.size(), std::max((int)(this->_curves.size() / 16),1)),
				[this, wy, wx](const tbb::blocked_range<int>& b)
			{
				for (int k = b.begin(); k < b.end(); ++k)
				{
					this->_discreteCurves[k].clear();
					const set<WxPlotter::Pair>& curve = this->_curves[k];
					set<WxPlotter::Pair>::const_iterator ci = curve.begin();
					while (ci != curve.end())
					{
						int x = (int)((log10(ci->x) - log10(this->_xmin)) / (this->_xPixConverter));
						int y = (int)((ci->y - this->_ymin) / (this->_yPixConverter));
						wxPoint p;
						p.x = this->_xTitleBorder + x;
						p.y = wy - this->_yTitleBorder - y;
						this->_discreteCurves[k].insert(p);
						++ci;
					}
				}
			});
		}
	}
}

void WxPlotter::On_Mouse_Move_Event(wxMouseEvent& evt)
{
	if (!this->_blockedPlot)
	{
		this->_editPlot = false;
		if ((this->_selectingMaximalYMarker) || (this->_selectingMinimalYMarker))
		{
			bool updateint = false;
			if (evt.Dragging())
			{
				float y = (float)evt.GetPosition().y;
				y = (-y + this->GetSize().y - this->_yTitleBorder)*this->_yPixConverter + this->_ymin;
				if (this->_selectingMinimalYMarker)
				{
					if (y < this->_intervalMarkerYMax)
					{
						if (this->_intervalMarkerYMin != y)
						{
							this->_intervalMarkerYMin = y;
							updateint = true;
						}
					}
				}
				else
				{
					if (y > this->_intervalMarkerYMin)
					{
						if (this->_intervalMarkerYMax != y)
						{
							this->_intervalMarkerYMax = y;
							updateint = true;
						}
					}
				}
			}
			if ((this->_changeIntervalYEvt) && (updateint))
			{
				this->_changeIntervalYEvt->On_Update_Y_Value(this->_intervalMarkerYMin, this->_intervalMarkerYMax);
			}
		}
		if ((this->_selectingMaximalXMarker) || (this->_selectingMinimalXMarker))
		{
			bool updateint = false;
			if (evt.Dragging())
			{
				float fxmin = this->_xmin;
				if (!this->_linearScale)
				{
					fxmin = log10(this->_xmin);
				}
				float x = (float)evt.GetPosition().x;
				x = (x - this->_xTitleBorder)*this->_xPixConverter + fxmin;
				if (this->_selectingMinimalXMarker)
				{
					if (x < this->_intervalMarkerXMax)
					{
						if (this->_intervalMarkerXMin != x)
						{
							this->_intervalMarkerXMin = x;
							updateint = true;
						}
					}
				}
				else
				{
					if (x > this->_intervalMarkerXMin)
					{
						if (this->_intervalMarkerXMax != x)
						{
							this->_intervalMarkerXMax = x;
							updateint = true;
						}
					}
				}
			}
			if ((this->_changeIntervalXEvt) && (updateint))
			{
				if (this->_linearScale)
				{
					this->_changeIntervalXEvt->On_Update_X_Value(this->_intervalMarkerXMin, this->_intervalMarkerXMax);
				}
				else
				{
					this->_changeIntervalXEvt->On_Update_X_Value(pow(10,this->_intervalMarkerXMin),pow(10,this->_intervalMarkerXMax));
				}

			}
		}
		this->_selectedCurve = -1;
		int d = this->_markerDistance;
		int x = evt.GetPosition().x;
		int y = evt.GetPosition().y;
		wxPoint ipp(x, y);
		for (int i = 0; i < this->_discreteCurves.size(); ++i)
		{
			set<wxPoint,WxPlotter::wxPoint_Comparer>& cvs = this->_discreteCurves[i];
			set<wxPoint, WxPlotter::wxPoint_Comparer>::iterator jj = cvs.lower_bound(ipp);
			if (jj != cvs.end())
			{
				wxPoint pp = *jj;
				int dd = WxPlotter::wxPoint_Comparer::Distance(pp, ipp);
				if (dd < d)
				{
					this->_discreteMousePositon = pp;
					d = dd;
					this->_selectedCurve = i;
				}
				if (jj != cvs.begin())
				{
					--jj;
					pp = *jj;
					dd = WxPlotter::wxPoint_Comparer::Distance(pp, ipp);
					if (dd < d)
					{
						this->_discreteMousePositon = pp;
						d = dd;
						this->_selectedCurve = i;
					}
				}
			}
		}
		this->_editPlot = true;
		this->Refresh();
	}
}


void WxPlotter::Draw_Point_Marks(wxDC& dc, int wx, int wy)
{
	wxBrush brush;
	brush.SetStyle(wxBRUSHSTYLE_SOLID);
	wxPen pen;
	for (int i = 0; i < this->_discretePointMarks.size(); ++i)
	{
		WxPlotter::PointMark p = this->_discretePointMarks[i];
		int x;
		int y;
		if (!this->_linearScale)
		{
			x = (int)((log10(p.Point.x) - log10(this->_xmin)) / (this->_xPixConverter));
			y = (int)((p.Point.y - this->_ymin) / (this->_yPixConverter));
		}
		else
		{
			x = (int)((p.Point.x - this->_xmin) / (this->_xPixConverter));
			y = (int)((p.Point.y - this->_ymin) / (this->_yPixConverter));
		}
		brush.SetColour(p.Color);
		pen.SetColour(p.Color);
		dc.SetBrush(brush);
		dc.SetPen(pen);
		dc.DrawEllipse(this->_xTitleBorder+x-4, wy - this->_yTitleBorder - y-4, 8, 8);
	}
}

void WxPlotter::Adjust_Axis()
{
	this->Adjust(this->GetSize().x, this->GetSize().y);
}

void WxPlotter::Reset()
{
	this->Adjust(this->GetSize().x, this->GetSize().y);
	this->Refresh();
}

void WxPlotter::Draw_Log10_Axis(wxDC& dc, int wx, int wy, bool screen)
{
	wxPen zpen;
	zpen.SetColour(this->_zeroColor);
	zpen.SetWidth(1);
	wxPen pengrid;
	pengrid.SetColour(this->_secondaryAxisColor);	
	wxPen pen;
	pen.SetColour(this->_mainAxisColor);
	pen.SetWidth(1);

	pengrid.SetStyle(wxPENSTYLE_SOLID);
	pen.SetStyle(wxPENSTYLE_SOLID);

	wxPen penborder;
	penborder.SetColour(this->_borderColor);
	penborder.SetStyle(wxPENSTYLE_SOLID);
	penborder.SetWidth(2);

	wxFont font;
	font.SetSymbolicSize(wxFONTSIZE_SMALL);
	dc.SetFont(font);
	int lw = dc.GetCharWidth();
	int lh = dc.GetCharHeight();

	int ixmin = (int)log10(this->_xmin);
	int ixmax = (int)log10(this->_xmax);
	float xmin = (float)ixmin;
	float xmax = (float)ixmax;
	int x;
	int y;
	int yi;
	float val;
	wxString s; 
	val = this->_ymin + this->_axisStepSizeY;
	while (val < this->_ymax - this->_axisStepSizeY * this->_yPixConverter / 2.0f)
	{
		y = (int)((val - this->_ymin) / this->_yPixConverter);
		x = (int)((-this->_xmin) / this->_xPixConverter);
		yi = wy - this->_yTitleBorder - y;
		if ((yi >= this->_yTitleBorder - 2) && (yi <= wy - this->_yTitleBorder + 2))
		{
			dc.SetPen(pengrid);
			dc.DrawLine(wxPoint(this->_xTitleBorder, yi),
				wxPoint(wx - this->_xTitleBorder, yi));
			dc.SetPen(penborder);
			dc.DrawLine(wxPoint(wx - this->_xTitleBorder - 2, yi),
				wxPoint(wx - this->_xTitleBorder + 3, yi));
			if (this->_labelsEnabled)
			{
				s = wxString::FromDouble(val, this->_precisionDigits);
				if (val >= 0)
				{
					s = wxString(" ") + s;
				}
				dc.SetPen(pen);
				dc.SetTextForeground(this->_mainAxisColor);
				dc.DrawText(s, wx - this->_xTitleBorder + lw, wy - this->_yTitleBorder - y - lh / 2 - 1);
			}
		}
		val = val + this->_axisStepSizeY;
	}
	val = this->_ymin;
	y = 0;
	s = wxString::FromDouble(val, this->_precisionDigits);
	dc.SetPen(pen);
	dc.SetTextForeground(this->_mainAxisColor);
	dc.DrawText(s, wx - this->_xTitleBorder + lw, wy - this->_yTitleBorder - y - lh / 2 - 1);

	val = this->_ymax;
	y = (int)((this->_ymax - this->_ymin) / this->_yPixConverter);
	s = wxString::FromDouble(val, this->_precisionDigits);
	if (val > 0)
	{
		s = wxString(" ") + s;
	}
	dc.SetPen(pen);
	dc.SetTextForeground(this->_mainAxisColor);
	dc.DrawText(s, wx - this->_xTitleBorder + lw, wy - this->_yTitleBorder - y - lh / 2 - 1);

	val = xmin;
	float divx = (float)(wx - 2 * this->_xTitleBorder);
	if (divx == 0)
	{
		divx = (float)1;
	}
	float xpix = (xmax - xmin) / divx;
	while (val <= xmax)
	{
		for (int i = 0; i < 9; ++i)
		{
			float cax = val + this->_log10Values[i];
			x = (int)((cax - xmin) / xpix);
			dc.SetPen(pengrid);
			if (i == 8)
			{
				x = (int)((val*this->_log10Values[i] - xmin) / xpix);
				dc.SetPen(pen);
			}
			if (this->_xTitleBorder + x <= wx - this->_xTitleBorder)
			{
				dc.DrawLine(wxPoint(this->_xTitleBorder + x, this->_yTitleBorder),
					wxPoint(this->_xTitleBorder + x, wy - this->_yTitleBorder));
				if (i == 8)
				{
					dc.SetPen(penborder);
					dc.DrawLine(wxPoint(this->_xTitleBorder + x, wy - this->_yTitleBorder - 2),
						wxPoint(this->_xTitleBorder + x, wy - this->_yTitleBorder+3));
				}
			}
		}
		if (this->_labelsEnabled)
		{
			s = wxString::FromDouble(pow(10,val), this->_precisionDigits);
			dc.SetPen(pen);
			dc.SetTextForeground(this->_mainAxisColor);
			int lt = s.Length();
			if (this->_xTitleBorder + x <= wx - this->_xTitleBorder)
			{
				dc.DrawText(s, this->_xTitleBorder + x - lt*lw / 2, wy - this->_yTitleBorder + lh / 4);
			}
		}
		val = val + 1;
	}
	if (this->_ymax*this->_ymin < 0)
	{
		y = (int)((-this->_ymin) / this->_yPixConverter);
		yi = wy - this->_yTitleBorder - y;
		dc.SetPen(zpen);
		dc.DrawLine(wxPoint(this->_xTitleBorder, yi),
			wxPoint(wx - this->_xTitleBorder, yi));
	}
}



void WxPlotter::Draw_Linear_Axis(wxDC& dc, int wx, int wy, bool screen)
{
	wxFont font;
	font.SetSymbolicSize(wxFONTSIZE_SMALL);
	wxPen pengrid;
	pengrid.SetColour(this->_secondaryAxisColor);
	dc.SetFont(font);
	int lw = dc.GetCharWidth();
	int lh = dc.GetCharHeight();
	wxPen zpen;
	zpen.SetColour(this->_zeroColor);
	zpen.SetWidth(1);

	wxPen pen;
	pen.SetColour(this->_mainAxisColor);
	pen.SetWidth(1);
	pengrid.SetStyle(wxPENSTYLE_SOLID);
	pen.SetStyle(wxPENSTYLE_SOLID);

	wxPen penborder;
	penborder.SetColour(this->_borderColor);
	penborder.SetStyle(wxPENSTYLE_SOLID);
	penborder.SetWidth(1);

	int x; 
	int y;	
	int yi;
	wxString s;
	int lt;
	float val = this->_xmin + this->_axisStepSizeX;
	while (val < this->_xmax-this->_axisStepSizeX/2.0f)
	{
		x = (int)((val - this->_xmin) / this->_xPixConverter);
		y = (int)((-this->_ymin) / this->_yPixConverter);
		dc.SetPen(pengrid);
		dc.DrawLine(wxPoint(this->_xTitleBorder+x, this->_yTitleBorder), wxPoint(this->_xTitleBorder+x,wy-this->_yTitleBorder));
		dc.SetPen(penborder);
		dc.DrawLine(wxPoint(this->_xTitleBorder + x, wy - this->_yTitleBorder - 2),
			wxPoint(this->_xTitleBorder + x, wy - this->_yTitleBorder+3));
		s = wxString::FromDouble(val, this->_precisionDigits);
		dc.SetPen(pen);
		dc.SetTextForeground(this->_mainAxisColor);
		lt = s.Length();
		dc.DrawText(s, this->_xTitleBorder+x-lt*lw/2,wy - this->_yTitleBorder + lh/4);
		val = val + this->_axisStepSizeX;
	}
	val = this->_xmin;
	x = 0;
	s = wxString::FromDouble(val, this->_precisionDigits);
	lt = s.Length();
	dc.SetPen(pen);
	dc.SetTextForeground(this->_mainAxisColor);
	dc.DrawText(s, this->_xTitleBorder + x - lt*lw / 2, wy - this->_yTitleBorder + lh / 4);
	
	val = this->_xmax;
	x = ((val - this->_xmin) / this->_xPixConverter);
	s = wxString::FromDouble(val, this->_precisionDigits);
	dc.SetPen(pen);
	dc.SetTextForeground(this->_mainAxisColor);
	lt = s.Length();
	dc.DrawText(s, this->_xTitleBorder + x - lt*lw / 2, wy - this->_yTitleBorder + lh / 4);

	val = this->_ymin + this->_axisStepSizeY;
	while (val < this->_ymax - this->_axisStepSizeY * this->_yPixConverter/2.0f)
	{
		y = (int)((val - this->_ymin) / this->_yPixConverter);
		x = (int)((-this->_xmin) / this->_xPixConverter);		
		yi = wy - this->_yTitleBorder - y;
		if ((yi >= this->_yTitleBorder - 2) && (yi <= wy - this->_yTitleBorder + 2))
		{
			dc.SetPen(pengrid);
			dc.DrawLine(wxPoint(this->_xTitleBorder, yi),
				wxPoint(wx - this->_xTitleBorder, yi));
			dc.SetPen(penborder);
			dc.DrawLine(wxPoint(wx - this->_xTitleBorder - 2, yi),
				wxPoint(wx - this->_xTitleBorder+3, yi));
			if (this->_labelsEnabled)
			{
				wxString s = wxString::FromDouble(val, this->_precisionDigits);
				if (val >= 0)
				{
					s = wxString(" ") + s;
				}
				dc.SetPen(pen);
				dc.SetTextForeground(this->_mainAxisColor);
				dc.DrawText(s, wx - this->_xTitleBorder + lw, wy - this->_yTitleBorder - y - lh / 2 - 1);
			}
		}
		val = val + this->_axisStepSizeY;
	}
	val = this->_ymin;
	y = 0;
	s = wxString::FromDouble(val, this->_precisionDigits);
	dc.SetPen(pen);
	dc.SetTextForeground(this->_mainAxisColor);
	dc.DrawText(s, wx - this->_xTitleBorder + lw, wy - this->_yTitleBorder - y - lh / 2 - 1);

	val = this->_ymax;
	y = (int)((this->_ymax - this->_ymin) / this->_yPixConverter);
	s = wxString::FromDouble(val, this->_precisionDigits);
	if (val > 0)
	{
		s = wxString(" ") + s;
	}
	dc.SetPen(pen);
	dc.SetTextForeground(this->_mainAxisColor);
	dc.DrawText(s, wx - this->_xTitleBorder + lw, wy - this->_yTitleBorder - y - lh / 2 - 1);

	if (this->_ymax*this->_ymin < 0)
	{
		y = (int)((-this->_ymin) / this->_yPixConverter);
		x = (int)((-this->_xmin) / this->_xPixConverter);
		yi = wy - this->_yTitleBorder - y;
		dc.SetPen(zpen);
		dc.DrawLine(wxPoint(this->_xTitleBorder, yi),
			wxPoint(wx - this->_xTitleBorder, yi));
	}
	if (this->_xmax*this->_xmin < 0)
	{
		x = (int)((-this->_xmin) / this->_xPixConverter);
		y = (int)((-this->_ymin) / this->_yPixConverter);
		dc.SetPen(zpen);
		dc.DrawLine(wxPoint(this->_xTitleBorder + x, this->_yTitleBorder), wxPoint(this->_xTitleBorder + x, wy - this->_yTitleBorder));
		dc.SetPen(penborder);
		dc.DrawLine(wxPoint(this->_xTitleBorder + x, wy - this->_yTitleBorder - 2),
			wxPoint(this->_xTitleBorder + x, wy - this->_yTitleBorder + 3));
	}
}

void WxPlotter::Draw_Linear_Axis(bool labels)
{
	this->_linearScale = true;
	this->_labelsEnabled = labels;
}

void WxPlotter::Enable_Logarithmic_Scale_AxisX()
{
	this->_linearScale = false;
}

void WxPlotter::Draw_Plot_DC(wxDC& dc, int wx, int wy, bool screen)
{
	if (screen)
	{
		this->Adjust(this->GetSize().x, this->GetSize().y);
		if (this->_backgroundDefined)
		{
			dc.DrawBitmap(this->_backgroundBitmap, wxPoint(0, 0));
		}
		else
		{
			dc.Clear();
			dc.GradientFillLinear(wxRect(0, 0, this->GetSize().x, this->GetSize().y), 
				wxColor(255, 255, 255), wxColor(245, 245, 245),wxDirection::wxDOWN);
		}
	}
	if (this->_showingVerticalInterval)
	{
		this->Draw_Interval_Vertical(dc);
	}
	if (this->_showingHorizontalInterval)
	{
		this->Draw_Interval_Horizontal(dc);
	}
	if (this->_linearScale)
	{
		this->Draw_Linear_Axis(dc, wx,wy, screen);
	}
	else
	{
		this->Draw_Log10_Axis(dc, wx,wy,screen);
	}
	wxPen pen;
	pen.SetStyle(wxPENSTYLE_SOLID);
	pen.SetColour(this->_borderColor);
	pen.SetWidth(2);
	dc.SetPen(pen);
	dc.DrawLine(this->_xTitleBorder, this->_yTitleBorder, wx - this->_xTitleBorder, this->_yTitleBorder);
	dc.DrawLine(this->_xTitleBorder, wy - this->_yTitleBorder,
		wx - this->_xTitleBorder, wy - this->_yTitleBorder);

	dc.DrawLine(this->_xTitleBorder, this->_yTitleBorder, this->_xTitleBorder, wy - this->_yTitleBorder);
	dc.DrawLine(wx - this->_xTitleBorder, this->_yTitleBorder,
		wx - this->_xTitleBorder, wy - this->_yTitleBorder);

	pen.SetWidth(2);
	for (int k = 0; k < this->_numberOfCurves; ++k)
	{
		wxColor pencolor = this->_curveColors[k];
		pen.SetColour(pencolor);
		dc.SetPen(pen);
		set<wxPoint,WxPlotter::wxPoint_Comparer>& curve = this->_discreteCurves[k];
		if (curve.size() > 0)
		{
			set<wxPoint, WxPlotter::wxPoint_Comparer>::iterator i = curve.begin();
			int x = i->x;
			int y = i->y;
			wxPoint p1(x, y);
			++i;
			while (i != curve.end())
			{
				wxPoint p2(i->x, i->y);
				dc.DrawLine(p1, p2);
				p1 = p2;
				++i;
			}
		}
	}

	if (this->_discretePointMarks.size() > 0)
	{
		this->Draw_Point_Marks(dc, wx, wy);
	}

	if ((this->_drawMousePosition)&&(screen))
	{
		this->Draw_Mouse_Pos(dc);
	}

	if (this->_hasTitles)
	{
		this->Draw_Titles(dc,wx,wy);
	}
}

void WxPlotter::On_Paint(wxPaintEvent& evt)
{
	if (!this->_blockedPlot)
	{
		this->_editPlot = false;
		wxAutoBufferedPaintDC dc(this);	
		dc.Clear();
		this->Draw_Plot_DC(dc, this->GetSize().x, this->GetSize().y, true);
		this->_editPlot = true;
	}
}

void WxPlotter::Save_File_Plot(const wxString& filename)
{
	this->_editPlot = false;
	wxSVGFileDC file(filename, 540,380);	
	this->Adjust(540, 380);
	this->Draw_Plot_DC(file, 540,380,false);
	this->Adjust(this->GetSize().x,this->GetSize().y);
	this->Refresh();
	this->_editPlot = true;
}

void WxPlotter::Draw_Interval_Vertical(bool dv)
{
	this->_showingVerticalInterval = dv;
}

void WxPlotter::Draw_Interval_Horizontal(bool dh)
{
	this->_showingHorizontalInterval = dh;
}

void WxPlotter::Define_X_Interval_Markers(float xmin, float xmax)
{
	if (this->_linearScale)
	{
		this->_intervalMarkerXMin = xmin;
		this->_intervalMarkerXMax = xmax;
	}
	else
	{
		this->_intervalMarkerXMin = log10(xmin);
		this->_intervalMarkerXMax = log10(xmax);
	}
}

float WxPlotter::Xmin_Marker_Interval() const
{
	float x = this->_intervalMarkerXMin;
	if (!this->_linearScale)
	{
		x = pow(10, this->_intervalMarkerXMin);
	}
	return(x);
}

float WxPlotter::Xmax_Marker_Interval() const
{
	float x = this->_intervalMarkerXMax;
	if (!this->_linearScale)
	{
		x = pow(10, this->_intervalMarkerXMax);
	}
	return(x);
}


void WxPlotter::Draw_Interval_Vertical(wxDC& gc)
{
	int ymin = (int)((this->_intervalMarkerYMin - this->_ymin)/ this->_yPixConverter);
	int ymax = (int)((this->_intervalMarkerYMax - this->_ymin) / this->_yPixConverter);
	wxBrush b;
	b.SetColour(this->_dynamicIntervalColor);
	b.SetStyle(wxBRUSHSTYLE_CROSSDIAG_HATCH);
	wxPen p;
	p.SetStyle(wxPENSTYLE_DOT);
	p.SetColour(this->_dynamicIntervalColor);
	p.SetWidth(1);

	gc.SetBrush(b);
	gc.SetPen(p);
	gc.DrawRectangle(this->_xTitleBorder, this->GetSize().y - this->_yTitleBorder-ymax, 
		this->GetSize().x - 2 * this->_xTitleBorder,ymax - ymin);
	p.SetWidth(4);
	p.SetStyle(wxPENSTYLE_SOLID);
	gc.SetPen(p);
	gc.DrawLine(wxPoint(this->_xTitleBorder, this->GetSize().y - this->_yTitleBorder - ymax),
		wxPoint(this->GetSize().x - this->_xTitleBorder, this->GetSize().y - this->_yTitleBorder - ymax));
	gc.DrawLine(wxPoint(this->_xTitleBorder, this->GetSize().y - this->_yTitleBorder - ymin),
		wxPoint(this->GetSize().x - this->_xTitleBorder, this->GetSize().y - this->_yTitleBorder - ymin));
}

void WxPlotter::Draw_Interval_Horizontal(wxDC& gc)
{
	float fxmin = this->_xmin;
	if (!this->_linearScale)
	{
		fxmin = (int)log10(this->_xmin);
	}

	int xmin = (int)((this->_intervalMarkerXMin - fxmin) / this->_xPixConverter);
	int xmax = (int)((this->_intervalMarkerXMax - fxmin) / this->_xPixConverter);

	wxPen p;
	p.SetColour(this->_dynamicIntervalColor);
	p.SetWidth(4);
	p.SetStyle(wxPENSTYLE_SOLID);
	gc.SetPen(p);
	gc.DrawLine(wxPoint(xmin+this->_xTitleBorder,this->_yTitleBorder),
		wxPoint(xmin+this->_xTitleBorder, this->GetSize().y - this->_yTitleBorder));
	gc.DrawLine(wxPoint(xmax+this->_xTitleBorder, this->_yTitleBorder),
		wxPoint(xmax+this->_xTitleBorder, this->GetSize().y - this->_yTitleBorder));
	gc.DrawText(this->_horizontalLegend,xmin + this->_xTitleBorder, this->_yTitleBorder-gc.GetCharHeight()-1);
}

void WxPlotter::Mouse_Button_Down(wxMouseEvent& evt)
{
	if (this->_showingVerticalInterval)
	{
		int ymin = (int)((this->_intervalMarkerYMin - this->_ymin) / this->_yPixConverter);
		int ymax = (int)((this->_intervalMarkerYMax - this->_ymin) / this->_yPixConverter);
		ymin = this->GetSize().y - this->_yTitleBorder - ymin;
		ymax = this->GetSize().y - this->_yTitleBorder - ymax;
		int x = evt.GetPosition().x;
		int y = evt.GetPosition().y;
		if ((x >= this->_xTitleBorder) && (x <= this->GetSize().x - this->_xTitleBorder))
		{
			if ((y >= ymin - 4) && (y < ymin + 4))
			{
				this->_selectingMinimalYMarker = true;
				this->_selectingMaximalYMarker = false;
			}
			if ((y >= ymax - 4) && (y < ymax + 4))
			{
				this->_selectingMinimalYMarker = false;
				this->_selectingMaximalYMarker = true;
			}
		}
	}
	if (this->_showingHorizontalInterval)
	{
		float fxmin = this->_xmin;
		if (!this->_linearScale)
		{
			fxmin = log10(this->_xmin);
		}
		int xmin = (int)((this->_intervalMarkerXMin - fxmin) / this->_xPixConverter);
		int xmax = (int)((this->_intervalMarkerXMax - fxmin) / this->_xPixConverter);
		xmin = this->_xTitleBorder + xmin;
		xmax = this->_xTitleBorder + xmax;
		int x = evt.GetPosition().x;
		int y = evt.GetPosition().y;
		if ((x >= this->_xTitleBorder) && (x <= this->GetSize().x - this->_xTitleBorder))
		{
			if ((x >= xmin - 4) && (x < xmin + 4))
			{
				this->_selectingMinimalXMarker = true;
				this->_selectingMaximalXMarker = false;
			}
			if ((x >= xmax - 4) && (x < xmax + 4))
			{
				this->_selectingMinimalXMarker = false;
				this->_selectingMaximalXMarker = true;
			}
		}
	}
}

void WxPlotter::Draw_Mouse_Pos(wxDC& gc)
{
	if (this->_selectedCurve >= 0)
	{
		int w = gc.GetCharWidth();
		wxBrush brush;
		brush.SetStyle(wxBRUSHSTYLE_SOLID);
		wxPen pen;
		brush.SetColour(this->_curveColors[this->_selectedCurve]);
		pen.SetColour(this->_curveColors[this->_selectedCurve]);
		WxPlotter::Pair p;
		wxPoint pp = this->_discreteMousePositon;
		p.y = (float)(-pp.y + this->GetSize().y-this->_yTitleBorder);
		p.x = (float)(pp.x-this->_xTitleBorder);
		p.y = p.y*this->_yPixConverter + this->_ymin;
		if (this->_linearScale)
		{
			p.x = p.x*this->_xPixConverter + this->_xmin;
		}
		else
		{
			p.x = p.x*this->_xPixConverter + log10(this->_xmin);
			p.x = pow(10, p.x);
		}
		set<WxPlotter::Pair>::iterator jj = this->_curves[this->_selectedCurve].lower_bound(p);		
		float d=0;
		if (jj != this->_curves[this->_selectedCurve].end())
		{
			set<WxPlotter::Pair>::iterator sj = jj;
			WxPlotter::Pair sp = *jj;			
			d = WxPlotter::Pair::Distance(sp, p);
			int ssps = 0;
			bool stop = false;			
			int maxsearch = std::min((int)(((float)this->_curves[this->_selectedCurve].size()) * this->_xPixConverter), (int)500);
			while ((ssps < maxsearch) && (!stop)&& (sj != this->_curves[this->_selectedCurve].begin()))
			{
				--sj;
				WxPlotter::Pair rp = *sj;
				float dd = WxPlotter::Pair::Distance(rp, p);
				if (dd < d)
				{
					sp = rp;
					d = dd;
				}
				if (dd > 1.25f*d)
				{
					stop = true;
				}
				++ssps;
			}
			ssps = 0;
			stop = false;
			sj = jj;
			++sj;
			while ((ssps < maxsearch) && (!stop) && (sj != this->_curves[this->_selectedCurve].end()))
			{
				WxPlotter::Pair rp = *sj;
				float dd = WxPlotter::Pair::Distance(rp, p);
				if (dd < d)
				{
					sp = rp;
					d = dd;
				}
				if (dd > 1.25f*d)
				{
					stop = true;
				}
				++ssps;
				++sj;
			}
			gc.SetPen(pen);
			wxFont f = gc.GetFont();
			gc.SetFont(f.MakeBold());
			gc.SetBrush(brush);
			wxString s = "(";
			s = s + wxString::FromDouble(sp.x, this->_precisionDigits) + "," + wxString::FromDouble(sp.y, this->_precisionDigits) + ")";
			gc.DrawText(s, pp.x + w, pp.y - 12);
			gc.SetFont(f);
			gc.DrawEllipse(pp.x - 4, pp.y - 4, 8,8);			
		}
	}
}

wxColor WxPlotter::Curve_Color(int id) const
{
	return(this->_curveColors[id]);
}

void WxPlotter::Set_Curve_Color(const wxColor& color, int id)
{
	this->_curveColors[id] = color;
	this->Refresh();
}

void WxPlotter::Set_Precision_Digits(int precision)
{
	this->_precisionDigits = precision;
}

void WxPlotter::Erase_All_Curves()
{
	this->_numberOfCurves = 0;
	this->_curveColors.clear();
	this->_curves.clear();
	this->_discreteCurves.clear();
	this->_discretePointMarks.clear();
}

void WxPlotter::Erase_Last_Curve()
{
	--this->_numberOfCurves;
	this->_curveColors.pop_back();
	this->_curves.pop_back();
	this->_discreteCurves.pop_back();
}

void WxPlotter::Add_Mark(WxPlotter::Pair pt, wxColor color)
{
	WxPlotter::PointMark pm;
	pm.Point = pt;
	pm.Color = color;
	this->_discretePointMarks.push_back(pm);
}

void WxPlotter::Move_Mark_Position(WxPlotter::Pair p, int id)
{
	if ((id >= 0) && (id < this->_discretePointMarks.size()))
	{
		this->_discretePointMarks[id].Point = p;
	}
	this->Refresh();
}

void WxPlotter::Set_Title(const wxString& title)
{
	this->_hasTitles = true;
	this->_title = title;
}

void WxPlotter::Set_X_Title(const wxString& title)
{
	this->_hasTitles = true;
	this->_titleX = title;
}

void WxPlotter::Set_Y_Title(const wxString& title)
{
	this->_hasTitles = true;
	this->_titleY = title;
}

void WxPlotter::Draw_Titles(wxDC& gc, int wx, int wy)
{
	if ((wx > 3*this->_xTitleBorder) && (wy > 3*this->_yTitleBorder))
	{
		wxFont font;
		font.SetSymbolicSize(wxFONTSIZE_MEDIUM);
		wxPen pen;
		pen.SetColour(this->_mainAxisColor);
		pen.SetWidth(2);
		gc.SetPen(pen);
		gc.SetTextForeground(this->_mainAxisColor);
		gc.SetFont(font);

		int dw = gc.GetCharWidth();
		int dh = gc.GetCharHeight();

		gc.DrawLabel(this->_title, wxRect(0, this->_yTitleBorder - 2*dh, wx, 20), wxALIGN_CENTER);
		font.SetSymbolicSize(wxFONTSIZE_MEDIUM);
		gc.SetFont(font);
		dw = gc.GetCharWidth();
		dh = gc.GetCharHeight();

		int y = wy - dw*this->_titleY.Length();
		int py = wy - y / 2;
		if ((py > 0) && (py < wy))
		{
			gc.DrawRotatedText(this->_titleY, wxPoint(this->_xTitleBorder -3*dh/2, wy - y / 2), 90);
		}
		py = wy - this->_yTitleBorder + dh + dh/2;
		if (py > 0)
		{
			gc.DrawLabel(this->_titleX, wxRect(0, py, wx, 20), wxALIGN_CENTER);
		}
	}
}

void WxPlotter::Set_Horizontal_Legend(wxString& legend)
{
	this->_enableHorizontalLegend = true;
	this->_horizontalLegend = legend;
}

