#ifndef WX_IMAGE_BRIDGE_H
#define WX_IMAGE_BRIDGE_H

#include <map>
#include "wx/wx.h"
#include "rw/binary_image/binary_image.h"

using std::map;
class WxImagePanel;


struct StrColor
{
	wxString legend;
	wxColor color;
};


/**
* An image bridge to the rw::BinaryImage::ImageBridge to set properties to wxImage
*/
class WxImageAdapter : public rw::BinaryImage::ImageAdapter
{
private:
	/**
	* Local wxImage to set data
	*/
	wxImage* _wxImg;

	/**
	* TRUE if the memory must be locally released
	*/
	bool _local;
public:
	WxImageAdapter();
	WxImageAdapter(uint width, uint height);
	WxImageAdapter(wxImage& img);
	~WxImageAdapter();
	void Reserve_Memory(uint width, uint height);
	rw::RGBColor operator()(uint x, uint y) const;
	void operator()(uint x, uint y, const rw::RGBColor& color);
	/**
	* @return The constant wxImage property
	*/
	const wxImage& Image() const;
};

class WxColorMap : public rw::BinaryImage::ColorMap
{
protected:
	friend class WxImagePanel;
	void Populate_Color_Map(map<uint,StrColor>& cmp) const;
public:
	WxColorMap();
};


#endif
