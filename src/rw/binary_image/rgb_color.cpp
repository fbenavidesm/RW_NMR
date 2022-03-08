#include "rgb_color.h"

namespace rw
{

	RGBColor::RGBColor()
	{
		this->_r = 0;
		this->_g = 0;
		this->_b = 0;
	}

	RGBColor::RGBColor(const RGBColor& color)
	{
		this->_r = color._r;
		this->_g = color._g;
		this->_b = color._b;
	}

	RGBColor::RGBColor(uchar r, uchar g, uchar b)
	{
		this->_r = r;
		this->_g = g;
		this->_b = b;
	}

	RGBColor& RGBColor::operator = (const RGBColor& color)
	{
		this->_r = color._r;
		this->_g = color._g;
		this->_b = color._b;
		return(*this);
	}
}