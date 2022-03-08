#include "win_sample.h"
#include "win_main.h"
#include "wx/progdlg.h"
#include "tbb/parallel_for.h"
#include "dlg_regularizer.h"
#include "dlg_import.h"
#include "dlg_pore_size.h"
#include "front_end/wx_image_adapter.h"
#include "front_end/persistent_ui/persistent_ui.h"
#include "front_end/wx_rgbcolor.h"

wxDEFINE_EVENT(wxWALK_EVENT, wxCommandEvent);
wxDEFINE_EVENT(wxWALK_END_EVENT, wxCommandEvent);

UpdateVolume::UpdateVolume() : VolumePanel::Observer()
{
	this->_buffIndx = 0;
	this->_buffered = false;
	this->_shaderCompiled = false;
	this->_vx = 0;
	this->_updateVtxBuffer = false;
}

void UpdateVolume::Set_Buffer()
{
	if ((this->_parentFormation->_walkerPosSize > 0)&&(this->_parentFormation->_formation->Showing_Walkers()))
	{
		this->Release_Buffer_Index();
		if (!this->_shaderCompiled)
		{
			this->_vtxShader.Load_File("files/walkershader.fsh");
			this->_vtxShader.Compile_Vertex();
			this->_vtxShader.Attach();
			this->_vtxShader.Execute();
			this->_lowColorId = glGetUniformLocation(this->_vtxShader.ProgramIDX(), "lowcolor");
			this->_highColorId = glGetUniformLocation(this->_vtxShader.ProgramIDX(), "highcolor");
			this->_kminId = glGetUniformLocation(this->_vtxShader.ProgramIDX(), "kmin");
			this->_kmaxId = glGetUniformLocation(this->_vtxShader.ProgramIDX(), "kmax");
			this->_zminId = glGetUniformLocation(this->_vtxShader.ProgramIDX(), "zmin");
			this->_zmaxId = glGetUniformLocation(this->_vtxShader.ProgramIDX(), "zmax");
			this->_shaderCompiled = true;
		}
		glGenVertexArrays(1, &this->_vx);
		glBindVertexArray(this->_vx);
		glGenBuffers(1, &this->_buffIndx);
		glBindBuffer(GL_ARRAY_BUFFER, this->_buffIndx);
		glBufferData(GL_ARRAY_BUFFER, this->_parentFormation->_walkerPosSize * 4 * sizeof(GLfloat),
			(GLvoid*)this->_parentFormation->_walkersPos, GL_DYNAMIC_DRAW);
		glVertexPointer(4, GL_FLOAT, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		this->_buffered = true;
	}
}

void UpdateVolume::Release_Buffer_Index()
{
	if (this->_buffered)
	{
		glDeleteBuffers(1, &this->_buffIndx);
		glDeleteVertexArrays(1, &this->_vx);
		this->_buffered = false;
	}
}

void UpdateVolume::Update_Buffers()
{
	if ((!this->_buffered)&&(this->_parentFormation->_walkerPosSize > 0))
	{
		this->Set_Buffer();
	}
	else if (this->_buffered)
	{
		glBindVertexArray(this->_vx);
		glBindBuffer(GL_ARRAY_BUFFER, this->_buffIndx);
		glBufferData(GL_ARRAY_BUFFER, this->_parentFormation->_walkerPosSize * 4 * sizeof(GLfloat),
			(GLvoid*)this->_parentFormation->_walkersPos, GL_DYNAMIC_DRAW);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		this->_updateVtxBuffer = false;
	}
}

void UpdateVolume::On_Update_Bar(float barmin, float barmax)
{
	this->_parentFormation->_minDepth = barmin;
	this->_parentFormation->_maxDepth = barmax;
}

void UpdateVolume::On_Update()
{
	if ((this->_parentFormation->_formation)&&(this->_parentFormation->_formation->Showing_Walkers()))
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		if (!this->_buffered)
		{
			this->Set_Buffer();
		}
		if (this->_updateVtxBuffer)
		{
			this->Update_Buffers();
		}
		if ((this->_buffered)&&(this->_parentFormation->_viewWalkers))
		{
			glUseProgram(this->_vtxShader.ProgramIDX());
			glUniform3f(this->_lowColorId, (GLfloat)this->_parentFormation->_lowEnergy.Red() / 255.0f,
				(GLfloat)this->_parentFormation->_lowEnergy.Green() / 255.0f, (GLfloat)this->_parentFormation->_lowEnergy.Blue() / 255.0f);
			glUniform3f(this->_highColorId, (GLfloat)this->_parentFormation->_highEnergy.Red() / 255.0f,
				(GLfloat)this->_parentFormation->_highEnergy.Green() / 255.0f, (GLfloat)this->_parentFormation->_highEnergy.Blue() / 255.0f);
			glUniform1f(this->_kminId, (GLfloat)this->_parentFormation->_minHits);
			glUniform1f(this->_kmaxId, (GLfloat)this->_parentFormation->_maxHits);
			glUniform1f(this->_zminId, (GLfloat)this->_parentFormation->_minDepth);
			glUniform1f(this->_zmaxId, (GLfloat)this->_parentFormation->_maxDepth);

			glBindVertexArray(this->_vx);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnable(GL_POINT_SMOOTH);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glPointSize(this->_parentFormation->_windowVolume->Voxel_Factor());
			glDrawArrays(GL_POINTS, 0, 4*this->_parentFormation->_walkerPosSize);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
			glDisable(GL_PROGRAM_POINT_SIZE);
			glDisable(GL_POINT_SMOOTH);
			glBindVertexArray(0);
			glUseProgram(0);
		}
	}
}


UpdateWalk::UpdateWalk()
{
	this->_passed = wxGetLocalTimeMillis();
}

void UpdateWalk::Observe_Walk(scalar perc, scalar Magnetization_Factor, scalar Decay_Domain)
{
	wxCommandEvent evt(wxWALK_EVENT);
	uint t = (uint)(perc*((scalar)100.0));
	evt.SetInt((int)(perc*((scalar)100.0)));
	wxLongLong ct = wxGetLocalTimeMillis();
	wxLongLong ps = ct - this->_passed;
	if (ps > 1000)
	{
		wxPostEvent(this->_parentWindow, evt);
		this->_passed = wxGetLocalTimeMillis();
	}
	int id = this->_parentWindow->_formation->Decay_Size() - 1;
}

void UpdateWalk::Walk_End()
{
	wxCommandEvent evt(wxWALK_END_EVENT);
	wxPostEvent(this->_parentWindow, evt);
	if (this->Parent_Formation().Showing_Walkers())
	{
		this->_parentWindow->Update_Walkers();
	}
}

void WindowSample::Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar)
{
	wxMenu* menu = new wxMenu();
	wxRibbonPanel* ribbonPanel = new wxRibbonPanel(ribbonPage, wxID_ANY, wxT("NMR Simulation"), wxNullBitmap,
		wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);
	wxRibbonToolBar* btnBar = new wxRibbonToolBar(ribbonPanel);
	btnBar->SetRows(1);
	wxImage img;
	wxBitmap bmp;
	int bmpsize = 32;
	img.LoadFile(wxT("icons/rock3d.png"));
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_PLACE, bmp, "Place rock sample on panel");
	menu->Append(wxID_PLACE, "Place rock sample on planel")->SetBitmap(bmp);
	img.LoadFile("icons/runner.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_START, bmp, "Start random walk simulation");
	menu->Append(wxID_START,"Start random walk simulation")->SetBitmap(bmp);
	btnBar->AddSeparator();
	menu->AppendSeparator();
	img.LoadFile("icons/balance.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_LAPLACE, bmp, "Select regularizer and apply Inverse Laplace Transform");
	menu->Append(wxID_LAPLACE, "Select regularizer and apply Inverse Laplace Transform")->SetBitmap(bmp);
	img.LoadFile("icons/laplf.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_LAPLACEF,  bmp, "Apply Laplace transform with current parameters");
	menu->Append(wxID_LAPLACEF, "Apply Laplace transform with current parameters")->SetBitmap(bmp);
	img.LoadFile("icons/eye-gray.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_VIEW_DETAILS,  bmp, "The walker's position is shown during the simulation");
	menu->Append(wxID_VIEW_DETAILS, "The walker's position is shown during the simulation")->SetBitmap(bmp);
	img.LoadFile("icons/edit.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddSeparator();
	menu->AppendSeparator();
	btnBar->AddTool(wxID_EDIT, bmp, "Edit inverse Laplace transform");
	menu->Append(wxID_EDIT, "Edit inverse Laplace transform")->SetBitmap(bmp);
	img.LoadFile("icons/import.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_IMPORT, bmp, "Import decay data");
	menu->Append(wxID_IMPORT, "Import decay data")->SetBitmap(bmp);
	img.LoadFile("icons/bar.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddSeparator();
	menu->AppendSeparator();
	btnBar->AddTool(wxID_PSD, bmp, "Pore size distribution");
	menu->Append(wxID_PSD, "Pore size distribution")->SetBitmap(bmp);
	btnBar->AddSeparator();
	menu->AppendSeparator();
	img.LoadFile("icons/brick-add.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddSeparator();
	btnBar->AddTool(wxID_SAVE, bmp, "Save simulation");
	menu->Append(wxID_SAVE, "Save simulation")->SetBitmap(bmp);
	menubar->Append(menu,"NMR Simulation");
	
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSample::Build_3D_Sample, this, wxID_PLACE);
	menu->Bind(wxEVT_MENU, &WindowSample::Build_3D_Sample, this, wxID_PLACE);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSample::Walk, this, wxID_START);
	menu->Bind(wxEVT_MENU, &WindowSample::Walk, this, wxID_START);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSample::Save_Simulation, this, wxID_SAVE);
	menu->Bind(wxEVT_MENU, &WindowSample::Save_Simulation, this, wxID_SAVE);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSample::Show_Regularizer_Dialog, this, wxID_LAPLACE);
	menu->Bind(wxEVT_MENU, &WindowSample::Show_Regularizer_Dialog, this, wxID_LAPLACE);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSample::Laplace, this, wxID_LAPLACEF);
	menu->Bind(wxEVT_MENU, &WindowSample::Laplace, this, wxID_LAPLACEF);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSample::View_Walkers, this, wxID_VIEW_DETAILS);
	menu->Bind(wxEVT_MENU, &WindowSample::View_Walkers, this, wxID_VIEW_DETAILS);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSample::Edit_Laplace, this, wxID_EDIT);
	menu->Bind(wxEVT_MENU, &WindowSample::Edit_Laplace, this, wxID_EDIT);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSample::Import_Decay, this, wxID_IMPORT);
	menu->Bind(wxEVT_MENU, &WindowSample::Import_Decay, this, wxID_IMPORT);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSample::Create_PSD, this, wxID_PSD);
	menu->Bind(wxEVT_MENU, &WindowSample::Create_PSD, this, wxID_PSD);
}

