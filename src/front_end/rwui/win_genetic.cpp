#include <fstream>
#include <ostream>
#include <string>
#include "win_genetic.h"
#include "rw/experiment_report.h"
#include "win_main.h"
#include "front_end/wx_plotter.h"
#include "wx/filefn.h"
#include "front_end/wx_rgbcolor.h"

using std::string;

WinGeneticObserver::WinGeneticObserver(WindowGenetic* parent)
{
	this->_maxXi = 1;
	this->_winGenetic = parent;
	this->_curveSize = 1024;
	this->_prgdlg = 0;
}

WinGeneticObserver::~WinGeneticObserver()
{

}


void WinGeneticObserver::Executing_Simulations(int current, int totsize)
{
	this->_winGenetic->_winText->Add_Info_Text(wxString("Executing simulations ") 
		<< current + 1 << ".." << std::min(current + 8, totsize)
		<< " from a total of " << totsize);	
}

void WinGeneticObserver::Diversifying_Island(int i)
{
	this->_winGenetic->_winText->Add_Info_Text(wxString("Island ") << i
		<< " is being diversified");
}

void WinGeneticObserver::Create_Offspring(int it)
{
	this->_winGenetic->_winText->Add_Info_Text("Evaluating offspring. Iteration " + wxString::Format("%d", it + 1));
}

scalar WinGeneticObserver::Maximal_Confidence(list<math_la::math_lac::genetic::Creature*>& lst)
{
	if (!lst.empty())
	{
		list<math_la::math_lac::genetic::Creature*>::iterator ic = lst.begin();
		int isize = this->_curveSize;
		vector<scalar> rd(isize, 0);
		vector<scalar> rmax(isize, 0);
		vector<scalar> rmin(isize, 1E90);
		scalar n = (scalar)lst.size();
		while (ic != lst.end())
		{
			rw::RelaxivityExperiment* c = (rw::RelaxivityExperiment*)(*ic);
			for (int i = 0; i < isize; ++i)
			{
				scalar x = ((scalar)i)*this->_maxXi / (scalar)isize;
				scalar v = c->Relaxivity_Distribution()(x);
				rd[i] = x;
				if (v < rmin[i])
				{
					rmin[i] = v;
				}
				if (v > rmax[i])
				{
					rmax[i] = v;
				}
			}
			++ic;
		}
		this->_dom = rd;
		this->_rhoMax = rmax;
		this->_rhoMin = rmin;

		scalar d = 0;
		scalar nd = 0;
		for (int i = 0; i < isize; ++i)
		{
			d = d + (this->_rhoMax[i] - this->_rhoMin[i]) / 2;
			nd = nd + this->_rhoMin[i];
		}
		return(d / nd);
	}
	else
	{
		return(0);
	}
}

scalar WinGeneticObserver::Gaussian_Confidence(list<math_la::math_lac::genetic::Creature*>& lst)
{
	if (!lst.empty())
	{
		scalar z = 3.99;
		list<math_la::math_lac::genetic::Creature*>::iterator ic = lst.begin();
		int isize = this->_curveSize;
		vector<scalar> rd(isize, 0);
		vector<scalar> rmean(isize, 0);
		vector<scalar> rdev(isize, 0);
		scalar n = (scalar)lst.size();
		scalar acc = 0;
		while (ic != lst.end())
		{
			math_la::math_lac::genetic::Creature* c = *ic;
			acc = acc + c->Fitness();
			++ic;
		}
		ic = lst.begin();
		while (ic != lst.end())
		{
			rw::RelaxivityExperiment* c = (rw::RelaxivityExperiment*)(*ic);
			for (int i = 0; i < isize; ++i)
			{
				scalar x = ((scalar)i)*this->_maxXi / (scalar)isize;
				scalar v = c->Relaxivity_Distribution()(x);
				rd[i] = x;
				rmean[i] = rmean[i] + c->Fitness() * v / acc;
				rdev[i] = rdev[i] + c->Fitness() * v*v / acc;
			}
			++ic;
		}
		scalar ns = 1;
		if (!lst.empty())
		{
			ns = sqrt((scalar)lst.size());
		}
		this->_dom = rd;
		scalar d = 0;
		this->_rhoMax = vector<scalar>(isize, 0);
		this->_rhoMin = vector<scalar>(isize, 0);
		for (int i = 0; i < isize; ++i)
		{
			scalar dev = rdev[i] - rmean[i] * rmean[i];
			dev = sqrt(dev);
			dev = z * dev / ns;
			this->_rhoMax[i] = rmean[i] + dev;
			this->_rhoMin[i] = rmean[i] - dev;
			dev = dev / this->_rhoMin[i];
			if (dev > d)
			{
				d = dev;
			}
		}
		return(d);
	}
	else
	{
		return(0);
	}
}



