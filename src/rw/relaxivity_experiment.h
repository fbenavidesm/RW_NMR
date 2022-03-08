#ifndef RELAXIVITY_EXPERIMENT_H
#define RELAXIVITY_EXPERIMENT_H

#include <vector>
#include "rw/exponential_fitting.h"
#include "simulator.h"
#include "plug.h"
#include "math_la/math_lac/genetic/creature.h"
#include "math_la/math_lac/genetic/population.h"
#include "hat.h"
#include "sigmoid.h"
#include "rw/random_walk_step_value.h"


namespace rw
{

	using std::vector;
	class RelaxivityOptimizer;

	class RelaxivityExperiment : public math_la::math_lac::genetic::Creature
	{
	private:
		friend class RelaxivityOptimizer;
		rw::ExponentialFitting _fitting;
		rw::Plug* _formation;
		scalar _strikeNormalizer;
		bool _simulateWalk;
		vector<rw::Step_Value> _decay;
		rw::Simulator* _simulator;
		rw::RelaxivityDistribution* _surfaceRelaxivityDistribution;
	public:
		RelaxivityExperiment(math_la::math_lac::genetic::Population* pop, int size);
		~RelaxivityExperiment();
		void Prepare();
		void Execute();
		void Update_Fitness(scalar& fitness);
		RelaxivityOptimizer* Parent_Optimizer() const;
		math_la::math_lac::full::Vector Laplace_Domain() const;
		math_la::math_lac::full::Vector Laplace_Bins() const;
		void Simulate_Walk(bool simulate);
		int Number_Of_Decay_Samples() const;
		rw::Step_Value Sample(int i) const;
		const RelaxivityDistribution& Relaxivity_Distribution() const;
		bool Functional() const;
		const rw::Plug& Plug_Formation() const;
		bool Simulated() const; 
	};

	inline bool RelaxivityExperiment::Simulated() const
	{
		return(this->_simulateWalk);
	}

	inline const rw::Plug& RelaxivityExperiment::Plug_Formation() const
	{
		return(*this->_formation);
	}
}

#endif