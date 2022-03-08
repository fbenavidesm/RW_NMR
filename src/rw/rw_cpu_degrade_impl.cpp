#include <tbb/parallel_for.h>
#include <tbb/partitioner.h>
#include <tbb/parallel_reduce.h>
#include <mkl_vsl.h>
#include <time.h>
#include "rw_cpu_degrade_impl.h"

namespace rw
{
	static const int TimeSize = 512;
	static const int Block_Size = 397312;
	
	RandomWalkCPUDegradeImplementor::RandomWalkCPUDegradeImplementor(rw::Plug* parent) : RandomWalkImplementor(parent)
	{
		this->_maxRnd = 6;
		this->_chunkSize = 8;
		this->_shared = false;
		this->_collisionDistribution = allocInt((int)this->Plug().Number_Of_Walking_Particles()*TimeSize);
		this->_magnetization = new scalar[TimeSize];
		this->_textureDepth = parent->Plug_Texture().Depth();
		this->_textureHeight = parent->Plug_Texture().Height();
		this->_textureWidth = parent->Plug_Texture().Width();
	}

	RandomWalkCPUDegradeImplementor::~RandomWalkCPUDegradeImplementor()
	{
		if (!this->_shared)
		{
			freeInt(this->_collisionDistribution);
		}
		if (this->_magnetization)
		{
			delete []this->_magnetization;
		}
	}

	RandomWalkCPUDegradeImplementor::RandomWalkCPUDegradeImplementor(RandomWalkCPUDegradeImplementor& p, split) : RandomWalkImplementor(p)
	{
		this->_shared = true;
		this->_collisionDistribution = p._collisionDistribution;
		this->_magnetization = new scalar[TimeSize];
		for (int k = 0; k < TimeSize; ++k)
		{
			this->_magnetization[k] = 0;
		}
		this->_chunkSize = p._chunkSize;
		this->_maxRnd = p._maxRnd;
		this->_textureDepth = p._textureDepth;
		this->_textureHeight = p._textureHeight;
		this->_textureWidth = p._textureWidth;
	}



	void RandomWalkCPUDegradeImplementor::init()
	{
		for (int k = 0; k < TimeSize; ++k)
		{
			this->_magnetization[k] = 0;
		}
	}

	void RandomWalkCPUDegradeImplementor::Set_Max_Rnd(uint maxrnd)
	{
		this->_maxRnd = maxrnd;
	}

	void RandomWalkCPUDegradeImplementor::operator()()
	{
		this->Execute();
	}

	void RandomWalkCPUDegradeImplementor::Set_Degrees_Of_Freedom()
	{
		this->_maxRnd = 6;
	}

