#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "math_la/mdefs.h"
#include "rw/field3d.h"

namespace rw
{

class Plug;
class RelaxivityDistribution;
class RelaxivityExperiment;

/**
* A Simulator is a class that mimics a random walk simulation without using a texture (image). It uses a basis function or a profile
* that groups the main characteristics of a real random walk. A Simulator executes a random walk based on this profile,
* distributing uniformly the walker's probability to hit. The walker's probability to hit is called XI (Colision rate)
* and it can be stable or changing along time, when the walker moves between different size pores.
*/
class Simulator
{
private:
	friend class rw::Plug;
	friend class rw::RelaxivityExperiment;
	/**
	* The Simulator mimics a Random Walk simulating a RW iteration. It can use a single Profile or a set of profiles
	* which can be updated after a certain number of iterations, using the rw::Formation::Simulator::Update method.
	* The rw::Formation::Simulator::Update method will be called every rw::Formation::Simulator::_profileUpdateInterval iterations.
	*/
	int _profileUpdateInterval;

	/**
	* A Random Walk simulation based on a Simulator does not require all walkers. This parameters sets the number of particles
	* to simulate a real Random Walk.
	*/
	int _totalParticles;

	/**
	* To simulate a Random Walk at least one relaxivity distribution is required, to associate the particle relaxivity.
	*/
	rw::RelaxivityDistribution* _distribution;

	/**
	* Field gradient inside the sample
	*/
	Field3D _gradient;
protected:

	/**
	* This method is called just after the method rw::Formation::Simulator::Set_NumberOfParticles is called.
	*/
	virtual void Set_Particle_Array() {};

	/**
	* This method is called just after setting the relaxivity distribution to the simulator
	*/
	virtual void Configure_Relaxivity_Distribution(rw::RelaxivityDistribution& distribution) {};
public:
	Simulator();
	virtual ~Simulator();

	/**
	* This method is called just before starting a simulation. For example, to initialize the particles energy to 1.
	*/
	virtual void Prepare() {};

	/**
	* This method is called every rw::Formation::Simulator::_profileUpdateInterval iterations during the simulation.
	*/
	virtual void Update(int itr) {};

	/**
	* @return The magnetization decay at current iteration. This function is called iteratively and must update the system
	* global energy.
	*/
	virtual scalar Magnetization() = 0;

	/**
	* The particle id is indexed by a scalar and not an integer, because the number of particles with which a 
	* distribution is sampled can vary. 
	* @param idx Normalized id of the particles (in the interval [0,1]. 
	* @return The probability to hit of the idx particle, normalized in the interval [0,1]. 
	*/
	virtual scalar Hit_Probability(scalar idx) const = 0;

	/**
	* This method is executed after each iteration, when the number of particles is already defined.
	* This is the reason the particle is identified by an integer.
	* @param id Id of the particles
	* @return The total amount of magnetization energy stored by the particle indexed by id
	*/
	virtual scalar Magnetization(int id) const;

	/**
	* @return The total number of killed particles
	*/
	virtual int Killed_Particles() const;

	/**
	* @param id Id of the particles
	* @return The total number of Hits of the particle id
	*/
	virtual int Hits(int id) const;

	/**
	* @param xi Collision rate
	* @return The normalized relaxivity associated to collision rate xi
	*/
	scalar Delta(scalar xi) const;

	/**
	* @param xi Collision rate
	* @return The non-normalized relaxivity associated to RTF xi
	*/
	scalar Rho(scalar xi) const;

	/**
	* @return The total number of simulation particles
	*/
	int Total_Particles() const;

	/**
	* @return The number of iterations after which the method rw::Formation::Simulator::Update is called
	*/
	int Profile_Update_Interval() const;

	/**
	* Set the size of the number of particles to be used during the simulation. This number should be smaal, as it works
	* as a compression mechanism for the real simulation.
	* @param particles Number of particles
	*/
	void Set_NumberOfParticles(int particles);

	/**
	* Sets the relaxivity distribution to be used during the simulation
	*/
	void Set_Relaxivity_Distribution(rw::RelaxivityDistribution* distribution);

	/**
	* @return The relaxivity distribution associated to the simulator
	*/
	const rw::RelaxivityDistribution& RelaxivityDistribution() const;

	/**
	* Desassociates the relaxivity distribution from the simulation
	*/
	void ReleaseDistribution();

	/**
	* Defines internal gradient of the simulation
	*/
	void Set_Gradient(const Field3D& field);

	/**
	* @return Internal gradient to simulate
	*/
	const Field3D& Gradient();
};

inline int Simulator::Profile_Update_Interval() const
{
	return(this->_profileUpdateInterval);
}

inline int Simulator::Total_Particles() const
{
	return(this->_totalParticles);
}


}
#endif