WindowSample::WindowSample(wxWindow* parent) :
	wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,0,"Rock sample")
{
	this->_editingLaplace = false;
	this->_editLaplaceColor = wxColor(155, 150, 155);
	this->_viewWalkers = false;
	this->_simulationSaved = false;
	this->_currentSimModified = false;
	this->_simPath = "";
	this->_width = 0;
	this->_height = 0;
	this->_depth = 0;
	this->_currentSimulated = false;
	this->_parentMain = (WindowMain*)parent;
	this->_lowEnergy = wxColor(100, 100, 255);
	this->_highEnergy = wxColor(255, 100, 100);
	this->_walkersPos = 0;
	this->_formation = 0;
	this->_imgPtr = 0;
	this->_currentSimulation = 0;
	this->_imageResolution = 500;
	this->Freeze();
	wxAuiManager* mgr = new wxAuiManager(this, wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_TRANSPARENT_HINT);
	this->_windowVolume = new WindowVolume(this);
	wxPropertyGrid* pg = new wxPropertyGrid(
		this,
		0,
		wxDefaultPosition,
		wxSize(400,400),
		wxPG_SPLITTER_AUTO_CENTER |
		wxPG_DEFAULT_STYLE);
	this->_pgr = pg;
	pg->SetColumnProportion(0, 200);
	pg->SetColumnProportion(1, 225);
	pg->Append(new wxPropertyCategory("Experiment parameters"));
	wxPGProperty* pp = 0;
	wxPGProperty* cp = 0;
	wxPGChoices exp_type;
	exp_type.Add("T1");
	exp_type.Add("T2");
	pp = pg->Append(new wxEnumProperty("Experiment type","TYPE",exp_type,0));
	pp->SetValue(1);
	wxPGChoices arrSqT;
	arrSqT.Add("nm^2/s");
	arrSqT.Add("um^2/s");
	arrSqT.Add("mm^2/s");

	wxPGChoices arrVel;
	arrVel.Add("nm/s");
	arrVel.Add("um/s");
	arrVel.Add("mm/s");

	wxPGChoices arrLength;
	arrLength.Add("nm");
	arrLength.Add("um");
	arrLength.Add("mm");

	wxPGChoices arrTime;
	arrTime.Add("ns");
	arrTime.Add("us");
	arrTime.Add("ms");
	arrTime.Add("s");

	wxPGChoices arrPC;
	arrPC.Add("Constant");
	arrPC.Add("Sigmoid");

	wxPGProperty* df = pg->Append(new wxStringProperty("Diffusion coefficient (D)", "D"));
	df->ChangeFlag(wxPG_PROP_READONLY, true);
	df->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pg->AppendIn(df, new wxFloatProperty("Value", "valD", 0.0022));
	pp = pg->AppendIn(df, new wxEnumProperty("Units", "unD", arrSqT, 0));
	pp->SetValue(2);

	wxPGProperty* srpc = pg->Append(new wxEnumProperty("Surface relaxivity rule", "PC", arrPC, 0));
	wxPGProperty* sr = pg->AppendIn(srpc, new wxStringProperty("Surface relaxivity (P)", "P"));
	sr->ChangeFlag(wxPG_PROP_READONLY, true);
	sr->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pg->AppendIn(sr, new wxFloatProperty("Value", "valP", 25));
	pp = pg->AppendIn(sr, new wxEnumProperty("Units", "unP", arrVel, 0));
	pp->SetValue(1);

	wxPGProperty* dd = pg->Append(new wxStringProperty("Pixel/Voxel resolution (length)", "S"));
	dd->ChangeFlag(wxPG_PROP_READONLY, true);
	dd->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pg->AppendIn(dd, new wxFloatProperty("Value", "valS", 1));
	pp = pg->AppendIn(dd, new wxEnumProperty("Units", "unS", arrLength, 0));
	pp->SetValue(1);

	wxPGProperty* t2p = pg->Append(new wxStringProperty("Bulk time", "TB"));
	t2p->ChangeFlag(wxPG_PROP_READONLY, true);
	t2p->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pg->AppendIn(t2p, new wxFloatProperty("Value", "valTB", 2.8));
	pp = pg->AppendIn(t2p, new wxEnumProperty("Units", "unTB", arrTime, 0));
	pp->SetValue(3);

	wxPGProperty* stp = pg->Append(new wxStringProperty("Stop criterion", "STOP"));
	stp->ChangeFlag(wxPG_PROP_READONLY, true);
	stp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp = pg->AppendIn(stp, new wxFloatProperty("Magnetization threshold", "MTH",0.001));
	pp = pg->AppendIn(stp, new wxFloatProperty("Maximal time (s)", "MT", 10.0));

	wxPGProperty* drs = pg->Append(new wxStringProperty("Rock sample size", "RSS"));
	drs->ChangeFlag(wxPG_PROP_READONLY, true);
	drs->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	cp = pg->AppendIn(drs, new wxFloatProperty("X (Width)", "x", 1.0f));
	cp->ChangeFlag(wxPG_PROP_READONLY, true);
	cp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	cp = pg->AppendIn(drs, new wxFloatProperty("Y (Height)", "y", 1.0f));
	cp->ChangeFlag(wxPG_PROP_READONLY, true);
	cp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	cp = pg->AppendIn(drs, new wxFloatProperty("Z (Depth)", "z", 1.0f));
	cp->ChangeFlag(wxPG_PROP_READONLY, true);
	cp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	cp = pg->AppendIn(drs, new wxEnumProperty("Units", "unXYZDF", arrLength));

	wxArrayString arrDR;
	arrDR.Add("Null");
	arrDR.Add("Anisotropic");
	arrDR.Add("Isotropic");
	pp = pg->Append(new wxEnumProperty("Diffusive relaxation (G)", "DG",arrDR));
	pg->AppendIn(pp, new wxStringProperty("Diffusive relaxivity","DGR","(0,0,0) (1/s)"));
	this->Configure_Gradient_Options();

	pg->Append(new wxIntProperty("Number of walkers","NW",1280));
	pg->Append(new wxBoolProperty("GPU acceleration", "GPU", false));

	pg->Append(new wxPropertyCategory("Post-processing parameters"));
	pg->Append(new wxFloatProperty("Noise amplitude", "NSA", 0.01f));
	pp = pg->Append(new wxFloatProperty("SNR", "SNR", 0.0f));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pg->Append(new wxFloatProperty("Minimal T(1,2) (in seconds)", "Tmin", 0.0001f));
	pg->Append(new wxFloatProperty("Maximal T(1,2) (in seconds)", "Tmax", 10.0f));
	pg->Append(new wxIntProperty("Number of bins ", "TBINS", 128));
	pg->Append(new wxIntProperty("Decay compression (number of samples)", "CPRS", 1024));
	pg->Append(new wxFloatProperty("Regularizer", "REG", 0.1));

	int s_width;
	int s_height;
	WindowMain* wm = static_cast<WindowMain*>(parent);
	int diff = wm->Ribbon_Size() + wm->Bar_Size();
	wxDisplaySize(&s_width, &s_height);
	s_width = 2 * s_width / 3;
	s_height = s_height - diff;

	mgr->SetDockSizeConstraint(0.3, 0.36);
	this->Close_Properties();
	this->Set_Tags();	
	this->Bind(wxWALK_EVENT, &WindowSample::On_Walk, this);
	this->Bind(wxWALK_END_EVENT, &WindowSample::On_Walk_End, this);
	this->_updWalk = new UpdateWalk();

	this->Thaw();	
	this->_mgr = mgr;
	this->_updVol = new UpdateVolume();	
	this->_updVol->_parentFormation = this;
	this->_windowVolume->Volume_Panel().Set_Update_Event(this->_updVol);
	this->_updWalk = new UpdateWalk();
	this->_updWalk->_parentWindow = this;
	this->_pgr->Bind(wxEVT_PG_CHANGED, &WindowSample::Change_Grid_Value, this);
	this->_pgr->Bind(wxEVT_PG_CHANGING, &WindowSample::Changing_Grid_Value, this);
	this->_plotterA = new WxPlotter(this, wxID_ANY);
	this->_plotterB = new WxPlotter(this, wxID_ANY);
	this->_mgr->AddPane(this->_plotterA, wxAuiPaneInfo().Bottom().BestSize(s_width/2,36*s_height/100).Caption("Plot A: Linear"));
	this->_mgr->AddPane(this->_plotterB, wxAuiPaneInfo().Bottom().BestSize(s_width/2,36*s_height/100).Caption("Plot B: Logarithmic"));
	
	this->_plotterA->Set_Interval_Limits(-0.0001f, 0.0001f, -0.1f, 0.1f);
	this->_plotterB->Set_Interval_Limits(0.0001f, 10.0f,-0.0001f,0.0001f);
	this->_plotterB->Enable_Logarithmic_Scale_AxisX();
	mgr->AddPane(pg, wxAuiPaneInfo().BestSize(s_width / 3, 64 * s_height / 100).Left().CloseButton(false).Movable(false).Caption("Simulation parameters"));
	mgr->AddPane(this->_windowVolume, wxAuiPaneInfo().Caption("Rock sample").Center().BestSize(2*s_width / 3, 64 * s_height / 100).CloseButton(false).Movable(false));
}

