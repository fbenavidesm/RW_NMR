#include "wx/stdpaths.h"
#include "wx/dir.h"
#include "win_sims.h"
#include "rw/binary_image/binary_image.h"
#include "rw/persistence/plug_persistent.h"
#include "win_main.h"
#include "front_end/wx_image_adapter.h"
#include "front_end/persistent_ui/persistent_ui.h"
#include "front_end/wx_rgbcolor.h"

WindowSims::WindowSims(wxWindow* parent) :
	wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "Rock sample")
{
	this->Freeze();
	this->_sortData = true;
	this->_windowMain = (WindowMain*)parent;
	this->_mgr = new wxAuiManager(this);
	wxStandardPaths& std_paths = wxStandardPaths::Get();
	wxString docs_dir = std_paths.GetDocumentsDir();
	docs_dir = docs_dir + "\\RWNMR_DB\\";
	if (!wxDir::Exists(docs_dir))
	{
		wxDir::Make(docs_dir);
	}
	wxString sims_dir = docs_dir + "\\SIMS\\";
	if (!wxDir::Exists(sims_dir))
	{
		wxDir::Make(sims_dir);
	}
	wxString imgs_dir = docs_dir + "\\IMAGES\\";
	if (!wxDir::Exists(imgs_dir))
	{
		wxDir::Make(imgs_dir);
	}
	wxString imgs_rep = docs_dir + "\\REPORTS\\";
	if (!wxDir::Exists(imgs_rep))
	{
		wxDir::Make(imgs_rep);
	}
	wxString gen_dir = docs_dir + "\\REPORTS\\GENETIC\\";
	if (!wxDir::Exists(gen_dir))
	{
		wxDir::Make(gen_dir);
	}

	wxPropertyGrid* pg = new wxPropertyGrid(
		this,
		0,
		wxDefaultPosition,
		wxDefaultSize,
		wxPG_SPLITTER_AUTO_CENTER |
		wxPG_DEFAULT_STYLE);	
	this->_propertyGrid = pg;
	pg->Append(new wxPropertyCategory("Database files"));
	wxPGProperty* pp = 0;
	pp = pg->Append(new wxDirProperty("Working directory", "WG", docs_dir));
	wxPGProperty* pcs = pg->Append(new wxEnumProperty("Current simulation","CS"));
	pg->AppendIn(pcs, new wxColourProperty("Simulation color", "SCol"));
	pp = pg->AppendIn(pcs, new wxStringProperty("Simulation date","SDate"));	
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);


	this->_treeImgs = new wxTreeListCtrl(this, wxID_ANY,wxDefaultPosition,wxDefaultSize,wxTL_SINGLE);
	this->_treeSims = new wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,wxTL_CHECKBOX);
	this->_treeSims->Bind(wxEVT_TREELIST_ITEM_CHECKED, &WindowSims::On_Check_Sim, this);
	this->_treeSims->Bind(wxEVT_TREELIST_SELECTION_CHANGED, &WindowSims::On_Select_Sim, this);
	this->_treeImgs->Bind(wxEVT_TREELIST_ITEM_CONTEXT_MENU, &WindowSims::On_Mouse_Down_Imgs, this);
	this->Bind(wxEVT_SIZE, &WindowSims::On_Resize, this);
	this->_popMenu = new wxMenu();
	
	wxImage img;
	wxBitmap bmp;
	img.LoadFile("icons/folder.png");
	img.Rescale(32, 32);
	bmp = wxBitmap(img);
	wxMenuItem* menu_item = 0;
	menu_item = new wxMenuItem(this->_popMenu, wxID_OPEN, "Load selected image");
	menu_item->SetBitmap(bmp);
	this->_popMenu->Append(menu_item);
	this->_popMenu->Bind(wxEVT_MENU,&WindowSims::Load_Image,this,wxID_OPEN);
	this->_propertyGrid->Bind(wxEVT_PG_CHANGED,&WindowSims::Change_Grid_Value,this);
	this->_treeSims->SetBackgroundColour(wxColor(245, 245, 255));
	this->_treeImgs->SetBackgroundColour(wxColor(245, 245, 255));
	this->_mgr->SetDockSizeConstraint(0.3, 0.36);
	this->Thaw();
	int s_width;
	int s_height;
	WindowMain* wm = static_cast<WindowMain*>(parent);
	int diff = wm->Ribbon_Size() + wm->Bar_Size();
	wxDisplaySize(&s_width, &s_height);
	s_width = 2 * s_width / 3;
	s_height = s_height - diff;

	this->_mgr->AddPane(this->_propertyGrid, wxAuiPaneInfo().Bottom().Caption("Current simulation properties").BestSize(s_width / 3, 36 * s_height / 100));
	this->_mgr->AddPane(this->_treeSims, wxAuiPaneInfo().Center().Caption("Simulation files").BestSize(s_width / 3, 32 * s_height / 100));
	this->_mgr->AddPane(this->_treeImgs, wxAuiPaneInfo().Center().Caption("Image files").BestSize(s_width / 3, 32 * s_height / 100));
	this->Update_Data();
	this->_ribbonPanel = 0;
}

