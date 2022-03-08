#ifndef BINARY_IMAGE_HANDLER_H
#define BINARY_IMAGE_HANDLER_H

#include "binary_image_executor.h"

namespace rw
{
	class BinaryImageMaskHandler 
	{
	private:
		map<int, vec(rw::Pos3i)> _masks;
		map<int, vec(rw::Pos3i)> _surfaceMasks;
	protected:
		virtual void Create_Mask(int diam, vec(rw::Pos3i)& em);
		virtual void Create_Surface_Mask(int diam, vec(rw::Pos3i)& em);
		void Add_Surface_Mask(int diam);
		void Add_Mask(int diam);
		vec(rw::Pos3i) Displace_Mask(const vec(rw::Pos3i)& mask, int dd = 1);
	public:
		BinaryImageMaskHandler();
		const vec(rw::Pos3i)& Corner_Mask(int rad) const;
		const vec(rw::Pos3i)& Surface_Mask(int rad) const;
		static vec(rw::Pos3i) Remove_Origin(const vec(rw::Pos3i)& mask);
		static vec(rw::Pos3i) Cross();
	};

	inline const vec(rw::Pos3i)& BinaryImageMaskHandler::Corner_Mask(int rad) const
	{
		return(this->_masks.find(rad)->second);
	}

	inline const vec(rw::Pos3i)& BinaryImageMaskHandler::Surface_Mask(int rad) const
	{
		return(this->_surfaceMasks.find(rad)->second);
	}

}


#endif

