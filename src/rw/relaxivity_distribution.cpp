#include <algorithm>
#include "tbb/parallel_for.h"
#include "relaxivity_distribution.h"
#include "persistence/plug_persistent.h"

namespace rw
{

	RelaxivityDistribution::RelaxivityDistribution()
	{
		this->_mappingSimulation = 0;
		this->_chunkSize = 4*DCHUNK_SIZE;
		this->_enableCycle = false;
		this->_last_Iteration = 0;
	}


	RelaxivityDistribution::~RelaxivityDistribution()
	{
	}

	scalar RelaxivityDistribution::Relaxivity_Factor(scalar rho) const
	{
		return(this->_mappingSimulation->Surface_Relaxivity_Factor(rho));
	}

	void RelaxivityDistribution::Set_Chunk_PP(uint chunk)
	{
		this->_chunkSize = chunk;
	}

	void RelaxivityDistribution::Init_Walkers(vec(Walker)& wset)
	{
		if (this->_chunkSize == 0)
		{
			this->_chunkSize = (uint)wset.size() / 4;
		}
		this->Init_Walkers(wset, this->_mappingSimulation->Total_Number_Of_Simulated_Iterations());
	}

	void RelaxivityDistribution::Init_Walkers(vec(Walker)& wset, int total_iterations)
	{
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)wset.size(), this->_chunkSize), 
			[&wset, total_iterations, this](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				Walker& ww = wset[i];
				scalar xi = (scalar)ww.Hits() / (scalar)total_iterations;
				scalar rho = this->Evaluate(xi);
				ww.Set_Rho(this->Relaxivity_Factor(rho));
				ww.Set_Strikes(0);
			}
		});
	}

	void RelaxivityDistribution::Update_Walker_Rho(vec(Walker)& wset, int itr)
	{
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)wset.size(), this->_chunkSize), [&wset, itr, this](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				Walker& ww = wset[i];
				scalar xi = (scalar)ww.Hits() / (scalar)(itr - this->_last_Iteration);
				scalar rho = this->Evaluate(xi);
				ww.Set_Rho(this->Relaxivity_Factor(rho));
				if (this->_enableCycle)
				{
					ww.Set_Strikes(0);
				}
			}
		});
		if (this->_enableCycle)
		{
			this->_last_Iteration = itr;
		}
	}

	void RelaxivityDistribution::Enable_Sequential_Profile(bool enable)
	{
		this->_enableCycle = enable;
	}

	void RelaxivityDistribution::Set_Simulation(const rw::PlugPersistent& sim)
	{
		this->_mappingSimulation = &sim;
	}


	void RelaxivityDistribution::Laplace_PSD(scalar rmin, scalar rmax, scalar step,
		const math_la::math_lac::full::Vector& time_domain, const math_la::math_lac::full::Vector& bins, scalar v,
		vector<scalar2>& distribution, scalar t2b)
	{
		distribution.clear();
		distribution.reserve(bins.Size());
		scalar r = rmin;
		while (r < rmax)
		{
			scalar c_r = r + step / 2;
			scalar ximin = (3 * v) / (4 * r);
			scalar ximax = (3 * v) / (4 * (r + step));
			scalar rhomin = this->Evaluate(ximin);
			scalar rhomax = this->Evaluate(ximax);
			scalar t2min = v / (4 * rhomin*ximin);
			scalar t2max = v / (4 * rhomax*ximax);
			if (t2min > t2max)
			{
				scalar aux = t2max;
				t2max = t2min;
				t2min = aux;
			}
			t2min = 1 / (1 / t2min + 1 / t2b);
			t2max = 1 / (1 / t2max + 1 / t2b);
			scalar bin = 0;
			if (r+step >= rmax)
			{
				t2max = 1e24;
			}
			for (int k = 0; k < time_domain.Size(); ++k)
			{
				scalar t2 = time_domain(k);
				if ((t2 >= t2min) && (t2 < t2max))
				{
					bin = bin + bins(k);
				}
			}
			scalar2 vv;
			vv.x = c_r;
			vv.y = bin;
			distribution.push_back(vv);
			r = r + step;
		}
	}

	

}