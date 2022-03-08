#ifndef WINDOW_IMAGE_H
#define WINDOW_IMAGE_H

#include <vector>
#include "wx/wx.h"
#include "wx/ribbon/bar.h"
#include "wx/ribbon/buttonbar.h"
#include "wx/ribbon/toolbar.h"
#include "wx/propgrid/propgrid.h"
#include "wx/propgrid/manager.h"
#include "wx/propgrid/property.h"
#include "wx/propgrid/advprops.h"
#include "front_end/wx_image_panel.h"
#include "wx/aui/aui.h"
#include "wx/progdlg.h"

#include "wx/string.h"
#include "rw/binary_image/binary_image.h"

class WindowMain;

using std::vector;
class WindowImage : public wxWindow
{
private:
	rw::BinaryImage _binImg;
	rw::BinaryImage _processedImage;
	rw::BinaryImage* _dispImage;
	WxImagePanel* _leftImg;
	WxImagePanel* _rightImg;
	vector< wxImage* > _imgs;
	wxArrayString _imgNames;
	wxSlider* _sliderImg;
	wxSlider* _sliderSolid;
	wxStaticText* _fileText;
	wxStaticText* _depthText;
	WindowMain* _windowMain;
	wxPropertyGrid* _propertyGrid;
	bool _paintPores;
	wxAuiManager* _mgr;
	wxPanel* _pnlSliderFile;
	bool _showProcessedImages;
	bool _gpuFinished;
	wxRibbonPanel* _ribbonPanel;
	wxRibbonToolBar* _btnBar;

	void Open_Spheres();

	wxProgressDialog* _pgdlg;

public:
	WindowImage(wxWindow* parent, WindowMain* windowMain);
	~WindowImage();
	void Load_Image_List();
	void Show_Images(int depth);
	void On_Scroll_Img(wxScrollEvent& evt);
	void On_Scroll_Cut(wxScrollEvent& evt);
	void Update_Layout();
	void Binarize_All_Samples(wxCommandEvent& evt);
	void On_Load(wxCommandEvent& evt);
	void Morpho_Open(wxCommandEvent& evt);
	void Paint_Pore(wxCommandEvent& evt);
	void Flip(wxCommandEvent& evt);
	void Cluster_Pores(wxCommandEvent& evt);
	void Binarize(wxImage& img);
	void Change_Grid_Value(wxPropertyGridEvent& evt);
	void Paint_Pore_Space(wxImage& img);
	const rw::BinaryImage& Binary_Image() const;
	void Save_Image(wxCommandEvent& evt);
	void Load_Binary_Image(const wxString& file_name, wxGenericProgressDialog* pgdlg);
	void Denoise(wxCommandEvent& evt);
	void Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar);
	void Hide_Button_Panel();
	void Show_Button_Panel();
};






#endif