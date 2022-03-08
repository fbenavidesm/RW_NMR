#include "rw_impl_creator.h"
#include "rw/plug.h"
#include "rw_cpu_degrade_impl.h"
#include "rw_gpu_degrade_impl.h"
#include "rw_simulator_impl.h"

namespace rw
{
	rw::RandomWalkImplementor* RandomWalkImplementorCreator::Create_Implementor(rw::Plug* formation,
		bool gpu,
		uint dimension,
		rw::Simulator* simulator)
	{
		rw::RandomWalkImplementor* implementor = 0;
		if (dimension == 3)
		{
			if (gpu)
			{
				implementor = new RandomWalkGPUDegradeImplementor(formation);
			}
			else
			{
				implementor = new RandomWalkCPUDegradeImplementor(formation);
			}
		}
		return(implementor);
	}

	void RandomWalkImplementorCreator::Create_And_Associate_Implementor(rw::Plug* formation)
	{
		if (formation->_implementor)
		{
			delete formation->_implementor;
			formation->_implementor = 0;
		}
		rw::RandomWalkImplementor* implementor =
			RandomWalkImplementorCreator::Create_Implementor(formation,formation->Simulation_Parameters().Gpu(),formation->Dimension(),0);
		formation->_implementor = implementor;
	}

	void RandomWalkImplementorCreator::Create_And_Associate_Implementor_Simulator(rw::Plug* formation, rw::Simulator* simulator)
	{
		if (formation->_implementor)
		{
			delete formation->_implementor;
			formation->_implementor = 0;
		}
		rw::RandomWalkSimulatorImplementor* implementor = new RandomWalkSimulatorImplementor(formation, simulator);
		formation->_implementor = implementor;
	}

	RandomWalkPlacer* RandomWalkImplementorCreator::Create_Uniform_Placer(rw::Plug* formation)
	{
		RandomWalkPlacer* placer = new RandomWalkPlacer(formation);
		return(placer);
	}
}