void WindowSample::Update_Layout()
{
	this->_mgr->Update();
}

void WindowSample::Create_PSD(wxCommandEvent& evt)
{
	if (this->_currentSimulation)
	{
		DlgPoreSize ps(this->_parentMain, 0);
		ps.Set_Simulation(*this->_currentSimulation);
		ps.ShowModal();
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this, wxString("There is no simulation to get a PSD"), wxString("No simulation"), wxOK);
		mgdlg.ShowModal();
	}
}

void WindowSample::Edit_Laplace(wxCommandEvent& evt)
{
	if ((this->_currentSimulation)&&(!this->_editingLaplace))
	{
		this->_editingLaplace = true;
		this->_plotterB->Draw_Interval_Horizontal(true);
		int idx = this->_plotterB->Add_Curves(1);
		this->_plotterB->Set_Curve_Color(this->_editLaplaceColor, idx);
		this->_plotterB->Set_Change_Interval_X_Event(this);
		this->_plotterB->Define_X_Interval_Markers(
			this->_currentSimulation->Laplace_T_Min(),
			this->_currentSimulation->Laplace_T_Max());
		this->_plotterB->Refresh();
	}
	else
	{
		this->_plotterB->Draw_Interval_Horizontal(false);
		this->_plotterB->Refresh();
		if ((this->_currentSimulation)&&(this->_editingLaplace))
		{
			wxMessageDialog askdlg(this, "Do you want to apply current modifications?",
				"Inverse Laplace modified", wxYES | wxNO);
			if (askdlg.ShowModal() == wxID_YES)
			{
				this->_currentSimulation->Replace_Laplace_Vector(this->_editedLaplace);
				this->_currentSimModified = true;
			}
		}
		this->Draw_Current_Sim();
		this->_editingLaplace = false;
	}
}

void WindowSample::View_Walkers(wxCommandEvent& evt)
{
	if (this->_currentSimulation)
	{
		if (!this->_viewWalkers)
		{
			this->_viewWalkers = true;
		}
		else
		{
			this->_viewWalkers = false;
		}
		wxImage img;
		wxBitmap bmp;
		if (this->_viewWalkers)
		{
			img.LoadFile("icons//eye.png");
		}
		else
		{
			img.LoadFile("icons//eye-gray.png");
		}
		img.Rescale(32, 32);
		bmp = wxBitmap(img);
		if (!this->_formation)
		{
			if ((this->_currentSimulation) && (this->_viewWalkers) && (this->_imgPtr))
			{
				this->_formation = new rw::Plug();
				this->_formation->Set_Image_Formation(*this->_imgPtr);
				this->_currentSimulation->Fill_Plug_Paremeters(*this->_formation);
			}
		}
		if ((this->_formation) && (this->_viewWalkers))
		{
			this->Update_Walkers();
		}
		this->_formation->Show_Walkers(this->_viewWalkers);
		this->_windowVolume->Refresh();
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this, wxString("No simulation to visualize walkers"), wxString("No current simulation"), wxOK);
		mgdlg.ShowModal();
	}

}

void WindowSample::Laplace(wxCommandEvent& evt)
{
	if (this->_currentSimulation)
	{
		this->_pgr->CommitChangesFromEditor();
		this->Postprocess_Simulation_Parameters();
		this->_currentSimulation->Apply_Laplace();
		this->_currentSimModified = true;
		this->Draw_Current_Sim();
	}
}

void WindowSample::Save_Simulation_Dialog()
{
	wxMessageDialog dlg(this, 
		"Do you want to save current simulation modifications?\nPath:\n"
					+wxString(this->_simPath), 
		"Current simulations has not been saved",wxOK | wxCANCEL | wxICON_HAND);
	int state = dlg.ShowModal();	
	if (state == wxID_OK)
	{
		this->_plotterA->Block();
		this->_plotterB->Block();
		this->_currentSimulation->Save_To_File(string(this->_simPath.c_str()));
		this->_currentSimModified = false;
		this->_simulationSaved = false;
		this->_plotterA->UnBlock();
		this->_plotterB->UnBlock();
	}
}

wxString WindowSample::Current_Sim_File_Name() const
{
	wxString r = "";
	if (this->_simPath.Length() > 0)
	{
		wxFileName f(this->_simPath);
		r = f.GetName();
	}
	return(r);
}

void WindowSample::Set_Current_Simulation(rw::PlugPersistent* sim, const wxString& sim_path)
{
	bool overwrite = true;
	if (this->_currentSimulation)
	{
		if (this->_simulationSaved)
		{
			if (this->_currentSimModified)
			{
				this->Save_Simulation_Dialog();
			}
		}
		else 
		{
			wxMessageDialog dlg(this, "Do you want to overwrite the recently executed simulation?",
				"Overwriting simulation", wxICON_QUESTION | wxOK | wxCANCEL);
			int r = dlg.ShowModal();
			if (r == wxID_CANCEL)
			{
				overwrite = false;
			}
		}
	}
	if (overwrite)
	{
		this->_currentSimulation = sim;
		this->_simulationSaved = true;
		this->_currentSimModified = false;
		this->_simPath = sim_path;
		this->Update_Current_Simulation();
	}
}

void WindowSample::Update_Sim_Parameters(const rw::PlugPersistent& sim)
{
	wxPGProperty* pp;
	pp = this->_pgr->GetPropertyByName("TYPE");
	int te = 1;
	if (sim.SimulationParams().T1_Relaxation())
	{
		te = 0;
	}
	pp->SetValue(te);
	pp = this->_pgr->GetPropertyByName("D.valD");
	pp->SetValue(sim.Diffusion_Coefficient());
	pp = this->_pgr->GetPropertyByName("D.unD");
	pp->SetValue(sim.Diffusion_Coefficient_Units());
	pp = this->_pgr->GetPropertyByName("PC.P.valP");
	pp->SetValue(sim.Surface_Relaxivity());
	pp = this->_pgr->GetPropertyByName("PC.P.unP");
	pp->SetValue(sim.Surface_Relaxivity_Units());
	pp = this->_pgr->GetPropertyByName("S.valS");
	pp->SetValue(sim.Voxel_Length());
	pp = this->_pgr->GetPropertyByName("S.unS");
	pp->SetValue(sim.Voxel_Length_Units());
	pp = this->_pgr->GetPropertyByName("TB.valTB");
	pp->SetValue(sim.Bulk_Time());
	pp = this->_pgr->GetPropertyByName("TB.unTB");
	pp->SetValue(3);
	int choice = 0;
	scalar gyro;
	uint gyro_units;
	Field3D gradient;
	uint gradient_units;
	scalar dt;
	uint dtunits;
	sim.Get_Gradient_Parameters(gyro, gyro_units, gradient, gradient_units, dt, dtunits);
	if ((gradient.x == gradient.y) && (gradient.y == gradient.z) &&(gradient.x != 0))
	{
		choice = 1;
	}
	else if ((gradient.x != gradient.y) || (gradient.y != gradient.z))
	{
		choice = 2;
	}
	pp = this->_pgr->GetPropertyByName("DG");
	pp->SetValue(choice);
	this->Configure_Gradient_Options();
	wxPGProperty* pv = 0;
	wxPGProperty* pu = 0;
	switch (choice)
	{
		case 1:
		{
			pp = this->_pgr->GetPropertyByName("DG");
			pp->SetValue(1);
			pv = this->_pgr->GetPropertyByName("DG.DGR.Y.valY");
			pu = this->_pgr->GetPropertyByName("DG.DGR.Y.unY");
			pv->SetValue(gyro);
			pu->SetValue((int)gyro_units);
	
			pv = this->_pgr->GetPropertyByName("DG.DGR.Gdt.valdt");
			pu = this->_pgr->GetPropertyByName("DG.DGR.Gdt.undt");
			pv->SetValue(dt);
			pu->SetValue((int)dtunits);

			pv = this->_pgr->GetPropertyByName("DG.DGR.G.valG");
			pu = this->_pgr->GetPropertyByName("DG.DGR.G.unG");
			pv->SetValue(gradient.x);
			pu->SetValue((int)gradient_units);

			break;
		}
		case 2:
		{
			pp = this->_pgr->GetPropertyByName("DG");
			pp->SetValue(2);
			pv = this->_pgr->GetPropertyByName("DG.DGR.Y.valY");
			pu = this->_pgr->GetPropertyByName("DG.DGR.Y.unY");
			pv->SetValue(gyro);
			pu->SetValue((int)gyro_units);

			pv = this->_pgr->GetPropertyByName("DG.DGR.Gdt.valdt");
			pu = this->_pgr->GetPropertyByName("DG.DGR.Gdt.undt");
			pv->SetValue(dt);
			pu->SetValue((int)dtunits);

			pv = this->_pgr->GetPropertyByName("DG.DGR.Gx.valGx");
			pv->SetValue(gradient.x);
			pv = this->_pgr->GetPropertyByName("DG.DGR.Gy.valGy");
			pv->SetValue(gradient.y);
			pv = this->_pgr->GetPropertyByName("DG.DGR.Gz.valGz");
			pv->SetValue(gradient.z);
			pu = this->_pgr->GetPropertyByName("DG.DGR.unG");
			pu->SetValue((int)gradient_units);

			break;
		}
		default:
		{
			pp = this->_pgr->GetPropertyByName("DG");
			pp->SetValue(0);
			pp = this->_pgr->GetPropertyByName("DG.DGR");
			pp->SetValue(wxString("(0,0,0) G/um"));
			break;
		}
	}
	pp = this->_pgr->GetPropertyByName("NW");
	pp->SetValue((int)sim.Number_Of_Walkers());
	pp = this->_pgr->GetPropertyByName("GPU");
	pp->SetValue((bool)sim.SimulationParams().Get_Bool(GPU_P));
	pp = this->_pgr->GetPropertyByName("NSA");
	pp->SetValue(sim.Noise_Distortion());
	pp = this->_pgr->GetPropertyByName("SNR");
	pp->SetValue(sim.Signal_To_Noise_Ratio());
	pp = this->_pgr->GetPropertyByName("Tmin");
	pp->SetValue(sim.Laplace_T_Min());
	pp = this->_pgr->GetPropertyByName("Tmax");
	pp->SetValue(sim.Laplace_T_Max());
	pp = this->_pgr->GetPropertyByName("TBINS");
	pp->SetValue(sim.Laplace_Resolution());
	pp = this->_pgr->GetPropertyByName("CPRS");
	pp->SetValue(sim.Decay_Reduction());
	pp = this->_pgr->GetPropertyByName("REG");
	pp->SetValue(sim.Regularizer());
	pp = this->_pgr->GetPropertyByName("STOP.MTH");
	pp->SetValue(sim.Magnetization_Threshold());
	pp = this->_pgr->GetPropertyByName("STOP.MT");
	pp->SetValue((scalar)sim.Total_Number_Of_Simulated_Iterations()*sim.Time_Step_Simulation());
	this->Set_Tags();
}

