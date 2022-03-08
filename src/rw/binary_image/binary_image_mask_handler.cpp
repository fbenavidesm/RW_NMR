#include "binary_image_mask_handler.h"

namespace rw
{
	BinaryImageMaskHandler::BinaryImageMaskHandler()
	{
	}

	vec(rw::Pos3i) BinaryImageMaskHandler::Remove_Origin(const vec(rw::Pos3i)& mask)
	{
		vec(rw::Pos3i) r;
		for (int i = 0; i < (int)mask.size(); ++i)
		{
			rw::Pos3i pp = mask[i];
			if (!((pp.x == 0) && (pp.y == 0) && (pp.z == 0)))
			{
				r.push_back(pp);
			}
		}
		return(r);
	}

	vec(rw::Pos3i) BinaryImageMaskHandler::Cross()
	{
		vec(rw::Pos3i) r;
		for (int rnd = 0; rnd < 6; ++rnd)
		{
			int dxy = ((int)(rnd) << 1);
			int dg1 = 1 - (dxy >> 3);
			int dg2 = dxy >> 2;

			int dx = dg1 * (1 - dg2)*(dxy - 1);
			int dy = dg1 * dg2*(dxy - 5);
			int dz = (1 - dg1)*(dg2 >> 1)*(dxy - 9)	;
			rw::Pos3i pp;
			pp.x = dx;
			pp.y = dy;
			pp.z = dz;
			r.push_back(pp);
		}
		return(r);
	}

	vec(rw::Pos3i) BinaryImageMaskHandler::Displace_Mask(const vec(rw::Pos3i)& mask, int dd)
	{
		vec(rw::Pos3i) r;
		for (int k = 0; k < (int)mask.size(); ++k)
		{
			rw::Pos3i pp = mask[k];
			pp.x = pp.x + dd;
			pp.y = pp.y + dd;
			pp.z = pp.z + dd;
			r.push_back(pp);
		}
		return(r);
	}

	void BinaryImageMaskHandler::Create_Surface_Mask(int diam, vec(rw::Pos3i)& em)
	{
		em.clear();
		if (diam < 3)
		{
			rw::Pos3i cp;
			cp.x = -1;
			cp.y = 0;
			cp.z = 0;
			em.push_back(cp);
			cp.x = 1;
			cp.y = 0;
			cp.z = 0;
			em.push_back(cp);
			cp.x = 0;
			cp.y = -1;
			cp.z = 0;
			em.push_back(cp);
			cp.x = 0;
			cp.y = 1;
			cp.z = 0;
			em.push_back(cp);
			cp.x = 0;
			cp.y = 0;
			cp.z = -1;
			em.push_back(cp);
			cp.x = 0;
			cp.y = 0;
			cp.z = 1;
			em.push_back(cp);
		}
		else
		{
			if (diam == 3)
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
				int r = diam * diam;
				for (int i = -diam / 2 - 2; i <= diam / 2 + 2; ++i)
				{
					for (int j = -diam / 2 - 2; j <= diam / 2 + 2; ++j)
					{
						int d = 4 * (i*i + j*j);
						if (even)
						{
							d = d - 4 * (i + j) + 2;
						}
						if (d <= r)
						{
							rw::Pos3i cp;
							cp.x = i;
							cp.y = j;
							cp.z = 0;
							em.push_back(cp);
							cp.x = i;
							cp.y = 0;
							cp.z = j;
							em.push_back(cp);
							cp.x = 0;
							cp.y = i;
							cp.z = j;
						}
					}
				}
			}
		}
	}

	void BinaryImageMaskHandler::Create_Mask(int diam, vec(rw::Pos3i)& em)
	{
		em.clear();
		if (diam < 3)
		{
			rw::Pos3i cp;
			cp.x = -1;
			cp.y = 0;
			cp.z = 0;
			em.push_back(cp);
			cp.x = 1;
			cp.y = 0;
			cp.z = 0;
			em.push_back(cp);
			cp.x = 0;
			cp.y = -1;
			cp.z = 0;
			em.push_back(cp);
			cp.x = 0;
			cp.y = 1;
			cp.z = 0;
			em.push_back(cp);
			cp.x = 0;
			cp.y = 0;
			cp.z = -1;
			em.push_back(cp);
			cp.x = 0;
			cp.y = 0;
			cp.z = 1;
			em.push_back(cp);
		}
		else
		{
			if (diam == 3)
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
				int r = diam * diam;
				for (int z = -diam / 2 - 2; z <= diam / 2 + 2; ++z)
				{
					for (int y = -diam / 2 - 2; y <= diam / 2 + 2; ++y)
					{
						for (int x = -diam / 2 - 2; x <= diam / 2 + 2; ++x)
						{
							int d = 4 * (x*x + y*y + z*z);
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
	}

	void BinaryImageMaskHandler::Add_Mask(int rad)
	{
		if (this->_masks.find(rad) == this->_masks.end())
		{
			vec(rw::Pos3i) em;
			this->Create_Mask(rad, em);
			this->_masks[rad] = em;
		}
	}

	void BinaryImageMaskHandler::Add_Surface_Mask(int rad)
	{
		if (this->_surfaceMasks.find(rad) == this->_surfaceMasks.end())
		{
			vec(rw::Pos3i) em;
			this->Create_Surface_Mask(rad, em);
			this->_surfaceMasks[rad] = em;
		}
	}
}