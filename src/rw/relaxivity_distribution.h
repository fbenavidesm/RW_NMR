#ifndef RELAXIVITY_DISTRIBUTION_H
#define RELAXIVITY_DISTRIBUTION_H

#include <set>
#include <vector>
#include "math_la/mdefs.h"
#include "plug.h"

namespace rw
{

	class PlugPersistent;
	using std::map;

	/**
	* A RelaxivityDistribution must be capable to assign a "RHO" value to each walker. "RHO" is the relaxivity coefficient associated
	* to a walker, according to the pore size it resides. 
	**/

	class RelaxivityDistribution
	{
	public:
		/**
		* The basic shape functions for the relaxivity distribution. 
		*/
		enum Shape
		{
			/**
			* The sigmid shape
			*/
			Sigmoid,
			/**
			* The box shape
			*/
			Hat
		};
	private:
		friend class Plug;
		/**
		* The relaxivity distribution contains a mapping simulation, which is capable to translate the relaxivity value, "RHO"
		* to a normalized value which can be applied to the walker when it hits a solid wall. A base simulation contains all the
		* needed parameters. 
		*/
		const PlugPersistent* _mappingSimulation;
		/**
		* The walkers are updated in parallel. The chunk size defines the size of the groups in which they are processed.
		*/
		uint _chunkSize;
		/**
		* The RTF of a walker can be updated globally or locally. This variable stores the number of iteration of the last update
		* to use it in the next update. 
		*/
		int _last_Iteration;
		/**
		* This decides if the RTF is updated locally or globally, along the entire simulation. When the RTF is updated globally, the
		* RTF is calculated by dividing the number of hits of the walker between the number of iterations. When it is updated locally
		* the RTF is calculated by dividing the walker number of hits between the number of iterations that passed since the
		* last update. When updated, the walker number of hits is set to 0. 
 		*/
		bool _enableCycle;

	public:
		/**
		* A relaxivity distribution is a function that uses a RTF=XI value and returns the respective RHO value. 
		* This is what this function does. 
		* @param xi RTF value
		* @return The corresponding RHO value
		*/
		virtual scalar Evaluate(scalar xi) const = 0;	
		/**
		* A relaxivity function defines a function or a linear combination of functions with a set of parameters. 
		* This function defines the number of parameters. 
		* @param n Number of parameters that define the function
		*/
		virtual void Set_Size(int n) = 0;

		/**
		* This functions sets the value of one of the parameters of the function
		* @param id The index of the parameter to set
		* @param val The value of the parameter
		*/
		virtual void Set_Value(int id, scalar Decay_Step_Value) = 0;

		RelaxivityDistribution();
		virtual ~RelaxivityDistribution();

		/**
		* Normalizes the RHO relaxivity using the parameters inside the private member _mappingSim. 
		*/
		scalar Relaxivity_Factor(scalar rho) const;
		/**
		* Updates the relaxivity of the walkers, using the specified number of iterations.
		* @param wser Set of walkers
		* @param total_iterations Number of iterations to which the walker RTF is normalized
		*/
		void Update_Walker_Rho(vec(Walker)& wset,int total_iterations);
		/**
		* Sets the mapping simulation for the relaxivity distribution
		* @param sim Mapping simulation
		*/
		void Set_Simulation(const rw::PlugPersistent& sim);
		/**
		* Sets the chunk size for parallel processing
		* @param chunk Chunk size 
		*/
		void Set_Chunk_PP(uint chunk);
		/**
		* Inits walkers, setting their relaxivity according to each walker's RTF (number of collisions) normalized by the
		* parameter itr. 
		* @param wset The set of walkers to be initialized
		* @param itr The number of iterations to which the walker's collisiions are normalized
		*/
		void Init_Walkers(vec(Walker)& wset, int itr);
		/**
		* Inits walkers, setting their relaxivity according to each walker's RTF (number of collisions) normalized by the
		* mapping simulation number of iterations.
		* @param wset The set of walkers to be initialized
		*/
		void Init_Walkers(vec(Walker)& wset);

		scalar operator()(scalar xi) const;
		/**
		* Enables or disable a sequential RTF update of the walkers, when the collision profile changes with time
		* @param enable TRUE if the collision profile is updated with time
		*/
		void Enable_Sequential_Profile(bool enable);

		/**
		* Recovers a pore size distribution, based on a Laplace transform using this surface relaxivity
		* @param rmin Minimal radius of the PSD
		* @param rmax Maximal radius of the PSD
		* @param step Step size between categories
		* @param time_domain Time domain of the Laplace transform
		* @param bins Laplace transform bins vector
		* @param v Voxel size
		* @param distribution Returned distribution
		* @param t2b Bulk relaxation time
		*/
		void Laplace_PSD(scalar rmin, scalar rmax, scalar step,
			const math_la::math_lac::full::Vector& time_domain, const math_la::math_lac::full::Vector& bins, scalar v,
			vector<scalar2>& distribution, scalar t2b);

		/**
		* @return Mapping simulation used to map the varying surface relaxivity, assigning a different
		* relaxivity to each walker.
		*/

		const PlugPersistent& Mapping_Simulation() const;
	};

	inline scalar RelaxivityDistribution::operator()(scalar xi) const
	{
		return(this->Evaluate(xi));
	}

	inline const PlugPersistent& RelaxivityDistribution::Mapping_Simulation() const
	{
		return(*this->_mappingSimulation);
	}

}

#endif