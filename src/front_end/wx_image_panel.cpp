#include <algorithm>
#include <wx/graphics.h>
#include <wx/dcsvg.h>
#include <wx/dcbuffer.h>
#include "wx_image_panel.h"

WxImagePanel::WxImagePanel(wxWindow* Parent, wxWindowID Id, const wxPoint& Position, const wxSize& Size, long Style) :
			wxControl(Parent, Id, Position, Size, Style)
{
	this->SetBackgroundStyle(wxBG_STYLE_PAINT);
	this->_border = 64;
	this->_colorMapBorder = 32;
	this->_drawColorMap = true;
	this->_width = this->GetSize().x;
	this->_height = this->GetSize().y;
	this->Bind(wxEVT_PAINT, &WxImagePanel::On_Paint, this);
	this->Bind(wxEVT_SIZE, &WxImagePanel::On_Resize, this);
	this->_imageLoaded = false;
	this->_rescale = true;
	this->_borderPen.SetColour(wxColor(180, 180, 250));
	this->_borderPen.SetWidth(1);
}

void WxImagePanel::On_Paint(wxPaintEvent & evt)
{
	wxAutoBufferedPaintDC dc(this);
	Render_Event(dc);
}

void WxImagePanel::Reset()
{
	wxClientDC dc(this);
	Render_Event(dc);
}

float WxImagePanel::Factor() const
{
	return(this->_factor);
}

void WxImagePanel::On_Resize(wxSizeEvent& evt)
{
	if ((this->_imageLoaded) && (this->_rescale))
	{
		this->Set_Image(this->_image);
	}
}

void WxImagePanel::Set_Image(const wxImage& img)
{
	if (&this->_image != &img)
	{
		this->_image = img;
	}
	if (this->_rescale)
	{
		int numx = this->GetSize().x - 35*(this->_border  + this->_colorMapBorder)/20;
		int denx = this->_image.GetSize().x;

		int numy = this->GetSize().y - 35*this->_border/20;
		int deny = this->_image.GetSize().y;

		float factor = std::min((float)numx / (float)denx, (float)numy / (float)deny);

		this->_height = std::max((int)(factor * (float)this->_image.GetSize().y),1);
		this->_width = std::max((int)(factor * (float)this->_image.GetSize().x),1);		
		this->_imageBuffer = wxBitmap(this->_image.Scale(this->_width, this->_height, wxIMAGE_QUALITY_NORMAL));
		this->_imageLoaded = true;
	}
	else
	{
		this->_imageLoaded = true;
		this->_imageBuffer = wxBitmap(this->_image);
	}
}

void WxImagePanel::Set_Color_Map(const WxColorMap& cmp)
{
	cmp.Populate_Color_Map(this->_colorMap);
	this->_drawColorMap = true;
}

void WxImagePanel::Draw_Color_Map(int x, int y, int sx, int sy, wxDC& dc)
{
	int n = this->_colorMap.size();
	if (n > 1)
	{
		wxFont font;
		font.SetSymbolicSize(wxFONTSIZE_SMALL);
		dc.SetFont(font);

		dc.DrawRectangle(wxPoint(x, y), wxSize(sx, sy));
		std::map<uint, StrColor>::const_iterator itr = this->_colorMap.begin();
		wxPen pen;
		int chh = dc.GetCharHeight();
		int height = (sy-chh)/n;
		int yy = y + 2*chh/3;
		int ty = yy - chh;
		for (int k = 0; k < this->_colorMap.size(); ++k)
		{
			pen.SetColour(itr->second.color);
			dc.SetBrush(itr->second.color);
			dc.SetPen(pen);
			dc.DrawRectangle(wxPoint(x + 8, yy), wxSize(32, height));
			if (yy >= ty + chh)
			{
				dc.DrawText(itr->second.legend,wxPoint(x + 45, yy));
				ty = yy;
			}
			yy = yy + height;			
			++itr;
		}
	}
}


void WxImagePanel::Render_Event(wxDC& dc)
{
	if (this->_imageLoaded)
	{
		int x = (this->GetSize().x-this->_border - this->_imageBuffer.GetSize().x)/2+this->_border/2-this->_colorMapBorder;
		int y = (this->GetSize().y-this->_border - this->_imageBuffer.GetSize().y)/2+this->_border/2;
		dc.SetPen(this->_borderPen);
		dc.GradientFillLinear(wxRect(0, 0, this->GetSize().x, this->GetSize().y), wxColour(255, 255, 255), wxColour(230, 230, 230), wxSOUTH);
		dc.DrawRectangle(wxPoint(x-2, y-2), wxSize(this->_imageBuffer.GetSize().x+3, this->_imageBuffer.GetSize().y + 3));
		dc.DrawBitmap(this->_imageBuffer, x, y, false);
		if (this->_drawColorMap)
		{
			dc.SetPen(this->_borderPen);
			dc.SetBrush(wxColor(255, 255, 255));
			this->Draw_Color_Map(x + this->_imageBuffer.GetSize().x + 8, y - 2,100, this->_imageBuffer.GetSize().y / 2,dc);
		}
	}
	else
	{
		dc.GradientFillLinear(wxRect(0, 0, this->GetSize().x, this->GetSize().y), wxColour(255, 255, 255), wxColour(230, 230, 230), wxDOWN);
	}
}

void WxImagePanel::Rescale_Image(bool sc)
{
	this->_rescale = sc;
}
