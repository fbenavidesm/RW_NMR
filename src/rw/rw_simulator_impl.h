#ifndef RANDOM_WALK_SIMULATOR_IMPLEMENTOR_H
#define RANDOM_WALK_SIMULATOR_IMPLEMENTOR_H

#include "random_walk_implementor.h"

namespace rw
{
	class RandomWalkSimulatorImplementor : public RandomWalkImplementor
	{
	private:
		uint _aliveWalkers;
		rw::Simulator* _simulator;
	public:
		RandomWalkSimulatorImplementor(rw::Plug* formation, rw::Simulator* simulator);
		~RandomWalkSimulatorImplementor();
		void operator()();
	};
}



#endif