void WindowSims::Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar)
{
	wxMenu* menu = new wxMenu();
	wxRibbonPanel* ribbonPanel = new wxRibbonPanel(ribbonPage, wxID_ANY, wxT("Persistence tools"), wxNullBitmap,
		wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);
	this->_ribbonPanel = ribbonPanel;
	wxRibbonToolBar* btnBar = new wxRibbonToolBar(ribbonPanel);
	

	wxImage img;
	wxBitmap bmp;
	img.LoadFile("icons/sort-ascending.png");
	img.Rescale(32, 32);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_SORT_ASCENDING, bmp, "Sort by date in ascending order");
	menu->Append(wxID_SORT_ASCENDING, "Sort by date in ascending order")->SetBitmap(bmp);
	img.LoadFile("icons/sort-descending.png");
	img.Rescale(32, 32);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_SORT_DESCENDING, bmp, "Sort by date in descending order");
	menu->Append(wxID_SORT_DESCENDING, "Sort by date in descending order")->SetBitmap(bmp);
	btnBar->AddSeparator();
	menu->AppendSeparator();
	img.LoadFile("icons/CSV.png");
	img.Rescale(32, 32);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_SAVE_BLOCK, bmp, "Save CSV files for loaded simulations");
	menu->Append(wxID_SAVE_BLOCK, "Save CSV files for loaded simulations")->SetBitmap(bmp);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSims::Report, this, wxID_SAVE_BLOCK);
	menu->Bind(wxEVT_MENU, &WindowSims::Report, this, wxID_SAVE_BLOCK);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSims::Sort_Ascending, this, wxID_SORT_ASCENDING);
	menu->Bind(wxEVT_MENU, &WindowSims::Sort_Ascending, this, wxID_SORT_ASCENDING);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowSims::Sort_Descending, this, wxID_SORT_DESCENDING);
	menu->Bind(wxEVT_MENU, &WindowSims::Sort_Descending, this, wxID_SORT_DESCENDING);
	menubar->Append(menu, "Simulation tools");
}

void WindowSims::Hide_Button_Panel()
{
	if (this->_ribbonPanel)
	{
		this->_ribbonPanel->Hide();
	}
}

void WindowSims::Show_Button_Panel()
{
	if (this->_ribbonPanel)
	{
		this->_ribbonPanel->Show();
	}
}

void WindowSims::Update_Layout()
{
	this->_mgr->Update();
}


void WindowSims::On_Resize(wxSizeEvent& evt)
{
	if ((this->_treeSims->GetColumnCount() > 0)&&(this->_treeImgs->GetColumnCount() > 0))
	{
		this->_treeSims->SetColumnWidth(0, 450);
		this->_treeImgs->SetColumnWidth(0, 450);
		this->_treeSims->Refresh();
		this->_treeImgs->Refresh();
	}
}


void WindowSims::Update_Data()
{
	this->Populate_Image_Directory();
	this->Populate_Sim_Directory();
	this->Populate_Trees();
}

