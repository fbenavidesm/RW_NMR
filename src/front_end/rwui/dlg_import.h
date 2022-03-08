#ifndef DLG_IMPORT_H
#define DLG_IMPORT_H

#include <vector>
#include <string>
#include "wx/wx.h"
#include "wx/grid.h"
#include "front_end/wx_plotter.h"
#include "math_la/mdefs.h"
#include "rw/persistence/plug_persistent.h"

using std::vector;

class DlgImport : public wxDialog
{
private:
	WxPlotter* _plotterDecay;
	wxGrid* _grid;
	int _idc;
	int _colSize;
	int _idReject;
	int _samples;
	wxListBox* _timeCombo;
	wxListBox* _decayCombo;
	wxListBox* _imgCombo;
	wxListBox* _realCombo;
	wxListBox* _orthoCombo;

	std::string _separators;
	vector <wxString> _header;
	vector<vector<float>> _data;
	vector<scalar> _mgn;
	vector<scalar> _time;
	vector<scalar> _noise;
	int _timeUnits;
	void Find_Header(const wxString& filename);
	void Collect_Data(const wxString& filename);
	void PopulateUI();
	void Pick_Time_Mgn(int& time, int& magn);
	void Set_Time_Mgn(int time, int magn);
	void Rotate(int time, int img, int real);
	void On_Rotate_Button(wxCommandEvent& evt);
	void OnChoose(wxCommandEvent& evt);
public:
	DlgImport(wxWindow* parent, const wxString& Title = "Decay file selector");
	bool Select_File(wxString& filename);
	void Analyze_File(const wxString& filename);
	void Fill_Sim(rw::PlugPersistent& sim);
	~DlgImport();
};



#endif

