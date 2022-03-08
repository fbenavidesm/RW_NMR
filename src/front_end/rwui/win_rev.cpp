#include "win_main.h"
#include "win_rev.h"


WindowRev::WindowRev(wxWindow* parent) :
	wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "Representative elementary volume"),
	rw::Rev::RevEvent()
{
	this->_greenLaplace = 0;
	this->_blueLaplace = 0;
	this->_windowMain = (WindowMain*)parent;
	this->_prgdlg = 0;
	this->_imgPtr = 0;
	this->_rev = 0;
	this->_mgr = new wxAuiManager(this);
	this->_mgr->SetDockSizeConstraint(0.3, 0.36);

	this->_plotterA = new WxPlotter(this);
	this->_plotterB = new WxPlotter(this);
	this->_propertyGrid = new wxPropertyGrid(
			this,
			0,
			wxDefaultPosition,
			wxDefaultSize,
			wxPG_SPLITTER_AUTO_CENTER |
			wxPG_DEFAULT_STYLE);

	this->_plotterA->Set_X_Title("X");
	this->_plotterA->Set_Y_Title("Y");
	this->_plotterA->Set_Title("Linear plot");
	this->_plotterA->Set_Interval_Limits(0.21f, 0.25f, 0.21f, 0.25f);

	this->_plotterB->Set_X_Title("X");
	this->_plotterB->Set_Y_Title("Y");
	this->_plotterB->Set_Title("Logarithmic plot");
	this->_plotterB->Set_Interval_Limits(0.000001f,0.1f,-0.001f,0.001f);
	this->_plotterB->Enable_Logarithmic_Scale_AxisX();

	this->_propertyGrid->Append(new wxPropertyCategory("Volume search parameters"));
	this->_propertyGrid->Append(new wxIntProperty("Section size (Start)", "STSEC",32));
	this->_propertyGrid->Append(new wxIntProperty("Section step", "STEPSEC",4));
	this->_propertyGrid->Append(new wxIntProperty("Number of sections", "SECS", 9));
	this->_propertyGrid->Append(new wxFloatProperty("Porosity test threshold", "THRS", 0.1f));

	this->_propertyGrid->Append(new wxPropertyCategory("Laplace parameters"));
	this->_propertyGrid->Append(new wxIntProperty("T2min (log10) (s)", "T2MIN", -4));
	this->_propertyGrid->Append(new wxIntProperty("T2max (log10) (s)", "T2MAX", 1));
	this->_propertyGrid->Append(new wxFloatProperty("Relaxivity factor (Delta)", "DELTA", 0.90));
	this->_propertyGrid->Append(new wxFloatProperty("Regularizer", "REG", 1));
	this->_propertyGrid->Append(new wxIntProperty("Number of bins", "BINS", 128));
	this->_propertyGrid->Append(new wxFloatProperty("Time step", "TSTEP", 5e-5));
	this->_propertyGrid->Append(new wxFloatProperty("Saturation factor", "SAT", 0.001f));

	this->_propertyGrid->Append(new wxPropertyCategory("Volume sections"));
	wxPGChoices arrstr;
	this->_propertyGrid->Append(new wxEnumProperty("Section list (Shown)", "VOLSECSLIST", arrstr, 0));
	this->_propertyGrid->Append(new wxEnumProperty("Section list (Compared)", "VOLSECSLISTC", arrstr, 0));
	wxPGProperty* pp = 0;
	this->_propertyGrid->Append(new wxPropertyCategory("Porosity global statistics"));
	pp = this->_propertyGrid->Append(new wxFloatProperty("Maximal porosity", "MAXPORE",0));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp = this->_propertyGrid->Append(new wxFloatProperty("Minimal porosity",  "MINPORE", 0));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp = this->_propertyGrid->Append(new wxFloatProperty("Mean porosity", "MEANPORE", 0));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp = this->_propertyGrid->Append(new wxFloatProperty("Porosity standard deviation", "PORE_STD_DEV", 0));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp = this->_propertyGrid->Append(new wxIntProperty("Representative volume size", "REP_SIZE", 0));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp = this->_propertyGrid->Append(new wxFloatProperty("Laboratory porosity", "LAB_PORE", 0));
	pp = this->_propertyGrid->Append(new wxFloatProperty("Confidence", "CONF", 0.99));
	pp = this->_propertyGrid->Append(new wxFloatProperty("Unresolved porosity", "UNR_PORE", 0));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp = this->_propertyGrid->Append(new wxStringProperty("Unresolved porosity percentage", "UNR_PORE_P", "0%"));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);


	this->_propertyGrid->Append(new wxPropertyCategory("Laplace global statistics"));
	pp = this->_propertyGrid->Append(new wxFloatProperty("Maximal Laplace correlation", "MAX_LAPL", 0));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp = this->_propertyGrid->Append(new wxFloatProperty("Minimal Laplace correlation", "MIN_LAPL", 0));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp = this->_propertyGrid->Append(new wxFloatProperty("Area difference", "AREA_LAPL", 0));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);

	int s_width;
	int s_height;
	WindowMain* wm = static_cast<WindowMain*>(parent);
	int diff = wm->Ribbon_Size() + wm->Bar_Size();
	wxDisplaySize(&s_width, &s_height);
	s_width = s_width / 3;
	s_height = s_height - diff;

	this->_mgr->AddPane(this->_propertyGrid, wxAuiPaneInfo().Bottom().MinSize(100, 196).BestSize(s_width, 36 * s_height / 100));
	this->_mgr->AddPane(this->_plotterA, wxAuiPaneInfo().Center().Caption("Plotter A").BestSize(s_width, 32 * s_height / 100));
	this->_mgr->AddPane(this->_plotterB, wxAuiPaneInfo().Center().Caption("Plotter B").BestSize(s_width, 32 * s_height / 100));

	this->_propertyGrid->Bind(wxEVT_PG_CHANGED, &WindowRev::Change_Grid_Value, this);
}

