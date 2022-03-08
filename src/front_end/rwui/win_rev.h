#ifndef WIN_REV_H
#define WIN_REV_H

#include "wx/aui/aui.h"
#include "wx/propgrid/propgrid.h"
#include "wx/ribbon/bar.h"
#include "wx/ribbon/buttonbar.h"
#include "front_end/wx_plotter.h"
#include "rw/binary_image/binary_image.h"
#include "rw/rev.h"

class WindowMain;

class WindowRev : public wxWindow, public rw::Rev::RevEvent
{
private:
	WindowMain* _windowMain;
	wxAuiManager* _mgr;
	WxPlotter* _plotterA;
	WxPlotter* _plotterB;
	wxPropertyGrid* _propertyGrid;
	wxGenericProgressDialog* _prgdlg;

	uint _greenLaplace;
	uint _blueLaplace;

	vector<float> _means;
	vector<float> _stdDev;
	scalar _maxPorosity;
	scalar _minPorosity;
	vector<float> _sectionSizes;
	const rw::BinaryImage* _imgPtr;
	rw::Rev* _rev;

	math_la::math_lac::full::Vector _maxLaplace;
	math_la::math_lac::full::Vector _minLaplace;
	math_la::math_lac::full::Vector _DT2;

	void Plot_Convergence();
	void Display_Sections();
	void On_Walk_Event(int k, scalar percentage);
	void Pick_Max_Laplace();
	void Draw_Laplace(uint id, const wxColor& color);
	void Update_Porosity_Global_Data();
	void Update_Laplace_Global_Data();
	void Update_Unresolved_Porosity();
public:
	WindowRev(wxWindow* parent);
	~WindowRev();
	void Update_Layout();
	void Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar);
	void Porosity(wxCommandEvent& evt);
	void Simulate_Pore_Shape(wxCommandEvent& evt);
	void Set_Image_Pointer(const rw::BinaryImage* imgptr);
	void Draw_Laplace_Area();
	void Change_Grid_Value(wxPropertyGridEvent& evt);

};




#endif