void WindowSample::Save_Sim(const wxString& sim_file_name, const wxDateTime& wdt)
{
	WxSim wsim(*this->_currentSimulation);
	wsim.Set_Date_Time(wdt);
	wsim.Set_Image_Path(this->_parentMain->Image_Name());
	wsim.Save_File(sim_file_name);
}

void WindowSample::Save_Simulation(wxCommandEvent& evt)
{
	if (this->_parentMain->Save_Simulation(evt))
	{
		this->_simulationSaved = true;
	}
}

void WindowSample::Build_3D_Sample(wxCommandEvent& evt)
{
	this->_parentMain->Build_3D_Model(evt);
	this->_windowVolume->Volume_Panel().Set_Bar_Border(0, 1);
}

void WindowSample::Change_Grid_Value(wxPropertyGridEvent& evt)
{
	wxPGProperty* pp = evt.GetProperty();
	if (pp->GetName() == wxString("DG"))
	{
		this->Configure_Gradient_Options();
	}
	if (pp->GetName() == wxString("NSA") || (pp->GetName() == wxString("Tmin"))
		||(pp->GetName() == wxString("Tmax"))||(pp->GetName() == wxString("CPRS"))
		||(pp->GetName() == wxString("TBINS"))||(pp->GetName() == wxString("REG")))
	{
		if (this->_currentSimulation)
		{
			if (this->_simulationSaved)
			{
				this->_currentSimModified = true;
			}
		}
	}
	if (pp->GetName() == wxString("NSA"))
	{
		if (this->_currentSimulation)
		{
			this->_currentSimulation->Set_Noise_Distortion(pp->GetValue().GetDouble());
			wxPGProperty* pg = this->_pgr->GetPropertyByName("SNR");
			pg->SetValue(this->_currentSimulation->Signal_To_Noise_Ratio());
			this->Draw_Current_Sim();
		}
	}
	this->Set_Tags();
}

void WindowSample::Draw_Current_Sim()
{
	this->_plotterA->Erase_All_Curves();
	this->_plotterB->Erase_All_Curves();
	this->_plotterA->Set_Interval_Limits(-0.0001f, 0.0001f, -0.1f, 0.1f);
	this->_plotterB->Set_Interval_Limits(0.0001f, 10.0f, -0.0001f, 0.0001f);
	if (this->_currentSimulation)
	{
		this->_plotterA->Add_Curves(1);
		this->_plotterA->Set_Curve_Color(Wx_Color(this->_currentSimulation->Sim_Color()), 0);
		this->_plotterB->Add_Curves(1);
		this->_plotterB->Set_Curve_Color(Wx_Color(this->_currentSimulation->Sim_Color()), 0);
		this->Update_Laplace_Plot();
		this->Update_Decay_Plot();
	}
	this->_parentMain->Redraw_Simulations();
}

void WindowSample::Changing_Grid_Value(wxPropertyGridEvent& evt)
{
	this->_pgr->CommitChangesFromEditor();
	this->Set_Tags();
}

void WindowSample::Configure_Gradient_Options()
{
	wxPGChoices units_magnetic;
	units_magnetic.Add("Hz/G");
	units_magnetic.Add("Hz/T");

	wxPGChoices arrTime;
	arrTime.Add("ns");
	arrTime.Add("us");
	arrTime.Add("ms");
	arrTime.Add("s");

	wxPGChoices arrGradient;
	arrGradient.Add("G/um");
	arrGradient.Add("T/um");

	wxPGProperty* pp = 0;
	wxPGProperty* pf = 0;

	pf = this->_pgr->GetPropertyByName("DG");
	pp = this->_pgr->GetPropertyByName("DG.DGR");
	int choice = pf->GetValue().GetInteger();
	if (choice == 2)
	{
		pp->DeleteChildren();
		pf = this->_pgr->AppendIn(pp,new wxStringProperty("Gyromagnetic ratio","Y"));
		pf->ChangeFlag(wxPG_PROP_READONLY, true);
		pf->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		this->_pgr->AppendIn(pf, new wxFloatProperty("Value", "valY", 4257.747892));
		this->_pgr->AppendIn(pf, new wxEnumProperty("Units", "unY",units_magnetic,0));
		pf = this->_pgr->AppendIn(pp, new wxStringProperty("Spin Echo interval (dt)", "Gdt"));
		pf->ChangeFlag(wxPG_PROP_READONLY, true);
		pf->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		this->_pgr->AppendIn(pf, new wxFloatProperty("Value", "valdt", 0.0f));
		this->_pgr->AppendIn(pf, new wxEnumProperty("Units", "undt", arrTime, 0));
		pf = this->_pgr->AppendIn(pp, new wxStringProperty("Spatial gradient (Gx)", "Gx"));
		pf->ChangeFlag(wxPG_PROP_READONLY, true);
		pf->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		this->_pgr->AppendIn(pf, new wxFloatProperty("Value", "valGx", 0.0f));
		pf = this->_pgr->AppendIn(pp, new wxStringProperty("Spatial gradient (Gy)", "Gy"));
		pf->ChangeFlag(wxPG_PROP_READONLY, true);
		pf->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		this->_pgr->AppendIn(pf, new wxFloatProperty("Value", "valGy", 0.0f));

		pf = this->_pgr->AppendIn(pp, new wxStringProperty("Spatial gradient (Gz)", "Gz"));
		pf->ChangeFlag(wxPG_PROP_READONLY, true);
		pf->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		this->_pgr->AppendIn(pf, new wxFloatProperty("Value", "valGz", 0.0f));
		pf = this->_pgr->GetPropertyByName("DG.DGR");
		this->_pgr->AppendIn(pf, new wxEnumProperty("Spatial gradient units", "unG", arrGradient, 0));
		this->_pgr->Refresh();
	}
	if (choice == 1)
	{
		pp->DeleteChildren();
		pf = this->_pgr->AppendIn(pp, new wxStringProperty("Gyromagnetic ratio", "Y"));
		pf->ChangeFlag(wxPG_PROP_READONLY, true);
		pf->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		this->_pgr->AppendIn(pf, new wxFloatProperty("Value", "valY", 4257.747892));
		this->_pgr->AppendIn(pf, new wxEnumProperty("Units", "unY", units_magnetic, 0));
		pf = this->_pgr->AppendIn(pp, new wxStringProperty("Spin Echo interval (dt)", "Gdt"));
		pf->ChangeFlag(wxPG_PROP_READONLY, true);
		pf->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		this->_pgr->AppendIn(pf, new wxFloatProperty("Value", "valdt", 0.0f));
		this->_pgr->AppendIn(pf, new wxEnumProperty("Units", "undt", arrTime, 0));
		pf = this->_pgr->AppendIn(pp, new wxStringProperty("Spatial gradient (G)", "G"));
		pf->ChangeFlag(wxPG_PROP_READONLY, true);
		pf->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		this->_pgr->AppendIn(pf, new wxFloatProperty("Value", "valG", 0.0f));
		this->_pgr->AppendIn(pf, new wxEnumProperty("Units", "unG", arrGradient, 0));
		this->_pgr->Refresh();
	}
	if (choice == 0)
	{
		pp->DeleteChildren();
	}
	this->_pgr->Refresh();
}

