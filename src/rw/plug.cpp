#include <ostream>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <math.h>
#include <limits>
#include "plug.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_reduce.h"
#include "relaxivity_distribution.h"
#include "random_walk_implementor.h"
#include "rw_impl_creator.h"
#include "rw_placer.h"

namespace rw
{

	bool Plug::_seedGenerated = false;
	std::ranlux48 Plug::_randomSeedGenerator;

	Plug::Plug()
	{
		this->_implementor = 0;
		this->_mask = 0;
		this->_simParams.Set_Bool(T2, true);
		this->_image = 0;
		this->_simParams.Set_Value(PROFILE_SIZE, 32768);
		this->_simParams.Set_Value(PROFILE_UPDATE, 0);
		this->_simParams.Set_Bool(VARYING, false);
		this->_simParams.Set_Value(ITERATION_LIMIT, std::numeric_limits<int>::max());
		this->_decayValues.reserve(16000);
		this->_surfaceRelaxationRate = (scalar)0.01;
		this->_bulkRelaxationTime = (scalar)0.0001;
		this->_timeStep = (scalar)5;
		this->_stopEnergyThreshold = (scalar)0.001;
		this->_updateEvent = 0;
		this->_simParams.Set_Value(NO_OF_WALKERS, 1024);
		this->_walkersPlaced = false;
		this->_pixelSize = 1;
		this->_simParams.Set_Value(MIN_WALKERS_PER_THREAD, 4 * DCHUNK_SIZE);
		this->_showWalkers = false;
		if (!Plug::_seedGenerated)
		{
			std::random_device rd;
			Plug::_randomSeedGenerator.seed(rd());
			Plug::_seedGenerated = true;
			Plug::_randomSeedGenerator.discard(256);
		}
		uint seed = (uint)Plug::_randomSeedGenerator();
		this->_simParams.Set_Value(SEED, seed);
	}

	void Plug::Set_Simulation_Parameter(const SimulationParams& params)
	{
		this->_simParams = params;
	}

	void Plug::Set_Total_Number_Of_Simulated_Iterations(uint itrs)
	{
		this->_totalIterations = itrs;
	}


	void Plug::Limit_Maximal_Number_Of_Iterations(uint maxit)
	{
		if (maxit == 0)
		{
			this->_simParams.Set_Value(ITERATION_LIMIT,std::numeric_limits<int>::max());
		}
		else
		{
			this->_simParams.Set_Value(ITERATION_LIMIT, maxit);
		}
	}

	Plug::Plug(const Plug& e, bool copyWalkers)
	{
		this->_mask = e._mask;
		this->_simParams = e._simParams;
		this->_image = e._image;
		this->_decayValues.reserve(16000);
		this->_surfaceRelaxationRate = (scalar)e._surfaceRelaxationRate;
		this->_bulkRelaxationTime = (scalar)e._bulkRelaxationTime;
		this->_timeStep = e._timeStep;
		this->_stopEnergyThreshold = (scalar)e._stopEnergyThreshold;
		this->_updateEvent = 0;
		this->_walkersPlaced = e._walkersPlaced;
		this->_pixelSize = e._pixelSize;
		if (copyWalkers)
		{
			this->_walkers = e._walkers;
			this->_walkersStartPosition = e._walkersStartPosition;
		}
		this->_gradient = e._gradient;
		this->_implementor = 0;
		this->_simParams.Set_Value(SEED,(uint)Plug::_randomSeedGenerator());
	}

	bool Plug::Masked(int idx) const
	{
		bool r = false;
		return(r);
	}

	Plug::~Plug()
	{
		if (this->_mask)
		{
			delete this->_mask;
		}
		if (this->_image)
		{
			delete this->_image;
		}
		this->Clear_Collision_Profile();
		if (this->_implementor)
		{
			delete this->_implementor;
		}
	}

	void Plug::Set_Image_Formation(const BinaryImage& image)
	{
		this->_image = new BinaryImage(image);
		this->_simParams.Set_Value(DIM_X,image.Width());
		this->_simParams.Set_Value(DIM_Y, image.Height());
		this->_simParams.Set_Value(DIM_Z, image.Depth());
	}

	void Plug::Lock()
	{
		this->_simulationMutex.lock();
	}

	void Plug::UnLock()
	{
		this->_simulationMutex.unlock();
	}

	
	void Plug::Deallocate_Walking_Particles()
	{
		this->_walkersPlaced = false;
	}


	void Plug::Set_Time_Step(scalar tsc)
	{
		this->_timeStep = tsc;
		this->_walkersPlaced = false;
	}

	scalar Plug::Time_Step() const
	{
		return(this->_timeStep);
	}


	void Plug::Set_Stop_Threshold(scalar perc)
	{
		if ((perc > 0) && (perc < 1))
		{
			this->_stopEnergyThreshold = perc;
		}
		this->_walkersPlaced = false;
	}

	scalar Plug::Stop_Threshold() const
	{
		return(this->_stopEnergyThreshold);
	}


