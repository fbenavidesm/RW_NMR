#include "rw_simulator_impl.h"

namespace rw
{

	RandomWalkSimulatorImplementor::RandomWalkSimulatorImplementor(rw::Plug* formation, rw::Simulator* simulator)
		: RandomWalkImplementor(formation)
	{
		this->_simulator = simulator;
	}

	RandomWalkSimulatorImplementor::~RandomWalkSimulatorImplementor()
	{
		if (this->_simulator)
		{
			delete this->_simulator;
		}
	}

	void RandomWalkSimulatorImplementor::operator()()
	{
		if (this->_simulator)
		{
			int alive = this->_simulator->Total_Particles();
			this->_aliveWalkers = alive;
			rw::Plug& f = this->Plug();
			scalar E = 1;
			scalar rate_factor = exp(-f.Time_Step() / f.TBulk_Seconds());
			uint iteration = 0;
			f.Clear_Decay_Steps();
			this->Reserve_Values_Memory_Space(300000);
			scalar rate = f.TBulk_Seconds();
			this->_simulator->Set_Gradient(f.Gradient());
			this->_simulator->Prepare();
			if (f.Simulation_Parameters().Degrade())
			{
				while ((E > f.Stop_Threshold()) && (iteration < f.Max_Number_Of_Iterations()))
				{
					scalar ENU = this->_simulator->Magnetization();
					scalar frac = ENU;
					E = frac;
					E = E * rate_factor;
					rw::Step_Value v;
					v.Magnetization = E;
					v.Iteration = iteration;
					v.Time = ((scalar)iteration)*f.Time_Step();
					this->Push_Seq_Step_Value(v);
					++iteration;
					if ((this->_simulator->Profile_Update_Interval() > 0) && 
						(iteration % this->_simulator->Profile_Update_Interval() == 0))
					{
						this->_simulator->Update(iteration);
					}
				}
			}
			else if (f.Simulation_Parameters().Kill())
			{
				while ((E > f.Stop_Threshold()) && (iteration < f.Max_Number_Of_Iterations()))
				{
					scalar ENU = this->_simulator->Magnetization();
					int Nt = alive - this->_simulator->Killed_Particles();
					scalar frac = (scalar)Nt / (scalar)this->_simulator->Total_Particles();
					E = frac;
					E = E * rate_factor;
					rw::Step_Value v;
					v.Magnetization = E;
					v.Iteration = iteration;
					v.Time = ((scalar)iteration)*f.Time_Step();
					this->Push_Seq_Step_Value(v);
					++iteration;
				}
			}
		}
		this->_simulator = 0;
	}
}