void WinGeneticObserver::Update_Winners(const math_la::math_lac::genetic::Creature& winner)
{
	this->_weightFitness = winner.Fitness();
	this->_K.clear();
	this->_A.clear();
	this->_xo.clear();
	this->_B.clear();
	this->_xmin.clear();
	this->_xmax.clear();
	if (this->_winGenetic->_optimizer->Function_Shape() == rw::RelaxivityDistribution::Sigmoid)
	{
		int size = this->_winGenetic->_optimizer->Phenotype_Size() / 4;
		const rw::RelaxivityExperiment& ec = (const rw::RelaxivityExperiment&)winner;
		wxString txt = "Coefficients \n\n";
		for (int i = 0; i < size; ++i)
		{
			scalar K = ((const rw::Sigmoid&)ec.Relaxivity_Distribution()).K(i);
			scalar A = ((const rw::Sigmoid&)ec.Relaxivity_Distribution()).A(i);
			scalar B = ((const rw::Sigmoid&)ec.Relaxivity_Distribution()).Slope(i);
			scalar xo = ((const rw::Sigmoid&)ec.Relaxivity_Distribution()).XI(i);
			this->_K.push_back(K);
			this->_A.push_back(A);
			this->_xo.push_back(xo);
			this->_B.push_back(B);
			txt << "K (" << i << ") -> " << K << "\n";
			txt << "A (" << i << ") -> " << A << "\n";
			txt << "Slope (" << i << ") -> " << B << "\n";
			txt << "X0 (" << i << ") -> " << xo << "\n";
			txt << "\n";
		}
		txt << "Fitness: " << winner.Fitness() << "\n";
		this->_winGenetic->_winText->Add_Low_Text(txt);
	}
	if (this->_winGenetic->_optimizer->Function_Shape() == rw::RelaxivityDistribution::Hat)
	{
		int size = this->_winGenetic->_optimizer->Phenotype_Size() / 3;
		const rw::RelaxivityExperiment& ec = (const rw::RelaxivityExperiment&)winner;
		for (int i = 0; i < size; ++i)
		{
			scalar B = ((const rw::Hat&)ec.Relaxivity_Distribution()).K(i);
			scalar xmin = ((const rw::Hat&)ec.Relaxivity_Distribution()).X_Min(i);
			scalar xmax = ((const rw::Hat&)ec.Relaxivity_Distribution()).X_Max(i);
			this->_B.push_back(B);
			this->_xmin.push_back(xmin);
			this->_xmax.push_back(xmax);
		}
	}
	this->_winGenetic->_optimizer->Lock_Working_Thread();
	this->_winGenetic->_plotterA->Block();
	this->_winGenetic->_plotterB->Block();
	int ee = 0;
	while ((!this->_winGenetic->_plotterA->Editable()) && (!this->_winGenetic->_plotterB->Editable()) && (ee < 50))
	{
		Sleep(25);
		++ee;
	}
	this->_winGenetic->_plotterA->Erase_All_Curves();
	this->_winGenetic->_plotterA->Add_Curves(3);
	this->_winGenetic->_plotterA->Set_Interval_Limits(-0.01f, 0.01f, -0.1f, 1.0f);
	scalar fit = winner.Fitness();
	fit = fit - this->_fitnessPercentage*fit;
	list<math_la::math_lac::genetic::Creature*> lst;
	this->_winGenetic->_optimizer->Pick_Creature_List(lst, fit);
	if (this->_errorEstimateStrategy == 0)
	{
		this->_parameterError = this->Maximal_Confidence(lst);
	}
	if (this->_errorEstimateStrategy == 1)
	{
		this->_parameterError = this->Gaussian_Confidence(lst);
	}
	this->Add_Confidence_Curves();
	this->Update_Relaxivity_Function(winner);
	this->Update_T2_Distribution(winner);

	this->_winGenetic->_plotterA->UnBlock();
	this->_winGenetic->_plotterB->UnBlock();
	this->_winGenetic->_optimizer->Unlock_Working_Thread();
	this->_winGenetic->_plotterA->Refresh();
	this->_winGenetic->_plotterB->Refresh();
}

void WinGeneticObserver::Update_Relaxivity_Function(const math_la::math_lac::genetic::Creature& c)
{
	this->_winGenetic->_plotterA->Set_Curve_Color(wxColor(180, 180, 180), 2);
	vector<scalar> vy(this->_curveSize, 0);
	vector<scalar> vx(this->_curveSize, 0);
	const rw::RelaxivityExperiment& ec = (const rw::RelaxivityExperiment&)c;
	for (int x = 0; x < this->_curveSize; ++x)
	{
		vx[x] = ((scalar)x)*this->_maxXi / (scalar)this->_curveSize;
		vy[x] = ec.Relaxivity_Distribution()(vx[x]);
	}
	for (int x = 0; x < this->_curveSize; ++x)
	{
		this->_winGenetic->_plotterA->Add_Curve_Point(2,vx[x], vy[x]);
	}
}

void WinGeneticObserver::Update_T2_Distribution(const math_la::math_lac::genetic::Creature& c)
{
	WxPlotter* plotterLaplace = this->_winGenetic->_plotterB;
	plotterLaplace->Erase_All_Curves();
	plotterLaplace->Set_Interval_Limits(0.01f, 1.0f, -0.001f, 0.001f);
	plotterLaplace->Add_Curves(2);
	plotterLaplace->Set_Curve_Color(wxColor(180, 180, 180), 1);
	plotterLaplace->Set_Curve_Color(Wx_Color(this->_winGenetic->_sim->Sim_Color()), 0);

	rw::RelaxivityExperiment& ge = (rw::RelaxivityExperiment&)c;
	math_la::math_lac::full::Vector Lt = ge.Laplace_Domain();
	math_la::math_lac::full::Vector Lf = ge.Laplace_Bins();

	for (int x = 0; x < Lt.Size(); ++x)
	{
		plotterLaplace->Add_Curve_Point(0,this->_refDomain(x), this->_refLaplace(x));
		plotterLaplace->Add_Curve_Point(1,Lt(x), Lf(x));
	}
}

void WinGeneticObserver::Add_Confidence_Curves()
{
	WxPlotter* plotterRhoFunction = this->_winGenetic->_plotterA;
	plotterRhoFunction->Set_Curve_Color(wxColor(255, 255, 150), 0);
	plotterRhoFunction->Set_Curve_Color(wxColor(225, 225, 150), 1);
	for (int i = 0; i < this->_curveSize; ++i)
	{
		WxPlotter::Pair p;
		p.x = this->_dom[i];
		p.y = this->_rhoMax[i];
		plotterRhoFunction->Add_Curve_Point(0,p);
		p.x = this->_dom[i];
		p.y = this->_rhoMin[i];
		plotterRhoFunction->Add_Curve_Point(1,p);
	}
}


void WinGeneticObserver::Fitting_Creature(const math_la::math_lac::genetic::Creature& creature, scalar fitness)
{
	wxString genes;
	for (int i = 0; i < this->_winGenetic->_optimizer->Phenotype_Size(); ++i)
	{
		scalar v = this->_winGenetic->_optimizer->Translate_To_Scalar(i, creature.Gene(i));
		genes << v << "  ;  ";
	}
	this->_winGenetic->_winText->Add_Info_Text("Creature genes: ");
	this->_winGenetic->_winText->Add_Info_Text(genes);
	this->_winGenetic->_winText->Add_Info_Text("Updating fitness -> " + wxString::FromDouble(fitness, 5));
}

