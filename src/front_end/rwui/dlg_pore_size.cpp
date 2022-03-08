#include <fstream>
#include <ostream>
#include "dlg_pore_size.h"
#include "front_end/wx_plotter.h"
#include "win_main.h"


DlgPoreSize::DlgPoreSize(wxWindow* Parent, wxWindowID Id, const wxPoint& Position,
	const wxSize& Size, long Style) : wxDialog(Parent, 0, "Pore size distribution")
{
	this->_mainWindow = (WindowMain*)Parent;
	this->SetSize(800, 480);
	wxBoxSizer* mb = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* hb = new wxBoxSizer(wxHORIZONTAL);

	wxPoint pt;
	pt.x = this->GetParent()->GetPosition().x + this->GetParent()->GetSize().x / 2 - this->GetSize().x / 2;
	pt.y = this->GetParent()->GetPosition().y + this->GetParent()->GetSize().y / 2 - this->GetSize().y / 2;
	this->SetPosition(pt);

	wxToolBar* wtb = new wxToolBar(this, 0, wxDefaultPosition, wxSize(100, 40));

	wxImage img;
	wxBitmap bmp;
	img = wxImage(wxT("icons/start.png"), wxBITMAP_TYPE_PNG);
	img.Rescale(32, 32, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = (wxBitmap)img;
	wtb->InsertTool(0, wxID_APPLY, "Create PSD", bmp, wxNullBitmap, wxITEM_NORMAL, "Create PSD");
	WxPlotter* wp = new WxPlotter(this);
	this->_plot = wp;
	this->_pg = new wxPropertyGrid(
		this,
		0,
		wxDefaultPosition,
		wxDefaultSize,
		wxPG_SPLITTER_AUTO_CENTER |
		wxPG_DEFAULT_STYLE | wxPG_EX_HELP_AS_TOOLTIPS);
	mb->Add(wtb, wxSizerFlags().Proportion(1).Expand());
	mb->Add(hb, wxSizerFlags().Proportion(64).Expand());
	hb->Add(this->_pg, wxSizerFlags().Proportion(2).Expand());
	hb->Add(wp, wxSizerFlags().Proportion(3).Expand());
	this->_pg->SetColumnProportion(0, 2);
	this->_pg->SetColumnProportion(1, 1);

	wxPGChoices arrstr;
	arrstr.Add("nm");
	arrstr.Add("um");
	arrstr.Add("mm");	
	wxPGProperty* pp = this->_pg->Append(new wxEnumProperty("Radius units ","units",arrstr,0));
	pp->SetValue(1);
	this->_pg->Append(new wxFloatProperty("Minimal radius", "minr", 0));	
	this->_pg->Append(new wxFloatProperty("Maximal radius", "maxr", 100));
	this->_pg->Append(new wxFloatProperty("Step", "step", 1));
	this->_pg->Append(new wxFloatProperty("T2 Bulk(s)", "T2B", 2.8));
	wxPGProperty* xip = this->_pg->Append(new wxStringProperty("Relaxivity distribution parameters", "RDP"));
	xip->ChangeFlag(wxPG_PROP_READONLY, true);
	xip->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	this->_pg->AppendIn(xip, new wxIntProperty("Number of shape functions", "nsf", 2));
	wxPGProperty* st = this->_pg->AppendIn(xip, new wxStringProperty("Shape function parameters", "sfp"));
	st->ChangeFlag(wxPG_PROP_READONLY, true);
	st->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	this->SetSizer(mb);
	this->Dimension_Functions();
	this->Create_Sigmoid();
	this->Draw_Function();
	wtb->Realize();
	wtb->Bind(wxEVT_TOOL, &DlgPoreSize::Create, this, wxID_APPLY);
	this->_pg->Bind(wxEVT_PG_CHANGED, &DlgPoreSize::Update, this);
}

void DlgPoreSize::Update(wxPropertyGridEvent& evt)
{
	wxPGProperty* pp = evt.GetProperty();
	if (pp->GetName() == wxString("RDP.nsf"))
	{
		this->Dimension_Functions();
	}
	this->Create_Sigmoid();
	this->Draw_Function();
}

void DlgPoreSize::Draw_Function()
{
	int mm = 1024;
	this->_plot->Erase_All_Curves();
	this->_plot->Add_Curves(1);
	this->_plot->Set_Curve_Color(wxColor(155, 0, 0),0);
	this->_plot->Set_Interval_Limits(0.001f, 1.0f, -0.1f, 0.1f);
	this->_plot->Enable_Logarithmic_Scale_AxisX();
	scalar xmin = log10(0.001);
	scalar xmax = log10(1);
	scalar dx = (xmax - xmin) / (scalar)mm;
	int rm;
	int rmin = 10000;
	int rmax = 0;
	for (int k = 0; k < 1024; ++k)
	{
		scalar x = xmin + ((scalar)k)*dx;
		x = pow(10, x);
		scalar y = this->_sigmoid->Evaluate(x);
		this->_plot->Add_Curve_Point(0, x, y);
		rm = (int)y;
		if (rm > rmax)
		{
			rmax = rm;
		}
		if (rm < rmin)
		{
			rmin = rm;
		}
	}
	this->_rhoMin = (scalar)rmin;
	this->_rhoMax = (scalar)(rmax + 1);
	this->_plot->Refresh();
}

void DlgPoreSize::Create(wxCommandEvent& evt)
{
	this->_pg->CommitChangesFromEditor();
	this->Create_Distribution();
}

void DlgPoreSize::Dimension_Functions()
{
	wxPGProperty* pp;
	pp = this->_pg->GetPropertyByName("RDP.nsf");
	int n = pp->GetValue().GetInteger();
	pp = this->_pg->GetPropertyByName("RDP.sfp");
	pp->DeleteChildren();
	for (int k = 0; k < n; ++k)
	{
		wxPGProperty* ppk = 
			this->_pg->AppendIn(pp, new wxStringProperty(wxString("Sigmoid ") << k+1 << " parameters", "sigm" + wxString::Format("%d", k)));
		ppk->ChangeFlag(wxPG_PROP_READONLY, true);
		ppk->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		this->_pg->AppendIn(ppk, new wxFloatProperty("Max. Asymptote", "max"+wxString::Format("%d", k), 25));
		this->_pg->AppendIn(ppk, new wxFloatProperty("Min. Asymptote", "min" + wxString::Format("%d", k), 5));
		this->_pg->AppendIn(ppk, new wxFloatProperty("Transition", "trans" + wxString::Format("%d", k), 0.5));
		this->_pg->AppendIn(ppk, new wxFloatProperty("Slope", "slope" + wxString::Format("%d", k), 512));
	}
}

rw::Sigmoid* DlgPoreSize::Create_Sigmoid()
{
	if (this->_sigmoid)
	{
		delete this->_sigmoid;
		this->_sigmoid = 0;
	}
	wxPGProperty* pp;
	pp = this->_pg->GetPropertyByName("RDP.nsf");
	int n = pp->GetValue().GetInteger();
	pp = this->_pg->GetPropertyByName("RDP.sfp");
	rw::Sigmoid* sigm = new rw::Sigmoid();
	sigm->Set_Size(n);
	sigm->Set_Slope_Scale(1);
	for (int k = 0; k < n; ++k)
	{
		wxString base = "RDP.sfp.sigm" + wxString::Format("%d", k) + ".";
		scalar K = this->_pg->GetPropertyByName(base + wxString("max") + wxString::Format("%d", k))->GetValue().GetDouble();
		scalar A = this->_pg->GetPropertyByName(base + wxString("min") + wxString::Format("%d", k))->GetValue().GetDouble();
		scalar slope = this->_pg->GetPropertyByName(base + wxString("slope") + wxString::Format("%d", k))->GetValue().GetDouble();
		scalar trans = this->_pg->GetPropertyByName(base + wxString("trans") + wxString::Format("%d", k))->GetValue().GetDouble();
		sigm->Set_Value(4 * k, K);
		sigm->Set_Value(4 * k+1, A);
		sigm->Set_Value(4 * k+2, trans);
		sigm->Set_Value(4 * k+3, slope);
	}
	this->_sigmoid = sigm;
	return(sigm);
}

void DlgPoreSize::Create_Distribution()
{
	scalar A = (scalar)0;
	rw::Sigmoid* sigm = this->_sigmoid;
	scalar ramin = this->_pg->GetPropertyByName("minr")->GetValue().GetDouble();
	scalar ramax = this->_pg->GetPropertyByName("maxr")->GetValue().GetDouble();
	scalar step = this->_pg->GetPropertyByName("step")->GetValue().GetDouble();
	scalar t2b = this->_pg->GetPropertyByName("T2B")->GetValue().GetDouble();

	scalar v = this->_baseSim->Voxel_Length();
	vector<scalar2> distributionPSD;
	sigm->Laplace_PSD(ramin, ramax, step, 
		this->_baseSim->Laplace_Time_Vector(),this->_baseSim->Laplace_Bin_Vector(),
		v, distributionPSD,t2b);

	vector<scalar2> distributionVD;	
	this->_mainWindow->Save_Distribution(distributionPSD, distributionVD);
}

void DlgPoreSize::Set_Simulation(const rw::PlugPersistent& sim)
{
	this->_baseSim = &sim;
}