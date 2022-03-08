#include "relaxivity_optimizer.h"
#include "exponential_fitting.h"
#include "persistence/plug_persistent.h"
#include "profile_simulator.h"

namespace rw
{

RelaxivityOptimizer::RelaxivityOptimizer(int size) : Population(size)
{
	this->_basisPlug = 0;
	this->_reductionT2 = 1024;
	this->_lambda = 1;
	this->Set_Max_Fitness(false);
	this->_D = 0;
	this->_S = 0;
	this->_comparisonMetric = RelaxivityOptimizer::Correlation;
	this->_basisFunctionShape = rw::RelaxivityDistribution::Sigmoid;
	this->_sigmoidSlopeFactor = 25;
	this->fBoxKmin = 0.1;
	this->_monotonicShape = RelaxivityOptimizer::Monotonic::None;
	this->_simulateWalk = false;
	this->_simulator = 0;
	this->_repeatPath = false;
	this->_seed = 0;
}

void RelaxivityOptimizer::Set_Slope_Scale(scalar scale)
{
	this->_sigmoidSlopeFactor = scale;
}

void RelaxivityOptimizer::Set_Hat_Line(scalar flag)
{
	this->fBoxKmin = flag;
}


RelaxivityOptimizer::~RelaxivityOptimizer()
{
	if (this->_simulator)
	{
		delete this->_simulator;
	}
	for (int k = 0; k < (int)this->_profileSequence.size(); ++k)
	{
		vector<scalar>* profile = this->_profileSequence[k];
		delete profile;
	}
	this->_profileSequence.clear();

}

math_la::math_lac::genetic::Creature* RelaxivityOptimizer::Create_Individual(const math_la::math_lac::genetic::Creature* parent) const
{
	RelaxivityExperiment* ge = new RelaxivityExperiment((Population*)this, this->Phenotype_Size());
	ge->_simulateWalk = this->_simulateWalk;
	ge->_simulator = new rw::ProfileSimulator();
	return(ge);
}

math_la::math_lac::full::Vector RelaxivityOptimizer::Laplace_Vector(const rw::Plug* e)
{
	ExponentialFitting ef;
	ef.Load_Formation(*e);
	ef = ef.Logarithmic_Reduction(this->_reductionT2);
	math_la::math_lac::full::Vector laplaceRange;
	math_la::math_lac::full::Vector laplaceDomain;
	ef.Kernel_T2_Mount(this->_laplaceT2min, this->_laplaceT2max, this->_laplaceResolution, this->_lambda);
	ef.Solve(laplaceDomain, laplaceRange);
	return(laplaceRange);
}

void RelaxivityOptimizer::Simulate_Walk(bool simulate, int particles)
{
	this->_simulateWalk = simulate;
	this->_totalNumberOfParticles = particles;
	if (!this->_simulateWalk)
	{
		if (this->_simulator)
		{
			delete this->_simulator;
			this->_simulator = 0;
		}
	}
}

void RelaxivityOptimizer::Set_Formation(const rw::Plug* e, bool calcLaplace)
{
	this->_basisPlug = e;
	this->Set_Chunk_Size(1);
	if (calcLaplace)
	{
		this->_laplaceTransform = this->Laplace_Vector(e);
	}
	if (this->_simulateWalk)
	{
		this->Set_Chunk_Size(8);
		if (this->_simulator)
		{
			delete this->_simulator;
			this->_simulator = 0;
		}
		rw::ProfileSimulator* simulator = new rw::ProfileSimulator();		
		math_la::math_lac::full::Vector domain;
		math_la::math_lac::full::Vector range;
		this->_mappingSimulation->Build_Collision_Rate_Distribution(domain, range);
		simulator->Set_Collision_Profile(domain, range);
		simulator->Set_NumberOfParticles(this->_totalNumberOfParticles);
		this->_simulator = simulator;
	}
}

void RelaxivityOptimizer::Set_Laplace_Parameters(scalar lambda, scalar LT2min, scalar LT2max, 
				int resolution)
{
	this->_lambda = lambda;
	this->_laplaceT2max = LT2max;
	this->_laplaceT2min = LT2min;
	this->_laplaceResolution = resolution;
}

RelaxivityOptimizer::Metric RelaxivityOptimizer::Comparison_Metric() const
{
	return(this->_comparisonMetric);
}

void RelaxivityOptimizer::Set_Metric(const RelaxivityOptimizer::Metric& metric)
{
	this->_comparisonMetric = metric;
	if (this->_comparisonMetric == RelaxivityOptimizer::Correlation)
	{
		this->Set_Max_Fitness(true);
	}
	if (this->_comparisonMetric == RelaxivityOptimizer::Euclidean)
	{
		this->Set_Max_Fitness(false);
	}
}


void RelaxivityOptimizer::Configure_Sigmoids(int Voxel_Length, scalar size, scalar pdrmax)
{
	int imax = (int)(size*(scalar)10000);
	imax = imax / Voxel_Length;
	int ipdrmax = int(pdrmax*(scalar)10000);
	this->Set_Phenotype_Size(4*Voxel_Length);

	for (int i = 0; i < Voxel_Length; ++i)
	{
		this->Set_Precision(4*i, 10000);
		this->Set_Gene_Limits(4*i, 0, imax);
		this->Set_Precision(4*i+1, 10000);
		this->Set_Gene_Limits(4*i+1,0,imax);
		this->Set_Precision(4*i+2, 10000);
		this->Set_Gene_Limits(4*i+2, -ipdrmax/100, ipdrmax+ipdrmax/100);
		this->Set_Precision(4*i+3, 1000);
		this->Set_Gene_Limits(4*i+3, 0, 1000);
	}
	this->_basisFunctionShape = rw::RelaxivityDistribution::Sigmoid;
	this->Set_Crossover_Rule(Population::WholeArithmetic);
}

void RelaxivityOptimizer::Configure_Hats(int Voxel_Length, scalar size, scalar pdrmax)
{
	int imax = (int)(size*(scalar)10000);
	int ipdrmax = int(pdrmax*(scalar)10000);
	this->Set_Phenotype_Size(3 * Voxel_Length);

	for (int i = 0; i < Voxel_Length; ++i)
	{
		this->Set_Precision(3 * i, 10000);
		this->Set_Gene_Limits(3 * i, 1, imax);
		this->Set_Precision(3 * i + 1, 10000);
		this->Set_Gene_Limits(3 * i + 1, -ipdrmax / 10, ipdrmax + ipdrmax / 10);
		this->Set_Precision(3 * i + 2, 10000);
		this->Set_Gene_Limits(3 * i + 2, -ipdrmax / 10, ipdrmax + ipdrmax / 10);
	}
	this->_basisFunctionShape = rw::RelaxivityDistribution::Hat;
	this->Set_Crossover_Rule(Population::WholeArithmetic);
	this->fBoxKmin = 0.1;
}

rw::RelaxivityDistribution::Shape RelaxivityOptimizer::Function_Shape() const
{
	return(this->_basisFunctionShape);
}

void RelaxivityOptimizer::Shape_Creature(math_la::math_lac::genetic::Creature* c)
{
	if (this->_basisFunctionShape == rw::RelaxivityDistribution::Hat)
	{
		int Voxel_Length = this->Phenotype_Size() / 3;
		for (int k = 0; k < Voxel_Length; ++k)
		{
			if (c->Gene(3 * k + 1) > c->Gene(3 * k + 2))
			{
				if ((k != 0) && (k != Voxel_Length-1))
				{
					int xmax = c->Gene(3 * k + 1);
					c->Set_Gene(3 * k + 1, c->Gene(3 * k + 2));
					c->Set_Gene(3 * k + 2, xmax);
				}
				else
				{
					if (k == 0)
					{
						c->Set_Gene(2,c->Gene(3*(Voxel_Length-1)+2));
					}
					if (k == Voxel_Length-1)
					{
						c->Set_Gene(3 * k + 1, 0);
					}
				}
			}
		}
	}
	if (this->_monotonicShape != RelaxivityOptimizer::None)
	{
		if (this->_monotonicShape == RelaxivityOptimizer::Increase)
		{
			if (this->_basisFunctionShape == rw::RelaxivityDistribution::Sigmoid)
			{
				int Voxel_Length = this->Phenotype_Size() / 4;
				for (int k = 0; k < Voxel_Length; ++k)
				{
					if (c->Gene(4 * k) < c->Gene(4 * k + 1))
					{
						int xmax = c->Gene(4 * k + 1);
						c->Set_Gene(4 * k + 1, c->Gene(4 * k));
						c->Set_Gene(4 * k, xmax);
					}
				}
			}
		}
		if (this->_monotonicShape == RelaxivityOptimizer::Decrease)
		{
			if (this->_basisFunctionShape == rw::RelaxivityDistribution::Sigmoid)
			{

				int Voxel_Length = this->Phenotype_Size() / 4;
				for (int k = 0; k < Voxel_Length; ++k)
				{
					if (c->Gene(4 * k) > c->Gene(4 * k + 1))
					{
						int xmax = c->Gene(4 * k + 1);
						c->Set_Gene(4 * k + 1, c->Gene(4 * k));
						c->Set_Gene(4 * k, xmax);
					}
				}
			}
		}
	}
}

void RelaxivityOptimizer::Set_Sigmoid_Cuts(scalar flag, scalar size, int idx)
{
	int imin = (int)(flag*(scalar)10000);
	int imax = (int)(size*(scalar)10000);
	this->Set_Gene_Limits(4 * idx + 2, imin-imin/100,imax+imax/100);
}

void RelaxivityOptimizer::Set_Monotonic(RelaxivityOptimizer::Monotonic monotonic)
{
	this->_monotonicShape = monotonic;
}


void RelaxivityOptimizer::Set_Sigmoid_Min_Max(scalar minmin, scalar maxmin, scalar minmax, scalar maxmax, int id)
{
	int iminmin = (int)(minmin*(scalar)10000);
	int imaxmin = (int)(maxmin*(scalar)10000);
	int iminmax = (int)(minmax*(scalar)10000);
	int imaxmax = (int)(maxmax*(scalar)10000);

	int Voxel_Length = this->Phenotype_Size() / 4;
	this->Set_Gene_Limits(4 * id + 1, iminmin, imaxmin);
	this->Set_Gene_Limits(4 * id, iminmax, imaxmax);
	this->Set_Precision(4 * id + 1, 10000);
	this->Set_Precision(4 * id, 10000);
}

void RelaxivityOptimizer::Set_Sigmoid_Slope_Min_Max(scalar flag, scalar size, int id)
{
	int imin = (int)(flag*(scalar)1000);
	int imax = (int)(size*(scalar)1000);
	int Voxel_Length = this->Phenotype_Size() / 4;
	this->Set_Gene_Limits(4 * id + 3, imin, imax);
	this->Set_Precision(4 * id + 3, 1000);
}

void RelaxivityOptimizer::Set_Hat_Amplitude(scalar kmin, scalar kmax, int id)
{
	int ikmin = (int)(kmin*scalar(10000));
	int ikmax = (int)(kmax*scalar(10000));
	this->Set_Gene_Limits(3*id,ikmin,ikmax);
	this->Set_Precision(3 * id, 10000);
}

void RelaxivityOptimizer::Set_Hat_Cuts(scalar xminmin, scalar xminmax, scalar xmaxmin, scalar xmaxmax, int id)
{
	int ixmin = (int)(xminmin*scalar(10000));
	int ixmax = (int)(xminmax*scalar(10000));
	this->Set_Gene_Limits(3 * id + 1, ixmin, ixmax);
	this->Set_Precision(3 + id + 1, 10000);
	ixmin = (int)(xmaxmin*scalar(10000));
	ixmax = (int)(xmaxmax*scalar(10000));
	this->Set_Gene_Limits(3 * id + 2, ixmin, ixmax);
	this->Set_Precision(3 + id + 2, 10000);
}

void RelaxivityOptimizer::Set_Hat_Cuts(scalar xmin, scalar xmax, int id)
{
	int ixmin = (int)(xmin*scalar(10000));
	int ixmax = (int)(xmax*scalar(10000));
	int Voxel_Length = this->Phenotype_Size() / 3;
	this->Set_Gene_Limits(3 * id + 1, ixmin, ixmax);
	this->Set_Gene_Limits(3 * id + 2, ixmin, ixmax);
	this->Set_Precision(3 * id + 1, 10000);
	this->Set_Precision(3 * id + 2, 10000);
}


void RelaxivityOptimizer::Fix_Sigmoid_Slope(scalar Decay_Step_Value, int id)
{
	int iv = (int)(Decay_Step_Value*(scalar)1000);
	this->Set_Gene_Limits(4 * id + 3, iv, iv);
	this->Fix(4 * id + 3, iv);
}

void RelaxivityOptimizer::Fix_Hat_Cut_Min(scalar Decay_Step_Value, int id)
{
	int iv = (int)(Decay_Step_Value*(scalar)10000);
	int Voxel_Length = this->Phenotype_Size() / 3;
	this->Set_Gene_Limits(3 * id + 1, iv, iv);
	this->Fix(3 * id + 1, iv);
}

void RelaxivityOptimizer::Fix_Hat_Cut_Max(scalar Decay_Step_Value, int id)
{
	int iv = (int)(Decay_Step_Value*(scalar)10000);
	int Voxel_Length = this->Phenotype_Size() / 3;
	this->Set_Gene_Limits(3 * id + 2, iv, iv);
	this->Fix(3 * id + 2, iv);
}

void RelaxivityOptimizer::Set_Mapping_Simulation(const rw::PlugPersistent* sim)
{
	if (sim)
	{
		this->_mappingSimulation = sim;
		if (sim->Laplace_Applied())
		{
			this->_laplaceTransform = sim->Laplace_Bin_Vector();
		}
	}
}

const rw::PlugPersistent& RelaxivityOptimizer::Mapping_Simulation() const
{
	return(*this->_mappingSimulation);
}

void RelaxivityOptimizer::Repeat_Paths(bool r, uint seed)
{
	this->_repeatPath = r;
	this->_seed = seed;
}

void RelaxivityOptimizer::Set_Cycle(uint cycle)
{
	this->_rtfUpdateInterval = cycle;
}

}