void WindowRev::Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar)
{
	wxMenu* menu = new wxMenu();
	wxRibbonPanel* ribbonPanel = new wxRibbonPanel(ribbonPage, wxID_ANY, wxT("REV"), wxNullBitmap,
		wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);
	wxRibbonToolBar* btnBar = new wxRibbonToolBar(ribbonPanel);
	btnBar->SetRows(1);
	wxImage img;
	wxBitmap bmp;
	int bmpsize = 32;

	img.LoadFile("icons/section.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_APPLY, bmp, "Select sample based on porosity criterion");
	menu->Append(wxID_APPLY, "Load image list")->SetBitmap(bmp);
	img.LoadFile("icons/motor.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_ADD, bmp, "Test pore geometries with a T2 distribution");
	menu->Append(wxID_ADD, "Test pore geometries with a T2 distribution")->SetBitmap(bmp);

	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowRev::Porosity, this, wxID_APPLY);
	menu->Bind(wxEVT_MENU, &WindowRev::Porosity, this, wxID_APPLY);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowRev::Simulate_Pore_Shape, this, wxID_ADD);
	menu->Bind(wxEVT_MENU, &WindowRev::Simulate_Pore_Shape, this, wxID_ADD);
}

void WindowRev::Update_Layout()
{
	this->_mgr->Update();
}


