#ifndef WXPLOTTER_H
#define WXPLOTTER_H

#include <wx/wx.h>
#include <vector>
#include <map>
#include <set>
#include <wx/dcbuffer.h>

using std::vector;
using std::set;
using std::map;

class WxPlotter : public wxControl
{
public:
	class ChangeIntervalYEvt
	{
	public:
		virtual void On_Update_Y_Value(float ymin, float ymax) {};
	};

	class ChangeIntervalXEvt
	{
	public:
		virtual void On_Update_X_Value(float xmin, float xmax) {};
	};

	struct Pair
	{
		float x;
		float y;
		Pair();
		Pair(float x, float y);
		bool operator < (const WxPlotter::Pair& pt) const;
		static float Distance(const WxPlotter::Pair& p1, const WxPlotter::Pair& p2);
	};
	
	struct PointMark
	{
		WxPlotter::Pair Point;
		wxColor Color;
	};

	struct wxPoint_Comparer
	{
		bool operator()(const wxPoint& a, const wxPoint& b) const
		{
			if (a.x == b.x)
			{
				return(a.y < b.y);
			}
			else
			{
				return(a.x < b.x);
			}
		};

		static int Distance(const wxPoint& a, const wxPoint& b)
		{
			int dx = a.x - b.x;
			int dy = a.y - b.y;
			return(dx*dx + dy*dy);
		}
	};

private:
	ChangeIntervalYEvt* _changeIntervalYEvt;
	ChangeIntervalXEvt* _changeIntervalXEvt;
	int _precisionDigits;
	int _pixelSpacing;
	bool _hasTitles; 
	wxString _title;
	wxString _titleX;
	wxString _titleY;
	int _numberOfCurves;
	int _markerDistance;
	int _selectedCurve;
	bool _drawMousePosition;
	wxPoint _discreteMousePositon;
	vector<set<WxPlotter::Pair>> _curves;
	vector<wxColor> _curveColors;
	vector<set<wxPoint,WxPlotter::wxPoint_Comparer>> _discreteCurves;
	vector<PointMark> _discretePointMarks;

	float _xmin;
	float _xmax;
	float _ymin;
	float _ymax;

	float _xPixConverter;
	float _yPixConverter;
	
	int _xSpacing;
	int _ySpacing;

	int _xTitleBorder;
	int _yTitleBorder;
	bool _showingVerticalInterval;
	bool _showingHorizontalInterval;

	wxColour _mainAxisColor;
	wxColour _secondaryAxisColor;
	wxColour _dynamicIntervalColor;
	wxColour _borderColor;
	wxColour _zeroColor;

	float _axisStepSizeX;
	float _axisStepSizeY;

	float _intervalMarkerYMin;
	float _intervalMarkerYMax;
	float _intervalMarkerXMin;
	float _intervalMarkerXMax;


	bool _selectingMinimalYMarker;
	bool _selectingMaximalYMarker;
	bool _selectingMinimalXMarker;
	bool _selectingMaximalXMarker;

	wxBitmap _backgroundBitmap;
	bool _backgroundDefined;
	bool _linearScale;
	bool _labelsEnabled;
	bool _blockedPlot;
	bool _enableHorizontalLegend;
	wxString _horizontalLegend;
	volatile bool _editPlot;
	float _log10Values[9];
	void Draw_Linear_Axis(wxDC& gc, int wx, int wy, bool screen);
	void Draw_Log10_Axis(wxDC& gc, int wx, int wy, bool screen);
	wxColor Pick_Curve_Color() const;
	void Draw_Point_Marks(wxDC& gc, int wx, int wy);
	void Draw_Mouse_Pos(wxDC& gc);
	void Draw_Titles(wxDC& gc, int wx, int wy);
	void Draw_Interval_Vertical(wxDC& gc);
	void Draw_Interval_Horizontal(wxDC& gc);
public:
	WxPlotter(wxWindow* Parent = NULL, wxWindowID Id = -1, const wxPoint& Position = wxDefaultPosition,
		const wxSize& Size = wxDefaultSize, long Style = 0);
	int Number_Of_Curves() const;
	int Add_Curves(int n);
	void Set_Interval_Limits(float xmin, float xmax, float ymin, float ymax);
	void Add_Curve_Point(int id, const WxPlotter::Pair& pt);
	void Add_Curve_Point(int id,float x, float y);
	void On_Paint(wxPaintEvent& evt);
	void On_Resize(wxSizeEvent& evt);
	void Mouse_Button_Down(wxMouseEvent& evt);
	void On_Erase_Background(wxEraseEvent& evt){};
	void On_Mouse_Move_Event(wxMouseEvent& evt);
	void On_Mouse_Leave_Event(wxMouseEvent& evt);
	void Adjust_Axis();
	void Set_Background_Bitmap(const wxBitmap& bmp);
	void Draw_Linear_Axis(bool labels = true);
	void Enable_Logarithmic_Scale_AxisX();
	wxColor Curve_Color(int id) const;
	void Set_Curve_Color(const wxColor& color, int id);
	void Erase_All_Curves();
	void Erase_Last_Curve();
	void Reset();
	void Add_Mark(WxPlotter::Pair p, wxColor color);
	void Move_Mark_Position(WxPlotter::Pair p, int id);
	void Set_Title(const wxString& title);
	void Set_X_Title(const wxString& title);
	void Set_Y_Title(const wxString& title);
	void Draw_Interval_Vertical(bool dv);
	void Define_Y_Interval_Markers(float ymin, float ymax);
	float Ymin_Marker_Interval() const;
	float Ymax_Marker_Interval() const;
	void Draw_Interval_Horizontal(bool dh);
	void Define_X_Interval_Markers(float ymin, float ymax);
	float Xmin_Marker_Interval() const;
	float Xmax_Marker_Interval() const;
	void Set_Change_Interval_Y_Event(WxPlotter::ChangeIntervalYEvt* evt);
	void Set_Change_Interval_X_Event(WxPlotter::ChangeIntervalXEvt* evt);
	void Save_File_Plot(const wxString& filename);
	void Draw_Plot_DC(wxDC& gc, int wx, int wy, bool screen);
	void Adjust(int wx, int wy);
	bool Editable() const;	
	void Block();
	void UnBlock();
	float XMin() const;
	float XMax() const;
	float YMin() const;
	float YMax() const;
	void Set_Horizontal_Legend(wxString& legend);
	void Set_Precision_Digits(int precision);
};

inline bool WxPlotter::Editable() const
{
	return(this->_editPlot);
}

inline float WxPlotter::XMin() const
{
	return(this->_xmin);
}

inline float WxPlotter::XMax() const
{
	return(this->_xmax);
}

inline float WxPlotter::YMin() const
{
	return(this->_ymin);
}

inline float WxPlotter::YMax() const
{
	return(this->_ymax);
}


#endif