void WindowSample::Close_Properties()
{
	wxPGProperty* pp = 0;
	this->_pgr->GetProperty("D")->SetExpanded(false);
	this->_pgr->GetProperty("PC")->SetExpanded(true);
	this->_pgr->GetProperty("PC.P")->SetExpanded(false);
	this->_pgr->GetProperty("TB")->SetExpanded(false);
	this->_pgr->GetProperty("S")->SetExpanded(false);
}

void WindowSample::On_Walk(wxCommandEvent& event)
{
	wxLongLong ct = wxGetLocalTimeMillis();
	int id = this->_formation->Decay_Size() - 1;
	rw::Step_Value v = this->_formation->Decay_Step_Value(id);
	this->_plotterA->Add_Curve_Point(0,v.Time, v.Magnetization);
	this->_plotterA->Refresh();
	this->Update_Walkers(true);
	this->_updVol->_updateVtxBuffer = true;
	this->_windowVolume->Refresh();
	this->_parentMain->Step_Progress_Bar(event.GetInt());
}

void WindowSample::On_Walk_End(wxCommandEvent& event)
{
	this->_updVol->_updateVtxBuffer = false;
	this->_currentSimulation->Get_Formation_Properties(*this->_formation);
	this->Postprocess_Simulation_Parameters();
	this->_parentMain->Step_Progress_Bar(100);
	wxCommandEvent e;
	this->Show_Regularizer_Dialog(e);
	this->Draw_Current_Sim();
	this->Update_Current_Simulation();
}

const rw::PlugPersistent& WindowSample::Current_Simulation() const
{
	return(*this->_currentSimulation);
}

void WindowSample::Show_Regularizer_Dialog(wxCommandEvent& evt)
{
	if ((this->_currentSimulation))
	{
		DlgRegularizer* dlgreg = new DlgRegularizer(this);
		dlgreg->Set_Simulation(*this->_currentSimulation);
		dlgreg->ShowModal();
		int result = dlgreg->Decision();
		if (result == wxID_OK)
		{
			scalar regularizer = dlgreg->Regularizer();
			this->_currentSimulation->Set_Laplace_Regularizer(regularizer);
			this->_currentSimModified = true;
			wxPGProperty* pv = 0;
			pv = this->_pgr->GetPropertyByName("REG");
			pv->SetValue(regularizer);
			this->_currentSimulation->Set_Laplace_Regularizer(regularizer);
			this->_currentSimulation->Apply_Laplace();
		}
		delete dlgreg;
		if (this->_simulationSaved)
		{
			wxProgressDialog dlg("Saving simulation", "Updating simulation regularizer\nPath:\n" + this->_simPath);
			dlg.Show();
			dlg.Pulse();
		}
	}
	else
	{
		wxMessageDialog dlg(this, "No simulation has been executed", "No decay data",wxOK | wxICON_EXCLAMATION);
		dlg.ShowModal();

	}
}

void WindowSample::Update_Decay_Plot()
{
	int size = this->_currentSimulation->Decay_Vector_Size();
	for (int i = 0; i < size; ++i)
	{
		rw::Step_Value v = this->_currentSimulation->Decay_Step_Value(i);
		this->_plotterA->Add_Curve_Point(0, v.Time, v.Magnetization);
	}
	this->_plotterA->Refresh();
}

void WindowSample::Update_Laplace_Plot()
{
	this->_plotterB->Erase_All_Curves();
	this->_plotterB->Add_Curves(1);
	this->_plotterB->Set_Curve_Color(Wx_Color(this->_currentSimulation->Sim_Color()),0);
	if (this->_currentSimulation->SimulationParams().T1_Relaxation())
	{
		this->_plotterB->Set_Title("T1 distribution");
		this->_plotterB->Set_X_Title("T1 (s)");
		this->_plotterB->Set_Y_Title("Bin");
	}
	if (this->_currentSimulation->SimulationParams().T2_Relaxation())
	{
		this->_plotterB->Set_Title("T2 distribution");
		this->_plotterB->Set_X_Title("T2 (s)");
		this->_plotterB->Set_Y_Title("Bin");
	}
	math_la::math_lac::full::Vector td = this->_currentSimulation->Laplace_Time_Vector();
	math_la::math_lac::full::Vector ld = this->_currentSimulation->Laplace_Bin_Vector();
	for (int i = 0; i < (int)td.Size(); ++i)
	{
		this->_plotterB->Add_Curve_Point(0, td(i), ld(i));
	}
	this->_plotterB->Refresh();
}

void WindowSample::Set_Tags()
{
	wxString pn;
	wxPGProperty* pp = 0;
	pp = this->_pgr->GetProperty("D.valD");
	pn = pp->GetValueAsString();
	pp = this->_pgr->GetProperty("D.unD");
	pn = pn + wxString(" ") + pp->GetValueAsString();
	pp = this->_pgr->GetProperty("D");
	pp->SetValue(pn);
	pp = this->_pgr->GetProperty("PC.P.valP");
	pn = pp->GetValueAsString();
	pp = this->_pgr->GetProperty("PC.P.unP");
	pn = pn + wxString(" ") + pp->GetValueAsString();
	pp = this->_pgr->GetProperty("PC.P");
	pp->SetValue(pn);
	pp = this->_pgr->GetProperty("TB.valTB");
	pn = pp->GetValueAsString();
	pp = this->_pgr->GetProperty("TB.unTB");
	pn = pn + wxString(" ") + pp->GetValueAsString();
	pp = this->_pgr->GetProperty("TB");
	pp->SetValue(pn);
	pp = this->_pgr->GetProperty("S.valS");
	pn = pp->GetValueAsString();
	pp = this->_pgr->GetProperty("S.unS");
	pn = pn + wxString(" ") + pp->GetValueAsString();
	pp = this->_pgr->GetProperty("S");
	pp->SetValue(pn);
	wxPGProperty* pv = 0;
	wxPGProperty* pu = 0;
	pp = this->_pgr->GetProperty("DG");
	int gradient_mode = pp->GetValue().GetInteger();
	switch (gradient_mode)
	{
	case 1:
	{
		pp = this->_pgr->GetPropertyByName("DG.DGR.Y");
		pv = this->_pgr->GetPropertyByName("DG.DGR.Y.valY");
		pu = this->_pgr->GetPropertyByName("DG.DGR.Y.unY");
		pp->SetValue(pv->GetValueAsString() + wxString(" ") + pu->GetValueAsString());

		pp = this->_pgr->GetPropertyByName("DG.DGR.Gdt");
		pv = this->_pgr->GetPropertyByName("DG.DGR.Gdt.valdt");
		pu = this->_pgr->GetPropertyByName("DG.DGR.Gdt.undt");
		pp->SetValue(pv->GetValueAsString() + wxString(" ") + pu->GetValueAsString());

		pp = this->_pgr->GetPropertyByName("DG.DGR.G");
		pv = this->_pgr->GetPropertyByName("DG.DGR.G.valG");
		pu = this->_pgr->GetPropertyByName("DG.DGR.G.unG");
		wxString gstr = pv->GetValueAsString();
		pp->SetValue(gstr + wxString(" ") + pu->GetValueAsString());
		pp = this->_pgr->GetPropertyByName("DG.DGR");
		pp->SetValue(wxString("(") + gstr + "," + gstr + "," + gstr + ")" + pu->GetValueAsString());
		break;
	}
	case 2:
	{
		pp = this->_pgr->GetPropertyByName("DG.DGR.Y");
		pv = this->_pgr->GetPropertyByName("DG.DGR.Y.valY");
		pu = this->_pgr->GetPropertyByName("DG.DGR.Y.unY");
		pp->SetValue(pv->GetValueAsString() + wxString(" ") + pu->GetValueAsString());

		pp = this->_pgr->GetPropertyByName("DG.DGR.Gdt");
		pv = this->_pgr->GetPropertyByName("DG.DGR.Gdt.valdt");
		pu = this->_pgr->GetPropertyByName("DG.DGR.Gdt.undt");
		pp->SetValue(pv->GetValueAsString() + wxString(" ") + pu->GetValueAsString());

		pp = this->_pgr->GetPropertyByName("DG.DGR.Gx");
		pv = this->_pgr->GetPropertyByName("DG.DGR.Gx.valGx");
		pu = this->_pgr->GetPropertyByName("DG.DGR.unG");
		wxString gstr = pv->GetValueAsString();
		pp->SetValue(gstr + wxString(" ") + pu->GetValueAsString());

		pp = this->_pgr->GetPropertyByName("DG.DGR.Gy");
		pv = this->_pgr->GetPropertyByName("DG.DGR.Gy.valGy");
		pu = this->_pgr->GetPropertyByName("DG.DGR.unG");
		gstr = pv->GetValueAsString();
		pp->SetValue(gstr + wxString(" ") + pu->GetValueAsString());

		pp = this->_pgr->GetPropertyByName("DG.DGR.Gz");
		pv = this->_pgr->GetPropertyByName("DG.DGR.Gz.valGz");
		pu = this->_pgr->GetPropertyByName("DG.DGR.unG");
		gstr = pv->GetValueAsString();
		pp->SetValue(gstr + wxString(" ") + pu->GetValueAsString());

		pp = this->_pgr->GetPropertyByName("DG.DGR");
		pp->SetValue(wxString("(") + gstr + "," + gstr + "," + gstr + ")" + pu->GetValueAsString());

		break;
	}
	default:
	{
		pp = this->_pgr->GetPropertyByName("DG.DGR");
		pp->SetValue(wxString("(0,0,0) G/um"));
		break;
	}
	}
	this->Update_Rock_Size();
}

