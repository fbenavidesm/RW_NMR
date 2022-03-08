#include <amp.h>
#include "rw_gpu_degrade_impl.h"
#include "binary_image/binary_image_executor.h"

namespace rw
{
	static const uint Precision = 10000;
	static const int TimeSize = 512;
	static const int Block_Size = 397312;

	RandomWalkGPUDegradeImplementor::RandomWalkGPUDegradeImplementor(rw::Plug* parent) : RandomWalkImplementor(parent)
	{
		this->_precision = Precision;
		int n = (int)this->Plug().Number_Of_Walking_Particles();
		this->_blockSize = n;
		this->_noblocks = (int)this->Plug().Number_Of_Walking_Particles() / Block_Size;
		if (this->_noblocks == 0)
		{
			this->_noblocks = 1;
		}
		else
		{
			if (Block_Size*this->_noblocks < (int)this->Plug().Number_Of_Walking_Particles())
			{
				this->_noblocks = this->_noblocks + 1;
			}
			this->_blockSize = Block_Size;
		}
		this->_imagnetization = vec(uint)(2 * TimeSize, 0);
		this->_magnetization = vec(scalar)(TimeSize, 0);
		this->_blockMagnetization = vec(scalar)(this->_noblocks*TimeSize, 0);
		this->_rndSeeds = 0;		
		this->_image = 0;
		this->_mask = 0;
		this->_image = BinaryImageExecutor::Create_GPU_Texture(this->Image());
		if (this->Masked())
		{
			this->_mask = BinaryImageExecutor::Create_GPU_Texture(this->Mask());
		}
		this->_seeds = allocUInt(this->_blockSize);
		this->_rndSeeds = new concurrency::array<uint, 1>(this->_blockSize);
		this->_walkers = new concurrency::array<rw::Walker, 1>(this->_blockSize);
		if (this->_noblocks == 1)
		{
			concurrency::copy(this->Walkers().begin(), this->Walkers().end(), *this->_walkers);
		}
	};

	RandomWalkGPUDegradeImplementor::~RandomWalkGPUDegradeImplementor()
	{
		if (this->_seeds)
		{
			freeUInt(this->_seeds);
		}
		if ((this->_noblocks == 1) && (this->_walkers))
		{
			delete this->_walkers;
		}
		if (this->_rndSeeds)
		{
			delete this->_rndSeeds;
		}
		if (this->_image)
		{
			delete this->_image;
		}
		if (this->_mask)
		{
			delete this->_mask;
		}
		concurrency::accelerator acc = concurrency::accelerator(concurrency::accelerator::default_accelerator);
		concurrency::accelerator_view aview = acc.get_default_view();
		aview.flush();
	};