	void Plug::Set_Number_Of_Walking_Particles(uint N)
	{
		this->_walkersPlaced = false;
		this->_simParams.Set_Value(NO_OF_WALKERS, N);
		if (this->_walkers.size() != N)
		{
			this->_walkers.clear();
			this->_walkersStartPosition.clear();
			this->_walkers.resize(N);
			this->_walkersStartPosition.resize(N);
		}
		this->_simParams.Set_Value(MIN_WALKERS_PER_THREAD,std::max((uint)(N/128),(uint)128));
	}

	bool Plug::Has_Walk_Event() const
	{
		return(this->_updateEvent != 0);
	}

	RandomWalkObserver* Plug::On_Walk_Event()
	{
		return(this->_updateEvent);
	}

	void Plug::Set_TBulk_Time_Seconds(scalar rate)
	{
		this->_bulkRelaxationTime = rate;
		this->_walkersPlaced = false;
	}

	void Plug::Set_Surface_Relaxivity_Delta(scalar rate)
	{
		this->_surfaceRelaxationRate = rate;
		this->_walkersPlaced = false;
	}

	scalar Plug::TBulk_Seconds() const
	{
		return(this->_bulkRelaxationTime);
	}

	scalar Plug::Surface_Relaxivity_Delta() const
	{
		return(this->_surfaceRelaxationRate);
	}


	void Plug::Set_On_Walk_Event(RandomWalkObserver* evt)
	{
		this->_updateEvent = evt;
		if (this->_updateEvent)
		{
			this->_updateEvent->_parentFormation = this;
			this->_updateEvent->_showWalkers = this->_showWalkers;
		}
	}

	void Plug::Show_Walkers(bool enable)
	{
		this->_showWalkers = enable;
		if (this->_updateEvent)
		{
			this->_updateEvent->_showWalkers = this->_showWalkers;
		}
	}

	void Plug::Random_Walk_Procedure()
	{
		rw::RandomWalkImplementorCreator::Create_And_Associate_Implementor(this);
		if (this->_implementor)
		{
			(*this->_implementor)();
		}
	}

	void Plug::Simulate_Random_Walk_Procedure(rw::Simulator& simulator)
	{
		rw::RandomWalkImplementorCreator::Create_And_Associate_Implementor_Simulator(this,&simulator);
		if (this->_implementor)
		{
			(*this->_implementor)();
		}
	}


	void Plug::Update_Collision_Profile(uint currentItr)
	{
		uint ss = this->_simParams.Get_Value(PROFILE_SIZE);
		uint compr = this->_simParams.Get_Value(PROFILE_UPDATE);
		int rr = std::max((int)(this->_walkers.size() / (10 * ss)), (int)1);
		map<int, int> xiprof;
		vector<scalar> hst(this->_walkers.size());
		vector<scalar>* chst = new vector<scalar>(compr);
		for (int t = 0; t < this->_walkers.size(); ++t)
		{
			Walker& w = this->_walkers[t];
			int xi = (int)w.Hits();
			map<int, int>::iterator iix = xiprof.find(xi);
			if (iix != xiprof.end())
			{
				iix->second = iix->second + 1;
			}
			else
			{
				xiprof[xi] = 1;
			}
		}
		map<int, int>::const_reverse_iterator iix = xiprof.rbegin();
		int ii = 0;
		while (iix != xiprof.rend())
		{
			for (int k = 0; k < iix->second; ++k)
			{
				hst[ii] = (scalar)iix->first/(scalar)currentItr;
				++ii;
			}
			++iix;
		}
		scalar factor = (scalar)this->_walkers.size() / (scalar)ss;
		for (uint i = 0; i < ss; ++i)
		{
			int k = (int)(((scalar)i)*factor);
			int flag = std::max(0, k - rr);
			int size = std::min(k + rr, (int)this->_walkers.size());
			int n = 0;
			scalar xi = 0;
			for (int j = flag; j < size; ++j)
			{
				xi = xi + hst[j];
				++n;
			}
			(*chst)[i] = xi / (scalar)n;
		}
		this->_profileSequence.push_back(chst);
	}

	void Plug::Set_Collision_Rate_Profile_Interval(int steps, int size)
	{
		if (steps % 2 != 0)
		{
			++steps;
		}
		this->_simParams.Set_Value(PROFILE_UPDATE,steps);
		this->_simParams.Set_Value(PROFILE_SIZE, size);
	}

	void Plug::Clear_Collision_Profile()
	{
		for (int k = 0; k < (int)this->_profileSequence.size(); ++k)
		{
			vector<scalar>* profile = this->_profileSequence[k];
			delete profile;
		}
		this->_profileSequence.clear();
	}

	void Plug::Set_Relaxivity_Distribution(RelaxivityDistribution* f, uint itr)
	{
		this->_simParams.Set_Bool(VARYING,true);
		f->Set_Chunk_PP(this->_simParams.Get_Value(MIN_WALKERS_PER_THREAD));	
		if (itr > 0)
		{
			f->Update_Walker_Rho(this->_walkers, itr);
		}
	}

	bool Plug::Kill_Walking_Particle(Walker& w, scalar decision)
	{
		bool r = false;
		w.Set_Strikes(w.Hits() + 1);
		if (decision >= w.Rho())
		{
			if (w.Alive())
			{
				r = true;
				w.Kill();
				w.Set_Magnetization(0);
			}
		}
		return(r);
	}

