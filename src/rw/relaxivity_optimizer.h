#ifndef RELAXIVITY_OPTIMIZER_H
#define RELAXIVITY_OPTIMIZER_H

#include "math_la/mdefs.h"
#include "rw/plug.h"
#include "relaxivity_experiment.h"
#include "math_la/math_lac/full/vector.h"
#include "math_la/math_lac/genetic/population.h"
#include "math_la/math_lac/genetic/creature.h"
#include "persistence/plug_persistent.h"
#include "relaxivity_distribution.h"
#include "simulator.h"

namespace rw
{

/**
* This function uses genetic algorithms to optimize the surface relaxivity, as a function of pore size (RTF=XI). 
*/
class RelaxivityOptimizer : public math_la::math_lac::genetic::Population
{
public:
	/**
	* Enumerates the surface relaxivity optimization possibilites to decreasing or increasing functions. Or no restriction at all
	*/
	enum Monotonic
	{
		/**
		* No restriction on the optimization
		*/
		None,
		/**
		* Only decreasing functions are considered for optimization
		*/
		Decrease,
		/**
		* Only increasing functions are considered for optimization
		*/
		Increase
	};

	/**
	* Comparison metric between simulation results. This is applied on the Laplace vectors
	*/
	enum Metric
	{
		/**
		* This comparison metric uses the Cauchy theorem to compare two vectors  v1 and v2.
		* The correlation is defined as (v1 * v2)^2 / (norm(v1)^2*norm(v2)^2)
		*/
		Correlation,
		/**
		* Euclidean distance to compare two vectors
		*/
		Euclidean
	};
private:
	friend class rw::RelaxivityExperiment;
	friend class rw::PlugPersistent;

	/**
	* The _basisFunctionShape defines the shape of the linear comnination of functions that define relaxivity. 
	*/
	RelaxivityDistribution::Shape _basisFunctionShape;

	/**
	* The basis formation on which image based random walks are iteratively executed to optimize the relaxivity function
	*/
	const rw::Plug* _basisPlug;

	/**
	* The mapping simulation, on which the RTF of every walkers is defined and on which other paremeters like pixel, Laplace resolution,
	* regularizer, etc. resolution are stored. 
	*/
	const rw::PlugPersistent* _mappingSimulation;

	/**
	* A base simulator for the random walk
	*/
	rw::Simulator* _simulator;

	/**
	* This vector stores a Laplace transform
	*/
	math_la::math_lac::full::Vector _laplaceTransform;

	/**
	* Minimal T2 time for the Laplace transform
	*/
	scalar _laplaceT2min;

	/**
	* Maximal T2 time for the Laplace transform
	*/
	scalar _laplaceT2max;
	/**
	* Resolution for the Laplace transform
	*/
	int _laplaceResolution;
	/**
	* Reduction for the exponential decay (number of samples)
	*/
	int _reductionT2;

	/**
	* Regularizer for the Laplace transform
	*/
	scalar _lambda;
	/**
	* Diffusion coefficient of the fluid
	*/
	scalar _D;
	/**
	* Voxel size of the sample
	*/
	scalar _S;

	/**
	* Unit index for the diffusion coefficient
	*/
	int _DUnit;

	/**
	* Unit index for the voxel sie
	*/
	int _SUnit;

	/**
	* Index unit for RHO
	*/
	int _rhoUnit;

	/**
	* Comparison metric for the Laplace transforms
	*/
	RelaxivityOptimizer::Metric _comparisonMetric;

	/**
	* Multiplier for the slope of the optimization sigmoids
	*/
	scalar _sigmoidSlopeFactor;

	/**
	* The minimal value a Hat function can take
	*/
	scalar fBoxKmin;
	/**
	* Determines if the optimized relaxiivty is decreasing or increasing or none of them.
	*/
	RelaxivityOptimizer::Monotonic _monotonicShape;

	/**
	* TRUE if the random walk is simulated
	*/
	bool _simulateWalk;

	/**
	* Number of particles for a simulated random walk
	*/
	int  _totalNumberOfParticles;

	/**
	* Seed for the random walk number generator, to repeat the paths of the mapping simulation
	*/
	uint _seed;

	/**
	* TRUE if the walker's path is repeated
	*/
	bool _repeatPath;

	/**
	* This is the number of iterations between each RTF update of the walker family
	*/
	uint _rtfUpdateInterval;

	/**
	* Sequential set of profiles of the random walk simulation
	*/
	vector<vector<scalar>*> _profileSequence;

	/**
	* This is the number of iterations that are necessary to pass from a Collision Profile to other
	*/
	int _updateProfileInterval;
protected:
	math_la::math_lac::genetic::Creature* Create_Individual(const math_la::math_lac::genetic::Creature* parent) const;
	void Shape_Creature(math_la::math_lac::genetic::Creature* c);
public:
	RelaxivityOptimizer(int size = 64);
	~RelaxivityOptimizer();
	void Set_Laplace_Parameters(scalar lambda, scalar LT2min, scalar LT2max, 
		int resolution);
	void Set_Formation(const rw::Plug* env, bool calcLaplace = true);
	math_la::math_lac::full::Vector Laplace_Vector(const rw::Plug* env);
	RelaxivityOptimizer::Metric Comparison_Metric() const;
	RelaxivityDistribution::Shape Function_Shape() const;
	void Set_Metric(const RelaxivityOptimizer::Metric& metric);
	void Configure_Sigmoids(int Voxel_Length, scalar size,scalar pdrmax);
	void Configure_Hats(int Voxel_Length, scalar size, scalar pdrmax);

	void Set_Sigmoid_Cuts(scalar flag, scalar size, int idx = 0);
	void Set_Sigmoid_Min_Max(scalar minmin, scalar maxmin, scalar minmax, scalar maxmax, int id);
	void Set_Sigmoid_Slope_Min_Max(scalar flag, scalar size, int id);

	void Set_Hat_Amplitude(scalar kmin, scalar kmax, int id);
	void Set_Hat_Cuts(scalar xminmin, scalar xminmax, scalar xmaxmin, scalar xmaxmax, int id);
	void Set_Hat_Cuts(scalar xmin, scalar xmaxmin, int id);
	void Fix_Sigmoid_Slope(scalar Decay_Step_Value, int id);
	void Fix_Hat_Cut_Min(scalar Decay_Step_Value, int id);
	void Fix_Hat_Cut_Max(scalar Decay_Step_Value, int id);

	void Set_Monotonic(RelaxivityOptimizer::Monotonic monotonic);
	scalar Hat_Line() const;
	void Simulate_Walk(bool simulate, int particles = 0);
	void Set_Slope_Scale(scalar scale);
	void Set_Hat_Line(scalar flag);
	void Set_Mapping_Simulation(const rw::PlugPersistent* sim);
	const rw::PlugPersistent& Mapping_Simulation() const;
	void Repeat_Paths(bool r, uint seed);
	void Set_Cycle(uint cycle);
};

inline scalar RelaxivityOptimizer::Hat_Line() const
{
	return(this->fBoxKmin);
}

}

#endif