#ifndef RANDOM_WALK_IMPLEMENTOR_CREATOR_H
#define RANDOM_WALK_IMPLEMENTOR_CREATOR_H

#include "random_walk_implementor.h"
#include "rw_placer.h"

namespace rw
{

	class RandomWalkImplementorCreator
	{
	public:
		static rw::RandomWalkImplementor* Create_Implementor(	rw::Plug* formation,
																bool gpu = false,
																uint dimension = 3,
																rw::Simulator* simulator = 0);

		static void Create_And_Associate_Implementor(rw::Plug* formation);
		static void Create_And_Associate_Implementor_Simulator(rw::Plug* formation, rw::Simulator* simulator);
		static RandomWalkPlacer* Create_Uniform_Placer(rw::Plug* formation);
	};
}

#endif
