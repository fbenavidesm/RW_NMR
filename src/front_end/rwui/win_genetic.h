#ifndef WIN_GENETIC_H
#define WIN_GENETIC_H

#include <list>
#include <set>
#include <vector>
#include "wx/wx.h"
#include "wx/ribbon/bar.h"
#include "wx/ribbon/buttonbar.h"
#include "wx/ribbon/toolbar.h"
#include "wx/propgrid/propgrid.h"
#include "wx/propgrid/manager.h"
#include "wx/propgrid/property.h"
#include "wx/propgrid/advprops.h"
#include "wx/aui/aui.h"
#include "wx/progdlg.h"
#include "front_end/wx_plotter.h"
#include "rw/relaxivity_distribution.h"
#include "rw/relaxivity_optimizer.h"
#include "math_la/math_lac/genetic/creature.h"
#include "math_la/math_lac/genetic/population.h"
#include "math_la/math_lac/full/vector.h"

using std::list;
using std::set;
using std::vector;

class WindowGenetic;

class WinGeneticObserver : public math_la::math_lac::genetic::Population::Observer
{
private:
	friend class WindowGenetic;
	scalar _maxXi;
	scalar _fitnessPercentage;
	int _errorEstimateStrategy;
	WindowGenetic* _winGenetic;
	scalar _weightFitness;

	vector<scalar> _K;
	vector<scalar> _A;
	vector<scalar> _xo;
	vector<scalar> _B;

	vector <scalar> _xmin;
	vector <scalar> _xmax;

	vector<scalar> _dom;
	vector<scalar> _rhoMax;
	vector<scalar> _rhoMin;

	set<int> _islandSet;
	int _curveSize;
	scalar _parameterError;

	math_la::math_lac::full::Vector _refLaplace;
	math_la::math_lac::full::Vector _refDomain;

	scalar Maximal_Confidence(list<math_la::math_lac::genetic::Creature*>& lst);
	scalar Gaussian_Confidence(list<math_la::math_lac::genetic::Creature*>& lst);

	void Update_Relaxivity_Function(const math_la::math_lac::genetic::Creature& c);
	void Update_T2_Distribution(const math_la::math_lac::genetic::Creature& c);
	void Add_Confidence_Curves();
	wxGenericProgressDialog* _prgdlg;
public:
	WinGeneticObserver(WindowGenetic* parent);
	void Executing_Simulations(int current, int totsize);
	void Diversifying_Island(int i);
	void Create_Offspring(int it);
	void Update_Winners(const math_la::math_lac::genetic::Creature& winner);
	void Fitting_Creature(const math_la::math_lac::genetic::Creature& creature, scalar fitness);
	void Conclude(list<const math_la::math_lac::genetic::Creature*> survivors);
	~WinGeneticObserver();
};

class WindowText : public wxDialog
{
private:
	wxTextCtrl* _info;
	wxTextCtrl* _low;
public:
	WindowText(wxWindow* parent);
	~WindowText();
	void Add_Info_Text(const wxString& text);
	void Add_Low_Text(const wxString& text);
};

class WindowMain;

class WindowGenetic : public wxWindow
{
private:
	friend class WinGeneticObserver;
	wxString _name;
	wxPropertyGrid * _pgr;
	wxAuiManager* _mgr;
	WxPlotter* _plotterA;
	WxPlotter* _plotterB;
	WinGeneticObserver* _eventHandler;
	rw::RelaxivityOptimizer* _optimizer;
	rw::PlugPersistent* _sim;
	rw::Plug* _formation;
	WindowText* _winText;
	WindowMain* _mainWindow;	

	void Update_Functions_Shape();
	void Update_Aut_Params();
	rw::RelaxivityDistribution::Shape Get_Function_Type() const;
	void Update_Grid(wxPropertyGridEvent& evt);
	void Update_Extinction();
	void Set_Box_Monotonic();
	rw::RelaxivityOptimizer::Monotonic Get_Monotonic_Property();
	void Update_Optimizer();
	void Update_Population(bool update);
	bool Simulated() const;
public:
	WindowGenetic(wxWindow* parent);
	void Set_Optimization_Options_Grid();
	void Start(wxCommandEvent& evt);
	void Stop(wxCommandEvent& evt);
	void Set_Simulation(rw::PlugPersistent* sim, rw::Plug* formation);
	void Set_Name(const wxString& name);
	void Set_Result_Directory(const wxString& dir);
	void Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar);
	void Update_Layout();
	~WindowGenetic();
};



#endif