WindowSample::~WindowSample()
{
	if ((this->_currentSimulation) && (this->_currentSimModified) && (this->_simulationSaved))
	{
		this->Save_Simulation_Dialog();
		delete this->_currentSimulation;
		this->_currentSimulation = 0;
	}
	delete this->_updVol;
	this->_updVol = 0;
	delete this->_updWalk;
	this->_updWalk = 0;
	if (this->_currentSimulated)
	{
		if (this->_currentSimulation)
		{
			delete this->_currentSimulation;
		}
	}
	if (this->_formation)
	{
		delete this->_formation;
	}
	this->_mgr->UnInit();
}

void WindowSample::Update_Rock_Size()
{
	wxPGProperty* pp =0;
	pp = this->_pgr->GetPropertyByName("S.valS");
	scalar length = pp->GetValue().GetDouble();
	pp = this->_pgr->GetPropertyByName("S.unS");
	int length_u = pp->GetValue().GetInteger();
	pp = this->_pgr->GetPropertyByName("RSS.unXYZDF");
	int dim_u = pp->GetValue().GetInteger();
	int diff_u = length_u-dim_u;
	length = length * pow(10, 3*diff_u);
	pp = this->_pgr->GetPropertyByName("RSS.x");
	pp->SetValue((scalar)this->_width*length);
	pp = this->_pgr->GetPropertyByName("RSS.y");
	pp->SetValue((scalar)this->_height*length);
	pp = this->_pgr->GetPropertyByName("RSS.z");
	pp->SetValue((scalar)this->_depth*length);
}

void WindowSample::Fix_Sample(const rw::BinaryImage& img, wxGenericProgressDialog* pgdlg, uint& sx, uint& sy, vector<uint>& indices)
{
	indices.clear();
	WxImageAdapter b_img;
	img.Layer(b_img, 0);
	wxImage limg = b_img.Image();
	float factor = 1;
	if (((limg.GetSize().x > this->_imageResolution)
		|| (limg.GetSize().y > this->_imageResolution)))
	{
		factor = (float)std::max(limg.GetSize().x, limg.GetSize().y) / (float)this->_imageResolution;
		factor = 1.0f / factor;
	}
	if ((int)((float)img.Depth() * factor) > this->_imageResolution)
	{
		factor = (float)this->_imageResolution / (float)img.Depth();
	}
	uint rdepth = img.Depth() * factor;
	if (rdepth < 1)
	{
		rdepth = 1;
	}
	if (rdepth % 2 != 0)
	{
		rdepth = rdepth + 1;
	}
	sx = limg.GetSize().x * factor;
	sy = limg.GetSize().y * factor;
	if (sx % 4 != 0)
	{
		sx = sx + 4 - (sx % 4);
	}
	if (sy % 4 != 0)
	{
		sy = sy + 4 - (sy % 4);
	}	
	if (sy < 256)
	{
		sy = 512;
	}
	if (sx < 16)
	{
		sx = 32;
	}
	pgdlg->Update(2, "Pairing image indices");
	pgdlg->Refresh();

	indices.reserve(std::max(rdepth,(uint)256));
	if (rdepth < 256)
	{
		uint s = 1;
		while ((s * rdepth < 256)&&(s < 256))
		{
			s = s + 1;
		}
		uint na = s;
		this->_windowVolume->Volume_Panel().Set_Z_Compression(1.0f/(float)na);
		if (na < 1)
		{
			na = 1;
		}	
		uint id = 0;
		for (uint k = 0; k < rdepth; ++k)
		{		
			for (uint j = 0; j < na; ++j)
			{				
				indices.push_back(k);
			}
		}
	}
	else
	{
		if (rdepth < (uint)this->_imageResolution)
		{
			for (uint k = 0; k < rdepth; ++k)
			{
				indices.push_back(k);
			}
		}
		else
		{
			for (uint k = 0; k < (uint)this->_imageResolution; ++k)
			{
				int s = (uint)(((float)(k * img.Depth())) / (float)rdepth);
				indices.push_back(s);
			}
		}
	}
}

void WindowSample::Build_3D_Model(const rw::BinaryImage& img, wxGenericProgressDialog* pgdlg)
{
	if (this->_formation)
	{
		delete this->_formation;
		this->_formation = 0;
		delete this->_updVol;
	}
	this->_width = img.Width();
	this->_height = img.Height();
	this->_depth = img.Depth();	
	this->_mgr->DetachPane(this->_windowVolume);
	this->_mgr->Update();
	delete this->_windowVolume;
	this->_windowVolume = new WindowVolume(this);
	this->_mgr->AddPane(this->_windowVolume, wxAuiPaneInfo().Caption("Rock sample").Center());
	this->_windowVolume->Freeze();
	this->_updVol = new UpdateVolume();
	this->_updVol->_parentFormation = this;
	this->_mgr->Update();

	pgdlg->SetRange(img.Depth()+4);
	pgdlg->Update(1,"Assembling 3D model");
	pgdlg->CenterOnScreen();
	pgdlg->Show();
	vector<uint> indices;
	uint sx = 0;
	uint sy = 0;
	this->Fix_Sample(img, pgdlg, sx, sy, indices);
	pgdlg->SetRange((uint)indices.size()+4);
	for (int k = 0; k < indices.size(); ++k)
	{
		uint s = indices[k];
		if (s > (uint)img.Depth() - 1)
		{
			s = img.Depth() - 1;
		}
		WxImageAdapter bb_img;
		img.Layer(bb_img, s);
		wxImage rimg = bb_img.Image();
		rimg = rimg.Scale(sx, sy, wxIMAGE_QUALITY_BOX_AVERAGE);
		this->_windowVolume->Add_Image(rimg);
		pgdlg->Update(k, wxString("Adding texture layer number ") << s);
		pgdlg->Refresh();
	}

	pgdlg->Update((uint)indices.size()+3, "Assembling 3D model");
	pgdlg->Refresh();
	this->_windowVolume->Volume_Panel().Set_Update_Event(this->_updVol);
	this->_windowVolume->Volume_Panel().Load_3D_Texture();
	this->_imgPtr = &img;
	this->_parentMain->Set_Image_Pointer(this->_imgPtr);
	if (this->_viewWalkers)
	{
		this->_viewWalkers = false;
		wxCommandEvent e;
		this->View_Walkers(e);
	}
	wxString lbl;
	scalar porosity = (scalar)img.Black_Voxels() / (scalar)(img.Width()*img.Height()*img.Depth());
	wxString ptxt = wxString::FromDouble(porosity, 4);
	lbl << "Black voxels: " << img.Black_Voxels() << ". Porosity: " << ptxt;
	
	this->_windowVolume->Set_Subtitle(lbl);
	this->Update_Rock_Size();
	pgdlg->Refresh();
	this->_windowVolume->Thaw();
	this->_windowVolume->Refresh();
	this->_windowVolume->Volume_Panel().Set_Bar_Border(0, 1);
	this->_windowVolume->Volume_Panel().Refresh();
	
	wxPGProperty* pp;
	pp = this->_pgr->GetPropertyByName("STOP.MT");
	pp->SetValue(10);
	this->_pgr->Refresh();
}

