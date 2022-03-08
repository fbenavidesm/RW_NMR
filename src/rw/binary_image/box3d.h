#ifndef BOX3D_H
#define BOX3D_H

#include <algorithm>
#include "pos3i.h"

namespace rw
{

	/**
	* A 3D box used to delimit regions of interest
	*/
	struct Box3D
	{
		/**
		* Border 1
		*/
		Pos3i _eA;

		/**
		* Border 2 
		*/
		Pos3i _eB;
		bool operator < (const Box3D& box) const;


		/**
		* @return TRUE if the box contains the point center
		* @param center Point to characterize
		*/
		bool In(const Pos3i& center) const;

		/**
		* @return TRUE if two boxes intersect
		*/
		bool Intersect(const Box3D& box) const;

		/**
		* @return TRUE if a point of b2 is inside b1
		*/
		static bool Intersect(const Box3D& b1, const Box3D& b2);
	};

	inline bool Box3D::operator < (const Box3D& box) const
	{
		if (this->_eA == box._eA)
		{
			return(this->_eB < box._eB);
		}
		else
		{
			return(this->_eA < box._eA);
		}
	}

	inline bool Box3D::In(const Pos3i& center) const
	{
		return ((center.x >= this->_eA.x) && (center.y >= this->_eA.y) && (center.z >= this->_eA.z)
			&& (center.x <= this->_eB.x) && (center.y <= this->_eB.y) && (center.z <= this->_eB.z));
	}

	inline bool Box3D::Intersect(const Box3D& box) const
	{
		return(Box3D::Intersect(*this, box) || Box3D::Intersect(box, *this));
	}

	inline bool Box3D::Intersect(const Box3D& b1, const Box3D& b2)
	{
		rw::Pos3i p1 = b1._eA;
		rw::Pos3i p2 = p1;
		rw::Pos3i p3 = p1;
		rw::Pos3i p4 = p1;
		p2.x = b2._eB.x;
		p3.y = b2._eB.y;
		p3.x = b2._eB.x;
		p4.y = b2._eB.y;

		rw::Pos3i e1 = b2._eB;
		rw::Pos3i e2 = e1;
		rw::Pos3i e3 = e1;
		rw::Pos3i e4 = e1;
		e2.x = b2._eA.x;
		e3.y = b2._eA.y;
		e3.x = b2._eA.x;
		e4.y = b2._eA.y;

		return(b1.In(p1) || b1.In(p2) || b1.In(p3) || b1.In(p4)
			|| b1.In(e1) || b1.In(e2) || b1.In(e3) || b1.In(e4));
	}

}
#endif