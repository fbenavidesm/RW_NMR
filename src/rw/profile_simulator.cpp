#include "profile_simulator.h"

namespace rw
{

ProfileSimulator::ProfileSimulator() : Simulator()
{
	this->_shared = false;
	this->_profile = 0;
	this->_particlesEnergy = 0;
	this->_killedWalkers = 0;
	this->_shared = false;
	this->_decisionArray = 0;
	this->_collisionProbabilities = 0;
	this->_avgMin = 10;
	this->_avgMax = 10;
	this->_degradeCoefficients = 0;

	Plug::_randomSeedGenerator.discard(Plug::_randomSeedGenerator() % 8);
	vslNewStream(&this->_randomNumberStream, VSL_BRNG_SFMT19937, (uint)Plug::_randomSeedGenerator());
}

const vector<scalar>& ProfileSimulator::Sequential_Profile(int id_seq) const
{
	if (id_seq >= (int)this->_profileSequence->size())
	{
		id_seq = (int)this->_profileSequence->size() - 1;
	}
	vector<scalar>* ptr = (*this->_profileSequence)[id_seq];
	return(*ptr);
}

void ProfileSimulator::Share_Profile(map<scalar, scalar>* profile)
{
	this->_shared = true;
	if (this->_profile)
	{
		delete this->_profile;
	}
	this->_profile = profile;
}

void ProfileSimulator::Set_Profile(map<scalar, scalar>* profile)
{
	if (!this->_shared)
	{
		if (this->_profile)
		{
			delete this->_profile;
		}
	}
	this->_shared = false;
	this->_profile = profile;
}

void ProfileSimulator::Set_Particle_Array()
{
	int particles = this->Total_Particles();
	if (this->_particlesEnergy)
	{
		delete this->_particlesEnergy;
		delete this->_collisions;
	}
	if (this->_decisionArray)
	{
		freeScalar(this->_decisionArray);
	}
	if (this->_collisionProbabilities)
	{
		freeScalar(this->_collisionProbabilities);
	}
	if (this->_degradeCoefficients)
	{
		freeScalar(this->_degradeCoefficients);
	}
	this->_particlesEnergy = new vector<scalar>(particles, (scalar)1);
	this->_collisions = new vector<int>(particles, 0);
	if (!this->Profile())
	{
		this->Set_Profile(new map<scalar, scalar>());
	}
	if (this->_killedWalkers)
	{
		delete this->_killedWalkers;
	}
	this->_killedWalkers = new set<int>();
	this->_decisionArray = allocScalar(3 * particles);
	this->_collisionProbabilities = allocScalar(particles);
	this->_degradeCoefficients = allocScalar(particles);
}

void ProfileSimulator::Prepare()
{
	if ((this->_particlesEnergy)&&(this->Profile_Update_Interval() == 0))
	{
		for (int k = 0; k < this->_particlesEnergy->size(); ++k)
		{
			(*this->_particlesEnergy)[k] = (scalar)1;
			scalar p = (scalar)k / (scalar)this->Total_Particles();
			this->_collisionProbabilities[k] = this->Hit_Probability(p);
			this->_degradeCoefficients[k] = this->Delta(this->_collisionProbabilities[k]);
		}
	}
	else
	{
		for (int k = 0; k < this->_particlesEnergy->size(); ++k)
		{
			(*this->_particlesEnergy)[k] = (scalar)1;
		}
		this->Update(0);
	}
}

void ProfileSimulator::Update(int itr)
{
	int id = itr / this->Profile_Update_Interval();
	const vector<scalar>& mp = this->Sequential_Profile(id);
	int rr = std::max((int)mp.size() / (int)(2*this->_particlesEnergy->size()),1);
	for (int k = 0; k < this->_particlesEnergy->size(); ++k)
	{
		int idk = (k*((int)mp.size())) / (int)this->_particlesEnergy->size();
		int flag = std::max(0, idk - rr);
		int size = std::min((int)mp.size(), idk + rr);
		int n = 0;
		scalar avg = 0;
		for (int j = flag; j < size; ++j)
		{
			avg = avg + mp[j];
			++n;
		}
		this->_collisionProbabilities[k] = avg / (scalar)n;
		this->_degradeCoefficients[k] = this->Delta(this->_collisionProbabilities[k]);
	}
}

void ProfileSimulator::Set_Collision_Profile(math_la::math_lac::full::Vector& domain, math_la::math_lac::full::Vector& colissions)
{
	map<scalar,scalar>* hst = new map<scalar, scalar>();
	int size = domain.Size();
	if (size == colissions.Size())
	{
		for (int k = 0; k < size; ++k)
		{
			(*hst)[domain(k)] = colissions(k);
		}
	}
	this->Set_Profile(hst);
}

scalar ProfileSimulator::Hit_Probability(scalar idx) const
{
	const map<scalar, scalar>* hst = this->Profile();
	map<scalar, scalar>::const_iterator ii = hst->lower_bound(idx);
	map<scalar, scalar>::const_iterator li = ii;	
	int pp = 0;
	scalar prob = 0;
	while ((pp < this->_avgMin) && (li != hst->begin()))
	{
		--li;
		prob = prob + li->second;
		++pp;
	}
	int avgmax = this->_avgMax + pp+1;
	li = ii;	
	while ((pp < avgmax) && (li != hst->end()))
	{
		prob = prob + li->second;
		++pp;
		++li;
	}
	prob = prob / (scalar)pp;
	return(prob);
}

scalar ProfileSimulator::Magnetization()
{
	scalar nu = 0;
	vdRngUniform(VSL_RNG_METHOD_UNIFORM_STD_ACCURATE, this->_randomNumberStream,
		3 * ((uint)this->_particlesEnergy->size()), this->_decisionArray, (scalar)0, (scalar)1);
	for (int j = 0; j < this->_particlesEnergy->size(); ++j)
	{
		scalar prob = this->_collisionProbabilities[j];
		if (this->_decisionArray[3 * j] <= prob)
		{
			scalar d = this->_degradeCoefficients[j];
			(*this->_particlesEnergy)[j] = ((*this->_particlesEnergy)[j])*d;
			(*this->_collisions)[j] = (*this->_collisions)[j] + 1;
			if (this->_decisionArray[3 * j + 1] >= d)
			{
				this->_killedWalkers->insert(j);
			}
		}
		else
		{
			int dir = (int)((scalar)3*this->_decisionArray[3 * j + 2]);
			int dx = dir/2;
			int dy = dir % 2;;
			int dz = 1 - dx - dy;
			scalar d = this->Gradient().Exponential_Factor(dx, dy, dz);
			(*this->_particlesEnergy)[j] = ((*this->_particlesEnergy)[j])*d;
		}
		nu = nu + (*this->_particlesEnergy)[j];
	}
	nu = nu / ((scalar)this->_particlesEnergy->size());
	return(nu);
}

void ProfileSimulator::Set_Delta(scalar delta)
{
	this->_rho = delta;
}

ProfileSimulator::~ProfileSimulator()
{
	if (this->_particlesEnergy)
	{
		delete this->_particlesEnergy;
	}

	if (this->_collisions)
	{
		delete this->_collisions;
	}
	if (this->_decisionArray)
	{
		freeScalar(this->_decisionArray);
	}
	if (this->_collisionProbabilities)
	{
		freeScalar(this->_collisionProbabilities);
	}
	vslDeleteStream(&this->_randomNumberStream);
	if (!this->_shared)
	{
		delete this->_profile;
	}
	if (this->_degradeCoefficients)
	{
		freeScalar(this->_degradeCoefficients);
	}
}

}