void WindowRev::Update_Unresolved_Porosity()
{
	wxPGProperty* pp = 0;
	pp = this->_propertyGrid->GetPropertyByName("MEANPORE");
	scalar mean = pp->GetValue().GetDouble();
	pp = this->_propertyGrid->GetPropertyByName("PORE_STD_DEV");
	scalar std_dev = pp->GetValue().GetDouble();
	pp = this->_propertyGrid->GetPropertyByName("LAB_PORE");
	scalar pl = pp->GetValue().GetDouble();
	pp = this->_propertyGrid->GetPropertyByName("CONF");
	scalar confidence = pp->GetValue().GetDouble();
	scalar alpha = rw::Rev::Unresolved_Porosity(mean, std_dev, pl, confidence);
	pp = this->_propertyGrid->GetPropertyByName("UNR_PORE");
	pp->SetValue(alpha);
	pp = this->_propertyGrid->GetPropertyByName("UNR_PORE_P");
	wxString perc = wxString::FromDouble((scalar)100*alpha / pl, 2) + wxString("%");
	pp->SetValue(perc);
}

void WindowRev::Update_Laplace_Global_Data()
{
	scalar max = 0;
	scalar min = 1e18;
	for (uint i = 0; i < this->_rev->Sample_Size(); ++i)
	{
		math_la::math_lac::full::Vector li;
		math_la::math_lac::full::Vector t2;
		scalar porosity = 0;
		this->_rev->Get_T2_Distribution(i, li, t2);
		scalar lin = li.Dot(li);
		for (uint j = i+1; j < this->_rev->Sample_Size(); ++j)
		{
			math_la::math_lac::full::Vector lj;
			this->_rev->Get_T2_Distribution(j, lj, t2);
			scalar ljn = lj.Dot(lj);
			scalar d = li.Dot(lj);
			scalar corr = d*d / (lin*ljn);
			corr = sqrt(corr);
			if (corr > max)
			{
				max = corr;
			}
			if (corr < min)
			{
				min = corr;
			}
		}
	}
	scalar accum = 0;
	for (uint k = 0; k < (uint)this->_maxLaplace.Size(); ++k)
	{
		accum = accum + this->_maxLaplace(k) - this->_minLaplace(k);
	}
	wxPGProperty* pp = 0;
	pp = this->_propertyGrid->GetPropertyByName("MAX_LAPL");
	pp->SetValue(max);
	pp = this->_propertyGrid->GetPropertyByName("MIN_LAPL");
	pp->SetValue(min);
	pp = this->_propertyGrid->GetPropertyByName("AREA_LAPL");
	pp->SetValue(accum);

}

void WindowRev::Update_Porosity_Global_Data()
{
	wxPGProperty* pp = 0;
	pp = this->_propertyGrid->GetPropertyByName("MAXPORE");
	pp->SetValue(this->_maxPorosity);
	pp = this->_propertyGrid->GetPropertyByName("MINPORE");
	pp->SetValue(this->_minPorosity);
	pp = this->_propertyGrid->GetPropertyByName("MEANPORE");
	pp->SetValue(this->_means[(uint)this->_means.size()-1]);
	pp = this->_propertyGrid->GetPropertyByName("PORE_STD_DEV");
	pp->SetValue(this->_stdDev[(uint)this->_stdDev.size() - 1]);
	pp = this->_propertyGrid->GetPropertyByName("REP_SIZE");
	pp->SetValue((int)this->_rev->Section_Size());
	pp = this->_propertyGrid->GetPropertyByName("LAB_PORE");
	if (pp->GetValue().GetDouble() == 0)
	{
		pp->SetValue(this->_means[(uint)this->_means.size() - 1]);
	}
}

void WindowRev::Draw_Laplace(uint id, const wxColor& color)
{
	math_la::math_lac::full::Vector laplace;
	math_la::math_lac::full::Vector t2;
	this->_rev->Get_T2_Distribution(id, laplace, t2);
	this->_plotterB->Add_Curves(1);
	uint cid = this->_plotterB->Number_Of_Curves()-1;
	this->_plotterB->Set_Curve_Color(color, cid);
	for (uint k = 0; k < (uint)t2.Size(); ++k)
	{
		this->_plotterB->Add_Curve_Point(cid, t2(k), laplace(k));
	}
}

