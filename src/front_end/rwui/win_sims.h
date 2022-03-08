#ifndef WIN_SIMS_H
#define WIN_SIMS_H

#include <vector>
#include <map>
#include <set>
#include <string>
#include "wx/wx.h"
#include "wx/treelist.h"
#include "wx/aui/aui.h"
#include "wx/aui/auibar.h"
#include "wx/ribbon/bar.h"
#include "wx/ribbon/buttonbar.h"
#include "wx/propgrid/propgrid.h"
#include "rw/persistence/plug_persistent.h"


using std::vector;
using std::map;
using std::set;
using std::string;

class WindowMain;

struct Image_Data
{
	int Width;
	int Height;
	int Depth;
	int Black;
	set<string> Associated_Sims;
};

struct Sim_Data
{
	string Sim_Name;
	string Associated_Image;
	int Total_Walkers;
	wxColor Sim_Color;
	string Sim_Parent;
};

class WindowSims : public wxWindow
{
private:
	friend class WindowMain;
	wxMenu* _popMenu;
	WindowMain* _windowMain;
	wxAuiManager* _mgr;
	wxTreeListCtrl* _treeSims;
	wxTreeListCtrl* _treeImgs;
	wxPropertyGrid* _propertyGrid;
	bool _sortData;

	wxImageList _imgTree;
	map<string,Image_Data> _imgsData;
	map<wxDateTime,Sim_Data> _simsData;

	wxArrayString _imgFiles;
	wxArrayString _simFiles;
	wxRibbonPanel* _ribbonPanel;

	void Populate_Image_Directory();
	void Populate_Sim_Directory();
	void Populate_Trees();
	void On_Check_Sim(wxTreeListEvent& evt);
	void On_Mouse_Down_Imgs(wxTreeListEvent& evt);
	void On_Select_Sim(wxTreeListEvent& evt);
	void Load_Image(wxCommandEvent& evt);
	void Sort_Ascending(wxCommandEvent& evt);
	void Sort_Descending(wxCommandEvent& evt);

	void Report(wxCommandEvent& evt);
	void Select_Sim(const wxString& sim_name);
	void On_Resize(wxSizeEvent& evt);
	void Remark_Sims(const map<wxString, bool>& sim_names);
public:
	WindowSims(wxWindow* parent);
	wxString Working_Directory() const;
	const wxArrayString& Image_Files() const;
	const wxArrayString& Sim_Files() const;
	void Update_File_Trees();
	void Fill_Loaded_Sims(const wxArrayString& nsims);
	void Update_Selected_Sim(const rw::PlugPersistent& sim);
	void Change_Grid_Value(wxPropertyGridEvent& evt);
	void Mark_Sim(const string& sim_name, bool check_plot = true);
	void Update_Data();
	void Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar);
	void Hide_Button_Panel();
	void Show_Button_Panel();
	void Update_Layout();
	~WindowSims();
};

#endif