void WinGeneticObserver::Conclude(list<const math_la::math_lac::genetic::Creature*> survivors)
{
	wxString dir_name = this->_winGenetic->_pgr->GetPropertyByName("G.dirp")->GetValue().GetString();
	dir_name = dir_name + this->_winGenetic->_name+ "\\";
	if (!wxDirExists(dir_name))
	{
		wxMkDir(dir_name);
	}
	wxString prefix = dir_name;
	list<const math_la::math_lac::genetic::Creature*>::iterator itr = survivors.begin();
	int i = 1;
	while ((itr != survivors.end())&&(i < 100))
	{
		
		const rw::RelaxivityExperiment* e = static_cast<const rw::RelaxivityExperiment*>(*itr);
		string report_file_name = string((prefix + wxString("\\Solution_") << i << "_")).c_str();
		rw::ExperimentReport rep(e, report_file_name);
		rep.Create_Decay_Report();
		rep.Create_Laplace_Report();
		rep.Create_Collision_Rate_Error_Report();
		rep.Create_Relaxivity_Distribution_Report();
		++itr;
		++i;
	}

	if (this->_prgdlg)
	{
		this->_prgdlg->Close();
		delete this->_prgdlg;
		this->_prgdlg = 0;
	}
}


WindowText::WindowText(wxWindow* parent)
	: wxDialog(parent,0,"Evolving creatures")
{
	this->SetSize(wxSize(600, 600));
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxTextCtrl* text = new wxTextCtrl(this,0,"",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
	sizer->Add(text, wxSizerFlags().Expand().Proportion(3));
	this->_info = text;
	text = new wxTextCtrl(this, 0, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	this->_low = text;
	sizer->Add(text, wxSizerFlags().Expand().Proportion(1));
	this->SetSizer(sizer);
}

void WindowText::Add_Info_Text(const wxString& text)
{
	this->_info->AppendText(text + "\n");	
}

void WindowText::Add_Low_Text(const wxString& text)
{
	this->_low->Clear();
	this->_low->AppendText(text);
	this->_low->Refresh();
}

WindowText::~WindowText()
{

}


WindowGenetic::WindowGenetic(wxWindow* parent)
	: wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "Genetic")
{

	this->_mainWindow = (WindowMain*)parent;
	this->_winText = 0;
	this->_eventHandler = new WinGeneticObserver(this);
	this->_mgr = new wxAuiManager(this);
	wxPropertyGrid* pg = new wxPropertyGrid(
		this,
		0,
		wxDefaultPosition,
		wxDefaultSize,
		wxPG_SPLITTER_AUTO_CENTER |
		wxPG_DEFAULT_STYLE);
	this->_pgr = pg;

	WxPlotter* plotterA = new WxPlotter(this);
	WxPlotter* plotterB = new WxPlotter(this);
	this->_plotterA = plotterA;
	this->_plotterB = plotterB;

	this->_plotterA->Set_Interval_Limits(0.0f, 1.0f, -0.0001f, 0.0001f);
	this->_plotterB->Set_Interval_Limits(0.0001f, 10.0f, -0.0001f, 0.0001f);
	this->_plotterB->Enable_Logarithmic_Scale_AxisX();

	this->_plotterA->Set_X_Title("Collision rate (XI)");
	this->_plotterA->Set_Y_Title("Surface relaxivity");
	this->_plotterA->Set_Title("Surface relaxivity estimate");

	this->_plotterB->Set_X_Title("T2(s)");
	this->_plotterB->Set_Y_Title("Bin");
	this->_plotterB->Set_Title("Laplace transform comparison");

	int s_width;
	int s_height;
	WindowMain* wm = static_cast<WindowMain*>(parent);
	int diff = wm->Ribbon_Size() + wm->Bar_Size();
	wxDisplaySize(&s_width, &s_height);
	s_width = s_width / 3;
	s_height = s_height - diff;
	this->_mgr->SetDockSizeConstraint(0.3, 0.36);
	this->_mgr->AddPane(pg, wxAuiPaneInfo().Bottom().BestSize(s_width, 36 * s_height / 100));
	this->_mgr->AddPane(plotterA, wxAuiPaneInfo().Center().Caption("Surface relaxivity function").BestSize(s_width, 16 * s_height / 100));
	this->_mgr->AddPane(plotterB, wxAuiPaneInfo().Center().Caption("Laplace match").BestSize(s_width, 16 * s_height / 100));
}

void WindowGenetic::Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar)
{
	wxMenu* menu = new wxMenu();
	wxRibbonPanel* ribbonPanel = new wxRibbonPanel(ribbonPage, wxID_ANY, wxT("Genetic search"), wxNullBitmap,
		wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);
	wxRibbonToolBar* btnBar = new wxRibbonToolBar(ribbonPanel);
	wxImage img;
	wxBitmap bmp;
	img.LoadFile("icons/start.png");
	img.Rescale(32, 32);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_APPLY, bmp, "Start genetic evolution");
	menu->Append(wxID_APPLY, "Start genetic evolution")->SetBitmap(bmp);
	img.LoadFile("icons/stop.png");
	img.Rescale(32, 32);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_STOP, bmp, "Stop evolution");
	menu->Append(wxID_STOP, "Stop evolution")->SetBitmap(bmp);

	this->Set_Optimization_Options_Grid();	
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowGenetic::Start, this, wxID_APPLY);
	menu->Bind(wxEVT_MENU, &WindowGenetic::Start, this, wxID_APPLY);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowGenetic::Stop, this, wxID_STOP);
	menu->Bind(wxEVT_MENU, &WindowGenetic::Stop, this, wxID_STOP);
	this->_pgr->Bind(wxEVT_PG_CHANGED, &WindowGenetic::Update_Grid, this);
}

void WindowGenetic::Update_Layout()
{
	this->_mgr->Update();
}