void WindowSims::On_Select_Sim(wxTreeListEvent& evt)
{
	wxTreeListItem root = this->_treeSims->GetRootItem();
	wxTreeListItem item = this->_treeSims->GetSelection();
	if (this->_treeSims->GetItemParent(item) == root)
	{
		wxString imgAssoc = this->_treeSims->GetItemText(item, 3);
		wxTreeListItem& rimgitem = this->_treeImgs->GetRootItem();
		wxTreeListItem& cimgitem = this->_treeImgs->GetFirstChild(rimgitem);
		bool found = false;
		while ((cimgitem.IsOk())&&(!found))
		{
			wxString cn = this->_treeImgs->GetItemText(cimgitem, 0);
			if (cn == imgAssoc)
			{
				this->_treeImgs->Select(cimgitem);
				found = true;
			}
			cimgitem = this->_treeImgs->GetNextSibling(cimgitem);
		}
	}
	this->_treeImgs->Refresh();
}

void WindowSims::Report(wxCommandEvent& evt)
{
	this->_windowMain->Report();
}

void WindowSims::Change_Grid_Value(wxPropertyGridEvent& evt)
{
	wxPGProperty* pp = evt.GetProperty();
	if (pp->GetName() == wxString("CS"))
	{
		wxString name = pp->GetValueAsString();
		rw::PlugPersistent* sim = this->_windowMain->Get_Sim_By_Name(name);
		this->_windowMain->Update_Selected_Sim(*sim);
		wxString path = this->Working_Directory() + wxString("\\SIMS\\") + name + wxString(".sim");
		this->_windowMain->Set_Current_Sim(sim, path);
	}
	if (pp->GetName() == wxString("CS.SCol"))
	{
		wxColourProperty* ppcolor = (wxColourProperty*)(pp);
		wxColor color;
		color << ppcolor->GetValue();
		this->_windowMain->Update_Sim_Color(color);
	}
}

void WindowSims::Select_Sim(const wxString& sim_name)
{
	wxPGProperty* pp = this->_propertyGrid->GetPropertyByName("CS");
	wxEnumProperty* ep = (wxEnumProperty*)pp;
	wxArrayString choices = ep->GetChoices().GetLabels();
	int sel_indx = -1;
	for (int i = 0; i < choices.size(); ++i)
	{
		if (string(choices[i].c_str()) == string(choices[i].c_str()))
		{
			sel_indx = i;
		}
	}
	if (sel_indx >= 0)
	{
		ep->SetValue(sel_indx);
	}	
}


void WindowSims::Fill_Loaded_Sims(const wxArrayString& nsims)
{
	wxPGProperty* pp = this->_propertyGrid->GetPropertyByName("CS");
	wxEnumProperty* ep = (wxEnumProperty*)pp;
	wxString selected = ep->GetValueAsString();
	int sel_indx = 0;
	wxPGChoices choices;
	for (int i = 0; i < nsims.size(); ++i)
	{
		if (string(nsims[i].c_str()) == string(selected.c_str()))
		{
			sel_indx = i;
		}
		choices.Add(nsims[i]);
	}
	ep->SetChoices(choices);
	ep->SetValue(sel_indx);
	rw::PlugPersistent* sim = this->_windowMain->Get_Sim_By_Name(ep->GetValueAsString());
	if (sim)
	{
		wxString path = this->Working_Directory() + wxString("\\SIMS\\") + ep->GetValueAsString() + wxString(".sim");
		this->_windowMain->Update_Selected_Sim(*sim);
		this->_windowMain->Set_Current_Sim(sim, path);
	}
}

void WindowSims::Load_Image(wxCommandEvent& evt)
{
	wxTreeListItem& item = this->_treeImgs->GetSelection();
	wxString imgname = this->_treeImgs->GetItemText(item, 0);
	imgname = this->Working_Directory() + wxString("\\IMAGES\\") + imgname + wxString(".bin");
	this->_windowMain->Load_Binary_Image(imgname);

}

void WindowSims::On_Mouse_Down_Imgs(wxTreeListEvent& evt)
{
	wxTreeListItem& item = this->_treeImgs->GetSelection();	
	if (this->_treeImgs->GetItemParent(item) == this->_treeImgs->GetRootItem())
	{
		this->_treeImgs->PopupMenu(this->_popMenu);
	}
}

