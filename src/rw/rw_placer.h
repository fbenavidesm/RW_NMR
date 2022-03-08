#ifndef RANDOM_WALK_PLACER
#define RANDOM_WALK_PLACER

#include <map>
#include "tbb/spin_mutex.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "math_la/mdefs.h"
#include "binary_image/pos3i.h"
#include "rw/walker.h"
#include "rw/plug.h"

namespace rw
{
	using std::map;
	class Plug;
	class RandomWalkPlacer
	{
	private:
		/**
		* Parent formation of the placing procedure
		*/
		Plug* _parentFormation;

		/**
		* Mutex to update walker positions along different threads
		*/
		tbb::spin_mutex* _mtx;

		/**
		* Walker´s positions
		*/
		map<rw::Pos3i, uint>* _positions;

		/**
		* Defines if a handler is copied or not
		*/
		bool _copied;
		bool _recharge;
	protected:
		rw::Plug& Formation();
		map<rw::Pos3i, uint>& Position_Map();
		tbb::spin_mutex& Mutex();
		rw::Walker& Walker(uint id);
		vec(rw::Walker)& Walkers();
		void Set_Walker_Start_Position(uint walker_id, const rw::Pos3i& position);
		void Assign_Position_Map_To_Walkers();
		void Reset_Seed();
		void Place_End();
	public:
		RandomWalkPlacer(rw::Plug* parent);
		RandomWalkPlacer(const RandomWalkPlacer& wp);
		~RandomWalkPlacer();
		/**
		* Copies sorted positions to the parent walker´s list
		*/
		virtual void operator()();
		void operator()(const tbb::blocked_range<int>& r) const;
		void Recharge(bool recharge);
		bool Recharging() const;
	};

	inline void RandomWalkPlacer::Recharge(bool recharge)
	{
		this->_recharge = recharge;
	}

	inline bool RandomWalkPlacer::Recharging() const
	{
		return(this->_recharge);
	}


	inline rw::Plug& RandomWalkPlacer::Formation()
	{
		return(*this->_parentFormation);
	}

	inline map<rw::Pos3i, uint>& RandomWalkPlacer::Position_Map()
	{
		return(*this->_positions);
	}

	inline tbb::spin_mutex& RandomWalkPlacer::Mutex()
	{
		return(*this->_mtx);
	}

	inline rw::Walker& RandomWalkPlacer::Walker(uint id)
	{
		return(this->_parentFormation->_walkers[id]);
	}

	inline vec(rw::Walker)& RandomWalkPlacer::Walkers()
	{
		return(this->_parentFormation->_walkers);
	}

	inline void RandomWalkPlacer::Set_Walker_Start_Position(uint walker_id, const rw::Pos3i& position)
	{
		this->_parentFormation->_walkersStartPosition[walker_id].x = position.x;
		this->_parentFormation->_walkersStartPosition[walker_id].y = position.y;
		this->_parentFormation->_walkersStartPosition[walker_id].z = position.z;
		this->_parentFormation->_walkers[walker_id].Set_Position(position);
	}

	inline void RandomWalkPlacer::Reset_Seed()
	{
		this->_parentFormation->_randomGenerator.seed((uint)this->_parentFormation->_randomSeedGenerator());
	}

	inline void RandomWalkPlacer::Place_End()
	{
		this->_parentFormation->_walkersPlaced = true;
		this->_parentFormation->_simParams.Set_Value(NO_OF_WALKERS,(uint)this->_parentFormation->_walkers.size());
	}

}

#endif