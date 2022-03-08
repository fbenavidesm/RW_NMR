#ifndef WIN_SAMPLE_H
#define WIN_SAMPLE_H

#include "wx/wx.h"
#include "wx/progdlg.h"
#include "wx/aui/aui.h"
#include "wx/propgrid/propgrid.h"
#include "wx/ribbon/bar.h"
#include "wx/ribbon/buttonbar.h"
#include "front_end/volume/win_volume.h"
#include "rw/binary_image/binary_image.h"
#include "rw/plug.h"
#include "GL/shader.h"
#include "rw/persistence/plug_persistent.h"
#include "front_end/wx_plotter.h"
#include "rw/random_walk_observer.h"

class UpdateWalk;
class UpdateVolume;
class WindowMain;


#define wxID_PLACE  wxID_HIGHEST + 1000
#define wxID_START  wxID_HIGHEST + 1001


/**
* This window renders the rock sample that is processed by the simulator.
* It contains a volume renderer and a configuration window, to define the simulation
* parameters.
*/
class WindowSample : public wxWindow, public WxPlotter::ChangeIntervalXEvt
{
private:
	friend class UpdateWalk;
	friend class UpdateVolume;
	WindowMain* _parentMain;
	WindowVolume* _windowVolume;
	wxAuiManager* _mgr;
	wxPropertyGrid* _pgr;
	wxPropertyGrid* _pgpp;

	int _imageResolution;
	volatile GLfloat* _walkersPos;
	int _walkerPosSize;
	int _strikesMax;
	float _minDepth;
	float _maxDepth;
	int _minHits;
	int _maxHits;
	wxColor _editLaplaceColor;
	bool _editingLaplace;
	math_la::math_lac::full::Vector _editedLaplace;
	wxColor _lowEnergy;
	wxColor _highEnergy;
	WxPlotter* _plotterA;
	WxPlotter* _plotterB;
	const rw::BinaryImage* _imgPtr;

	uint _width;
	uint _height;
	uint _depth;

	rw::Plug* _formation;
	rw::PlugPersistent* _currentSimulation;
	wxString _simPath;
	bool _currentSimulated;
	bool _simulationSaved;
	bool _currentSimModified;
	bool _viewWalkers;

	UpdateVolume* _updVol;
	UpdateWalk* _updWalk;

	void On_Walk(wxCommandEvent& event);
	void On_Walk_End(wxCommandEvent& event);

	void On_Update_X_Value(float xmin, float xmax);
public:
	WindowSample(wxWindow* parent);
	void Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar);
	void Set_Tags();
	void Close_Properties();
	void Build_3D_Model(const rw::BinaryImage& img, wxGenericProgressDialog* pgdlg);
	void Update_Walkers(bool updatePos = true);
	void Configure_Gradient_Options();
	void Change_Grid_Value(wxPropertyGridEvent& evt);
	void Changing_Grid_Value(wxPropertyGridEvent& evt);
	void Preprocess_Simulation_Parameters(const rw::BinaryImage& img);
	void Postprocess_Simulation_Parameters();

	void Build_3D_Sample(wxCommandEvent& evt);
	void Walk(wxCommandEvent& event);
	void Show_Regularizer_Dialog(wxCommandEvent& evt);
	void Save_Simulation(wxCommandEvent& evt);
	void Laplace(wxCommandEvent& evt);
	void View_Walkers(wxCommandEvent& evt);
	void Edit_Laplace(wxCommandEvent& evt);
	void Import_Decay(wxCommandEvent& evt);
	void Create_PSD(wxCommandEvent& evt);

	bool Has_Current_Simulation() const;
	void Set_Current_Simulation(rw::PlugPersistent* sim, const wxString& sim_path);
	const rw::PlugPersistent& Current_Simulation() const;
	void Save_Sim(const wxString& sim_file_name, const wxDateTime& wdt);

	void Update_Decay_Plot();
	void Update_Laplace_Plot();

	void Add_Sim_Plot(const rw::PlugPersistent& sim, int& id);
	void Erase_Plots();
	void Update_Sim_Parameters(const rw::PlugPersistent& sim);
	void Set_Sample_Name(const wxString& name);
	void Update_Rock_Size();
	rw::PlugPersistent* Release_Current_Simulation(bool show_dialog = true);
	void Save_Simulation_Dialog();
	void Check_Simulation_Pointer_To_Delete(const rw::PlugPersistent* sim);
	void Draw_Current_Sim();
	void Update_Current_Simulation();
	wxString Current_Sim_File_Name() const;
	void Pick_Current_Sim_And_Formation(rw::PlugPersistent** sim, rw::Plug** formation);
	void Set_Current_Simulation_Color(const wxColor& color);
	void Mark_Volume(int px, int py, int pz, int dx, int dy, int dz);
	void Update_Layout();
	void Fix_Sample(const rw::BinaryImage& img, wxGenericProgressDialog* pgdlg,uint& sx, uint& sy, vector<uint>& indices);
	~WindowSample();
};

class UpdateVolume : public VolumePanel::Observer
{
private:
	friend class WindowSample;
	WindowSample* _parentFormation;
	GLuint _buffIndx;
	GLuint _vx;
	bool _buffered;
	gl::Shader _vtxShader;
	bool _shaderCompiled;
	GLint _lowColorId;
	GLint _highColorId;
	GLint _kminId;
	GLint _kmaxId;
	GLint _knId;
	GLint _zminId;
	GLint _zmaxId;
	bool _updateVtxBuffer;
public:
	UpdateVolume();
	void Set_Buffer();
	void Release_Buffer_Index();
	void On_Update();
	void On_Update_Bar(float barmin, float barmax);
	void Update_Buffers();
};


class UpdateWalk : public rw::RandomWalkObserver
{
private:
	friend class WindowSample;
	WindowSample* _parentWindow;
	wxLongLong _passed;
public:
	UpdateWalk();
	void Observe_Walk(scalar perc, scalar Magnetization_Factor, scalar Decay_Domain);
	void Walk_End();
};




#endif