void WindowGenetic::Set_Optimization_Options_Grid()
{
	wxArrayString ss;
	ss.Clear();
	ss.Add("Sigmoid");
	ss.Add("Hat");
	wxPropertyGrid* pg = this->_pgr;
	wxPGProperty* st = 0;
	wxPGProperty* rhop = pg->Append(new wxStringProperty("Relaxivity distribution parameters", "P"));
	rhop->ChangeFlag(wxPG_PROP_READONLY, true);
	rhop->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pg->AppendIn(rhop, new wxEnumProperty("Basis shape functions", "bsf", ss));
	pg->AppendIn(rhop, new wxIntProperty("Number of shape functions", "nsf", 2));
	pg->AppendIn(rhop, new wxBoolProperty("Repeat walker paths", "PATH", true));
	pg->AppendIn(rhop, new wxIntProperty("Relaxivity function parameter cycle", "CYCLE", 0));
	st = pg->AppendIn(rhop, new wxStringProperty("Shape function parameters", "sfp"));
	st->ChangeFlag(wxPG_PROP_READONLY, true);
	st->ChangeFlag(wxPG_PROP_NOEDITOR, true);

	wxPGProperty* simp = pg->Append(new wxStringProperty("Simulation", "sim"));
	simp->ChangeFlag(wxPG_PROP_READONLY, true);
	simp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	ss.Clear();
	ss.Add("Collision histogram");
	ss.Add("Image based");
	pg->AppendIn(simp, new wxEnumProperty("Simulation type", "simt", ss));
	pg->AppendIn(simp, new wxIntProperty("Number of particles", "nprt", 1024));

	wxPGProperty* dd = pg->Append(new wxStringProperty("Optimization (Genetic algorithm)", "G"));
	dd->ChangeFlag(wxPG_PROP_READONLY, true);
	dd->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	ss.Clear();
	ss.Add("Correlation");
	ss.Add("Euclidean distance");
	pg->AppendIn(dd, new wxEnumProperty("Comparison metric", "met", ss));
	wxPGProperty* stprop = pg->AppendIn(dd, new wxStringProperty("Massive extinction", "mext"));
	stprop->ChangeFlag(wxPG_PROP_READONLY, true);
	stprop->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	this->Update_Extinction();
	pg->AppendIn(dd, new wxIntProperty("Number of agents (creatures)", "nc", 24));
	pg->AppendIn(dd, new wxIntProperty("Number of groups (islands)", "ng", 4));
	pg->AppendIn(dd, new wxIntProperty("Max number of iterations", "mi", 64));
	pg->AppendIn(dd, new wxIntProperty("Migration rate", "mr", 8));
	pg->AppendIn(dd, new wxFloatProperty("Migration percentage", "mp", 0.25));
	pg->AppendIn(dd, new wxFloatProperty("Mutation rate", "mut", 0.1));
	pg->AppendIn(dd, new wxFloatProperty("Mutation displacement", "mutd", 0.9));
	ss.Clear();
	ss.Add("Arithmetic");
	ss.Add("Uniform and arithmetic");
	wxPGProperty* cr = pg->AppendIn(dd, new wxEnumProperty("Crossover rule", "cr", ss));
	pg->AppendIn(cr, new wxFloatProperty("Min. crossover weight", "mincr", 0.0));
	pg->AppendIn(cr, new wxFloatProperty("Max. crossover weight", "maxcr", 0.5));
	pg->AppendIn(cr, new wxFloatProperty("Not arithmetic rate", "narate", 0.2));
	pg->AppendIn(dd, new wxFloatProperty("Incest relative error", "pi", 0.2));
	pg->AppendIn(dd, new wxFloatProperty("% Incestual preservation", "ipp", 0.25));
	pg->AppendIn(dd, new wxFloatProperty("Fitness error interval", "fefpe", 0.01));
	wxArrayString strstr;
	strstr.Add("Maximal");
	strstr.Add("Gaussian");
	pg->AppendIn(dd, new wxEnumProperty("Error estimate strategy", "estr", strstr));

	pg->AppendIn(dd, new wxDirProperty("Results directory", "dirp"));
	this->Update_Functions_Shape();
	this->Update_Aut_Params();
}

void WindowGenetic::Set_Result_Directory(const wxString& dir)
{
	wxPGProperty* pp = this->_pgr->GetPropertyByName("G.dirp");
	pp->SetValue(dir);
}

