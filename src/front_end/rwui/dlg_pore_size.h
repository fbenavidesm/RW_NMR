#ifndef WIN_PORE_SIZE_H
#define WIN_PORE_SIZE_H

#include "wx/wx.h"
#include "wx/arrstr.h"
#include "wx/propgrid/propgrid.h"
#include "front_end/wx_plotter.h"
#include "rw/persistence/plug_persistent.h"

class WindowMain;

class DlgPoreSize : public wxDialog
{
private:
	wxPropertyGrid* _pg;
	WxPlotter* _plot;
	const rw::PlugPersistent* _baseSim;
	rw::Sigmoid* _sigmoid;
	scalar _rhoMin;
	scalar _rhoMax;
	WindowMain* _mainWindow;
	rw::Sigmoid* Create_Sigmoid();	
	void Update(wxPropertyGridEvent& evt);
	void Draw_Function();
public:
	DlgPoreSize(wxWindow* Parent = NULL, wxWindowID Id = -1, const wxPoint& Position = wxDefaultPosition,
		const wxSize& Size = wxDefaultSize, long Style = 0);
	void Dimension_Functions();
	void Create_Distribution();	
	void Create(wxCommandEvent& evt);
	void Set_Simulation(const rw::PlugPersistent& sim);
};



#endif