	void Plug::Recharge_Walkers_Magnetization()
	{
		tbb::parallel_for(tbb::blocked_range<int>(0, this->Number_Of_Walking_Particles(), BCHUNK_SIZE), [this](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i != b.end(); ++i)
			{
				Walker& w = this->Walking_Particle(i);
				w.Set_Magnetization(1);
				w.Set_Strikes(0);
			}
		});
		this->Clear_Decay_Steps();
	}

	
	scalar Plug::Pick_Random_Normalized_Number()
	{
		this->Lock();
		scalar n = (scalar)(this->_randomGenerator() - this->_randomGenerator.min());
		this->UnLock();
		scalar d = (scalar)(this->_randomGenerator.max() - this->_randomGenerator.min());
		return(n / d);
	}


	void Plug::Activate_GPU_Walk(bool gpu)
	{
		this->_simParams.Set_Bool(GPU_P, gpu);
	}

	void Plug::Start_Walk()
	{
		if (this->_simParams.Get_Bool(GPU_P))
		{
			std::thread t(&Plug::Random_Walk_Procedure,this);
			t.detach();
		}
		else
		{
			std::thread t(&Plug::Random_Walk_Procedure,this);
			t.detach();
		}
	}

	void Plug::Clear_Decay_Steps()
	{
		this->_decayValues.clear();
	}

	rw::Step_Value Plug::Decay_Step_Value(uint i) const
	{
		return(this->_decayValues[i]);
	}

	void Plug::Add_Decay_Step_Value(rw::Step_Value& value)
	{
		this->_decayValues.push_back(value);
	}

	scalar Plug::Porosity() const
	{
		scalar p = (scalar)this->_image->Black_Voxels();
		scalar n = (scalar)(this->_image->Width()*this->_image->Height()*this->_image->Depth());
		return(p/n);
	}

	void Plug::Place_Walkers_In_Restricted_Collision_Rate(scalar min, scalar max, scalar factor)
	{
		vec(Walker) nw;
		nw.reserve(this->_walkers.size()/2);
		for (int i = 0; i < this->_walkers.size(); ++i)
		{
			Walker w = this->_walkers[i];
			scalar str = (scalar)w.Hits() / factor;
			if ((str >= min) && (str <= max))
			{
				w.Set_Magnetization(1);
				w.Set_Strikes(0);
				nw.push_back(w);
			}
		}
		this->_walkers = nw;	
		this->_simParams.Set_Value(NO_OF_WALKERS,(uint)this->_walkers.size());
	}

	Pos3i Plug::Walker_Starting_Position(int id) const
	{
		return(this->_walkersStartPosition[id]);
	}


	void Plug::Reassign_Walkers_To_Starting_Position()
	{
		int N = (int)this->_walkers.size();
		for (int i = 0; i < N; ++i)
		{
			rw::Pos3i& pp = this->_walkersStartPosition[i];
			Walker& w = this->_walkers[i];
			int x = w(eX);
			int y = w(eY);
			int z = w(eZ);
			w(eX,pp.x);
			w(eY,pp.y);
			w(eZ,pp.z);
			pp.x = x;
			pp.y = y;
			pp.z = z;
		}
	}



	bool Plug::Repeating_Walkers_Paths() const
	{
		return(this->_simParams.Get_Bool(REPEAT));
	}

	void Plug::Repeat_Walkers_Paths(bool v, uint seed)
	{
		this->_simParams.Set_Bool(REPEAT, v);
		if (this->_simParams.Get_Bool(REPEAT))
		{
			this->_simParams.Set_Value(SEED, seed);
		}
	}

	int Plug::Image_Size(int i) const
	{
		i = i % 3;
		uint j = DIM_X+i;
		return(this->_simParams.Get_Value(j));
	}

	void Plug::Set_Internal_Gradient(const Field3D& gradient)
	{
		this->_gradient = gradient;
	}

	void Plug::Init_Walkers_Position()
	{
		uint nw = this->_simParams.Get_Value(NO_OF_WALKERS);
		for (uint i = 0; i < nw; ++i)
		{
			Walker& w = this->_walkers[i];
			w(0, this->_walkersStartPosition[i].x);
			w(1, this->_walkersStartPosition[i].y);
			w(2, this->_walkersStartPosition[i].z);
		}
	}

	void Plug::Set_Mask(const rw::BinaryImage& image)
	{
		this->_mask = new BinaryImage(image);
	}

	void Plug::Place_Walking_Particles()
	{
		RandomWalkPlacer* placer = RandomWalkImplementorCreator::Create_Uniform_Placer(this);
		(*placer)();
		delete placer;
	}

	uint Plug::Pick_New_Seed()
	{
		uint seed = (uint)Plug::_randomSeedGenerator();
		this->_simParams.Set_Value(SEED, seed);
		return(seed);
	}

	uint Plug::Total_Number_Of_Simulated_Iterations() const
	{
		return((uint)this->_totalIterations);
	}

}