void WindowGenetic::Update_Functions_Shape()
{	
	wxPropertyGrid* pgr = this->_pgr;
	wxPGProperty* ps = pgr->GetPropertyByName("P.sfp");
	int nf = pgr->GetPropertyByName("P.nsf")->GetValue().GetInteger();
	ps->DeleteChildren();
	wxArrayString ss;
	ss.Add("No restriction");
	ss.Add("Monotonic increasing");
	ss.Add("Monotonic decreasing");
	pgr->AppendIn(ps, new wxEnumProperty("Restriction", "rs", ss));
	if (this->Get_Function_Type() == rw::RelaxivityDistribution::Sigmoid)
	{
		pgr->AppendIn(ps, new wxFloatProperty("Global max", "gmax", 80.0));
		pgr->AppendIn(ps, new wxFloatProperty("Transition", "gtrans", this->_eventHandler->_maxXi));
		pgr->AppendIn(ps, new wxFloatProperty("Slope scale", "slopesc", 1.0));
		wxPGProperty* sigpp = pgr->AppendIn(ps, new wxStringProperty("Sigmoid functions", "sigf"));
		sigpp->ChangeFlag(wxPG_PROP_READONLY, true);
		sigpp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		wxPGProperty* sigp = 0;
		for (int j = 0; j < nf; ++j)
		{
			sigp = pgr->AppendIn(sigpp, new wxStringProperty("Sigmoid " + wxString::Format("%d", j + 1), wxString("f") +
				wxString::Format("%d", j)));
			pgr->AppendIn(sigp, new wxFloatProperty("Min. asympt. Min.", "minminav", 0.0));
			pgr->AppendIn(sigp, new wxFloatProperty("Min. asympt. Max.", "minmaxav", 40.0));
			pgr->AppendIn(sigp, new wxFloatProperty("Max. asympt. Min.", "maxminav", 0.0));
			pgr->AppendIn(sigp, new wxFloatProperty("Max. asympt. Max.", "maxmaxav", 40.0));
			pgr->AppendIn(sigp, new wxFloatProperty("Min. slope", "minslope", 0.0));
			pgr->AppendIn(sigp, new wxFloatProperty("Max. slope", "maxslope", 800.0));
			pgr->AppendIn(sigp, new wxFloatProperty("Min. transition", "transmin", 0.0));
			pgr->AppendIn(sigp, new wxFloatProperty("Max. transition", "transmax", this->_eventHandler->_maxXi));
		}
	}
	if (this->Get_Function_Type() == rw::RelaxivityDistribution::Hat)
	{
		pgr->AppendIn(ps, new wxFloatProperty("Global max", "gmax", 60.0));
		pgr->AppendIn(ps, new wxFloatProperty("Transitions", "gtrans", this->_eventHandler->_maxXi));
		pgr->AppendIn(ps, new wxFloatProperty("Min. value", "minv", 0.1));
		wxPGProperty* hatpp = pgr->AppendIn(ps, new wxStringProperty("Hat functions", "hatf"));
		hatpp->ChangeFlag(wxPG_PROP_READONLY, true);
		hatpp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
		wxPGProperty* hatp = 0;
		for (int j = 0; j < nf; ++j)
		{
			hatp = pgr->AppendIn(hatpp, new wxStringProperty("Hat " + wxString::Format("%d", j + 1), wxString("f") +
				wxString::Format("%d", j)));
			pgr->AppendIn(hatp, new wxFloatProperty("Max. Amplitude", "amax", 60.0));
			pgr->AppendIn(hatp, new wxFloatProperty("Min. Amplitude", "amin", 0.0));
			wxPGProperty* pcut;
			pcut = pgr->AppendIn(hatp, new wxFloatProperty("Min. Cut minimum", "cminmin", 0.0));
			pcut = pgr->AppendIn(hatp, new wxFloatProperty("Min. Cut maximum", "cminmax", this->_eventHandler->_maxXi));
			pgr->AppendIn(pcut, new wxBoolProperty("Fixed", "bfmin", false));
			pcut = pgr->AppendIn(hatp, new wxFloatProperty("Max. Cut minimum", "cmaxmin", 0.0));
			pcut = pgr->AppendIn(hatp, new wxFloatProperty("Max. Cut maximum", "cmaxmax", this->_eventHandler->_maxXi));
			pgr->AppendIn(pcut, new wxBoolProperty("Fixed", "bfmax", false));
		}
	}
}

rw::RelaxivityDistribution::Shape WindowGenetic::Get_Function_Type() const
{
	rw::RelaxivityDistribution::Shape r = rw::RelaxivityDistribution::Sigmoid;
	wxPGProperty* pp = 0;
	pp = this->_pgr->GetPropertyByName("P.bsf");
	int shape = pp->GetValue().GetInteger();
	if (shape == 0)
	{
		r = rw::RelaxivityDistribution::Sigmoid;
	}
	if (shape == 1)
	{
		r = rw::RelaxivityDistribution::Hat;
	}
	return(r);
}


void WindowGenetic::Update_Aut_Params()
{
	wxPGProperty* pp = 0;
	pp = this->_pgr->GetPropertyByName("P.nsf");
	int n = pp->GetValue().GetInteger();
	pp = this->_pgr->GetPropertyByName("G.mp");
	scalar mp = pp->GetValue().GetDouble();
	rw::RelaxivityDistribution::Shape ft = this->Get_Function_Type();
	int nparam = 1;
	if (ft == rw::RelaxivityDistribution::Sigmoid)
	{
		nparam = 4 * n;
	}
	if (ft == rw::RelaxivityDistribution::Hat)
	{
		nparam = 3 * n;
	}
	pp = this->_pgr->GetPropertyByName("G.ng");
	int ni = pp->GetValue().GetInteger();
	int NC = (int)((scalar)nparam / ((scalar)1 - mp) + 2);
	pp = this->_pgr->GetPropertyByName("G.mr");
	pp->SetValue(NC);
	int NT = NC * ni;
	pp = this->_pgr->GetPropertyByName("G.nc");
	pp->SetValue(NT);
	pp = this->_pgr->GetPropertyByName("G.mi");
	pp->SetValue(NT * 15);
}

void WindowGenetic::Update_Extinction()
{
	wxPGProperty* pp = this->_pgr->GetPropertyByName("G.mext");
	pp->DeleteChildren();
	wxPGProperty* pm = this->_pgr->GetPropertyByName("G.met");
	int v = pm->GetValue().GetInteger();
	if (v == 0)
	{
		this->_pgr->AppendIn(pp, new wxFloatProperty("Apocalyptic correlation ", "apcorr", 0.99));
		this->_pgr->AppendIn(pp, new wxFloatProperty("% Apocalyptic incest", "uper", 0.80));
	}
	else
	{
		this->_pgr->AppendIn(pp, new wxFloatProperty("Apocalyptic distance ", "apdist", 0.01));
		this->_pgr->AppendIn(pp, new wxFloatProperty("% Apocalyptic incest ", "uper", 0.85));
	}
}

rw::RelaxivityOptimizer::Monotonic WindowGenetic::Get_Monotonic_Property()
{
	rw::RelaxivityOptimizer::Monotonic mon = rw::RelaxivityOptimizer::None;
	wxPGProperty* pp = 0;
	pp = this->_pgr->GetPropertyByName("P.sfp.rs");
	int vincr = pp->GetValue().GetInteger();
	if (vincr == 1)
	{
		mon = rw::RelaxivityOptimizer::Increase;
	}
	if (vincr == 2)
	{
		mon = rw::RelaxivityOptimizer::Decrease;
	}
	return(mon);
}