void WindowSims::On_Check_Sim(wxTreeListEvent& evt)
{
	wxTreeListItem& checked_item = evt.GetItem();
	if (this->_treeSims->GetItemParent(checked_item) == this->_treeSims->GetRootItem())
	{
		wxCheckBoxState st = this->_treeSims->GetCheckedState(checked_item);
		wxString name = this->_treeSims->GetItemText(checked_item, 0);
		if (st == wxCHK_CHECKED)
		{
			this->_treeSims->AppendItem(checked_item, "Plotted");	
			this->_treeSims->Expand(checked_item);
			this->_windowMain->Load_Simulation(string(name.c_str()));
			this->_treeSims->SetItemText(checked_item,4,"True");
		}
		else
		{
			wxTreeListItem& di = this->_treeSims->GetFirstChild(checked_item);
			this->_treeSims->DeleteItem(di);
			this->_windowMain->Erase_Plot_Simulation(string(name.c_str()));
			this->_treeSims->SetItemText(checked_item, 4, "False");
			this->_windowMain->Unload_Simulation(string(name.c_str()));
		}
	}
	else
	{
		wxCheckBoxState st = this->_treeSims->GetCheckedState(checked_item);
		wxTreeListItem& name_item = this->_treeSims->GetItemParent(checked_item);
		wxString name = this->_treeSims->GetItemText(name_item, 0);
		if (st == wxCHK_CHECKED)
		{
			this->_windowMain->Add_Plot_Simulation(string(name.c_str()));
		}
		else
		{
			this->_windowMain->Erase_Plot_Simulation(string(name.c_str()));
		}	
	}
}

wxString WindowSims::Working_Directory() const
{
	wxPGProperty* pp = 0;
	pp = this->_propertyGrid->GetPropertyByName("WG");	
	return(pp->GetValueAsString());
}

void WindowSims::Populate_Image_Directory()
{
	this->_imgFiles.Clear();
	this->_imgsData.clear();
	this->_simsData.clear();
	this->_simFiles.Clear();
	wxString dir = this->Working_Directory();
	dir = dir + "\\IMAGES\\";
	wxDir::GetAllFiles(dir, &this->_imgFiles,"*.bin",wxDIR_FILES);
	for (int k = 0; k < this->_imgFiles.size(); ++k)
	{
		wxFileName fn(this->_imgFiles[k]);
		Image_Data img_data;
		rw::BinaryImage::Get_File_Image_Size(img_data.Width, img_data.Height,
			img_data.Depth, img_data.Black, std::string(this->_imgFiles[k].c_str()));
		this->_imgFiles[k] = fn.GetName();
		this->_imgsData[string(this->_imgFiles[k].c_str())] = img_data;
	}
}

void WindowSims::Populate_Sim_Directory()
{
	this->_simsData.clear();
	this->_simFiles.clear();
	wxString dir = this->Working_Directory();
	dir = dir + "SIMS\\";
	wxDir::GetAllFiles(dir, &this->_simFiles, "*.sim");
	for (int k = 0; k < this->_simFiles.size(); ++k)
	{
		wxFileName fn(this->_simFiles[k]);
		rw::PlugPersistent sim;
		WxSim wsim(sim);
		wsim.Load_Header(this->_simFiles[k]);
		this->_simFiles[k] = fn.GetName();
		Sim_Data sdata;
		sdata.Sim_Color = Wx_Color(sim.Sim_Color());
		sdata.Total_Walkers = sim.Number_Of_Walkers();
		sdata.Associated_Image = sim.Image_Path();		
		sdata.Sim_Name = this->_simFiles[k];
		if (sim.Associated_Sims() == 1)
		{
			sdata.Sim_Parent = string(sim.Associated_Sim(0).c_str());
		}
		wxDateTime dt = wsim.Date_Time();
		this->_simsData[dt] = sdata;
		map<string, Image_Data>::iterator imi = this->_imgsData.find(sdata.Associated_Image);
		if (imi != this->_imgsData.end())
		{
			Image_Data& id = imi->second;
			id.Associated_Sims.insert(string(this->_simFiles[k].c_str()));
		}
	}
}

