#include "persistent_ui.h"
#include "rw/binary_image/rgb_color.h"
#include "front_end/wx_rgbcolor.h"

wxImage* WxSim::ref_icon = 0;

WxSim::WxSim(rw::PlugPersistent& sim)
{
	if (!WxSim::ref_icon)
	{
		WxSim::ref_icon = new wxImage();
		WxSim::ref_icon->LoadFile("icons//b16.png");
	}
	this->_sim = &sim;
}

wxDateTime WxSim::Date_Time() const
{
	wxDateTime r;
	r.SetFromDOS(this->_sim->Date_Time());
	return(r);
}

wxDateTime WxSim::Date_Time(uint dt)
{
	wxDateTime r;
	r.SetFromDOS(dt);
	return(r);
}

wxColor WxSim::Sim_Color() const
{
	return(Wx_Color(this->_sim->Sim_Color()));
}

wxImage WxSim::Sim_Icon(const wxColor& color)
{
	wxImage r = *WxSim::ref_icon;
	for (int x = 0; x < r.GetSize().x; ++x)
	{
		for (int y = 0; y < r.GetSize().y; ++y)
		{
			if (r.GetRed(x, y) == 0)
			{
				r.SetRGB(x, y, color.Red(), color.Green(), color.Blue());
			}
		}
	}
	return(r);
}

wxImage WxSim::Sim_Icon()
{
	wxImage r = *WxSim::ref_icon;
	rw::RGBColor color = this->_sim->Sim_Color();
	for (int x = 0; x < r.GetSize().x; ++x)
	{
		for (int y = 0; y < r.GetSize().y; ++y)
		{
			if (r.GetRed(x, y) == 0)
			{
				r.SetRGB(x, y, color.Red(), color.Green(), color.Blue());
			}
		}
	}
	return(r);
}

const rw::PlugPersistent& WxSim::Sim() const
{
	return(*this->_sim);
}

void WxSim::Set_Image_Path(const wxString& path)
{
	this->_sim->Set_Image_Path(string(path.c_str()));
}

void WxSim::Save_File(const wxString& path)
{
	this->_sim->Save_To_File(string(path.c_str()));
}

void WxSim::Set_Sim_Color(const wxColor& color)
{
	this->_sim->Set_Sim_Color(Rw_Color(color));
}

void WxSim::Set_Date_Time(const wxDateTime& dt)
{
	uint idt = dt.GetAsDOS();
	this->_sim->Set_Date_Time(idt);
}
void WxSim::Load_Header(const wxString filename)
{
	this->_sim->Load_Header(string(filename.c_str()));
}

