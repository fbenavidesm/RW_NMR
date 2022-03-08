#include <ostream>
#include <fstream>
#include <algorithm>
#include <wx/progdlg.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h> 
#include <wx/stopwatch.h>
#include <wx/gauge.h>
#include <wx/choicdlg.h>
#include <amp.h>
#include "win_main.h"
#include "win_sample.h"
#include "win_image.h"
#include "rw/exponential_fitting.h"
#include "rw/relaxivity_optimizer.h"
#include "rw/profile_simulator.h"
#include "rw/sigmoid.h"
#include "rw/rev.h"
#include "wx/textdlg.h"
#include "front_end/wx_progress_adapter.h"

using std::max;
using std::min;


WindowMain::WindowMain(const wxString& title) : wxFrame(NULL, wxID_ANY, title)
{
	int s_width;
	int s_height;
	this->_ribbonSize = 96;
	this->_barSize = 16;
	wxDisplaySize(&s_width,&s_height);
	wxImage::AddHandler(new wxPNGHandler());
	wxImage::AddHandler(new wxJPEGHandler());
	wxImage::AddHandler(new wxBMPHandler());
	wxImage::AddHandler(new wxTIFFHandler());
	wxImage::AddHandler(new wxICOHandler());
	wxImage::AddHandler(new wxGIFHandler());
	wxImage::AddHandler(new wxXPMHandler());
	wxImage::AddHandler(new wxPCXHandler());

	wxMenuBar* menubar = new wxMenuBar();
	wxMenu* panel_menu = new wxMenu();
	

	this->Maximize();
	wxImage img;
	wxBitmap bmp;
	wxRibbonBar* ribbonBar = new wxRibbonBar(this, -1, wxDefaultPosition, wxSize(100, 68), wxRIBBON_BAR_FLOW_HORIZONTAL
		| wxRIBBON_BAR_SHOW_PAGE_LABELS
		| wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS | wxRIBBON_BAR_SHOW_TOGGLE_BUTTON 
		| wxRIBBON_BAR_SHOW_PANEL_MINIMISE_BUTTONS | wxRIBBON_BAR_SHOW_HELP_BUTTON);
	//wxRibbonArtProvider* art = new wxRibbonAUIArtProvider();
	//art->SetColor();
	//art->SetColourScheme(wxColor(245, 245, 245), wxColor(240, 240, 255), wxColor(235, 235, 235));
	//ribbonBar->SetArtProvider(art);
	int ribbonImgSize = 32;

	wxRibbonPage* ribbonPage = new wxRibbonPage(ribbonBar, wxID_ANY, wxT("Display panels"), wxNullBitmap);
	wxRibbonPanel* ribbonPanel = new wxRibbonPanel(ribbonPage, wxID_ANY, wxT("NMR panels"), wxNullBitmap,
		wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);

	wxRibbonButtonBar* btnBar = new wxRibbonButtonBar(ribbonPanel);
	this->_btnPanelBar = btnBar;

	img = wxImage(wxT("icons//rock_lab.png"), wxBITMAP_TYPE_PNG);
	img.Rescale(ribbonImgSize, ribbonImgSize, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(img);
	btnBar->AddToggleButton(wxID_PREFERENCES, wxT("Sample panel"), bmp, "Show sample panel");
	btnBar->ToggleButton(wxID_PREFERENCES, true);
	panel_menu->Append(wxID_PREFERENCES, "Sample panel")->SetBitmap(bmp);
	
	img = wxImage(wxT("icons//rock3d.png"), wxBITMAP_TYPE_PNG);
	img.Rescale(ribbonImgSize, ribbonImgSize, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(img);
	btnBar->AddToggleButton(wxID_IMAGE_LIST, wxT("Image panel"), bmp, "Show image processing panel");
	btnBar->ToggleButton(wxID_IMAGE_LIST, true);
	panel_menu->Append(wxID_IMAGE_LIST, "Image panel")->SetBitmap(bmp);

	img = wxImage(wxT("icons//hd.png"), wxBITMAP_TYPE_PNG);
	img.Rescale(ribbonImgSize, ribbonImgSize, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(img);
	btnBar->AddToggleButton(wxID_FILECTRL, wxT("Simulation list"), bmp, "Show simulation list panel");
	btnBar->ToggleButton(wxID_FILECTRL, false);
	panel_menu->Append(wxID_FILECTRL, "Simulation list panel")->SetBitmap(bmp);

	img = wxImage(wxT("icons//genetic.png"), wxBITMAP_TYPE_PNG);
	img.Rescale(ribbonImgSize, ribbonImgSize, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(img);
	btnBar->AddToggleButton(wxID_EXEC_GEN, wxT("Genetic algorithm"), bmp, "Use genetic algorithm to recover surface relaxivity");
	btnBar->ToggleButton(wxID_EXEC_GEN, false);
	panel_menu->Append(wxID_EXEC_GEN, "Genetic algorithm panel")->SetBitmap(bmp);

	img = wxImage(wxT("icons//rev.png"), wxBITMAP_TYPE_PNG);
	img.Rescale(ribbonImgSize, ribbonImgSize, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(img);
	btnBar->AddToggleButton(wxID_REV, wxT("Representative volume"), bmp, "Representative elementary volume");
	btnBar->ToggleButton(wxID_REV, false);
	panel_menu->Append(wxID_REV, "Representative volume panel")->SetBitmap(bmp);
	menubar->Append(panel_menu, "NMR Panels");
	btnBar->Bind(wxEVT_RIBBONBUTTONBAR_CLICKED, &WindowMain::Show_Image_Panel, this, wxID_IMAGE_LIST);
	btnBar->Bind(wxEVT_RIBBONBUTTONBAR_CLICKED, &WindowMain::Show_Lab_Panel, this, wxID_PREFERENCES);
	btnBar->Bind(wxEVT_RIBBONBUTTONBAR_CLICKED, &WindowMain::Show_Sim_Panel, this, wxID_FILECTRL);
	btnBar->Bind(wxEVT_RIBBONBUTTONBAR_CLICKED, &WindowMain::Show_Genetic_Panel, this, wxID_EXEC_GEN);
	btnBar->Bind(wxEVT_RIBBONBUTTONBAR_CLICKED, &WindowMain::Show_Rev_Panel, this, wxID_REV);
	panel_menu->Bind(wxEVT_MENU, &WindowMain::Show_Image_Panel, this, wxID_IMAGE_LIST);
	panel_menu->Bind(wxEVT_MENU, &WindowMain::Show_Lab_Panel, this, wxID_PREFERENCES);
	panel_menu->Bind(wxEVT_MENU, &WindowMain::Show_Sim_Panel, this, wxID_FILECTRL);
	panel_menu->Bind(wxEVT_MENU, &WindowMain::Show_Genetic_Panel, this, wxID_EXEC_GEN);
	panel_menu->Bind(wxEVT_MENU, &WindowMain::Show_Rev_Panel, this, wxID_REV);
	this->_mgr = new wxAuiManager(this, wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_TRANSPARENT_HINT);	
	this->_mgr->AddPane(ribbonBar, 
		wxAuiPaneInfo().BestSize(s_width,this->_ribbonSize).ToolbarPane().Top().Movable(false).Dockable(false).Floatable(false).Gripper(false));
	int diff = this->_ribbonSize + this->_barSize;
	this->_winImage = new WindowImage(this, this);
	this->_mgr->AddPane(this->_winImage, wxAuiPaneInfo().Right().BestSize(s_width/3, s_height-diff).CaptionVisible(false).Name("IMG"));
	this->_winSample = new WindowSample(this);
	this->_winSample->Add_Button_Tools(ribbonPage,menubar);
	this->_winImage->Add_Button_Tools(ribbonPage,menubar);
	this->_mgr->AddPane(this->_winSample, wxAuiPaneInfo().Center().BestSize(2*s_width/3, s_height-diff).CaptionVisible(false).Name("SMP"));
	this->_winSims = new WindowSims(this);
	this->_mgr->AddPane(this->_winSims, wxAuiPaneInfo().Right().BestSize(s_width/3, s_height-diff).Caption("Database").Name("SL").CaptionVisible(false));
	this->_winSims->Add_Button_Tools(ribbonPage,menubar);
	this->_winGenetic = new WindowGenetic(this);
	this->_winGenetic->Add_Button_Tools(ribbonPage, menubar);
	this->_mgr->AddPane(this->_winGenetic, wxAuiPaneInfo().Right().BestSize(s_width / 3, s_height-diff).Caption("Genetic RHO recovery").Name("GL").CaptionVisible(false));
	this->_winRev = new WindowRev(this);
	this->_mgr->AddPane(this->_winRev, wxAuiPaneInfo().Right().BestSize(s_width / 3, s_height-diff).Caption("Representative elementary volume").Name("RV").CaptionVisible(false));
	this->_winRev->Add_Button_Tools(ribbonPage, menubar);

	wxGauge *gauge = new wxGauge(this,0,100);
	this->_progressBar = gauge;
	this->_mgr->AddPane(gauge, wxAuiPaneInfo().Bottom().MinSize(s_width, this->_barSize).CloseButton(false).PaneBorder(false).Fixed().CaptionVisible(false));	
	this->_mgr->Bind(wxEVT_AUI_PANE_CLOSE, &WindowMain::On_Close_Pane, this);

	this->Current_Simulation_Update(false);
	ribbonBar->Realize();
	this->_mgr->SetDockSizeConstraint(0.3, 0.36);
	this->Hide_Panels();
	this->_mgr->GetPane(this->_winSims).Show();
	this->_mgr->Update();
	this->_winSims->Update_Layout();
	this->Hide_Panels();
	this->_mgr->GetPane(this->_winGenetic).Show();
	this->_mgr->Update();
	this->_winGenetic->Update_Layout();
	this->Hide_Panels();
	this->_mgr->GetPane(this->_winRev).Show();
	this->_mgr->Update();
	this->_winRev->Update_Layout();
	this->Hide_Panels();
	this->_mgr->GetPane(this->_winImage).Show();
	this->_mgr->Update();
	this->_winImage->Update_Layout();
	this->_winSample->Update_Layout();

	this->SetMenuBar(menubar);
	this->_mgr->Update();
	this->Update();
}

void WindowMain::Hide_Panels()
{
	this->_mgr->GetPane(this->_winImage).Hide();
	this->_mgr->GetPane(this->_winSims).Hide();
	this->_mgr->GetPane(this->_winGenetic).Hide();
	this->_mgr->GetPane(this->_winRev).Hide();
}

void WindowMain::Set_Image_Pointer(const rw::BinaryImage* imgptr)
{
	this->_imgPtr = imgptr;
}

void WindowMain::On_Close_Pane(wxAuiManagerEvent& evt)
{
	wxAuiPaneInfo i = *evt.GetPane();
	if (i.name == "IMG")
	{
		this->_btnPanelBar->ToggleButton(wxID_IMAGE_LIST,false);
	}
	if (i.name == "SMP")
	{
		this->_btnPanelBar->ToggleButton(wxID_PREFERENCES, false);
	}
	if (i.name == "SL")
	{
		this->_btnPanelBar->ToggleButton(wxID_FILECTRL,false);
	}	
}

void WindowMain::Show_Image_Panel(wxCommandEvent& evt)
{
	wxAuiPaneInfo i = this->_mgr->GetPane(this->_winImage);
	if (i.IsShown())
	{
		this->_mgr->GetPane(this->_winImage).Hide();
		this->_winImage->Hide_Button_Panel();
		this->_btnPanelBar->ToggleButton(wxID_IMAGE_LIST, false);
	}
	else
	{
		this->_mgr->GetPane(this->_winImage).Show();
		this->_winImage->Show_Button_Panel();
		this->_btnPanelBar->ToggleButton(wxID_IMAGE_LIST,true);		
	}
	this->_mgr->Update();
}

void WindowMain::Show_Rev_Panel(wxCommandEvent& evt)
{
	wxAuiPaneInfo i = this->_mgr->GetPane(this->_winRev);
	if (i.IsShown())
	{
		this->_mgr->GetPane(this->_winRev).Hide();
	}
	else
	{
		this->_mgr->GetPane(this->_winRev).Show();
	}
	this->_mgr->Update();
}

void WindowMain::Show_Genetic_Panel(wxCommandEvent& evt)
{
	wxString dir = this->_winSims->Working_Directory();
	dir = dir + "\\REPORTS\\GENETIC\\";
	this->_winGenetic->Set_Result_Directory(dir);
	wxAuiPaneInfo i = this->_mgr->GetPane(this->_winGenetic);
	if (i.IsShown())
	{
		this->_mgr->GetPane(this->_winGenetic).Hide();		
	}
	else
	{
		this->_mgr->GetPane(this->_winGenetic).Show();
	}
	this->_mgr->Update();
}

void WindowMain::Show_Lab_Panel(wxCommandEvent& evt)
{
	wxAuiPaneInfo i = this->_mgr->GetPane(this->_winSample);
	if (i.IsShown())
	{
		this->_mgr->GetPane(this->_winSample).Hide();
		this->_btnPanelBar->ToggleButton(wxID_PREFERENCES, false);
	}
	else
	{
		this->_mgr->GetPane(this->_winSample).Show();
		this->_btnPanelBar->ToggleButton(wxID_PREFERENCES, true);
	}
	this->_mgr->Update();
}

void WindowMain::Show_Sim_Panel(wxCommandEvent& evt)
{
	wxAuiPaneInfo i = this->_mgr->GetPane(this->_winSims);
	if (i.IsShown())
	{
		this->_mgr->GetPane(this->_winSims).Hide();
		this->_btnPanelBar->ToggleButton(wxID_FILECTRL, false);
	}
	else
	{
		this->_mgr->GetPane(this->_winSims).Show();
		this->_btnPanelBar->ToggleButton(wxID_FILECTRL, true);
	}
	this->_mgr->Update();
}

void WindowMain::Set_Progress_Dialog(wxProgressDialog* pgdlg)
{
	this->_dlg = pgdlg;
}

void WindowMain::Release_Progress_Dialog()
{
	this->_dlg = 0;
}


void WindowMain::Build_3D_Model(wxCommandEvent& evt)
{
	const rw::BinaryImage& img = this->_winImage->Binary_Image();
	if (img.Depth() > 0)
	{
		bool create_dialog = false;
		if (!this->_dlg)
		{
			this->_dlg = new wxGenericProgressDialog("Assembling 3D image", "Assembling layers", img.Depth());
			create_dialog = true;
		}
		this->_winSample->Build_3D_Model(img, this->_dlg);
		this->_winRev->Set_Image_Pointer(&img);
		if (create_dialog)
		{
			this->_dlg->Close();
			delete this->_dlg;
			this->_dlg = 0;
		}
	}
	else
	{
		wxMessageDialog dlg(this, "No 3D or 2D texture has been associated to simulation", "No Image", wxOK | wxICON_EXCLAMATION);
		dlg.ShowModal();

	}
}

bool WindowMain::Save_Simulation(wxCommandEvent& evt)
{
	bool saved = false;
	wxString dir = this->_winSims->Working_Directory();
	if (this->_winSample->Has_Current_Simulation())
	{
		wxDateTime wdt = wxDateTime::Now();
		int vl = this->_winSample->Current_Simulation().Voxel_Length();
		int vlu = this->_winSample->Current_Simulation().Voxel_Length_Units();
		int te = 1;
		if (this->_winSample->Current_Simulation().SimulationParams().T1_Relaxation())
		{
			te = 0;
		}			
		wxArrayString texp;
		texp.Add("T1");
		texp.Add("T2");
		wxArrayString lu;
		lu.Add("nm");
		lu.Add("um");
		lu.Add("mm");
		wxString res_text; 
		res_text << vl;
		res_text = res_text+lu[vlu];
		wxString sug_name = wxString("SIM_")+texp[te]+
			wxString("__")+wdt.Format("D_%d_%b_%Y__H%H_%M__IMG_")+this->_imgName
			+wxString("__R_")+res_text;
		dir = dir + "\\SIMS\\";
		wxTextValidator vldtr(wxFILTER_EMPTY | wxFILTER_EXCLUDE_CHAR_LIST);
		vldtr.SetCharExcludes("\\.,;:\"'+~!@#$%^&*()'");
		wxTextEntryDialog wtxt(this, "Simulation file name", "Select a simulation filename");		
		wtxt.SetTextValidator(vldtr);
		wtxt.SetValue(sug_name);
		wtxt.ShowModal();
		if (wtxt.GetReturnCode() == wxID_OK)
		{
			dir = dir + wtxt.GetValue() + ".sim";
			if (!wxFileExists(dir))
			{
				this->_winSample->Save_Sim(dir,wdt);
				this->_simName = wtxt.GetValue();
				saved = true;
			}
			else
			{
				wxMessageDialog msgdlg(this,
					"You can select another name for the simulation file.\nSelect YES to overwrite,\nNO to define another name, \nCANCEL to exit",
					"File already exists!",
					wxYES_NO | wxCANCEL | wxICON_QUESTION);
				int result = msgdlg.ShowModal();
				if (result == wxID_YES)
				{
					this->_simName = wtxt.GetValue();
					this->_winSample->Save_Sim(dir,wdt);
					saved = true;
				}
				if (result == wxID_NO)
				{
					wxCommandEvent nevt;
					this->Save_Simulation(nevt);
				}
			}
		}
	}
	if (saved)
	{
		this->_winSims->Update_File_Trees();
		Mem_Sim ms;
		ms.sim = this->_winSample->Release_Current_Simulation(false);	
		map<string, Mem_Sim>::iterator ii = this->_loadedSimulations.begin();
		while (ii != this->_loadedSimulations.end())
		{
			this->_winSims->Mark_Sim(ii->first, false);
			ii->second.curve_id = -1;
			++ii;
		}
		this->_loadedSimulations[string(this->_simName.c_str())] = ms;
		this->_winSims->Mark_Sim(string(this->_simName.c_str()));
		this->_winSample->Erase_Plots();
		this->_winSample->Add_Sim_Plot(*ms.sim, ms.curve_id);
		this->Update_Selection_Sims();
		this->_winSims->Select_Sim(this->_simName);
		this->_winSims->Update_Selected_Sim(*ms.sim);
		wxMessageDialog msgdlg(this, dir, "Simulation saved", wxOK | wxICON_INFORMATION);
		msgdlg.ShowModal();
	}
	return(saved);
}

wxString WindowMain::Image_Name() const
{
	return(this->_imgName);
}

void WindowMain::Save_Image(wxCommandEvent& evt)
{
	bool saved = false;
	wxString dir = this->_winSims->Working_Directory();
	if (this->_winImage->Binary_Image().Length() > 0)
	{
		dir = dir + "\\IMAGES\\";
		wxTextValidator vldtr(wxFILTER_EMPTY | wxFILTER_EXCLUDE_CHAR_LIST);
		vldtr.SetCharExcludes("\\.,;:\"'+~!@#$%^&*()'");
		wxTextEntryDialog wtxt(this, "Image file name", "Select an image filename");
		wtxt.SetValue(evt.GetString());
		wtxt.SetTextValidator(vldtr);
		if (wtxt.ShowModal() == wxID_OK)
		{
			dir = dir + wtxt.GetValue() + ".bin";
			if (!wxFileExists(dir))
			{				
				wxProgressDialog pgdlg("Saving image .... ", wxString("Saving ") + dir);
				pgdlg.SetRange(3);
				//pgdlg.ShowModal();
				pgdlg.Update(1, "Saving ..... ");
				this->_winImage->Binary_Image().Save_File(std::string(dir.c_str()));
				this->_imgName = wtxt.GetValue();
				saved = true;
			}
			else
			{
				wxMessageDialog msgdlg(this, 
					"You can select another name for the image file. \nSelect YES to overwrite,\nNO to define another name, \nCANCEL to exit",
					"File already exists!",
					wxYES_NO | wxCANCEL | wxICON_QUESTION);
				int result = msgdlg.ShowModal();
				if (result == wxID_YES)
				{
					this->_imgName = wtxt.GetValue();
					wxGenericProgressDialog pgdlg("Saving image .... ", wxString("Saving ") + dir);
					pgdlg.Show();
					pgdlg.Pulse(wxString("Saving ") + dir);
					this->_winImage->Binary_Image().Save_File(std::string(dir.c_str()));
					saved = true;
				}
				if (result == wxID_NO)
				{
					wxCommandEvent nevt;
					this->Save_Image(nevt);
				}
			}
		}
	}
	if (saved)
	{
		this->_winSims->Update_File_Trees();
		wxMessageDialog msgdlg(this, dir,"Image saved", wxOK | wxICON_INFORMATION);
		msgdlg.ShowModal();
	}
}


void WindowMain::Step_Progress_Bar(int perc)
{
	this->_progressBar->SetValue(perc);
	this->_progressBar->Refresh();
}

void WindowMain::Load_Simulation(const string& sim_name) 
{
	wxGenericProgressDialog pgdlg("Loading simulation","Loading selected simulation in memory");		
	string name = sim_name;
	wxString path = this->_winSims->Working_Directory();
	path = path + "\\SIMS\\" + wxString(name.c_str()) + ".sim";
	Mem_Sim ms;
	ms.curve_id = -1;
	rw::PlugPersistent* sim = new rw::PlugPersistent();
	ms.sim = sim;
	pgdlg.Pulse("Loading simulation: "+wxString(name.c_str()));
	sim->Load_From_File(string(path.c_str()));
	map<string, Mem_Sim>::iterator isim = this->_loadedSimulations.find(name);
	if (isim == this->_loadedSimulations.end())
	{
		this->_loadedSimulations[name] = ms;
	}
	else
	{
		delete isim->second.sim;
		this->_loadedSimulations.erase(isim);		
		this->_loadedSimulations[name] = ms;
	}
	this->Update_Selection_Sims();
	pgdlg.Close();
}

void WindowMain::Unload_Simulation(const string& sim_name)
{
	rw::PlugPersistent* td = 0;
	wxGenericProgressDialog pgdlg("Erasing simulation from memory", "Loading selected simulation in memory");
	string name = sim_name;
	wxString path = this->_winSims->Working_Directory();
	path = path + "\\SIMS\\" + wxString(name.c_str()) + ".sim";
	pgdlg.Pulse("Erasing simulation: " + wxString(name.c_str()));
	map<string, Mem_Sim>::iterator isim = this->_loadedSimulations.find(name);
	if (isim != this->_loadedSimulations.end())
	{
		if (isim->second.curve_id < 0)
		{
			td = isim->second.sim;
			this->_loadedSimulations.erase(isim->first);
		}
	}
	pgdlg.Close();
	Sleep(100);
	if (td)
	{
		this->_winSample->Check_Simulation_Pointer_To_Delete(td);
		delete td;
	}
	bool f = this->_winSample->Has_Current_Simulation();
	this->Current_Simulation_Update(f);
	this->_winSample->Draw_Current_Sim();
	this->Update_Selection_Sims();
}

bool WindowMain::Add_Plot_Simulation(const string& name)
{
	bool r = false;
	wxGenericProgressDialog pgdlg("Adding plot", "");
	this->_winSample->Freeze();
	map<string, Mem_Sim>::iterator ii = this->_loadedSimulations.find(name);
	pgdlg.Pulse("Adding simulation plot of: " +wxString(name.c_str()));
	if (ii != this->_loadedSimulations.end())
	{
		Mem_Sim& ms = ii->second;
		this->_winSample->Add_Sim_Plot(*ms.sim, ms.curve_id);
		r = true;
	}
	this->_winSample->Thaw();
	pgdlg.Close();
	return(r);
}

void WindowMain::Current_Simulation_Update(bool simulated)
{
	if (simulated)
	{
		wxImage img;
		wxBitmap bmp;
		img.LoadFile("icons/mem.png");
		bmp = wxBitmap(img);
		this->_mgr->GetPane(this->_winSample).Caption("Laboratory Sample: SIMULATED").Icon(bmp);
	}
	else
	{
		wxImage img;
		wxBitmap bmp;
		if (this->_winSample->Has_Current_Simulation())
		{
			img.LoadFile("icons/hdicon.png");
			bmp = wxBitmap(img);
			wxString fn = this->_winSample->Current_Sim_File_Name();
			this->_mgr->GetPane(this->_winSample).Caption("Laboratory Sample: HD "+fn).Icon(bmp);
		}
		else
		{
			img.LoadFile("icons/empty.png");
			bmp = wxBitmap(img);
			this->_mgr->GetPane(this->_winSample).Caption("Laboratory Sample: EMPTY").Icon(bmp);
		}
	}
	this->_mgr->Update();
}

bool WindowMain::Erase_Plot_Simulation(const string& name)
{
	wxGenericProgressDialog pgdlg("Erasing simulation", "Erasing selected simulation plot");
	bool r = false;
	this->_winSample->Freeze();
	map<string, Mem_Sim>::iterator ii = this->_loadedSimulations.find(name);
	if (ii != this->_loadedSimulations.end())
	{
		pgdlg.Pulse("Erasing plot of: " + wxString(name.c_str()));
		this->_winSample->Check_Simulation_Pointer_To_Delete(ii->second.sim);
		this->_winSample->Erase_Plots();
		ii->second.curve_id = -1;
	}
	this->_winSample->Thaw();
	this->_winSample->Draw_Current_Sim();
	return(r);
}

void WindowMain::Load_Binary_Image(const wxString& filename)
{
	wxGenericProgressDialog pgdlg(wxString("Loading image"), "Loading image");
	this->_dlg = &pgdlg;
	wxFileName fn(filename);
	if (fn.Exists())
	{
		this->_imgName = fn.GetName();
		this->_winImage->Load_Binary_Image(filename,&pgdlg);
		this->_winSample->Set_Sample_Name(wxString("Rock sample: ") + this->_imgName);
	}
	this->Refresh();
	this->SetFocus();
	this->_dlg = 0;
}

void WindowMain::Update_Selection_Sims()
{
	wxArrayString sim_names;
	map<string, Mem_Sim>::iterator ii = this->_loadedSimulations.begin();
	while (ii != this->_loadedSimulations.end())
	{
		wxString str = wxString(ii->first.c_str());
		sim_names.Add(str);
		++ii;
	}
	this->_winSims->Fill_Loaded_Sims(sim_names);
}

rw::PlugPersistent* WindowMain::Get_Sim_By_Name(const wxString& name)
{
	rw::PlugPersistent* rsim = 0;
	string n = string(name.c_str());
	map<string,Mem_Sim>::iterator ii = this->_loadedSimulations.find(n);
	if (ii != this->_loadedSimulations.end())
	{
		Mem_Sim msim = ii->second;
		rsim = msim.sim;
	}
	return(rsim);
}

void WindowMain::Update_Selected_Sim(const rw::PlugPersistent& sim)
{
	this->_winSample->Update_Sim_Parameters(sim);
	this->_winSims->Update_Selected_Sim(sim);	
}

void WindowMain::Set_Current_Sim(rw::PlugPersistent* sim, const wxString& path)
{
	this->_winSample->Set_Current_Simulation(sim,path);
}

void WindowMain::Redraw_Simulations()
{
	map<string, Mem_Sim>::iterator itr = this->_loadedSimulations.begin();
	while (itr != this->_loadedSimulations.end())
	{
		if (itr->second.curve_id >= 0)
		{
			const rw::PlugPersistent* sim = itr->second.sim;
			int id;
			this->_winSample->Add_Sim_Plot(*sim, id);
			itr->second.curve_id = id;
		}
		++itr;
	}
}

void WindowMain::Report() const
{
	if (this->_loadedSimulations.size() > 0)
	{
		wxProgressDialog pgdlg("Creating CSV Report", "", this->_loadedSimulations.size() * 3);
		pgdlg.Show();

		wxString wd = this->_winSims->Working_Directory();
		wd = wd + wxString("\\REPORTS\\");

		int k = 0;
		map<string, Mem_Sim>::const_iterator itr = this->_loadedSimulations.cbegin();
		wxString fd;
		while (itr != this->_loadedSimulations.end())
		{
			wxString name = wxString(itr->first.c_str());
			pgdlg.Update(3 * k, "Creating decay file for:\n" + name);
			fd = wd + name + wxString("_DECAY.csv");
			itr->second.sim->Save_Decay_CSV_File(string(fd.c_str()));
			pgdlg.Update(3 * k + 1, "Creating laplace file for:\n" + name);
			fd = wd + name + wxString("_LAPLACE.csv");
			itr->second.sim->Save_Laplace_CSV_File(string(fd.c_str()));
			pgdlg.Update(3 * k + 2, "Creating collision distribution file for:\n" + name);
			fd = wd + name + wxString("_XIRATE.csv");
			itr->second.sim->Save_Collision_Rate_Distribution_CSV_File(string(fd.c_str()));
			fd = wd + name + wxString("_XIRATE_DISTRIBUTION.csv");
			itr->second.sim->Save_Collision_Rate_CSV_Weight_File(string(fd.c_str()));
			++itr;
			++k;
		}
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this, wxString("No loaded simulations to report"), wxString("No simulations"), wxOK);
		mgdlg.ShowModal();
	}
}

void WindowMain::Save_Sphere_PSD(const rw::BinaryImage& img) const
{
	wxString fname = this->_imgName + "_SPHERE_DISTRIBUTION.csv";
	fname = this->_winSims->Working_Directory() + wxString("\\REPORTS\\") +  fname;
	img.Cluster_PSD_File(string(fname.c_str()),0.5,100);	
}

void WindowMain::Save_Distribution(const vector<scalar2>& dstpsd, const vector<scalar2>& dstvd)
{
	wxString fname;
	fname = this->_simName + "_PORE_SIZE_DISTRIBUTION.csv";
	fname = this->_winSims->Working_Directory() + wxString("\\REPORTS\\") + fname;
	std::ofstream file;
	file.open(std::string(fname.c_str()));
	file << "PORE RADIUS, WEIGHT \n";
	for (int k = 0; k < dstpsd.size(); ++k)
	{
		file << dstpsd[k].x << "," << dstpsd[k].y << "\n";
	}
	file.close();
	fname = this->_simName + "_RELAXIVITY_VOLUME_DISTRIBUTION.csv";
	fname = this->_winSims->Working_Directory() + wxString("\\REPORTS\\") + fname;
	file.open(std::string(fname.c_str()));
	file << "RELAXIVITY, WEIGHT \n";
	for (int k = 0; k < dstvd.size(); ++k)
	{
		file << dstvd[k].x << "," << dstvd[k].y << "\n";
	}
	file.close();
}

void WindowMain::Create_Cluster_PSD(rw::BinaryImage& img) const
{
	wxString fname = this->_imgName + "CLUSTER_PSD_DISTRIBUTION.csv";
	fname = this->_winSims->Working_Directory() + wxString("\\REPORTS\\") + fname;
	wxProgressDialog pgdlg("Segmenting pore space ...", "Segmenting pore space ...", 100);
	pgdlg.Show();		
	WxProgressAdapter adapter(&pgdlg);
	img.Cluster_Pores(&adapter);
	img.Cluster_PSD_File(string(fname.c_str()),0.5,50);
}

void WindowMain::Prepare_Genetic_Inversion()
{
	rw::PlugPersistent* sim = 0;
	rw::Plug* formation = 0;
	this->_winSample->Pick_Current_Sim_And_Formation(&sim, &formation);
	if ((sim) && (formation))
	{
		this->_winGenetic->Set_Simulation(sim, formation);
		this->_winGenetic->Set_Name(this->_simName);
	}
	else
	{
		wxMessageDialog msgdlg(this,
			"There is no simulation to optimize. \nCheck if a rock formation and a simulation has been loaded", 
			"Simulation has not been created",wxICON_ERROR);
		msgdlg.ShowModal();
	}
}

void WindowMain::Remark_Sims()
{
	map<wxString, bool> remarks;
	map<string, Mem_Sim>::iterator ii = this->_loadedSimulations.begin();
	while (ii != this->_loadedSimulations.end())
	{
		wxString nn = wxString(ii->first.c_str());
		bool r = (ii->second.curve_id >= 0);
		remarks[nn] = r;
		++ii;
	}
	this->_winSims->Remark_Sims(remarks);
}

void WindowMain::Update_Sim_Color(const wxColor& color)
{
	this->_winSample->Erase_Plots();
	this->_winSample->Set_Current_Simulation_Color(color);
	this->_winSims->Update_Data();
	this->Redraw_Simulations();
	this->Remark_Sims();
}

void WindowMain::Mark_Sub_Volume(int px, int py, int pz, int dx, int dy, int dz)
{
	this->_winSample->Mark_Volume(px, py, pz, dx, dy, dz);
}

void WindowMain::Set_Parent_Application(wxApp* parent)
{
	this->_parentApplication = parent;
}

WindowMain::~WindowMain()
{
	this->_mgr->UnInit();	
}