void WindowSims::Populate_Trees()
{
	this->_treeImgs->ClearColumns();
	this->_treeSims->ClearColumns();
	this->_treeImgs->DeleteAllItems();
	this->_treeSims->DeleteAllItems();
	this->_treeImgs->AppendColumn("Image name",wxCOL_WIDTH_AUTOSIZE,wxALIGN_CENTER,wxCOL_RESIZABLE |wxCOL_SORTABLE);
	this->_treeImgs->AppendColumn("Width", wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	this->_treeImgs->AppendColumn("Height", wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	this->_treeImgs->AppendColumn("Depth", wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	this->_treeImgs->AppendColumn("Porosity", wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	map<string,Image_Data>::iterator ii = this->_imgsData.begin();
	wxTreeListItem& root = this->_treeImgs->GetRootItem();
	while (ii != this->_imgsData.end())
	{
		Image_Data imdat = ii->second;	
		wxTreeListItem& item = this->_treeImgs->AppendItem(root, ii->first);
		wxString ss;
		ss.clear();
		ss << imdat.Width;
		this->_treeImgs->SetItemText(item, 1,ss);
		ss.clear();
		ss << imdat.Height;
		this->_treeImgs->SetItemText(item, 2, ss);
		ss.clear();
		ss << imdat.Depth;
		this->_treeImgs->SetItemText(item, 3, ss);
		scalar porosity = (scalar)imdat.Black / ((scalar)imdat.Width*imdat.Height*imdat.Depth);
		ss = wxString::FromDouble(porosity,4);
		this->_treeImgs->SetItemText(item, 4, ss);
		set<string>::iterator iis = imdat.Associated_Sims.begin();
		item = this->_treeImgs->AppendItem(item, "Associated simulations");
		while (iis != imdat.Associated_Sims.end())
		{
			this->_treeImgs->AppendItem(item, wxString((*iis).c_str()));
			++iis;
		}
		++ii;
	}
	this->_treeSims->AppendColumn("Simulation", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	this->_treeSims->AppendColumn("Date", wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER, wxCOL_RESIZABLE);
	this->_treeSims->AppendColumn("Particles", wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER, wxCOL_SORTABLE | wxCOL_RESIZABLE);
	this->_treeSims->AppendColumn("Image", wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER, wxCOL_SORTABLE | wxCOL_RESIZABLE);
	this->_treeSims->AppendColumn("Loaded", wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER, wxCOL_SORTABLE | wxCOL_RESIZABLE);
	this->_imgTree.RemoveAll();
	this->_imgTree.Create(16, 16, true);
	
	if (this->_sortData)
	{
		map<wxDateTime, Sim_Data>::iterator iis = this->_simsData.begin();
		root = this->_treeSims->GetRootItem();
		this->_treeSims->SetImageList(&this->_imgTree);
		while (iis != this->_simsData.end())
		{
			Sim_Data sdat = iis->second;
			if (sdat.Sim_Parent.length() == 0)
			{
				wxImage imgs = WxSim::Sim_Icon(sdat.Sim_Color);
				int id = this->_imgTree.Add(imgs);
				wxTreeListItem& item = this->_treeSims->AppendItem(root, wxString(sdat.Sim_Name), id, id);
				wxString datestr = iis->first.Format("%b %d-%Y, %H::%M");
				this->_treeSims->SetItemText(item, 1, datestr);
				wxString ss;
				ss << sdat.Total_Walkers;
				this->_treeSims->SetItemText(item, 2, ss);
				this->_treeSims->SetItemText(item, 3, wxString(sdat.Associated_Image.c_str()));
				this->_treeSims->SetItemText(item, 4, "False");
				item = this->_treeSims->AppendItem(item, "");
				this->_treeSims->DeleteItem(item);
			}
			++iis;			
		}
	}
	else
	{
		map<wxDateTime, Sim_Data>::reverse_iterator iis = this->_simsData.rbegin();
		root = this->_treeSims->GetRootItem();
		this->_treeSims->SetImageList(&this->_imgTree);
		while (iis != this->_simsData.rend())
		{
			Sim_Data sdat = iis->second;
			if (sdat.Sim_Parent.length() == 0)
			{
				wxImage imgs = WxSim::Sim_Icon(sdat.Sim_Color);
				int id = this->_imgTree.Add(imgs);
				wxTreeListItem& item = this->_treeSims->AppendItem(root, wxString(sdat.Sim_Name), id, id);
				wxString datestr = iis->first.Format("%b %d-%Y , %H::%M");
				this->_treeSims->SetItemText(item, 1, datestr);
				wxString ss;
				ss << sdat.Total_Walkers;
				this->_treeSims->SetItemText(item, 2, ss);
				this->_treeSims->SetItemText(item, 3, wxString(sdat.Associated_Image.c_str()));
				this->_treeSims->SetItemText(item, 4, "False");
				item = this->_treeSims->AppendItem(item, "");
				this->_treeSims->DeleteItem(item);
			}
			++iis;
		}
	}
}

const wxArrayString& WindowSims::Image_Files() const
{
	return(this->_imgFiles);
}

const wxArrayString& WindowSims::Sim_Files() const
{
	return(this->_simFiles);
}

void WindowSims::Sort_Ascending(wxCommandEvent& evt)
{
	this->_sortData = false;
	this->Populate_Sim_Directory();
	this->Populate_Trees();
	this->_windowMain->Remark_Sims();
}

void WindowSims::Sort_Descending(wxCommandEvent& evt)
{
	this->_sortData = true;
	this->Populate_Sim_Directory();
	this->Populate_Trees();
	this->_windowMain->Remark_Sims();
}


void WindowSims::Update_File_Trees()
{
	this->Populate_Image_Directory();
	this->Populate_Sim_Directory();
	this->Populate_Trees();
}

void WindowSims::Update_Selected_Sim(const rw::PlugPersistent& sim)
{
	wxPGProperty* pp = this->_propertyGrid->GetPropertyByName("CS.SCol");
	wxColourProperty* pcol = (wxColourProperty*)pp;
	pcol->SetValue(wxVariant(Wx_Color(sim.Sim_Color())));
	wxString stdate = WxSim::Date_Time(sim.Date_Time()).Format("%A, %e %B %Y, %H:%M:%S");
	pp = this->_propertyGrid->GetPropertyByName("CS.SDate");
	pp->SetValue(stdate);
}

void WindowSims::Mark_Sim(const string& sim_name, bool check_plot)
{
	wxTreeListItem& citem = this->_treeSims->GetFirstItem();
	bool marked = false;
	while ((citem.IsOk())&&(!marked))
	{
		wxString citemname = this->_treeSims->GetItemText(citem, 0);
		string scn = string(citemname.c_str());
		if (scn == sim_name)
		{
			marked = true;
			this->_treeSims->CheckItem(citem, wxCHK_CHECKED);
			wxTreeListItem chitem = this->_treeSims->GetFirstChild(citem);
			if (!chitem.IsOk())
			{
				chitem = this->_treeSims->AppendItem(citem, "Plotted");
			}
			if (check_plot)
			{
				this->_treeSims->CheckItem(chitem, wxCHK_CHECKED);
			}
			else
			{
				this->_treeSims->CheckItem(chitem, wxCHK_UNCHECKED);
			}
		}
		citem = this->_treeSims->GetNextItem(citem);
	}
}

void WindowSims::Remark_Sims(const map<wxString, bool>& sim_names)
{
	wxTreeListItem& citem = this->_treeSims->GetFirstItem();
	while (citem.IsOk())
	{
		wxString citemname = this->_treeSims->GetItemText(citem, 0);
		map<wxString, bool>::const_iterator itr = sim_names.find(citemname);
		if (itr != sim_names.end())
		{
			this->_treeSims->CheckItem(citem, wxCHK_CHECKED);
			wxTreeListItem chitem = this->_treeSims->GetFirstChild(citem);
			this->_treeSims->SetItemText(citem, 4, "True");
			if (!chitem.IsOk())
			{
				chitem = this->_treeSims->AppendItem(citem, "Plotted");
			}
			if (itr->second == true)
			{
				this->_treeSims->CheckItem(chitem, wxCHK_CHECKED);
				this->_treeSims->Expand(citem);
			}
		}
		citem = this->_treeSims->GetNextItem(citem);
	}
}


WindowSims::~WindowSims()
{
	this->_mgr->UnInit();
}