	void RandomWalkGPUDegradeImplementor::Init_Seeds(VSLStreamStatePtr& stream)
	{
		viRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream,
			(int)this->_blockSize, (int*)this->_seeds, -2147483647, 2147483647);
		concurrency::copy(this->_seeds, *this->_rndSeeds);
		for (int k = 0; k < TimeSize; ++k)
		{
			this->_imagnetization[2 * k] = 0;
			this->_imagnetization[2 * k + 1] = 0;
		}

	}

	void RandomWalkGPUDegradeImplementor::Init()
	{
		for (int k = 0; k < TimeSize; ++k)
		{
			this->_magnetization[k] = 1;
		}
	}

	void RandomWalkGPUDegradeImplementor::Degrade_Walker_GPU_Cross_Periodic(int id, int timestep, rw::Walker& walker, uint rnd, int depth, int width, int height,
		const concurrency::array<uint, 1>& texture, concurrency::array<uint, 1>& magnetization, uint precision) GPUP
	{
		rw::Pos3i size;
		size.x = width;
		size.y = height;
		size.z = depth;

		rw::Pos3i pp;

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
		if ((pp.x >= 0) && (pp.x < width) &&
			(pp.y >= 0) && (pp.y < height) &&
			(pp.z >= 0) && (pp.z < depth))
		{
			int v = BinaryImageExecutor::Accesor_Read(texture, pp, size);
			if (v == 0)
			{
				walker.Set_Position(pp);
			}
			else
			{
				uint mm = (uint)(walker.Magnetization()*((scalar)precision));
				concurrency::atomic_fetch_add(&magnetization[2 * timestep], mm);
				walker.Degrade();
				mm = (uint)(walker.Magnetization()*((scalar)precision));
				concurrency::atomic_fetch_add(&magnetization[2 * timestep + 1], mm);
			}
		}
	}

	void RandomWalkGPUDegradeImplementor::Execute_Walking_Step(VSLStreamStatePtr& stream)
	{
		concurrency::accelerator acc = concurrency::accelerator(concurrency::accelerator::default_accelerator);
		concurrency::accelerator_view aview = acc.get_default_view();
		
		int width = this->Plug().Image_Size(0);
		int height = this->Plug().Image_Size(1);
		int depth = this->Plug().Image_Size(2);;
		rw::Plug& f = this->Plug();

		concurrency::array<uint, 1>& texture = *this->_image;
		for (int b = 0; b < this->_noblocks; ++b)
		{
			int begin = b * this->_blockSize;
			int size = this->_blockSize;
			if (begin + size >(int)f.Number_Of_Walking_Particles())
			{
				size = (int)f.Number_Of_Walking_Particles() - begin;
			}
			if (this->_noblocks > 1)
			{
				vec(rw::Walker)::const_iterator begit = this->Walkers().begin() + begin;
				vec(rw::Walker)::const_iterator endit = begit + size;
				concurrency::copy(begit, endit, *this->_walkers);
			}
			this->Init_Seeds(stream);
			concurrency::array_view<rw::Walker, 1>& walkers = this->_walkers->section(0,size);
			concurrency::array<uint, 1>& seeds = *this->_rndSeeds;
			concurrency::array<uint, 1> magnetization(2 * TimeSize, this->_imagnetization.begin());
			uint precision = this->_precision;
			Field3D gradient = f.Gradient();
			parallel_for_each(walkers.extent,
				[walkers, &seeds, &magnetization, &texture, depth, width, height, precision, gradient]
			(concurrency::index<1> idx) restrict(amp)
			{
				for (int k = 0; k < TimeSize; ++k)
				{
					int i = idx[0];
					rw::Walker& w = walkers[i];
					seeds[i] = 1664525 * seeds[i] + 1013904223;
					scalar frnd = (scalar)seeds[i] / (scalar)4294967295;
					frnd = frnd * 6;
					int rnd = (int)(frnd);
					RandomWalkGPUDegradeImplementor::Degrade_Walker_GPU_Cross_Periodic(i, k, w, rnd, depth, width, height, texture,
						magnetization, precision);
				}
			});
			aview.wait();
			concurrency::copy(magnetization, this->_imagnetization.begin());
			for (int k = 0; k < TimeSize; ++k)
			{
				this->_blockMagnetization[b*TimeSize + k] = (scalar)this->_imagnetization[2 * k + 1] / (scalar)this->_precision -
					(scalar)this->_imagnetization[2 * k] / (scalar)this->_precision;
				this->_blockMagnetization[b*TimeSize + k] = this->_blockMagnetization[b*TimeSize + k]
					/ (scalar)this->Plug().Number_Of_Walking_Particles();
			}
			if (this->_noblocks > 1)
			{
				vec(rw::Walker)::iterator begit = this->Walkers().begin() + begin;				
				concurrency::copy(walkers, begit);
			}
		}
		for (int k = 0; k < TimeSize; ++k)
		{
			for (int i = 0; i < this->_noblocks; ++i)
			{
				this->_magnetization[k] = this->_magnetization[k] + this->_blockMagnetization[i*TimeSize + k];
			}
			if (k < TimeSize - 1)
			{
				this->_magnetization[k + 1] = this->_magnetization[k];
			}
		}
	}

	void RandomWalkGPUDegradeImplementor::Copy_Walkers_To_CPU()
	{
		if (this->_noblocks == 1)
		{
			concurrency::copy(*this->_walkers, this->Walkers().begin());
		}
	}

	void RandomWalkGPUDegradeImplementor::Execute()
	{
		clock_t tstart = clock();
		this->Plug().Clear_Decay_Steps();
		this->Reserve_Values_Memory_Space(300000);
		int nw = (int)this->Plug().Number_Of_Walking_Particles();
		scalar E = 1;
		uint N = 0;
		uint voxel_length = 0;
		uint currentIteration = 0;
		scalar rate = this->Plug().TBulk_Seconds();
		uint seed = 0;
		if (this->Plug().Repeating_Walkers_Paths())
		{
			seed = this->Plug().Seed_For_Random_Number_Generation();
		}
		else
		{
			seed = (uint)this->Pick_New_Seed();
		}
		VSLStreamStatePtr stream;
		vslNewStream(&stream, VSL_BRNG_SFMT19937, seed);
		this->Set_Seed(seed);
		uint b = 6;
		scalar rate_factor = exp(-this->Plug().Time_Step() / this->Plug().TBulk_Seconds());
		scalar Ebulk = (scalar)1.0;
		this->Init();
		rw::Plug& f = this->Plug();
		Field3D gradient = this->Plug().Gradient();
		int updprof = f.Update_Profile_Interval();
		int updsr = f.Update_Varying_Relaxivity_Steps();
		bool walkers_copied = false;
		while ((E > f.Stop_Threshold()) && (currentIteration < f.Max_Number_Of_Iterations()))
		{
			this->Execute_Walking_Step(stream);
			bool updateCollision = false;
			bool updateRho = false;
			walkers_copied = false;
			for (int k = 0; k < TimeSize; ++k)
			{
				scalar frac = this->_magnetization[k];
				Ebulk = Ebulk * rate_factor;
				E = frac;
				E = E * Ebulk;
				rw::Step_Value v;
				E = E * (gradient.Exponential_Factor(1, 0, 0) +
					gradient.Exponential_Factor(0, 1, 0) +
					gradient.Exponential_Factor(0, 0, 1));
				E = E / (scalar)3;
				v.Magnetization = E;
				v.Iteration = currentIteration;
				v.Time = ((scalar)currentIteration)*this->Plug().Time_Step();
				this->Push_Seq_Step_Value(v);
				++currentIteration;
				if ((updprof > 0) && (currentIteration % this->Plug().Update_Profile_Interval())
					&&(!walkers_copied))
				{
					this->Copy_Walkers_To_CPU();
					walkers_copied = true;
					updateCollision = true;
				}
				if ((updsr > 0) && (currentIteration % updsr == 0) 
					&&(!walkers_copied))
				{
					this->Copy_Walkers_To_CPU();
					walkers_copied = true;
					updateRho = true;
				}		
			}
			if (updateCollision)
			{
				f.Update_Collision_Profile(currentIteration);
			}
			if (this->Has_Walk_Event())
			{
				if ((this->Plug().Observer().Show_Walkers())&&(!walkers_copied))
				{
					this->Copy_Walkers_To_CPU();
					walkers_copied = true;
				}
				this->Observe(currentIteration, E);
			}
			this->_magnetization[0] = this->_magnetization[TimeSize - 1];
		}
		this->Plug().Set_Total_Number_Of_Simulated_Iterations(currentIteration);
		if (!walkers_copied)
		{
			this->Copy_Walkers_To_CPU();
		}
		vslDeleteStream(&stream);
		this->Check_T1_Experiment();
		clock_t tend = clock();
		if (this->Has_Walk_Event())
		{
			clock_t diff = tend - tstart;
			float ms_elapsed = static_cast<float>(diff) * 1000 / CLOCKS_PER_SEC;
			int ims_elapsed = (int)ms_elapsed;
			this->Walk_End(ims_elapsed/1000);
		}
		if (this->_seeds)
		{
			freeUInt(this->_seeds);
			this->_seeds = 0;
		}
		if ((this->_noblocks == 1) && (this->_walkers))
		{
			delete this->_walkers;
			this->_walkers = 0;
		}
		if (this->_rndSeeds)
		{
			delete this->_rndSeeds;
			this->_rndSeeds = 0;
		}		
		if (this->_image)
		{
			delete this->_image;
			this->_image = 0;
		}
	}

	void RandomWalkGPUDegradeImplementor::operator()()
	{
		this->Execute();
	}
}