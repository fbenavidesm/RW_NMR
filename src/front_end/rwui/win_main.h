#ifndef MAIN_WIN_H
#define MAIN_WIN_H

#include <vector>
#include <map>
#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/listbook.h>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/buttonbar.h>
#include "win_sample.h"
#include "win_image.h"
#include "win_sims.h"
#include "win_genetic.h"
#include "win_rev.h"


using std::vector;
using std::map;

#define wxID_OPEN_ROCK  wxID_HIGHEST + 1
#define wxID_OPEN_LIST  wxID_HIGHEST + 2
#define wxID_SHOW_PROP  wxID_HIGHEST + 3
#define wxID_ASSEMBLE_3D  wxID_HIGHEST + 4
#define wxID_SHOW_MORPH  wxID_HIGHEST + 5
#define wxID_WALK  wxID_HIGHEST + 6
#define wxID_CONFIGURE_WALK  wxID_HIGHEST + 7
#define wxID_VIEW_WALK  wxID_HIGHEST + 8
#define wxID_HISTOGRAM wxID_HIGHEST + 9
#define wxID_IMAGE_LIST  wxID_HIGHEST + 10
#define wxID_ERODE  wxID_HIGHEST + 11
#define wxID_DILATE  wxID_HIGHEST + 12
#define wxID_ERASE_LIST wxID_HIGHEST + 13
#define wxID_LAPLACE wxID_HIGHEST + 14
#define wxID_SAVE_BLOCK wxID_HIGHEST + 15
#define wxID_SAVE_CSV_LAPLACE wxID_HIGHEST + 16
#define wxID_SAVE_CSV_DECAY wxID_HIGHEST + 17
#define wxID_LOAD_BLOCK wxID_HIGHEST + 18
#define wxID_EDIT_SIM wxID_HIGHEST + 19
#define wxID_LAPLACEF wxID_HIGHEST + 20
#define wxID_SAVE_HST wxID_HIGHEST + 21
#define wxID_REG wxID_HIGHEST + 22
#define wxID_OPEN_EXP wxID_HIGHEST + 23
#define wxID_IMPORT wxID_HIGHEST + 24
#define wxID_SYNTH_IMG wxID_HIGHEST + 25
#define wxID_EXEC_GEN wxID_HIGHEST + 26
#define wxID_SHOW_EXP wxID_HIGHEST + 27
#define wxID_VIEW_START  wxID_HIGHEST + 28
#define wxID_REV  wxID_HIGHEST + 29
#define wxID_EXEC_RTF_INV wxID_HIGHEST + 30
#define wxID_PSD wxID_HIGHEST + 31
#define wxID_MORPH_PSD wxID_HIGHEST + 32

class WindowImage;

struct Mem_Sim
{
	rw::PlugPersistent* sim;
	int curve_id;
};

class WindowMain : public wxFrame
{
private:
	int _ribbonSize;
	int _barSize;
	wxAuiManager* _mgr;
	wxImageList* _imgList;
	WindowImage* _winImage;
	WindowSample* _winSample;
	WindowSims* _winSims;
	WindowRev* _winRev;
	WindowGenetic* _winGenetic;
	wxGauge* _progressBar;
	wxRibbonButtonBar* _btnPanelBar;
	wxString _imgName;
	wxString _simName;
	map<string, Mem_Sim> _loadedSimulations;
	wxGenericProgressDialog* _dlg;
	const rw::BinaryImage* _imgPtr;
	wxApp* _parentApplication;
	void Hide_Panels();
public:
	WindowMain(const wxString& title = "RANDOM WALK NUCLEAR MAGNETIC RESONANCE");	
	~WindowMain();

	void Step_Progress_Bar(int perc);
	void Show_Lab_Panel(wxCommandEvent& evt);
	void Show_Sim_Panel(wxCommandEvent& evt);
	void Show_Image_Panel(wxCommandEvent& evt);
	void Show_Genetic_Panel(wxCommandEvent& evt);
	void Show_Rev_Panel(wxCommandEvent& evt);

	void Set_Parent_Application(wxApp* parent);

	void Build_3D_Model(wxCommandEvent& evt);
	void On_Close_Pane(wxAuiManagerEvent& evt);
	bool Save_Simulation(wxCommandEvent& evt);
	void Save_Image(wxCommandEvent& evt);
	void Load_Simulation(const string& sim_name);
	void Unload_Simulation(const string& sim_name);
	wxString Image_Name() const;
	bool Add_Plot_Simulation(const string& name);
	bool Erase_Plot_Simulation(const string& name);
	void Redraw_Simulations();
	void Load_Binary_Image(const wxString& filename);
	void Update_Selection_Sims();
	rw::PlugPersistent* Get_Sim_By_Name(const wxString& name);
	void Update_Selected_Sim(const rw::PlugPersistent& sim);
	void Set_Current_Sim(rw::PlugPersistent* sim, const wxString& path);
	void Current_Simulation_Update(bool simulated);
	void Report() const;
	void Save_Sphere_PSD(const rw::BinaryImage& img) const;
	void Create_Cluster_PSD(rw::BinaryImage& img) const;
	void Prepare_Genetic_Inversion();
	void Set_Progress_Dialog(wxProgressDialog* pgdlg);
	void Release_Progress_Dialog();
	void Save_Distribution(const vector<scalar2>& dstpsd, const vector<scalar2>& dstvd);
	void Remark_Sims();
	void Set_Image_Pointer(const rw::BinaryImage* imgptr);
	void Update_Sim_Color(const wxColor& color);
	int Ribbon_Size() const;
	int Bar_Size() const;
	void Mark_Sub_Volume(int px, int py, int pz, int dx, int dy, int dz);
};

inline int WindowMain::Ribbon_Size() const
{
	return(this->_ribbonSize);
}

inline int WindowMain::Bar_Size() const
{
	return(this->_barSize);
}

#endif