void WindowGenetic::Set_Box_Monotonic()
{
	scalar maxpsi = this->_eventHandler->_maxXi;
	wxPGProperty* pp = 0;
	pp = this->_pgr->GetPropertyByName("P.nsf");
	int nofshapes = pp->GetValue().GetInteger();
	for (int i = 0; i < nofshapes; ++i)
	{
		wxString bf = wxString("P.sfp.hatf.f") + wxString::Format("%d", i) + wxString(".");
		rw::RelaxivityOptimizer::Monotonic mt = this->Get_Monotonic_Property();
		if (mt == rw::RelaxivityOptimizer::Increase)
		{
			if (i == 0)
			{
				pp = this->_pgr->GetPropertyByName(bf + "cminmax");
				pp->SetValue(0);
				pp = this->_pgr->GetPropertyByName(bf + "cminmin");
				pp->SetValue(0);
				pp = this->_pgr->GetPropertyByName(bf + "cminmax.bfmin");
				pp->SetValue(true);
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmin");
				pp->SetValue(maxpsi);
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmax");
				pp->SetValue(maxpsi);
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmax.bfmax");
				pp->SetValue(true);
			}
			else
			{
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmin");
				pp->SetValue(maxpsi);
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmax");
				pp->SetValue(maxpsi);
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmax.bfmax");
				pp->SetValue(true);
			}
		}
		if (mt == rw::RelaxivityOptimizer::Decrease)
		{
			if (i == 0)
			{
				pp = this->_pgr->GetPropertyByName(bf + "cminmax");
				pp->SetValue(0);
				pp = this-> _pgr->GetPropertyByName(bf + "cminmin");
				pp->SetValue(0);
				pp = this->_pgr->GetPropertyByName(bf + "cminmax.bfmin");
				pp->SetValue(true);
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmin");
				pp->SetValue(maxpsi);
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmax");
				pp->SetValue(maxpsi);
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmax.bfmax");
				pp->SetValue(true);
			}
			else
			{
				pp = this->_pgr->GetPropertyByName(bf + "cminmin");
				pp->SetValue(0);
				pp = this->_pgr->GetPropertyByName(bf + "cminmax");
				pp->SetValue(0);
				pp = this->_pgr->GetPropertyByName(bf + "cminmax.bfmax");
				pp->SetValue(true);
			}
		}
	}
}



void WindowGenetic::Update_Grid(wxPropertyGridEvent& evt)
{
	wxPGProperty* pp = evt.GetProperty();
	if ((pp->GetName() == "P.sfp.rs"))
	{
		if (this->Get_Function_Type() == rw::RelaxivityDistribution::Hat)
		{
			this->Set_Box_Monotonic();
		}
	}
	if ((pp->GetName() == "P.bsf") || (pp->GetName() == "P.nsf"))
	{
		this->Update_Functions_Shape();
		this->Update_Aut_Params();
	}
	if (pp->GetName() == "G.ng")
	{
		this->Update_Aut_Params();
	}
	if (pp->GetName() == "G.met")
	{
		this->Update_Extinction();
	}
	if (pp->GetName() == "P.sfp.gtrans")
	{
		scalar tval = pp->GetValue().GetDouble();
		this->_eventHandler->_maxXi = tval;
		rw::RelaxivityDistribution::Shape ft = this->Get_Function_Type();
		pp = this->_pgr->GetPropertyByName("P.nsf");
		int nofshapes = pp->GetValue().GetInteger();
		if (ft == rw::RelaxivityDistribution::Sigmoid)
		{
			for (int i = 0; i < nofshapes; ++i)
			{
				wxString bf = wxString("P.sfp.sigf.f") + wxString::Format("%d", i) + wxString(".");
				pp = this->_pgr->GetPropertyByName(bf + "transmax");
				pp->SetValue(tval);
			}
		}
		if (ft == rw::RelaxivityDistribution::Hat)
		{
			for (int i = 0; i < nofshapes; ++i)
			{
				wxString bf = wxString("P.sfp.hatf.f") + wxString::Format("%d", i) + wxString(".");
				pp = this->_pgr->GetPropertyByName(bf + "cminmax");
				pp->SetValue(tval);
				pp = this->_pgr->GetPropertyByName(bf + "cmaxmax");
				pp->SetValue(tval);
			}
		}
	}
	if (pp->GetName() == "P.sfp.gmax")
	{
		scalar div = pp->GetValue().GetDouble();
		pp = this->_pgr->GetPropertyByName("P.nsf");
		int nofshapes = pp->GetValue().GetInteger();
		div = div / (scalar)nofshapes;
		rw::RelaxivityDistribution::Shape ft = this->Get_Function_Type();
		if (ft == rw::RelaxivityDistribution::Sigmoid)
		{
			for (int i = 0; i < nofshapes; ++i)
			{
				wxString bf = wxString("P.sfp.sigf.f") + wxString::Format("%d", i) + wxString(".");
				pp = this->_pgr->GetPropertyByName(bf + "minmaxav");
				pp->SetValue(div);
				pp = this->_pgr->GetPropertyByName(bf + "maxmaxav");
				pp->SetValue(div);
			}
		}
		if (ft == rw::RelaxivityDistribution::Hat)
		{
			for (int i = 0; i < nofshapes; ++i)
			{
				wxString bf = wxString("P.sfp.hatf.f") + wxString::Format("%d", i) + wxString(".");
				pp = this->_pgr->GetPropertyByName(bf + "amax");
				pp->SetValue(div);
			}
		}
	}
}

WindowGenetic::~WindowGenetic()
{
	this->_mgr->UnInit();
}

void WindowGenetic::Start(wxCommandEvent& evt)
{

	if (!this->_winText)
	{
		this->_winText = new WindowText(this);		
	}
	this->_winText->Show();
	this->_mainWindow->Prepare_Genetic_Inversion();
	if (this->_sim)
	{
		this->Update_Optimizer();
		this->_optimizer->Set_Mapping_Simulation(this->_sim);
		this->_optimizer->Set_Formation(this->_formation, false);
		this->_optimizer->Set_Event_Handler(this->_eventHandler);
		this->_optimizer->Start();
	}
	else
	{
		this->_winText->Close();
	}
}