void WindowRev::Change_Grid_Value(wxPropertyGridEvent& evt)
{
	wxPGProperty* pp = evt.GetProperty();
	if (pp->GetName() == wxString("VOLSECSLIST"))
	{
		if (this->_rev->Porosity_Test_Executed())
		{
			int id = pp->GetValue().GetInteger();
			rw::Pos3i pos;
			scalar porosity = 0;
			this->_rev->Section(id, pos, porosity);
			this->_windowMain->Mark_Sub_Volume(pos.x, pos.y, pos.z,
				this->_rev->Section_Size(),
				this->_rev->Section_Size(),
				this->_rev->Section_Size());
			if (this->_rev->Random_Walk_Executed())
			{
				this->_plotterB->Erase_All_Curves();
				this->Draw_Laplace_Area();
				this->_blueLaplace = id;
				this->Draw_Laplace(this->_blueLaplace, wxColor(0, 0, 255));
				this->Draw_Laplace(this->_greenLaplace, wxColor(0, 255, 0));
				this->_plotterB->Refresh();
			}
		}
	}
	if (pp->GetName() == wxString("VOLSECSLISTC"))
	{
		if (this->_rev->Porosity_Test_Executed())
		{
			int id = pp->GetValue().GetInteger();
			if (this->_rev->Random_Walk_Executed())
			{
				this->_plotterB->Erase_All_Curves();
				this->Draw_Laplace_Area();
				this->_greenLaplace = id;
				this->Draw_Laplace(this->_blueLaplace, wxColor(0, 0, 255));
				this->Draw_Laplace(this->_greenLaplace, wxColor(0, 255, 0));
				this->_plotterB->Refresh();
			}
		}
	}
	if ((pp->GetName() == wxString("LAB_PORE"))||(pp->GetName() == wxString("CONF")))
	{
		if (this->_rev->Porosity_Test_Executed())
		{
			this->Update_Unresolved_Porosity();
		}
	}
}


void WindowRev::Simulate_Pore_Shape(wxCommandEvent& evt)
{
	if ((this->_rev) && (this->_rev->Porosity_Test_Executed()))
	{
		this->_propertyGrid->CommitChangesFromEditor();
		wxPGProperty* pp = 0;
		pp = this->_propertyGrid->GetPropertyByName("T2MIN");
		scalar t2min = pp->GetValue().GetDouble();
		pp = this->_propertyGrid->GetPropertyByName("T2MAX");
		scalar t2max = pp->GetValue().GetDouble();
		pp = this->_propertyGrid->GetPropertyByName("DELTA");
		scalar delta = pp->GetValue().GetDouble();
		pp = this->_propertyGrid->GetPropertyByName("REG");
		scalar reg = pp->GetValue().GetDouble();
		pp = this->_propertyGrid->GetPropertyByName("BINS");
		uint res = pp->GetValue().GetDouble();
		pp = this->_propertyGrid->GetPropertyByName("TSTEP");
		scalar dt = pp->GetValue().GetDouble();
		this->_rev->Set_Event(this);
		this->_prgdlg = new wxGenericProgressDialog("Simulating", "Executing random walk simulations", 150);
		this->_prgdlg->Show();
		int nw = this->_rev->Section_Number_Of_Walkers();
		pp = this->_propertyGrid->GetPropertyByName("SAT");
		float sat = (float)pp->GetValue().GetDouble();
		nw = (int)(((float)nw)*sat);
		this->_rev->Walk_Inside_Sections(nw, delta, t2min, t2max, res, reg,dt);
		
		this->Pick_Max_Laplace();
		
		this->_plotterB->Erase_All_Curves();
		this->_plotterB->Set_Interval_Limits(pow(10, t2min), pow(10, t2max), -0.001f, 0.001f);
		this->_plotterB->Set_Precision_Digits(6);
		this->_plotterB->Set_Title("T2 Distributions");
		this->_plotterB->Set_X_Title("T2(s)");
		this->_plotterB->Set_Y_Title("Volumetric weight");
		this->_plotterB->Enable_Logarithmic_Scale_AxisX();

		wxPropertyGridEvent evt;
		pp = this->_propertyGrid->GetPropertyByName("VOLSECSLIST");
		pp->SetValue(0);
		evt.SetProperty(pp);
		pp = this->_propertyGrid->GetPropertyByName("VOLSECSLISTC");
		pp->SetValue(0);		

		this->_prgdlg->Update(125, "Processing laplace data");
		this->Update_Laplace_Global_Data();
		this->Change_Grid_Value(evt);

		this->_propertyGrid->Refresh();
		
		this->_prgdlg->Close();
		delete this->_prgdlg;
		this->_prgdlg = 0;
		this->Draw_Laplace_Area();
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this, wxString("Porosity test must be previously executed"), wxString("No porosity test"), wxOK);
		mgdlg.ShowModal();
	}
}

