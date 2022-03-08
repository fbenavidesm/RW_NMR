#include "dlg_regularizer.h"
#include "math_la/txt/parameters.h"
#include "math_la/txt/converter.h"
#include "math_la/txt/separator.h"
#include "wx/progdlg.h"
#include "front_end/wx_rgbcolor.h"


DlgRegularizer::DlgRegularizer(wxWindow* parent, const wxString& Title)
	: wxDialog(parent,0,Title)
{
	this->_decision = wxID_CANCEL;
	math_la::math_lac::txt::Params p;
	p.Load_From_File("files\\reg.conf");

	wxToolBar* wtb = new wxToolBar(this, 0, wxDefaultPosition, wxSize(100, 38));

	this->_OKBtnPressed = false;
	this->_compensate = false;
	this->_criterionIndex = math_la::math_lac::txt::Converter::Convert_To_Int(p["criterion"]);
	this->_lambdaMin = math_la::math_lac::txt::Converter::Convert_To_Scalar(p["lambdamin"]);
	this->_lambdaMax = math_la::math_lac::txt::Converter::Convert_To_Scalar(p["lambdamax"]);
	this->_laplaceResolution = math_la::math_lac::txt::Converter::Convert_To_Int(p["max_laplace_res"]);
	this->_timeReduction = math_la::math_lac::txt::Converter::Convert_To_Int(p["max_time_red"]);
	this->_regularizerResolution = math_la::math_lac::txt::Converter::Convert_To_Int(p["reg_res"]);
	this->_sigmoidCut = math_la::math_lac::txt::Converter::Convert_To_Scalar(p["sigma_cut"]);
	this->_sigmoidMultiplier = math_la::math_lac::txt::Converter::Convert_To_Int(p["sigma_mult"]);
	this->_lCurveStabilizingFactor = math_la::math_lac::txt::Converter::Convert_To_Scalar(p["lcurve_stab"]);
	this->_lCurveMagnitudeInteger = math_la::math_lac::txt::Converter::Convert_To_Int(p["l_magnitude_integer"]);
	string compensate = p["compensate_lcurve"];
	if (math_la::math_lac::txt::Separator::Find_Key_Word("TRUE", compensate))
	{
		this->_compensate = true;
	}
	
	wxBoxSizer* msizer = new wxBoxSizer(wxVERTICAL);
	msizer->Add(wtb, wxSizerFlags().Expand().Proportion(1));
	wxBoxSizer* psizer = new wxBoxSizer(wxHORIZONTAL);
	msizer->Add(psizer, wxSizerFlags().Expand().Proportion(500));
	this->_textRegularizer = new wxStaticText(this,wxID_ANY,"Regularizer: ");
	msizer->Add(this->_textRegularizer, wxSizerFlags().Expand().Proportion(12));
	wxSlider* slider = new wxSlider(this,wxID_ANY,0,0,this->_regularizerResolution-1);
	this->_regSlider = slider;
	msizer->Add(slider, wxSizerFlags().Expand().Proportion(24));
	slider->Bind(wxEVT_SCROLL_CHANGED, &DlgRegularizer::On_Scroll, this);
	slider->Bind(wxEVT_SCROLL_THUMBTRACK, &DlgRegularizer::On_Scroll, this);


	this->_leftPlotter = new WxPlotter(this);
	this->_leftPlotter->Enable_Logarithmic_Scale_AxisX();
	this->_rightPlotter = new WxPlotter(this);
	this->_rightPlotter->Enable_Logarithmic_Scale_AxisX();
	psizer->Add(this->_leftPlotter, wxSizerFlags().Expand().Proportion(1));
	psizer->Add(this->_rightPlotter, wxSizerFlags().Expand().Proportion(1));
	wxBoxSizer* bottomsizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton* okbtn = new wxButton(this, 0, "OK");
	wxButton* cnbtn = new wxButton(this, 0, "Cancel");
	wxButton* sabtn = new wxButton(this, 0, "Save data");

	wxImage image;
	wxBitmap bmp;
	image = wxImage(wxT("icons//check.png"), wxBITMAP_TYPE_PNG);
	image.Rescale(32, 32, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(image);
	okbtn->SetBitmap(bmp);

	image = wxImage(wxT("icons//close.png"), wxBITMAP_TYPE_PNG);
	image.Rescale(32, 32, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(image);
	cnbtn->SetBitmap(bmp);

	image = wxImage(wxT("icons//CSV.png"), wxBITMAP_TYPE_PNG);
	image.Rescale(32, 32, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(image);
	sabtn->SetBitmap(bmp);

	bottomsizer->Add(okbtn, wxSizerFlags().Center());
	bottomsizer->Add(cnbtn, wxSizerFlags().Center());
	bottomsizer->Add(sabtn, wxSizerFlags().Center());
	msizer->Add(bottomsizer, wxSizerFlags().Center());
	this->SetSizer(msizer);
	this->_rightPlotter->Set_Interval_Limits(0.001f, 0.001f, 0.0001f, 0.0001f);
	this->_leftPlotter->Set_Interval_Limits(0.001f, 0.001f, 0.0001f, 0.0001f);
	this->_rightPlotter->Enable_Logarithmic_Scale_AxisX();
	this->_leftPlotter->Enable_Logarithmic_Scale_AxisX();
	okbtn->Bind(wxEVT_BUTTON, &DlgRegularizer::On_Ok_Button, this);
	cnbtn->Bind(wxEVT_BUTTON, &DlgRegularizer::On_Close_Button, this);
	sabtn->Bind(wxEVT_BUTTON, &DlgRegularizer::On_Save_Button, this);
	this->_leftPlotter->Set_Title("L curve criterion");
	this->_leftPlotter->Set_X_Title("Approximation error");
	this->_leftPlotter->Set_Y_Title("Bin norm");

	this->_rightPlotter->Set_Title("S curve criterion");
	this->_rightPlotter->Set_X_Title("Regularizer");
	this->_rightPlotter->Set_Y_Title("Approximation error");
	this->SetSize(1032, 600);
}

void DlgRegularizer::On_Scroll(wxScrollEvent& evt)
{
	int sel = evt.GetPosition();
	this->Select_Index(sel);
	this->Refresh();
}

void DlgRegularizer::Set_Simulation(const rw::PlugPersistent& sim)
{
	wxGenericProgressDialog wdlg("Preprocessing data", "Reducing dimensionality", 8);
	wdlg.Show();

	int laplres = sim.Laplace_Resolution();
	if (laplres > this->_laplaceResolution)
	{
		laplres = this->_laplaceResolution;
	}
	int tred = sim.Decay_Reduction();
	if ((tred > this->_timeReduction) || (tred == 0))
	{
		tred = this->_timeReduction;
	}
	WxPlotter::Pair p;
	this->_leftPlotter->Add_Mark(p, Wx_Color(sim.Sim_Color()));
	this->_rightPlotter->Add_Mark(p, Wx_Color(sim.Sim_Color()));

	this->_exponentialFitting.Load_Sim(sim);
	this->_exponentialFitting = this->_exponentialFitting.Logarithmic_Reduction(tred);

	wdlg.Update(2, "Mounting SVD algorithm");
	math_la::math_lac::full::Vector circ = this->_exponentialFitting.Regularizer_Mount(log10(sim.Laplace_T_Min()), log10(sim.Laplace_T_Max()),
		laplres, this->_lambdaMin, this->_lambdaMax, this->_regularizerResolution);
	this->_curvatureVector << circ;
	this->_leftPlotter->Add_Curves(1);
	this->_leftPlotter->Set_Curve_Color(Wx_Color(sim.Sim_Color()), 0);
	this->_rightPlotter->Add_Curves(1);
	this->_rightPlotter->Set_Curve_Color(Wx_Color(sim.Sim_Color()), 0);

	wdlg.Update(3, "Updating plotter");
	this->_fx = this->_exponentialFitting.Laplace_Domain();
	this->_fy = this->_exponentialFitting.Laplace_Range();
	scalar diff = -1;
	scalar loglambdamin = log10(this->_lambdaMin);
	scalar loglambdamax = log10(this->_lambdaMax);
	scalar dx = (loglambdamax - loglambdamin) / (scalar)this->_regularizerResolution;
	this->_regularizerStep = dx;

	this->_lambdas = math_la::math_lac::full::Vector(this->_fx.Size());
	wdlg.Update(4, "Updating plots");
	for (int j = 0; j < this->_fx.Size(); ++j)
	{
		WxPlotter::Pair pt;
		pt.x = this->_fx(j);
		pt.y = this->_fy(j);
		this->_leftPlotter->Add_Curve_Point(0, pt);
		scalar loglambda = loglambdamin + ((scalar)j)*dx;
		scalar lambda = pow(10, loglambda);
		pt.x = lambda;
		pt.y = log10(this->_fx(j));
		this->_rightPlotter->Add_Curve_Point(0,pt);
		this->_lambdas(j,pt.x);
	}
	wdlg.Update(5, "Updating lambda space discretization");
	wdlg.Update(6, "Choosing regularizer");
	int sel = 0;
	if (this->_criterionIndex == 1)
	{
		sel = this->Get_L_Selection();
	}
	if (this->_criterionIndex == 2)
	{
		sel = this->Get_Sigma_Selection();
	}
	if (this->_criterionIndex == 3)
	{
		int sel1 = this->Get_L_Selection();
		int sel2 = this->Get_Sigma_Selection();
		sel = std::max(sel1, sel2);
	}
	if (this->_criterionIndex == 4)
	{
		int sel1 = this->Get_L_Selection();
		int sel2 = this->Get_Sigma_Selection();
		sel = (sel1 + sel2) / 2;
	}
	this->_regSlider->SetValue(sel);
	this->Select_Index(sel);
}

int DlgRegularizer::Get_L_Selection()
{
	int sel = 0;
	scalar cmax = 0;
	int accumNeg = 0;
	for (int j = 0; j < this->_fx.Size(); ++j)
	{
		if (j > 0)
		{
			if (this->_curvatureVector(j) > cmax)
			{
				sel = j;
				cmax = this->_curvatureVector(j);
			}
			if (this->_curvatureVector(j) < 0)
			{
				accumNeg = accumNeg + 1;
				if (accumNeg > this->_regularizerResolution / 10)
				{
					j = this->_fx.Size();
				}
			}
		}
	}
	if (this->_compensate)
	{
		int factor = 1;
		while (cmax < (scalar)0.5)
		{
			cmax = cmax * 10;
			factor = factor * 10;
		}
		cmax = (scalar)(this->_lCurveMagnitudeInteger) / (scalar)factor;
		bool selreg = false;
		int id = sel;
		while ((!selreg) && (id < this->_curvatureVector.Size()))
		{
			if (this->_curvatureVector(id) < this->_lCurveStabilizingFactor*cmax)
			{
				sel = id;
				selreg = true;
			}
			++id;
		}
	}
	return(sel);
}

int DlgRegularizer::Get_Sigma_Selection()
{
	scalar diff = -1;
	scalar cdiff = 2;
	int sel = 0;
	scalar ch = 0;
	bool block = false;
	scalar dx = this->_regularizerStep;
	for (int j = 0; j < this->_fx.Size(); ++j)
	{
		if (j > 0)
		{
			diff = log10(this->_fx(j)) - log10(this->_fx(j - 1));
			diff = fabs(diff / dx - this->_sigmoidCut);
			if (diff > ((scalar)this->_sigmoidMultiplier)*this->_sigmoidCut)
			{
				block = true;
			}
			if ((diff > 0) && (diff < cdiff) && (!block))
			{
				sel = j;
				cdiff = diff;
			}
		}
	}
	return(sel);
}

void DlgRegularizer::Select_Index(int id)
{
	scalar x = this->_fx(id);
	scalar y = this->_fy(id);
	WxPlotter::Pair p;
	p.x = x;
	p.y = y;
	this->_leftPlotter->Move_Mark_Position(p, 0);
	p.x = this->_lambdas(id);
	p.y = log10(this->_fx(id));
	this->_rightPlotter->Move_Mark_Position(p,0);

	this->_regularizer = this->_lambdas(id);
	wxString txt = wxString::FromDouble(this->_regularizer, 4);
	txt = wxString("Regularizer: ") + txt;
	this->_textRegularizer->SetLabel(txt);
	this->_textRegularizer->Refresh();
	this->_rightPlotter->Refresh();
	this->_leftPlotter->Refresh();
}

scalar DlgRegularizer::Regularizer() const
{
	return(this->_regularizer);
}

int DlgRegularizer::Decision() const
{
	return(this->_decision);
}

void DlgRegularizer::On_Ok_Button(wxCommandEvent& evt)
{
	this->_decision = wxID_OK;
	this->Close();
}

void DlgRegularizer::On_Close_Button(wxCommandEvent& evt)
{
	this->_decision = wxID_CANCEL;
	this->Close();
}

void DlgRegularizer::On_Save_Button(wxCommandEvent& evt)
{

}



DlgRegularizer::~DlgRegularizer()
{

}