void WindowGenetic::Stop(wxCommandEvent& evt)
{
	this->_optimizer->Stop();
	this->_eventHandler->_prgdlg = new wxGenericProgressDialog("Stopping last simulations", "Please wait");
	this->_eventHandler->_prgdlg->Show();
	this->_eventHandler->_prgdlg->Pulse("Executing....");
}

void WindowGenetic::Set_Simulation(rw::PlugPersistent* sim, rw::Plug* formation)
{
	this->_sim = sim;
	this->_formation = formation;
	this->_eventHandler->_refDomain = sim->Laplace_Time_Vector();
	this->_eventHandler->_refLaplace = sim->Laplace_Bin_Vector();
}

void WindowGenetic::Update_Population(bool update)
{
	wxPGProperty* pp = 0;
	pp = this->_pgr->GetPropertyByName("P.sfp.rs");
	int vincr = pp->GetValue().GetInteger();
	if (vincr == 1)
	{
		this->_optimizer->Set_Monotonic(rw::RelaxivityOptimizer::Increase);
	}
	if (vincr == 2)
	{
		this->_optimizer->Set_Monotonic(rw::RelaxivityOptimizer::Decrease);
	}
	pp = this->_pgr->GetPropertyByName("G.fefpe");
	scalar fefpe = pp->GetValue().GetDouble();
	this->_eventHandler->_fitnessPercentage = fefpe;
	pp = this->_pgr->GetPropertyByName("G.estr");
	this->_eventHandler->_errorEstimateStrategy = pp->GetValue().GetInteger();
	pp = this->_pgr->GetPropertyByName("G.met");
	int s = pp->GetValue().GetInteger();
	if (s == 0)
	{
		this->_optimizer->Set_Metric(rw::RelaxivityOptimizer::Correlation);
		pp = this->_pgr->GetPropertyByName("G.mext.apcorr");
		scalar apcorr = pp->GetValue().GetDouble();
		pp = this->_pgr->GetPropertyByName("G.mext.uper");
		scalar perc = pp->GetValue().GetDouble();
		this->_optimizer->Set_Apocalyptic_Fitness(apcorr);
		this->_optimizer->Set_Apocalyptic_Percentage(perc);
	}
	if (s == 1)
	{
		this->_optimizer->Set_Metric(rw::RelaxivityOptimizer::Euclidean);
		pp = this->_pgr->GetPropertyByName("G.mext.apdist");
		scalar apcorr = pp->GetValue().GetDouble();
		pp = this->_pgr->GetPropertyByName("G.mext.uper");
		scalar perc = pp->GetValue().GetDouble();
		this->_optimizer->Set_Apocalyptic_Fitness(apcorr);
		this->_optimizer->Set_Apocalyptic_Percentage(perc);
	}
	int n = 0;
	scalar dd = 0;
	scalar dd2 = 0;
	if (update)
	{
		pp = this->_pgr->GetPropertyByName("G.nc");
		n = pp->GetValue().GetInteger();
		this->_optimizer->Set_Population_Size(n);
		pp->ChangeFlag(wxPG_PROP_READONLY, true);
		pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);

		pp = this->_pgr->GetPropertyByName("G.ng");
		n = pp->GetValue().GetInteger();
		this->_optimizer->Set_Number_Of_Islands(n);
		pp->ChangeFlag(wxPG_PROP_READONLY, true);
		pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	}
	pp = this->_pgr->GetPropertyByName("G.mi");
	n = pp->GetValue().GetInteger();
	this->_optimizer->Set_Maximal_Number_Of_Generations(n);
	pp = this->_pgr->GetPropertyByName("G.mr");
	n = pp->GetValue().GetInteger();
	this->_optimizer->Set_Migration_Rate(n);
	pp = this->_pgr->GetPropertyByName("G.mp");
	dd = pp->GetValue().GetDouble();
	this->_optimizer->Set_Migration_Percentage(dd);
	pp = this->_pgr->GetPropertyByName("G.mut");
	dd = pp->GetValue().GetDouble();
	this->_optimizer->Set_Mutation_Probability(dd);
	pp = this->_pgr->GetPropertyByName("G.mutd");
	dd = pp->GetValue().GetDouble();
	this->_optimizer->Set_Mutation_Displacement_Factor(dd);
	pp = this->_pgr->GetPropertyByName("G.cr");
	n = pp->GetValue().GetInteger();
	if (n == 0)
	{
		this->_optimizer->Set_Crossover_Rule(rw::RelaxivityOptimizer::CrossOver::WholeArithmetic);
	}
	if (n == 1)
	{
		this->_optimizer->Set_Crossover_Rule(rw::RelaxivityOptimizer::CrossOver::Whole_Uniform);
	}
	pp = this->_pgr->GetPropertyByName("G.cr.mincr");
	dd = pp->GetValue().GetDouble();
	pp = this->_pgr->GetPropertyByName("G.cr.maxcr");
	dd2 = pp->GetValue().GetDouble();
	this->_optimizer->Set_Weight_Interval(dd, dd2);
	pp = this->_pgr->GetPropertyByName("G.cr.narate");
	dd = pp->GetValue().GetDouble();
	this->_optimizer->Set_Non_Arithmetic_Rate(dd);
	pp = this->_pgr->GetPropertyByName("G.pi");
	dd = pp->GetValue().GetDouble();
	this->_optimizer->Set_Incestual_Relative_Error(dd);
	pp = this->_pgr->GetPropertyByName("G.ipp");
	dd = pp->GetValue().GetDouble();
	this->_optimizer->Set_Incestual_Preservation(dd);
}


