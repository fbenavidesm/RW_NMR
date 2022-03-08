#include "relaxivity_experiment.h"
#include "relaxivity_optimizer.h"
#include "profile_simulator.h"

namespace rw
{

RelaxivityExperiment::RelaxivityExperiment(math_la::math_lac::genetic::Population* pop, int size) : Creature(pop,size)
{
	this->_formation = 0;
	this->_strikeNormalizer = (scalar)this->Parent_Optimizer()->_basisPlug->Decay_Size();
	this->_simulateWalk = false;
	this->_surfaceRelaxivityDistribution = 0;
}

const rw::RelaxivityDistribution& RelaxivityExperiment::Relaxivity_Distribution() const
{
	return(*this->_surfaceRelaxivityDistribution);
}

bool RelaxivityExperiment::Functional() const
{
	return(this->_surfaceRelaxivityDistribution != 0);
}


RelaxivityExperiment::~RelaxivityExperiment()
{
	if (this->_surfaceRelaxivityDistribution)
	{
		delete this->_surfaceRelaxivityDistribution;
	}
	delete this->_formation;
	this->_formation = 0;
}

void RelaxivityExperiment::Simulate_Walk(bool simulate)
{
	this->_simulateWalk = simulate;
}

RelaxivityOptimizer* RelaxivityExperiment::Parent_Optimizer() const
{
	return((RelaxivityOptimizer*)this->Creature::Parent());
}

void RelaxivityExperiment::Prepare()
{
	Plug* e = 0;
	if (!this->_simulateWalk)
	{
		e = new rw::Plug(*this->Parent_Optimizer()->_basisPlug);
		this->_formation = e;
		const RelaxivityOptimizer* go = (const rw::RelaxivityOptimizer*)this->Parent_Optimizer();
		e->Repeat_Walkers_Paths(go->_repeatPath, go->_seed);		
		RelaxivityDistribution* distr = 0;		
		if (go->Function_Shape() == rw::RelaxivityDistribution::Sigmoid)
		{
			Sigmoid* sigm = new Sigmoid();
			distr = sigm;
			sigm->Set_Simulation(((const rw::RelaxivityOptimizer*)this->Parent_Optimizer())->Mapping_Simulation());
			sigm->Set_Slope_Scale(go->_sigmoidSlopeFactor);
			sigm->Set_Size(go->Phenotype_Size() / 4);
			for (int i = 0; i < go->Phenotype_Size(); ++i)
			{
				sigm->Set_Value(i, go->Translate_To_Scalar(i, this->Gene(i)));
			}
		}
		if (go->Function_Shape() == rw::RelaxivityDistribution::Hat)
		{
			Hat* hat = new Hat();
			distr = hat;
			hat->Set_Simulation(((const rw::RelaxivityOptimizer*)this->Parent_Optimizer())->Mapping_Simulation());
			hat->Set_Ground(go->fBoxKmin);
			hat->Set_Size(go->Phenotype_Size() / 3);
			for (int i = 0; i < go->Phenotype_Size(); ++i)
			{
				hat->Set_Value(i, go->Translate_To_Scalar(i, this->Gene(i)));
			}
		}
		e->_simParams.Set_Bool(PROFILE_UPDATE,go->_rtfUpdateInterval);
		e->Set_Relaxivity_Distribution(distr, this->Parent_Optimizer()->Mapping_Simulation().Total_Number_Of_Simulated_Iterations());
		e->Recharge_Walkers_Magnetization();
		this->_surfaceRelaxivityDistribution = distr;
	}
	else
	{
		e = new rw::Plug(*this->Parent_Optimizer()->_basisPlug,false);
		this->_formation = e;
		rw::Simulator* simulator = this->_simulator;
		RelaxivityOptimizer* go = (RelaxivityOptimizer*)this->Parent_Optimizer();
		rw::ProfileSimulator* sr = (rw::ProfileSimulator*)this->_simulator;
		this->_simulator->_profileUpdateInterval = go->_updateProfileInterval;
		sr->_profileSequence = &go->_profileSequence;
		sr->Share_Profile(((rw::ProfileSimulator*)go->_simulator)->Profile());
		this->_simulator->Set_NumberOfParticles(go->_simulator->Total_Particles());
		if (go->Function_Shape() == rw::RelaxivityDistribution::Sigmoid)
		{
			Sigmoid* sigm = new Sigmoid();
			sigm->Set_Simulation(((const rw::RelaxivityOptimizer*)this->Parent_Optimizer())->Mapping_Simulation());
			sigm->Set_Slope_Scale(go->_sigmoidSlopeFactor);
			sigm->Set_Size(go->Phenotype_Size() / 4);
			for (int i = 0; i < go->Phenotype_Size(); ++i)
			{
				sigm->Set_Value(i, go->Translate_To_Scalar(i, this->Gene(i)));
			}
			simulator->Set_Relaxivity_Distribution(sigm);
		}
		if (go->Function_Shape() == rw::RelaxivityDistribution::Hat)
		{
			Hat* hat = new Hat();
			hat->Set_Simulation(((const rw::RelaxivityOptimizer*)this->Parent_Optimizer())->Mapping_Simulation());
			hat->Set_Ground(go->fBoxKmin);
			hat->Set_Size(go->Phenotype_Size() / 3);
			for (int i = 0; i < go->Phenotype_Size(); ++i)
			{
				hat->Set_Value(i, go->Translate_To_Scalar(i, this->Gene(i)));
			}
			simulator->Set_Relaxivity_Distribution(hat);
		}
	}
}

void RelaxivityExperiment::Execute()
{
	Plug* e = this->_formation;
	e->_decayValues.clear();
	if (!this->_simulateWalk)
	{		
		e->Init_Walkers_Position();
		e->Random_Walk_Procedure();		
	}
	else
	{
		e->Simulate_Random_Walk_Procedure(*this->_simulator);
		this->_surfaceRelaxivityDistribution = this->_simulator->_distribution;
	}
	this->Parent_Optimizer()->Lock_Event_Trigger();
	this->_fitting.Load_Decay(e->_decayValues);
	this->_fitting = this->_fitting.Sequential_Logarithmic_Reduction(this->Parent_Optimizer()->_reductionT2);
	this->Parent_Optimizer()->UnLock_Event_Trigger();
	e->_image = 0;
	this->_strikeNormalizer = (scalar)e->_decayValues.size();
	this->_decay = e->_decayValues;
}

void RelaxivityExperiment::Update_Fitness(scalar& fitness)
{
	this->_fitting.Kernel_T2_Mount(this->Parent_Optimizer()->_laplaceT2min, this->Parent_Optimizer()->_laplaceT2max,
		this->Parent_Optimizer()->_laplaceResolution, this->Parent_Optimizer()->_lambda);
	math_la::math_lac::full::Vector domain;
	math_la::math_lac::full::Vector la;
	this->_fitting.Solve(domain, la);
	RelaxivityOptimizer* go = (RelaxivityOptimizer*)this->Parent_Optimizer();
	if (go->Comparison_Metric() == RelaxivityOptimizer::Euclidean)
	{
		math_la::math_lac::full::Vector d = la - this->Parent_Optimizer()->_laplaceTransform;
		fitness = d.Dot(d) / this->Parent_Optimizer()->_laplaceTransform.Dot(this->Parent_Optimizer()->_laplaceTransform);
	}
	else if (go->Comparison_Metric() == RelaxivityOptimizer::Correlation)
	{
		math_la::math_lac::full::Vector lr = this->Parent_Optimizer()->_laplaceTransform;
		scalar u1 = la.Dot(la);
		scalar u2 = lr.Dot(lr);
		scalar u = la.Dot(lr);
		if (u1 == 0)
		{
			u1 = 1;
		}
		if (u2 == 0)
		{
			u2 = 1;
		}
		fitness = (u*u) / (u1*u2);
	}
}

math_la::math_lac::full::Vector RelaxivityExperiment::Laplace_Domain() const
{
	return(this->_fitting.Laplace_Domain());
}

math_la::math_lac::full::Vector RelaxivityExperiment::Laplace_Bins() const
{
	return(this->_fitting.Laplace_Range());
}

int RelaxivityExperiment::Number_Of_Decay_Samples() const
{
	return((int)this->_decay.size());
}

rw::Step_Value RelaxivityExperiment::Sample(int i) const
{
	return(this->_decay[i]);
}


}