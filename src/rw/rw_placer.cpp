#include "rw_placer.h"

namespace rw
{

	RandomWalkPlacer::RandomWalkPlacer(rw::Plug* parent)
	{
		this->_parentFormation = parent;
		this->_mtx = new tbb::spin_mutex();
		this->_positions = new map<rw::Pos3i, uint>();
		this->_copied = false;
		this->_recharge = true;
	}

	RandomWalkPlacer::RandomWalkPlacer(const RandomWalkPlacer& wp)
	{
		this->_parentFormation = wp._parentFormation;
		this->_mtx = wp._mtx;
		this->_positions = wp._positions;
		this->_copied = true;
		this->_recharge = wp._recharge;
	}


	RandomWalkPlacer::~RandomWalkPlacer()
	{
		if (!this->_copied)
		{
			delete this->_positions;
			delete this->_mtx;
		}
		this->_parentFormation = 0;
	}

	void RandomWalkPlacer::Assign_Position_Map_To_Walkers()
	{
		map<rw::Pos3i, uint>::const_iterator itr = this->_positions->begin();
		uint id = 0;
		while (itr != this->_positions->end())
		{
			rw::Pos3i pp = itr->first;
			for (int k = 0; k < (int)itr->second; ++k)
			{
				this->Set_Walker_Start_Position(id,pp);
				++id;
			}
			++itr;
		}
	}

	void RandomWalkPlacer::RandomWalkPlacer::operator()(const tbb::blocked_range<int>& r) const
	{
		for (int t = r.begin(); t < r.end(); ++t)
		{
			rw::Walker& w = this->_parentFormation->Walking_Particle(t);
			w.Set_Rho(this->_parentFormation->Surface_Relaxivity_Delta());
			if (this->_recharge)
			{
				w.Set_Magnetization(1);
			}
			bool placed = false;
			scalar ic = this->_parentFormation->Pick_Random_Normalized_Number();
			while (!placed)
			{
				int id = (int)(ic*((int)this->_parentFormation->_image->Length()));
				rw::Pos3i pp;
				Pos3i::Int_To_Pos3i(id, this->_parentFormation->_image->Width(),
					this->_parentFormation->_image->Height(),
					this->_parentFormation->_image->Depth(), pp);
				if ((*this->_parentFormation->_image)(pp) == 0)
				{
					Pos3i nxyz;
					Pos3i::Int_To_Pos3i(id, this->_parentFormation->Plug_Texture().Width(),
						this->_parentFormation->Plug_Texture().Height(),
						this->_parentFormation->Plug_Texture().Depth(), nxyz);
					placed = true;
					map<rw::Pos3i, uint>::iterator fitr = this->_positions->find(nxyz);
					if (fitr == this->_positions->end())
					{
						std::pair<rw::Pos3i, uint> p;
						p.first = nxyz;
						p.second = 1;
						this->_mtx->lock();
						this->_positions->insert(p);
						this->_mtx->unlock();
					}
					else
					{
						this->_mtx->lock();
						fitr->second++;
						this->_mtx->unlock();
					}
				}
				else
				{
					ic = this->_parentFormation->Pick_Random_Normalized_Number();
				}
			}
		}
	}

	void RandomWalkPlacer::operator()()
	{
		this->Formation().Clear_Collision_Profile();		
		this->Reset_Seed();
		int nw = this->Formation().Number_Of_Walking_Particles();
		tbb::parallel_for(tbb::blocked_range<int>(0, nw, this->Formation().Minimal_Walkers_Per_Thread()), *this);
		this->Assign_Position_Map_To_Walkers();		
		this->Place_End();
	}
}