void WindowGenetic::Update_Optimizer()
{
	bool update = false;
	if (this->_optimizer)
	{
		this->_optimizer->Regenerate();
	}
	else
	{
		this->_optimizer = new rw::RelaxivityOptimizer();
		update = true;
	}
	this->_optimizer->Set_Mapping_Simulation(this->_sim);
	wxPGProperty* pp = 0;
	pp = this->_pgr->GetPropertyByName("P.nsf");
	int nofshapes = pp->GetValue().GetInteger();
	pp = this->_pgr->GetPropertyByName("P.PATH");
	bool repeat = pp->GetValue().GetBool();
	pp = this->_pgr->GetPropertyByName("P.CYCLE");
	int cycle = pp->GetValue().GetInteger();
	this->_optimizer->Repeat_Paths(repeat, this->_sim->SimulationParams().Get_Value(SEED));
	this->_optimizer->Set_Cycle(cycle);
	if (this->Get_Function_Type() == rw::RelaxivityDistribution::Sigmoid)
	{
		pp = this->_pgr->GetPropertyByName("P.sfp.gmax");
		scalar size = pp->GetValue().GetDouble();
		pp = this->_pgr->GetPropertyByName("P.sfp.gtrans");
		scalar psi = pp->GetValue().GetDouble();
		this->_eventHandler->_maxXi = psi;
		pp = this->_pgr->GetPropertyByName("P.sfp.slopesc");
		scalar scale = pp->GetValue().GetDouble();

		this->_optimizer->Configure_Sigmoids(nofshapes, size, psi);
		this->_optimizer->Set_Slope_Scale(scale);

		for (int i = 0; i < nofshapes; ++i)
		{
			wxString bf = wxString("P.sfp.sigf.f") + wxString::Format("%d", i) + wxString(".");
			pp = this->_pgr->GetPropertyByName(bf + "minminav");
			scalar minmin = pp->GetValue().GetDouble();
			pp = this->_pgr->GetPropertyByName(bf + "minmaxav");
			scalar minmax = pp->GetValue().GetDouble();
			pp = this->_pgr->GetPropertyByName(bf + "maxminav");
			scalar maxmin = pp->GetValue().GetDouble();
			pp = this->_pgr->GetPropertyByName(bf + "maxmaxav");
			scalar maxmax = pp->GetValue().GetDouble();
			this->_optimizer->Set_Sigmoid_Min_Max(minmin, minmax, maxmin, maxmax, i);

			pp = this->_pgr->GetPropertyByName(bf + "minslope");
			scalar minslope = pp->GetValue().GetDouble();
			pp = this->_pgr->GetPropertyByName(bf + "maxslope");
			scalar maxslope = pp->GetValue().GetDouble();
			this->_optimizer->Set_Sigmoid_Slope_Min_Max(minslope, maxslope, i);

			pp = this->_pgr->GetPropertyByName(bf + "transmin");
			scalar mincut = pp->GetValue().GetDouble();
			pp = this->_pgr->GetPropertyByName(bf + "transmax");
			scalar maxcut = pp->GetValue().GetDouble();
			this->_optimizer->Set_Sigmoid_Cuts(mincut, maxcut, i);
		}
	}
	if (this->Get_Function_Type() == rw::RelaxivityDistribution::Hat)
	{
		pp = this->_pgr->GetPropertyByName("P.sfp.gmax");
		scalar size = pp->GetValue().GetDouble();
		pp = this->_pgr->GetPropertyByName("P.sfp.gtrans");
		scalar psi = pp->GetValue().GetDouble();
		this->_eventHandler->_maxXi = psi;
		pp = this->_pgr->GetPropertyByName("P.sfp.minv");
		scalar mink = pp->GetValue().GetDouble();
		this->_optimizer->Configure_Hats(nofshapes, size, psi);
		this->_optimizer->Set_Hat_Line(mink);
		for (int i = 0; i < nofshapes; ++i)
		{
			scalar vmin = 0;
			scalar vmax = 0;
			scalar vmin2 = 0;
			scalar vmax2 = 0;
			wxString bf = wxString("P.sfp.hatf.f") + wxString::Format("%d", i) + wxString(".");
			pp = this->_pgr->GetPropertyByName(bf + "amax");
			vmax = pp->GetValue().GetDouble();
			pp = this->_pgr->GetPropertyByName(bf + "amin");
			vmin = pp->GetValue().GetDouble();
			this->_optimizer->Set_Hat_Amplitude(vmin, vmax, i);
			pp = this->_pgr->GetPropertyByName(bf + "cminmin");
			vmin = pp->GetValue().GetDouble();
			pp = this->_pgr->GetPropertyByName(bf + "cminmax");
			vmax = pp->GetValue().GetDouble();
			pp = this->_pgr->GetPropertyByName(bf + "cmaxmin");
			vmin2 = pp->GetValue().GetDouble();
			pp = this->_pgr->GetPropertyByName(bf + "cmaxmax");
			vmax2 = pp->GetValue().GetDouble();
			this->_optimizer->Set_Hat_Cuts(vmin, vmax, vmin2, vmax2, i);
			pp = this->_pgr->GetPropertyByName(bf + "cmaxmax.bfmax");
			bool s;
			s = pp->GetValue().GetBool();
			if (s)
			{
				this->_optimizer->Fix_Hat_Cut_Max(vmax2, i);
			}
			pp = this->_pgr->GetPropertyByName(bf + "cminmax.bfmin");
			s = pp->GetValue().GetBool();
			if (s)
			{
				this->_optimizer->Fix_Hat_Cut_Min(vmax, i);
			}
		}
	}
	this->Update_Population(update);
	if (this->Simulated())
	{
		pp = this->_pgr->GetPropertyByName("sim.nprt");
		int np = pp->GetValue().GetInteger();
		this->_optimizer->Simulate_Walk(true, np);
	}
	else
	{
		this->_optimizer->Simulate_Walk(false);
	}
	this->_sim->Fill_Relaxivity_Optimizer_Parameters(*this->_optimizer);
	this->_optimizer->Set_Event_Handler(this->_eventHandler);
	this->_optimizer->Set_Formation(this->_formation, false);
}

bool WindowGenetic::Simulated() const
{
	bool r = false;
	wxPGProperty* p = this->_pgr->GetPropertyByName("sim.simt");
	int v = p->GetValue().GetInteger();
	if (v == 0)
	{
		r = true;
	}
	return(r);
}

void WindowGenetic::Set_Name(const wxString& name)
{
	this->_name = name;
}
