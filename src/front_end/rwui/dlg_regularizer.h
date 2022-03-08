#ifndef DLG_REGULARIZER_H
#define DLG_REGULARIZER_H

#include "math_la/mdefs.h"
#include "math_la/math_lac/full/vector.h"
#include "wx/wx.h"
#include "front_end/wx_plotter.h"
#include "rw/persistence/plug_persistent.h"
#include "wx/slider.h"
#include "rw/exponential_fitting.h"

class DlgRegularizer : public wxDialog
{
private:
	WxPlotter* _leftPlotter;
	WxPlotter* _rightPlotter;
	wxSlider* _regSlider;
	wxStaticText* _textRegularizer;
	
	math_la::math_lac::full::Vector _lambdas;
	math_la::math_lac::full::Vector _curvatureVector;
	math_la::math_lac::full::Vector _fx;
	math_la::math_lac::full::Vector _fy;


	scalar _lambdaMin;
	scalar _lambdaMax;
	int _laplaceResolution;
	int _timeReduction;
	int _regularizerResolution;
	scalar _sigmoidCut;
	int _sigmoidMultiplier;
	bool _OKBtnPressed;
	wxString _regularizerDataFileName;
	scalar _regularizerStep;
	int _criterionIndex;
	scalar _lCurveStabilizingFactor;
	int _lCurveMagnitudeInteger;
	bool _compensate;
	rw::ExponentialFitting _exponentialFitting;
	scalar _regularizer;
	int _decision;

	int Get_L_Selection();
	int Get_Sigma_Selection();
	void Select_Index(int id);
	void On_Scroll(wxScrollEvent& evt);
	void On_Ok_Button(wxCommandEvent& evt);
	void On_Close_Button(wxCommandEvent& evt);
	void On_Save_Button(wxCommandEvent& evt);
public:
	DlgRegularizer(wxWindow* parent, const wxString& Title = "Regularizer selector");
	void Set_Simulation(const rw::PlugPersistent& sim);
	scalar Regularizer() const;
	int Decision() const;
	~DlgRegularizer();
};


#endif
