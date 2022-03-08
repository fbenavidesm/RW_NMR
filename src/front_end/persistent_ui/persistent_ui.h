#ifndef WX_SIM_H

#include "wx/wx.h"
#include "rw/persistence/plug_persistent.h"

class WxSim
{
private:
	rw::PlugPersistent* _sim;
	static wxImage* ref_icon;
public:
	WxSim(rw::PlugPersistent& sim);
	wxDateTime Date_Time() const;
	wxImage Sim_Icon();
	static wxImage Sim_Icon(const wxColor& color);
	static wxDateTime Date_Time(uint dt);
	const rw::PlugPersistent& Sim() const;
	void Set_Date_Time(const wxDateTime& dt);
	wxColor Sim_Color() const;
	void Set_Image_Path(const wxString& path);
	void Save_File(const wxString& path);
	void Set_Sim_Color(const wxColor& color);
	void Load_Header(const wxString filename);
};
#endif
