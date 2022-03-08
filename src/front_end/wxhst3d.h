#ifndef WX_HST_3D_H
#define WX_HST_3D_H

#include <vector>
#include "wx/wx.h"
#include "GL/glew.h"
#include "wx_gl_panel.h"
#include "gl/trackball.h"
#include "math_la/math_lac/space/mtx4.h"
#include "math_la/math_lac/space/vec3.h"
#include "math_la/mdefs.h"

using std::vector;

class WxHst3D;

struct Series_Color
{
	wxColor front_color;
	wxColor side_color;
	wxColor up_color;
};

class WinHst3D : public wxWindow
{
private:
	WxHst3D* _histogramPanel;
public:
	WinHst3D(wxWindow* Parent, wxWindowID Id = -1, const wxPoint& Position = wxDefaultPosition,
		const wxSize& Size = wxDefaultSize, long Style = 0);
	WxHst3D& Drawing_Panel();
	~WinHst3D();
};


class WxHst3D : public WxGLPanel
{
private:
	friend class WinHst3D;
	WXHDC _hdc;
	bool _drawSeriesNames;
	bool _drawTitle;
	bool _drawCategorySubtitle;
	wxString _graphTitle;
	wxString _categorySubtitle;
	vector<wxString> _seriesNames;
	vector<float*> _seriesValues;
	vector<Series_Color> _seriesColor;
	uint _displayCharList;
	uint _displayTitleList;
	int _selectedCategory;
	int _selectedSeries;
	int _textIndent;
	int _charHeight;
	int _charTitleHeight;
	float _stepCategory;
	float _dindent;
	float _maxCategory;
	int _digits;
	wxColor _axisColor;
	wxColor _saxisColor;
	int _categories;
	gl::TrackBall _trackball;
	bool _mouseDown;
	bool _charListSet;
	math_la::math_lac::space::Vec3 _cameraPosition;
	math_la::math_lac::space::Mtx4 _projectionMatrix;
	void Define_Projection_Matrix();
	void Draw_Grid();
	void OnLeftMouseDown(wxMouseEvent& evt);
	void OnRightMouseDown(wxMouseEvent& evt);
	void OnLeftMouseUp(wxMouseEvent& evt);
	void OnLeftMouseMove(wxMouseEvent& evt);
	void Set_Char_List();
	void Draw_Box(	int id, int category, float y, float dx, float dz, float sq, 
					float3 f1color, float3 f2color, float3 f3color);
	void Draw_One_Series(int id, float dx, float dz, float sq, 
						 float3 f1color, float3 f2color, float3 f3color);
	void Draw_Series(float dx, float dz, float sq);
	void Draw_Series_Names();
	void Draw_Title();
	void Draw_Subtitle();
	void Picking_Ray(int u, int v, math_la::math_lac::space::Vec3& direction, math_la::math_lac::space::Vec3& position);
	float Intersect_Box(const math_la::math_lac::space::Vec3& ray, const math_la::math_lac::space::Vec3& position, int s, int k);
public:
	WxHst3D(wxWindow* Parent, wxWindowID Id = -1, int* attrs = 0, const wxPoint& Position = wxDefaultPosition,
		const wxSize& Size = wxDefaultSize, long Style = 0);
	void On_Update();
	void Set_Categories_Size(int categories_size);
	void Add_Series(const wxColor& color, const wxString& name = "S");
	void Set_Value(int id, int category, float value);
	void Set_Max_Weight(float max);
	void Set_Graph_Title(const wxString& title);
	void Set_Category_Subtitle(const wxString& subtitle);
};

inline WxHst3D& WinHst3D::Drawing_Panel()
{
	return(*this->_histogramPanel);
}


#endif
