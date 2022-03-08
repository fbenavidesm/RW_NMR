#ifndef PROFILE_SIMULATOR
#define PROFILE_SIMULATOR

#include <map>
#include <vector>
#include "math_la/mdefs.h"
#include "plug.h"
#include "field3d.h"
#include "simulator.h"

namespace rw
{

	/**
	* A ProfileSimulator is a random walk simulator based on the colision profile. There's an open possibiliy to implement
	* other kinds of simulators (for example, using other kinds of distribution, such as the maximal ball, or a decoupled profile). 
	* In order to create a new kind of simulator, it must be inherited from the class Simulator.
	*/

	class ProfileSimulator : public Simulator
	{
	private:
		friend class Plug;
		friend class RelaxivityOptimizer;
		friend class RelaxivityExperiment;
		/**
		* TRUE if the profile is being shared between several simulators. This is important for the pointer _profile
		* which can be shared amongst several simulations that use the same profile with different relaxivity distributions
		*/
		bool _shared;

		/**
		* The colision profile according to which the simulation will be executed
		*/
		map<scalar, scalar>* _profile;

		/**
		* The collision profile of the walkers evolves with time. This can be saved in layers and these layers can be used to reproduce
		* the simulation. This sequence is used to change the collision profile with time. 
		*/
		vector<vector<scalar>*>* _profileSequence;

		/**
		* The magnetization of all particles that are being simulations
		*/
		vector<scalar>* _particlesEnergy;

		/**
		* The vector of collision probabilites of the particles
		*/
		scalar* _collisionProbabilities;

		/**
		* This vector is used to count the number of collisions of each particle
		*/
		vector<int>* _collisions;

		/**
		* The set of killed walkers
		*/
		set<int>* _killedWalkers;

		/**
		* The random number generator according to which the simulation is executed
		*/
		VSLStreamStatePtr _randomNumberStream;

		/**
		* The value of uniform relaxivity
		*/
		scalar _rho;


		/**
		* The Hit probability for a certain idx is averaged a discrete number of steps before
		* and after the idx value. This is the number of steps before the idx probability.
		*/
		int _avgMin; 

		/**
		* The Hit probability for a certain idx is averaged a discrete number of steps before
		* and after the idx value. This is the number of steps before the idx probability.
		*/

		int _avgMax;

		/**
		* An array of the precalculated probabilites of hitting for each particle
		*/
		scalar* _decisionArray;

		/**
		* The degrading coefficients of each particle, according to a varying surface
		* relaxivity.
		*/
		scalar* _degradeCoefficients;
	protected:
		/**
		* Creates particle arrays. This is called after the number of particles has been
		* established. 
		*/
		void Set_Particle_Array();

		/**
		* @return The profile assigned to the simulator
		*/
		map<scalar, scalar>* Profile();

		/**
		* @return The const profile assigned to the simulator
		*/

		const map<scalar, scalar>* Profile() const;

		/**
		* Sets the simulator profile, but it is being shared with other profiles
		* so it is not released after destruction.
		* @param profile 
		*/
		void Share_Profile(map<scalar, scalar>* profile);

		/**
		* Sets the simulator profile without sharing its memory
		*/
		void Set_Profile(map<scalar, scalar>* profile);
	public:
		ProfileSimulator();
		~ProfileSimulator();
		/**
		* This is the method that actually executes the random walk simulation without using a 3D or 2D image.
		* That simulation is only based on the collision profile. 
		* @return The total magnetization of the simulator at the current iteration. When it's called, the reduction 
		* operation is performed. 
		*/
		scalar Magnetization();

		/**
		* @return The hit probability indexed by idx in the interval [0,1]. 
		*/
		scalar Hit_Probability(scalar idx) const;

		/**
		* This method is called just before the simulation start. It can be used to init the particles energy
		* just before random walk begins
		*/
		void Prepare();

		/**
		* Updates _collisionProbabilites and _degradeCoefficients according to the iteration given by the parameter
		* itr.
		* @param itr Iteration according to which the coefficients are going to be updated. 
		*/
		void Update(int itr);

		/**
		* Sets domain and range of the collision profile
		*/
		void Set_Collision_Profile(math_la::math_lac::full::Vector& domain, math_la::math_lac::full::Vector& colissions);

		/**
		* Sets the relaxation factor (normalized in the interval [0,1])
		*/
		void Set_Delta(scalar delta);
		/**
		* @return The current magnetization associated to particle id
		* @param id Index of the particle
		*/
		scalar Magnetization(int id) const;

		/**
		* Number of killed particles
		*/
		int Killed_Particles() const;

		/**
		* @return Total number of hits if the particle id
		* @param id Index of the particle
		*/
		int Hits(int id) const;

		/**
		* @return The sequential profile indexed by id_seq
		* @param id_seq Identifier of the profile
		*/
		const vector<scalar>& Sequential_Profile(int id_seq) const;
	};

	inline scalar ProfileSimulator::Magnetization(int id) const
	{
		return((*this->_particlesEnergy)[id]);
	}

	inline int ProfileSimulator::Killed_Particles() const
	{
		return((int)this->_killedWalkers->size());
	}

	inline int ProfileSimulator::Hits(int id) const
	{
		return((*this->_collisions)[id]);
	}

	inline 	map<scalar, scalar>* ProfileSimulator::Profile()
	{
		return(this->_profile);
	}

	const inline map<scalar, scalar>* ProfileSimulator::Profile() const
	{
		return(this->_profile);
	}


}

#endif