void WindowRev::Pick_Max_Laplace()
{
	math_la::math_lac::full::Vector max_lapl;
	math_la::math_lac::full::Vector min_lapl;

	math_la::math_lac::full::Vector vt2_time;
	math_la::math_lac::full::Vector laplace;
	for (uint k = 0; k < this->_rev->Section_Size(); ++k)
	{
		this->_rev->Get_T2_Distribution(k, laplace, vt2_time);
		if (max_lapl.Size() == 0)
		{
			max_lapl.Set_Size(laplace.Size());
			min_lapl.Set_Size(laplace.Size());
			for (uint i = 0; i < (uint)min_lapl.Size(); ++i)
			{
				min_lapl(i, (scalar)10000000);
			}
		}
		for (uint i = 0; i < (uint)laplace.Size(); ++i)
		{
			if (laplace(i) > max_lapl(i))
			{
				max_lapl(i, laplace(i));
			}
			if (laplace(i) < min_lapl(i))
			{
				min_lapl(i, laplace(i));
			}
		}
	}
	this->_maxLaplace = max_lapl;
	this->_minLaplace = min_lapl;
	this->_DT2 = vt2_time;
}

void WindowRev::Draw_Laplace_Area()
{
	this->_plotterB->Add_Curves(2);
	this->_plotterB->Set_Curve_Color(wxColor(255, 0, 0),0);
	this->_plotterB->Set_Curve_Color(wxColor(255, 0, 0),1);
	for (uint i = 0; i < (uint)this->_maxLaplace.Size(); ++i)
	{
		WxPlotter::Pair p;
		p.x = this->_DT2(i);
		p.y = this->_minLaplace(i);
		this->_plotterB->Add_Curve_Point(0, p);
		p.y = this->_maxLaplace(i);
		this->_plotterB->Add_Curve_Point(1, p);
	}
}

void WindowRev::On_Walk_Event(int k ,scalar percentage)
{
	if (this->_prgdlg)
	{
		this->_prgdlg->Update((uint)(percentage * (scalar)100), wxString("RW simulation ")<<k+1<<wxString(" executed"));
		this->_prgdlg->Refresh();
	}
}

void WindowRev::Plot_Convergence()
{
	int msize = std::min(this->_imgPtr->Width(), std::min(this->_imgPtr->Height(),this->_imgPtr->Depth()));
	this->_plotterA->Erase_All_Curves();
	this->_plotterA->Set_Title("Section convergency");
	this->_plotterA->Set_X_Title("Section size (Hundreds of voxels)");
	this->_plotterA->Set_Y_Title("Coefficient of variation");
	this->_plotterA->Set_Interval_Limits(0.0f,(float)msize/100.0f, -0.01f, 0.01f);
	this->_plotterA->Add_Curves(1);
	this->_plotterA->Set_Curve_Color(wxColor(255, 0, 0), 0);
	for (uint k = 0; k < (uint)this->_sectionSizes.size(); ++k)
	{
		WxPlotter::Pair pt;
		pt.x = this->_sectionSizes[k]/100.0f;
		pt.y = this->_stdDev[k] / this->_means[k];
		this->_plotterA->Add_Curve_Point(0, pt);
		this->_plotterA->Add_Mark(pt, wxColor(255, 0, 0));
	}
	this->_plotterA->Refresh();
}

