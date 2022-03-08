#include <set>
#include <vector>
#include <math.h>
#include "tbb/parallel_for.h"
#include "tbb/parallel_reduce.h"
#include "exponential_fitting.h"
#include "rw/persistence/plug_persistent.h"
#include "rev.h"

namespace rw
{

Rev::Rev()
{
	this->_porosityTestThreshold = 0.1;
	this->_subStep = 16;
	this->_setSize = 32;
	this->_nwStep = 8192;

	this->_nwcorr = 0.99;
	this->_reg = 0.1;
	this->_currentSectionSize = 256;

	this->_event = 0;
	this->_secX = 0;
	this->_secY = 0;
	this->_secZ = 0;
	this->_base = 0;
	this->_gen.seed((uint)this->_base->_randomSeedGenerator());
}

void Rev::Set_Event(rw::Rev::RevEvent* event)
{
	this->_event = event;
}


void Rev::Build_Subsets()
{	
	this->_porosities.clear();
	this->_origins.clear();
	std::set<Pos3i> oset;
	Pos3i pp;
	pp.x = 0;
	pp.y = 0;
	pp.z = 0;
	oset.insert(pp);
	pp.x = this->_secX - this->_currentSectionSize;
	pp.y = 0;
	pp.z = 0;
	oset.insert(pp);
	pp.x = 0;
	pp.y = this->_secY - this->_currentSectionSize;
	pp.z = 0;
	oset.insert(pp);
	pp.x = this->_secX - this->_currentSectionSize;
	pp.y = this->_secY - this->_currentSectionSize;
	pp.z = 0;
	oset.insert(pp);
	pp.x = 0;
	pp.y = 0;
	pp.z = this->_secZ - this->_currentSectionSize;
	oset.insert(pp);
	pp.x = this->_secX - this->_currentSectionSize;
	pp.y = 0;
	pp.z = this->_secZ - this->_currentSectionSize;
	oset.insert(pp);
	pp.x = 0;
	pp.y = this->_secY - this->_currentSectionSize;
	pp.z = this->_secZ - this->_currentSectionSize;
	oset.insert(pp);
	pp.x = this->_secX - this->_currentSectionSize;
	pp.y = this->_secY - this->_currentSectionSize;
	pp.z = this->_secZ - this->_currentSectionSize;
	oset.insert(pp);

	pp.x = std::max(this->_secX/2 - this->_currentSectionSize/2,0);
	pp.y = std::max(this->_secY/2- this->_currentSectionSize/2,0);
	pp.z = std::max(this->_secZ/2 - this->_currentSectionSize/2,0);
	if (pp.x + this->_currentSectionSize >= this->_secX)
	{
		pp.x = this->_secX - this->_currentSectionSize;
	}
	if (pp.y + this->_currentSectionSize >= this->_secY)
	{
		pp.y = this->_secY - this->_currentSectionSize;
	}
	if (pp.z + this->_currentSectionSize >= this->_secZ)
	{
		pp.z = this->_secZ - this->_currentSectionSize;
	}
	oset.insert(pp);

	if ((int)oset.size() < this->_setSize)
	{
		while ((int)oset.size() < this->_setSize)
		{
			Pos3i pp;
			float rndx = (float)(this->_gen() - this->_gen.min()) / ((float)this->_gen.max());
			pp.x = (int)(rndx*((float)this->_secX));
			if (pp.x + this->_currentSectionSize >= this->_secX)
			{
				rndx = (float)(this->_gen() - this->_gen.min()) / ((float)this->_gen.max());
				int dx = (int)(rndx*((float)this->_currentSectionSize));
				pp.x = std::max(this->_secX-this->_currentSectionSize - dx,0);
			}

			float rndy = (float)(this->_gen() - this->_gen.min()) / ((float)this->_gen.max());
			pp.y = (int)(rndy*((float)this->_secY));
			if (pp.y + this->_currentSectionSize >= this->_secY)
			{
				rndy = (float)(this->_gen() - this->_gen.min()) / ((float)this->_gen.max());
				int dy = (int)(rndy*((float)this->_currentSectionSize));
				pp.y = std::max(this->_secY - this->_currentSectionSize - dy,0);
			}

			float rndz = (float)(this->_gen() - this->_gen.min()) / ((float)this->_gen.max());
			pp.z = (int)(rndz*((float)this->_secZ));
			if (pp.z + this->_currentSectionSize >= this->_secZ)
			{
				rndz = (float)(this->_gen() - this->_gen.min()) / ((float)this->_gen.max());
				int dz = (int)(rndz*((float)this->_currentSectionSize));
				pp.z = std::max(this->_secZ - this->_currentSectionSize - dz,0);
			}
			oset.insert(pp);
		}
	}
	set<uint> c_indices;
	if (oset.size() > this->_setSize)
	{
		while ((int)c_indices.size() < this->_setSize)
		{
			uint idx = (uint) ((((float)oset.size())*((float)(this->_gen() - this->_gen.min()))) / ((float)this->_gen.max()));
			c_indices.insert(idx);
		}
	}	

	uint k = 0;
	set<Pos3i>::const_iterator ii = oset.begin();
	while (ii != oset.end())
	{
		if ((uint)c_indices.size() > 0)
		{
			if (c_indices.find(k) != c_indices.end())
			{
				this->_origins.push_back(*ii);
				this->_porosities.push_back(0);
			}
		}
		else
		{
			this->_origins.push_back(*ii);
			this->_porosities.push_back(0);
		}
		++ii;
		++k;
	}
}

scalar Rev::Section_Porosity(const Pos3i& pp) const
{
	int np = 0; 
	int dp = this->_currentSectionSize*this->_currentSectionSize*this->_currentSectionSize;
	for (int z = 0; z < this->_currentSectionSize; ++z)
	{
		for (int y = 0; y < this->_currentSectionSize; ++y)
		{
			for (int x = 0; x < this->_currentSectionSize; ++x)
			{
				Pos3i cp;
				cp.x = pp.x + x;
				cp.y = pp.y + y;
				cp.z = pp.z + z;
				if ((*this->_imgPtr)(cp) == 0)
				{
					++np;
				}
			}
		}
	}
	scalar r = (scalar)np / (scalar)dp;
	return(r);
}

void Rev::Porosity_Distribution(scalar& mean, scalar& std_dev, scalar& min, scalar& max)
{
	tbb::spin_mutex mtx;
	min = (scalar)1;
	max = (scalar)0;
	tbb::parallel_for(tbb::blocked_range<int>(0, (int)this->_origins.size(), 4), [this,&mtx,&min,&max](const tbb::blocked_range<int>& b)
	{
		for (int i = b.begin(); i < b.end(); ++i)
		{
			Pos3i origin = this->_origins[i];
			this->_porosities[i] = this->Section_Porosity(origin);
			if (this->_porosities[i] < min)
			{
				mtx.lock();
				min = this->_porosities[i];
				mtx.unlock();
			}
			if (this->_porosities[i] > max)
			{
				mtx.lock();
				max = this->_porosities[i];
				mtx.unlock();
			}
		}
	});
	mean = 0;
	std_dev = 0;
	for (int i = 0; i < (int)this->_porosities.size(); ++i)
	{
		mean = mean + this->_porosities[i];
		std_dev = std_dev + this->_porosities[i] * this->_porosities[i];
	}
	mean = mean / (scalar)this->_porosities.size();
	std_dev = std_dev - (mean * mean) * (scalar)this->_porosities.size();
	std_dev = std_dev / (scalar)((int)this->_porosities.size() - 1);
	std_dev = sqrt(std_dev);
}

void Rev::Section(int id, rw::Pos3i& origin, scalar& porosity) const
{
	origin = this->_origins[id % this->_currentSectionSize];
	porosity = this->_porosities[id % this->_currentSectionSize];
}

void Rev::Set_Section_Size_Step(int size)
{
	this->_subStep = size;
}


void Rev::Set_Sample_Size(uint size)
{
	this->_setSize = size;
}

uint Rev::Sample_Size() const
{
	return(this->_setSize);
}

void Rev::Set_Porosity_Test_Threshold(scalar threshold)
{
	this->_porosityTestThreshold = threshold;
}

bool Rev::Porosity_Test(scalar& mean, scalar& std_dev, scalar& min, scalar& max)
{
	bool pass = false;
	this->Build_Subsets();
	scalar porosity = (scalar)this->_imgPtr->Black_Voxels()
		/ ((scalar)(this->_imgPtr->Width()*this->_imgPtr->Height()*this->_imgPtr->Depth()));
	mean = 0; 
	std_dev = 0;
	this->Porosity_Distribution(mean, std_dev,min,max);
	if (std_dev/mean < this->_porosityTestThreshold)
	{
		pass = true;		
	}
	return(pass);
}

bool Rev::Porosity_Test_Executed() const
{
	return((uint)this->_porosities.size() > 0);
}



void Rev::Walk_Inside_Sections(int nw, scalar delta, scalar t2min, scalar t2max, uint res,scalar reg, scalar dt)
{
	uint s_size = this->_currentSectionSize;
	bool pass = false;
	int i = 0;
	rw::Plug* formation = 0;

	for (int k = 0; k < (int)this->_origins.size(); ++k)
	{
		formation = new Plug(*this->_base, false);

		math_la::math_lac::full::Vector laplace;
		math_la::math_lac::full::Vector t2;
		rw::ExponentialFitting ef;
		rw::Pos3i pp = this->_origins[k];
		BinaryImage img = this->_imgPtr->Sub(pp.x, pp.y, pp.z, 
			this->_currentSectionSize, this->_currentSectionSize, this->_currentSectionSize);

		formation->Set_TBulk_Time_Seconds(10);
		formation->Set_Time_Step(dt);
		formation->Set_Surface_Relaxivity_Delta(delta);
		formation->Set_Image_Formation(img);
		formation->Set_Number_Of_Walking_Particles(nw);
		formation->Place_Walking_Particles();
		//formation->Activate_GPU_Walk(true);
		formation->Random_Walk_Procedure();
		ef.Load_Formation(*formation);
		ef = ef.Logarithmic_Reduction(1024);
		ef.Kernel_T2_Mount(t2min, t2max, res, reg);
		ef.Solve(t2, laplace);
		this->_laplaces.push_back(laplace);
		this->_timeT2.push_back(t2);
		scalar p = (scalar)k / (scalar)this->_origins.size();
		if (this->_event)
		{
			this->_event->On_Walk_Event(k,p);
		}
		delete formation;
	}
}

void Rev::Set_Section_Size(int size)
{
	this->_currentSectionSize = size;
}

uint Rev::Section_Size() const
{
	return(this->_currentSectionSize);
}

void Rev::Set_Image(const rw::BinaryImage& img)
{
	this->_imgPtr = &img;
	this->_base = new Plug();
	this->_secX = this->_imgPtr->Width();
	this->_secY = this->_imgPtr->Height();
	this->_secZ = this->_imgPtr->Depth();
}

uint Rev::Section_Number_Of_Walkers() const
{
	uint nw = 0;
	for (uint k = 0; k < (uint)this->_porosities.size(); ++k)
	{
		uint v = (uint)(this->_porosities[k] 
			* ((scalar)(this->_currentSectionSize*this->_currentSectionSize*this->_currentSectionSize)));
		if (v > nw)
		{
			nw = v;
		}
	}
	return(nw);
}

void Rev::Get_T2_Distribution(uint idx, math_la::math_lac::full::Vector& laplace, math_la::math_lac::full::Vector& T2v)
{
	idx = idx % (uint)this->_laplaces.size();
	laplace = this->_laplaces[idx];
	T2v = this->_timeT2[idx];
}

bool Rev::Random_Walk_Executed() const
{
	return((int)this->_laplaces.size() > 0);
}

scalar Rev::Unresolved_Porosity(scalar mean, scalar std_dev, scalar lab_pore, scalar confidence)
{
	scalar alpha = 0;
	scalar step = 0.01;
	bool found = false;
	scalar sq = sqrt((scalar)2);
	while ((!found) && (alpha <= (scalar)1))
	{
		scalar prob = 0;
		prob = prob + (scalar)0.5*((scalar)1 + std::erf((lab_pore - alpha - mean) / (std_dev*sq)));
		prob = prob + (scalar)1 - (scalar)0.5*((scalar)1 + std::erf((lab_pore + alpha - mean) / (std_dev*sq)));
		if (prob < (scalar)1 - confidence)
		{
			found = true;
		}
		else
		{
			alpha = alpha + step;
		}
	}
	return(alpha);
}

}