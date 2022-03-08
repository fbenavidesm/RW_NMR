#ifndef RGB_COLOR_INTERFACE
#define RGB_COLOR_INTERFACE

#include "math_la/mdefs.h"

namespace rw
{
	/**
	* RGB color for the binary image
	*/
	class RGBColor
	{
	private:
		uchar _r;
		uchar _g;
		uchar _b;
	public:
		RGBColor();
		RGBColor(const RGBColor& color);
		RGBColor(uchar r, uchar g, uchar b);
		uchar Red() const;
		uchar Green() const;
		uchar Blue() const;
		RGBColor& operator = (const RGBColor& color);
	};

	inline uchar RGBColor::Red() const
	{
		return(this->_r);
	}

	inline uchar RGBColor::Green() const
	{
		return(this->_g);
	}

	inline uchar RGBColor::Blue() const
	{
		return(this->_b);
	}
}



#endif
