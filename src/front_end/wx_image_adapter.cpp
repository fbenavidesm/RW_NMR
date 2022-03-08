#include "wx_image_adapter.h"


WxColorMap::WxColorMap() : rw::BinaryImage::ColorMap()
{

}

void WxColorMap::Populate_Color_Map(map<uint, StrColor>& cmp) const
{
	cmp.clear();
	const map<uint, rw::RGBColor>& omp = this->Color_Map();
	map<uint, rw::RGBColor>::const_iterator itr = omp.begin();
	while (itr != omp.end())
	{
		rw::RGBColor bcol = itr->second;
		wxColor color(bcol.Red(), bcol.Green(), bcol.Blue());
		wxString leg = wxString::Format("%i", itr->first);
		leg = leg + " px/vx";
		StrColor el;
		el.legend = leg;
		el.color = color;
		cmp.insert(std::pair<uint,StrColor>(itr->first,el));
		++itr;
	}
}

WxImageAdapter::WxImageAdapter() :rw::BinaryImage::ImageAdapter()
{
	this->_wxImg = 0;
	this->_local = false;
}

WxImageAdapter::~WxImageAdapter()
{
	if ((this->_local) && (this->_wxImg))
	{
		delete this->_wxImg;
	}
}

WxImageAdapter::WxImageAdapter(uint width, uint height) : rw::BinaryImage::ImageAdapter(width, height)
{
	this->Reserve_Memory(width, height);
	this->_local = true;
}

WxImageAdapter::WxImageAdapter(wxImage& img)
{
	this->Set_Size(img.GetSize().x, img.GetSize().y);
	this->_wxImg = &img;
	this->_local = false;
}

const wxImage& WxImageAdapter::Image() const
{
	return(*this->_wxImg);
}

void WxImageAdapter::Reserve_Memory(uint width, uint height)
{
	this->Set_Size(width, height);
	if ((this->_local) && (this->_wxImg))
	{
		delete this->_wxImg;
	}
	this->_wxImg = new wxImage();
	this->_wxImg->Create(width, height);
	this->_local = true;
}

rw::RGBColor WxImageAdapter::operator()(uint x, uint y) const
{
	rw::RGBColor 
		r(this->_wxImg->GetRed(x, y), this->_wxImg->GetGreen(x, y), 
			this->_wxImg->GetBlue(x, y));
	return(r);
}

void WxImageAdapter::operator()(uint x, uint y, const rw::RGBColor& color)
{
	this->_wxImg->SetRGB(x, y, (uchar)color.Red(), (uchar)color.Green(), (uchar)color.Blue());
}
