#ifndef RGBCOLOR_BRIDGE
#define RGBCOLOR_BRIDGE

#include <wx/wx.h>
#include "rw/binary_image/rgb_color.h"

wxColour Wx_Color(const rw::RGBColor& color);
rw::RGBColor Rw_Color(const wxColor& color);

inline wxColour Wx_Color(const rw::RGBColor& color)
{
	wxColor rcol(color.Red(), color.Green(), color.Blue());
	return(rcol);
}

inline rw::RGBColor Rw_Color(const wxColor& color)
{
	rw::RGBColor rcol(color.Red(), color.Green(), color.Blue());
	return(rcol);
}


#endif