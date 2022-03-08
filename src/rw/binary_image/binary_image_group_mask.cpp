#include "binary_image_group_mask.h"

namespace rw
{
	BinaryImageGroupMask::BinaryImageGroupMask() : BinaryImageMaskHandler()
	{

	}

	void BinaryImageGroupMask::Create_Mask(int diam, vec(rw::Pos3i)& em)
	{
		em.clear();
		if (diam <= 3)
		{
			for (int z = -1; z <= 1; ++z)
			{
				for (int y = -1; y <= 1; ++y)
				{
					for (int x = -1; x <= 1; ++x)
					{
						rw::Pos3i cp;
						cp.x = x;
						cp.y = y;
						cp.z = z;
						em.push_back(cp);
					}
				}
			}
		}
		else
		{
			bool even = (diam % 2 == 0);
			int r = (diam + 2) * (diam + 2);
			for (int z = -diam / 2 - 2; z <= diam / 2 + 2; ++z)
			{
				for (int y = -diam / 2 - 2; y <= diam / 2 + 2; ++y)
				{
					for (int x = -diam / 2 - 2; x <= diam / 2 + 2; ++x)
					{
						int d = 4 * (x*x + y * y + z * z);
						if (even)
						{
							d = d - 4 * (x + y + z) + 3;
						}
						if (d <= r)
						{
							rw::Pos3i cp;
							cp.x = x;
							cp.y = y;
							cp.z = z;
							em.push_back(cp);
						}
					}
				}
			}
		}
	}


	void BinaryImageGroupMask::Create_Surface_Mask(int rad, vec(rw::Pos3i)& em)
	{
		this->Create_Mask(rad, em);
	}

}