void WindowRev::Display_Sections()
{
	wxPGChoices choices;
	for (uint k = 0; k < this->_rev->Sample_Size(); ++k)
	{
		rw::Pos3i pos;
		scalar porosity = 0;
		this->_rev->Section(k, pos, porosity);

		wxString name_section = "Section ";
		name_section << k+1 << " (";
		name_section << pos.x << "," << pos.y << "," << pos.z << ")";

		choices.Add(name_section);
	}

	wxPGProperty* pp = 0;
	wxEnumProperty* ep = (wxEnumProperty*)this->_propertyGrid->GetPropertyByName("VOLSECSLIST");
	ep->SetChoices(choices);
	ep = (wxEnumProperty*)this->_propertyGrid->GetPropertyByName("VOLSECSLISTC");
	ep->SetChoices(choices);
}

void WindowRev::Porosity(wxCommandEvent& evt)
{
	if (this->_imgPtr)
	{
		this->_propertyGrid->CommitChangesFromEditor();
		wxPGProperty* pp = 0;
		this->_means.clear();
		this->_stdDev.clear();
		this->_sectionSizes.clear();
		pp = this->_propertyGrid->GetPropertyByName("STSEC");
		int c_size = pp->GetValue().GetInteger();
		pp = this->_propertyGrid->GetPropertyByName("STEPSEC");
		int step_size = pp->GetValue().GetInteger();
		pp = this->_propertyGrid->GetPropertyByName("THRS");
		scalar thrs = (scalar)pp->GetValue().GetDouble();
		pp = this->_propertyGrid->GetPropertyByName("SECS");
		uint sample_size = pp->GetValue().GetInteger();
		if (this->_rev)
		{
			this->_rev->Set_Porosity_Test_Threshold(thrs);
			this->_rev->Set_Sample_Size(sample_size);
			wxGenericProgressDialog prgdlg("Finding porosity representative size", wxString("Section size: ") << c_size, 10);
			prgdlg.Show();
			this->_rev->Set_Section_Size(c_size);
			scalar mean = 0;
			scalar std_dev = 0;			
			while (!this->_rev->Porosity_Test(mean, std_dev,this->_minPorosity,this->_maxPorosity))
			{
				this->_means.push_back(mean);
				this->_stdDev.push_back(std_dev);
				this->_sectionSizes.push_back((float)c_size);
				prgdlg.Pulse(wxString("Section size: ") << c_size << "\nMean: " << mean << "\nStandard deviation:" << std_dev);
				prgdlg.Refresh();
				c_size = c_size + step_size;
				this->_rev->Set_Section_Size(c_size);
			}
			this->_means.push_back(mean);
			this->_stdDev.push_back(std_dev);
			this->_sectionSizes.push_back((float)c_size);
			this->Plot_Convergence();
			this->Display_Sections();
			this->Update_Porosity_Global_Data();
			this->Update_Unresolved_Porosity();
			prgdlg.Close();
		}
		else
		{
			wxMessageDialog msgdlg(this, "An image sample must be selected", "No image to segment", wxOK);
			msgdlg.ShowModal();
		}
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this, wxString("There is no image to segment"), wxString("No image"), wxOK);
		mgdlg.ShowModal();
	}
}

void WindowRev::Set_Image_Pointer(const rw::BinaryImage* imgptr)
{
	this->_imgPtr = imgptr;
	this->_rev = new rw::Rev();
	this->_rev->Set_Image(*imgptr);
}


WindowRev::~WindowRev()
{
	this->_mgr->UnInit();
}