	void RandomWalkCPUDegradeImplementor::Execute()
	{
		this->Set_Degrees_Of_Freedom();
		rw::Plug& frm_sample = this->Plug();
		this->_chunkSize = frm_sample.Minimal_Walkers_Per_Thread();
		tbb::affinity_partitioner partitioner;
		clock_t tstart = clock();
		frm_sample.Clear_Decay_Steps();
		this->Reserve_Values_Memory_Space(300000);
		int nw = (int)frm_sample.Number_Of_Walking_Particles();
		scalar E = 1;
		scalar rate_factor = exp(-frm_sample.Time_Step() / frm_sample.TBulk_Seconds());
		uint N = 0;
		uint Voxel_Length = 0;		
		this->Init_Iterations();
		uint seed = 0;
		if (frm_sample.Repeating_Walkers_Paths())
		{
			seed = frm_sample.Seed_For_Random_Number_Generation();
		}
		else
		{
			seed = (uint)this->Pick_New_Seed();
		}
		VSLStreamStatePtr stream;
		vslNewStream(&stream, VSL_BRNG_SFMT19937, seed);

		this->Set_Seed(seed);
		scalar Ebulk = (scalar)1.0;
		uint currentIteration = 0;
		int updprof = frm_sample.Update_Profile_Interval();
		int updsr = frm_sample.Update_Varying_Relaxivity_Steps();
		while ((E > frm_sample.Stop_Threshold()) && (currentIteration < (int)frm_sample.Max_Number_Of_Iterations()))
		{
			this->init();
			viRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream,
				(int)frm_sample.Number_Of_Walking_Particles()*TimeSize, this->_collisionDistribution, 0, this->_maxRnd);
			tbb::parallel_reduce(tbb::blocked_range<int>(0, nw, this->_chunkSize), *this, partitioner);
			bool updateCollision = false;
			for (int k = 0; k < TimeSize; ++k)
			{
				scalar frac = this->_magnetization[k] / (scalar)(frm_sample.Number_Of_Walking_Particles());
				Ebulk = Ebulk * rate_factor;
				E = frac;
				E = E * Ebulk;
				rw::Step_Value v;
				v.Magnetization = E;
				v.Iteration = currentIteration;
				v.Time = ((scalar)currentIteration)*frm_sample.Time_Step();
				this->Push_Seq_Step_Value(v);
				++currentIteration;						
				if ((updprof > 0) && (currentIteration % updprof == 0))
				{
					updateCollision = true;
				}
			}
			if (updateCollision)
			{
				frm_sample.Update_Collision_Profile(currentIteration);
			}
			this->Observe(currentIteration, E);
		}		
		frm_sample.Set_Total_Number_Of_Simulated_Iterations(currentIteration);
		this->Check_T1_Experiment();
		clock_t tend = clock();
		clock_t diff = tend - tstart;
		float ms_elapsed = static_cast<float>(diff) * 1000 / CLOCKS_PER_SEC;
		int isecs = (int)ms_elapsed;
		isecs = isecs / 1000;
		this->Walk_End(isecs);
		freeInt(this->_collisionDistribution);
		delete[]this->_magnetization;
		this->_collisionDistribution = 0;
		this->_magnetization = 0;
		vslDeleteStream(&stream);
	};

	void RandomWalkCPUDegradeImplementor::join(RandomWalkCPUDegradeImplementor& p)
	{
		for (int k = 0; k < TimeSize; ++k)
		{
			this->_magnetization[k] = this->_magnetization[k] + p._magnetization[k];
		}
	};

	void RandomWalkCPUDegradeImplementor::operator()(const blocked_range<int>& range)
	{
		rw::Plug& f = this->Plug();
		int nw = f.Number_Of_Walking_Particles();
		for (int t = range.begin(); t < range.end(); ++t)
		{
			for (int k = 0; k < TimeSize; ++k)
			{
				rw::Walker& w = this->Walker(t);		
				int* rnd = this->_collisionDistribution + t * TimeSize + k;
				this->Process_Walker(t, w, this->_collisionDistribution[t*TimeSize + k],f.Gradient());
				this->_magnetization[k] = this->_magnetization[k] + w.Magnetization();
			}
		}
	};

	void RandomWalkCPUDegradeImplementor::Process_Walker(int id, rw::Walker& walker, int rnd, const Field3D& gradient)
	{
		Pos3i pp;

		int dxy = ((int)(rnd) << 1);
		int dg1 = 1 - (dxy >> 3);
		int dg2 = dxy >> 2;

		int dx = dg1 * (1 - dg2)*(dxy - 1);
		int dy = dg1 * dg2*(dxy - 5);
		int dz = (1 - dg1)*(dg2 >> 1)*(dxy - 9);

		rw::Pos3i pw = walker.Position();
		pp.x = pw.x + dx;
		pp.y = pw.y + dy;
		pp.z = pw.z + dz;

		if ((pp.x >= 0) && (pp.x < (int)this->_textureWidth) &&
			(pp.y >= 0) && (pp.y < (int)this->_textureHeight) &&
			(pp.z >= 0) && (pp.z < (int)this->_textureDepth))
		{
			int v = this->Image()(pp);
			if (v == 0)
			{
				walker.Set_Position(pp);
				walker.Set_Magnetization(walker.Magnetization()*gradient.Exponential_Factor(dx, dy, dz));
			}
			else
			{
				walker.Degrade();
			}
		}
	}

}