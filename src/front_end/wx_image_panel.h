#ifndef IMG_PNL_H
#define IMG_PNL_H

#include <wx/wx.h>
#include <map>
#include "wx_image_adapter.h"

class WxImagePanel : public wxControl
{
private:
	wxImage _image;
	wxBitmap _imageBuffer;
	int _width;
	int _height;
	bool _imageLoaded;
	bool _rescale;
	float _factor;
	int _border;
	int _colorMapBorder;
	bool _drawColorMap;
	wxPen _borderPen;
	std::map<uint, StrColor> _colorMap;
protected:
	void Render_Event(wxDC& dc);
	void On_Paint(wxPaintEvent & evt);
	void Reset();
	void On_Resize(wxSizeEvent& evt);
	void On_Erase_Background(wxEraseEvent& evt){};
	void Draw_Color_Map(int x, int y, int sx, int sy, wxDC& dc);
public:
	WxImagePanel(wxWindow* Parent = NULL, wxWindowID Id = -1, const wxPoint& Position = wxDefaultPosition,
		const wxSize& Size = wxDefaultSize, long Style = 0);
	void Set_Image(const wxImage& img);
	void Rescale_Image(bool sc);
	float Factor() const;
	void Set_Color_Map(const WxColorMap& cmp);
};

#endif