void WindowSample::Update_Walkers(bool updatePos)
{
	this->_windowVolume->Volume_Panel().Lock_Refresh();
	if (this->_walkerPosSize != this->_formation->Number_Of_Walking_Particles())
	{
		this->_walkerPosSize = this->_formation->Number_Of_Walking_Particles();
		if (this->_walkersPos)
		{
			delete []this->_walkersPos;
		}
		this->_walkersPos = new GLfloat[4 * this->_walkerPosSize];
	}
	const rw::Plug* mw = this->_formation;
	if (this->_formation->Dimension() == 2)
	{
		tbb::parallel_for(tbb::blocked_range<int>(0, this->_walkerPosSize, this->_walkerPosSize / 8),
			[this, mw](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				const rw::Walker& ww = mw->Walking_Particle(i);
				GLfloat x = (GLfloat)((float)ww(eX));
				GLfloat y = (GLfloat)((float)ww(eY));
			}
		});
	}
	else
	{
		this->_strikesMax = 0;
		if ((this->_formation) && (!this->_formation->Empty_Particles()))
		{
			if (updatePos)
			{
				const rw::Plug* m = this->_formation;
				tbb::spin_mutex mtx;
				tbb::parallel_for(tbb::blocked_range<int>(0, this->_walkerPosSize, this->_walkerPosSize / 16),
					[this,m, &mtx](const tbb::blocked_range<int>& b)
				{
					for (int i = b.begin(); i < b.end(); ++i)
					{
						Vec3 sps;
						const rw::Walker& ww = m->Walking_Particle(i);
						scalar x = (scalar)ww(eX) / (scalar)m->Image_Size(0);
						scalar y = (scalar)ww(eY) / (scalar)m->Image_Size(1);
						scalar z = (scalar)ww(eZ) / (scalar)m->Image_Size(2);

						Vec3 box = this->_windowVolume->Volume_Panel().Box();
						box(eZ, box(eZ) / this->_windowVolume->Volume_Panel().ZCompression());
						x = x * box(eX);
						y = y * box(eY);
						z = z * box(eZ);

						sps(eX, x  -  box(eX)/2.0f);
						sps(eY, -y +  box(eY)/2.0f);
						sps(eZ, z  -  box(eZ)/2.0f);
						Vec3 cf = this->_windowVolume->Factor3D();
						scalar fe = ww.Magnetization();
						cf = ((scalar)0.5)*cf;
						sps = sps + 0.5*cf;
						this->_walkersPos[4 * i] = (GLfloat)sps(eX);
						this->_walkersPos[4 * i + 1] = (GLfloat)sps(eY);
						this->_walkersPos[4 * i + 2] = (GLfloat)sps(eZ);
						this->_walkersPos[4 * i + 3] = ww.Hits();
						if (ww.Hits() > this->_strikesMax)
						{
							mtx.lock();
							this->_strikesMax = ww.Hits();														
							mtx.unlock();
						}
					}
				});
			}
		}
	}
	this->_minHits = 0;
	this->_maxHits = this->_formation->Decay_Size();
	this->_windowVolume->Volume_Panel().Unlock_Refresh();
}

void WindowSample::Preprocess_Simulation_Parameters(const rw::BinaryImage& img)
{
	if ((this->_currentSimulation)&&(this->_currentSimModified)&&(this->_simulationSaved))
	{
		this->Save_Simulation_Dialog();
		delete this->_currentSimulation;
		this->_currentSimulation = 0;
	}
	if (this->_currentSimulation)
	{
		if (!this->_simulationSaved)
		{
			delete this->_currentSimulation;
		}
		this->_currentSimulation = 0;
	}
	this->_simulationSaved = false;
	this->_currentSimModified = true;

	if (this->_formation)
	{
		delete this->_formation;
		this->_formation = 0;
	}
	this->_pgr->CommitChangesFromEditor();
	this->_currentSimulation = new rw::PlugPersistent();
	this->_currentSimulated = true;
	wxPGProperty* pv = 0;
	wxPGProperty* pu = 0;
	pv = this->_pgr->GetPropertyByName("D.valD");
	pu = this->_pgr->GetPropertyByName("D.unD");
	this->_currentSimulation->Set_Diffusion_Coefficient(pv->GetValue().GetDouble(), pu->GetValue().GetInteger());
	int rho_mode = this->_pgr->GetPropertyByName("PC")->GetValue().GetInteger();
	if (rho_mode == 0)
	{
		pv = this->_pgr->GetPropertyByName("PC.P.valP");
		pu = this->_pgr->GetPropertyByName("PC.P.unP");
		this->_currentSimulation->Set_Surface_Relaxivity(pv->GetValue().GetDouble(), pu->GetValue().GetInteger());
	}
	pv = this->_pgr->GetPropertyByName("TB.valTB");
	pu = this->_pgr->GetPropertyByName("TB.unTB");
	this->_currentSimulation->Set_Bulk_Time(pv->GetValue().GetDouble(), pu->GetValue().GetInteger());
	pv = this->_pgr->GetPropertyByName("S.valS");
	pu = this->_pgr->GetPropertyByName("S.unS");
	this->_currentSimulation->Set_Voxel_Length(pv->GetValue().GetDouble(), pu->GetValue().GetInteger());

	pv = this->_pgr->GetPropertyByName("NW");
	int NW = pv->GetValue().GetInteger();

	pv = this->_pgr->GetPropertyByName("DG");
	int gradient_mode = pv->GetValue().GetInteger();
	Field3D gradient;
	gradient.x = 0;
	gradient.y = 0;
	gradient.z = 0;
	switch (gradient_mode)
	{
	case 1:
	{
		pv = this->_pgr->GetPropertyByName("DG.DGR.Y.valY");
		pu = this->_pgr->GetPropertyByName("DG.DGR.Y.unY");
		scalar gyro = pv->GetValue().GetDouble();
		uint gyrou = pu->GetValue().GetInteger();

		pv = this->_pgr->GetPropertyByName("DG.DGR.Gdt.valdt");
		pu = this->_pgr->GetPropertyByName("DG.DGR.Gdt.undt");
		scalar dt = pv->GetValue().GetDouble();
		uint dtu = pu->GetValue().GetInteger();

		pv = this->_pgr->GetPropertyByName("DG.DGR.G.valG");
		pu = this->_pgr->GetPropertyByName("DG.DGR.G.unG");
		scalar g = pv->GetValue().GetDouble();
		scalar gu = pv->GetValue().GetInteger();
		gradient.x = g;
		gradient.y = g;
		gradient.z = g;
		this->_currentSimulation->Set_Gradient_Parameters(gyro, gyrou, gradient, gu, dt, dtu);
		break;
	}
	case 2:
	{
		pv = this->_pgr->GetPropertyByName("DG.DGR.Y.valY");
		pu = this->_pgr->GetPropertyByName("DG.DGR.Y.unY");
		scalar gyro = pv->GetValue().GetDouble();
		uint gyrou = pu->GetValue().GetInteger();

		pv = this->_pgr->GetPropertyByName("DG.DGR.Gdt.valdt");
		pu = this->_pgr->GetPropertyByName("DG.DGR.Gdt.undt");
		scalar dt = pv->GetValue().GetDouble();
		uint dtu = pu->GetValue().GetInteger();
		scalar g;
		pv = this->_pgr->GetPropertyByName("DG.DGR.Gx.valGx");
		g = pv->GetValue().GetDouble();
		gradient.x = g;
		pv = this->_pgr->GetPropertyByName("DG.DGR.Gy.valGy");
		g = pv->GetValue().GetDouble();
		gradient.y = g;
		pv = this->_pgr->GetPropertyByName("DG.DGR.Gz.valGz");
		g = pv->GetValue().GetDouble();
		gradient.z = g;
		pu = this->_pgr->GetPropertyByName("DG.DGR.unG");
		uint gu = pu->GetValue().GetInteger();
		this->_currentSimulation->Set_Gradient_Parameters(gyro, gyrou, gradient, gu, dt, dtu);
		break;
	}
	default:
	{
		this->_currentSimulation->Set_Gradient_Parameters(0, 0, gradient, 0, 0, 0);
		break;
	}
	}

	pv = this->_pgr->GetPropertyByName("STOP.MTH");
	this->_currentSimulation->Set_Magnetization_Threshold(pv->GetValue().GetDouble());
	scalar tstep = (scalar)this->_currentSimulation->Time_Step_Simulation();
	pv = this->_pgr->GetPropertyByName("STOP.MT");
	scalar tstop = pv->GetValue().GetDouble();
	int itrs = (int)(tstop / tstep) + 1;
	pv->SetValue(((scalar)itrs)*tstep);
	this->_formation = new rw::Plug();
	pv = this->_pgr->GetPropertyByName("TYPE");
	
	this->_formation->Limit_Maximal_Number_Of_Iterations(itrs);
	this->_formation->Set_Stop_Threshold(this->_currentSimulation->Magnetization_Threshold());
	int te = pv->GetValue().GetInteger();
	rw::SimulationParams params;
	if (te == 0)
	{
		params.Set_Bool(T2, false);
	}
	else
	{
		params.Set_Bool(T2, true);
	}
	this->_formation->Set_Internal_Gradient(this->_currentSimulation->Gradient());
	this->_formation->Set_Image_Formation(img);
	this->_formation->Set_On_Walk_Event(this->_updWalk);
	this->_formation->Set_Number_Of_Walking_Particles(NW);	
	this->_formation->Set_Surface_Relaxivity_Delta(
		this->_currentSimulation->Surface_Relaxivity_Factor(this->_currentSimulation->Surface_Relaxivity()));
	this->_formation->Set_TBulk_Time_Seconds(this->_currentSimulation->Bulk_Time());	
	this->_formation->Set_Time_Step(this->_currentSimulation->Time_Step_Simulation());
}

void WindowSample::Postprocess_Simulation_Parameters()
{
	wxPGProperty* pv = 0;
	pv = this->_pgr->GetPropertyByName("NSA");
	this->_currentSimulation->Set_Noise_Distortion(pv->GetValue().GetDouble());
	pv = this->_pgr->GetPropertyByName("SNR");
	pv->SetValue(this->_currentSimulation->Signal_To_Noise_Ratio());
	pv = this->_pgr->GetPropertyByName("Tmin");
	scalar tmin = pv->GetValue().GetDouble();
	pv = this->_pgr->GetPropertyByName("Tmax");
	scalar tmax = pv->GetValue().GetDouble();
	pv = this->_pgr->GetPropertyByName("TBINS");
	uint resolution = pv->GetValue().GetInteger();
	pv = this->_pgr->GetPropertyByName("CPRS");
	uint decay_compression = pv->GetValue().GetInteger();
	pv = this->_pgr->GetPropertyByName("REG");
	scalar regularizer = pv->GetValue().GetDouble();
	this->_currentSimulation->Set_Laplace_Parameters(tmin, tmax, resolution, regularizer);
	this->_currentSimulation->Set_Decay_Reduction(decay_compression);
}

void WindowSample::Walk(wxCommandEvent& event)
{
	if (this->_imgPtr)
	{
		this->Preprocess_Simulation_Parameters(*this->_imgPtr);
	}
	if ((this->_formation)&&(this->_imgPtr))
	{
		this->_mgr->GetPane(this->_plotterA).Caption("Plot A: Decay");
		this->_plotterA->Set_X_Title("Time (s)");
		this->_plotterA->Set_Y_Title("Magnetization");
		this->_plotterA->Set_Title("NMR Decay");
		this->_plotterA->Erase_All_Curves();
		this->_plotterA->Add_Curves(1);
		this->Preprocess_Simulation_Parameters(*this->_imgPtr);
		this->_currentSimulation->Set_Sim_Color(Rw_Color(this->_plotterA->Curve_Color(0)));
		this->_formation->Place_Walking_Particles();
		bool gpu = this->_pgr->GetPropertyByName("GPU")->GetValue().GetBool();
		this->_formation->Show_Walkers(this->_viewWalkers);
		this->_formation->Activate_GPU_Walk(gpu);
		this->_formation->Start_Walk();
		this->_plotterA->Refresh();
		this->_mgr->Update();
	}
	else
	{
		wxMessageDialog dlg(this, "Lacking image formation to perform random walk",
			"Simulation cannot be executed", wxOK | wxICON_ERROR);
		dlg.ShowModal();
	}
}

bool WindowSample::Has_Current_Simulation() const
{
	return(this->_currentSimulation != 0);
}

void WindowSample::Add_Sim_Plot(const rw::PlugPersistent& sim, int& id)
{
	this->_mgr->GetPane(this->_plotterB).Caption("Plot B: Inverse Laplace transform");
	if (sim.SimulationParams().T1_Relaxation())
	{
		this->_plotterB->Set_Title("T1 distribution");
		this->_plotterB->Set_X_Title("T1 (s)");
		this->_plotterB->Set_Y_Title("Bin");
	}
	if (sim.SimulationParams().T2_Relaxation())
	{
		this->_plotterB->Set_Title("T2 distribution");
		this->_plotterB->Set_X_Title("T2 (s)");
		this->_plotterB->Set_Y_Title("Bin");
	}
	this->_mgr->GetPane(this->_plotterA).Caption("Plot A: Decay");
	this->_plotterA->Set_X_Title("Time (s)");
	this->_plotterA->Set_Y_Title("Magnetization");
	this->_plotterA->Set_Title("NMR Decay");

	id = this->_plotterA->Add_Curves(1);
	int s_id = this->_plotterB->Add_Curves(1);
	if (id == s_id)
	{
		this->_plotterA->Set_Curve_Color(Wx_Color(sim.Sim_Color()), id);
		this->_plotterB->Set_Curve_Color(Wx_Color(sim.Sim_Color()), id);
		for (int k = 0; k < sim.Decay_Vector_Size(); ++k)
		{
			rw::Step_Value v = sim.Decay_Step_Value(k);
			WxPlotter::Pair pp;
			pp.x = v.Time;
			pp.y = v.Magnetization;
			this->_plotterA->Add_Curve_Point(id, pp);
		}
		math_la::math_lac::full::Vector dom = sim.Laplace_Time_Vector();
		math_la::math_lac::full::Vector mm = sim.Laplace_Bin_Vector();
		for (int k = 0; k < (int)dom.Size(); ++k)
		{
			WxPlotter::Pair pp;
			pp.x = dom(k);
			pp.y = mm(k);
			this->_plotterB->Add_Curve_Point(id, pp);
		}
	}
	this->_plotterA->Refresh();
	this->_plotterB->Refresh();
}

void WindowSample::Erase_Plots()
{
	this->_plotterA->Erase_All_Curves();
	this->_plotterB->Erase_All_Curves();
	this->_plotterA->Set_Interval_Limits(-0.05f, 0.2f, -0.1f, 0.1f);
	this->_plotterB->Set_Interval_Limits(0.0001f, 10.0f, -0.0001f, 0.0001f);
	this->_plotterA->Refresh();
	this->_plotterB->Refresh();
}

void WindowSample::Update_Current_Simulation()
{
	if (this->_simulationSaved)
	{
		this->_parentMain->Current_Simulation_Update(false);
	}
	else
	{
		if (this->_currentSimulation)
		{
			this->_parentMain->Current_Simulation_Update(true);
		}
		else
		{
			this->_parentMain->Current_Simulation_Update(false);
		}
	}
}

void WindowSample::Set_Sample_Name(const wxString& name)
{
	this->_mgr->GetPane(this->_windowVolume).Caption(name);
	this->_mgr->Update();
}

void WindowSample::Check_Simulation_Pointer_To_Delete(const rw::PlugPersistent* sim)
{
	if (sim)
	{
		if (sim == this->_currentSimulation)
		{
			if ((this->_currentSimModified) && (this->_simulationSaved))
			{
				this->Save_Simulation_Dialog();
			}
			this->_currentSimulation = 0;
			this->_simulationSaved = false;
			this->_currentSimModified = false;
			this->_simPath = "";
			this->Update_Current_Simulation();
		}
	}
}

rw::PlugPersistent* WindowSample::Release_Current_Simulation(bool show_dialog)
{
	if ((show_dialog)&&(this->_currentSimulation))
	{
		if ((this->_currentSimModified) && (this->_simulationSaved))
		{
			this->Save_Simulation_Dialog();
		}
	}
	rw::PlugPersistent* s = 0;
	s = this->_currentSimulation;
	this->_currentSimulated = false;
	this->_currentSimulation = 0;
	this->_currentSimModified = false;
	this->_simulationSaved = false;
	if (this->_formation)
	{
		rw::BinaryImage img = this->_formation->Plug_Texture();
		delete this->_formation;
		this->_formation = 0;
	}
	this->Update_Current_Simulation();

	return(s);
}

void WindowSample::Pick_Current_Sim_And_Formation(rw::PlugPersistent** sim, rw::Plug** formation)
{
	if ((this->_currentSimulation)&&(this->_imgPtr))
	{
		(*sim) = this->_currentSimulation;
		if (this->_formation)
		{
			delete this->_formation;
		}
		this->_formation = new rw::Plug();
		this->_formation->Set_Image_Formation(*this->_imgPtr);
		this->_currentSimulation->Fill_Plug_Paremeters(*this->_formation);
		(*formation) = this->_formation;
	}
}

void WindowSample::Import_Decay(wxCommandEvent& evt)
{
	if (this->_currentSimulation)
	{
		wxString filename;
		DlgImport dlg(this);
		if (dlg.Select_File(filename))
		{
			if (dlg.ShowModal() == wxID_OK)
			{
				dlg.Fill_Sim(*this->_currentSimulation);
				wxCommandEvent e;
				this->Show_Regularizer_Dialog(e);
				this->Draw_Current_Sim();
				this->Update_Current_Simulation();
			}
		}
	}
	else
	{
		wxMessageDialog msgdlg(this, "There is no current simulation", "No simulation to load data",wxOK);
		msgdlg.ShowModal();
	}
}

void WindowSample::On_Update_X_Value(float xmin, float xmax)
{
	if (this->_currentSimulation)
	{
		scalar loss;
		math_la::math_lac::full::Vector laplace_v = this->_currentSimulation->Laplace_Bin_Vector();
		math_la::math_lac::full::Vector time_v = this->_currentSimulation->Laplace_Time_Vector();
		this->_editedLaplace = rw::ExponentialFitting::Laplace_Filter(time_v, laplace_v, xmin, xmax, loss);
		wxString legend;
		legend = "Loss: ";
		legend = legend + wxString::FromDouble(loss * 100, 1) + wxString("%");
		this->_plotterB->Set_Horizontal_Legend(legend);
		this->_plotterB->Erase_Last_Curve();
		int idx = this->_plotterB->Add_Curves(1);
		this->_plotterB->Set_Curve_Color(this->_editLaplaceColor, idx);
		for (int k = 0; k < (int)this->_editedLaplace.Size(); ++k)
		{
			this->_plotterB->Add_Curve_Point(idx, time_v(k),this->_editedLaplace(k));
		}
		this->_plotterB->Refresh();
	}
}

void WindowSample::Set_Current_Simulation_Color(const wxColor& color)
{
	if (this->_currentSimulation)
	{
		WxSim wsim(*this->_currentSimulation);
		wsim.Set_Sim_Color(color);
		wsim.Save_File(string(this->_simPath.c_str()));
		this->_parentMain->Redraw_Simulations();
	}
}

void WindowSample::Mark_Volume(int px, int py, int pz, int dx, int dy, int dz)
{
	if (this->_imgPtr)
	{
		float fpx = (float)px / (float)this->_imgPtr->Width();
		float fpy = 1.0f - (float)py / (float)this->_imgPtr->Height();
		float fpz = (float)pz / (float)this->_imgPtr->Depth();

		float fdx = (float)dx / (float)this->_imgPtr->Width();
		float fdy = (float)dy / (float)this->_imgPtr->Height();
		float fdz = (float)dz / (float)this->_imgPtr->Depth();

		this->_windowVolume->Volume_Panel().Set_Mark_Box(fpx, fpy-fdy, fpz, fpx + fdx, fpy, fpz + fdz);
		this->_windowVolume->Refresh();

	}
}
