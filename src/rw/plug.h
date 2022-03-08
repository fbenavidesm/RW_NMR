#ifndef WALKER_H
#define WALKER_H

#include <vector>
#include <map>
#include <set>
#include <string>
#include <mutex>
#include <thread>
#include <random>
#include <algorithm>
#include <amp.h>
#include <amp_math.h>
#include "binary_image/pos3i.h"
#include "binary_image/binary_image.h"
#include "math_la/mdefs.h"
#include "math_la/math_lac/full/vector.h"
#include "tbb/blocked_range.h"
#include "tbb/spin_mutex.h"
#include "tbb/concurrent_hash_map.h"
#include "mkl_vsl.h"
#include "field3d.h"
#include "walker.h"
#include "simulator.h"
#include "random_walk_observer.h"
#include "random_walk_step_value.h"
#include "sim_params.h"


namespace rw
{

	#define MAX_RANDOM 0xAFFFFFFF
	#define MASK_DEGRADE_STRATEGY 0x0000FFFF
	#define MASK_SURFACE_HITS 0xFFFFF400
	#define MASK_SURFACE_RTF  0x000003FF
	#define SURFACE_MOVE_BITS 11
	#define STRAT_MOVE_BITS 16

	using std::string;
	using std::vector;
	using std::map;
	using std::min;
	using std::set;
	using tbb::blocked_range;
	using tbb::split;

	class RelaxivityExperiment; 
	class RelaxivityOptimizer;
	class PlugPersistent;
	class RelaxivityDistribution;
	class Rev;
	class Mod;
	class RandomWalkImplementorCreator;
	class RandomWalkImplementor;
	class RandomWalkPlacer;

	/**
	* A Plug is a combination of fluid and a rock sample. It contains all necessary elements to
	* simulate a T2 or T1 Nuclear Magnetic Ressonance experiment. 
	* Specifically, a Plug contains a BinaryImage to define the pore shape structure and a set of walkers
	* to simulate the fluid saturation. A Plug executes its simulation through a RandomWalkImplementor, an
	* abstract class that can be overloaded in order to defione NMR simulation rules. 
	* Other simulation parameters are stored in the class SimulationParams in which, for example, the seed of all
	* random number generation is stored. This seed is necessary to reconstruct the simulation as well as
	* the walkers initial position, a collision profile sequence (the evolution of the collision
	* profile in different time intervals). 
	* This class only handles simulations aspects, but storing all these parameters in a file is handled by the
	* class PlugPersistent. 
	*/

	class Plug
	{
	private:
		friend class Rev;
		friend class ProfileSimulator;
		friend class SimStore;
		friend class RandomWalkImplementor;
		friend class RandomWalkImplementorCreator;
		friend class RelaxivityExperiment;
		friend class RelaxivityOptimizer;
		friend class PlugPersistent;
		friend class RandomWalkPlacer;

		/**
		* Simulation parameters that can be defined in an array of unsigned integers
		*/
		SimulationParams _simParams;

		/**
		* Vector of walkers moving inside the formation
		*/
		vec(Walker) _walkers;

		/**
		* Starting position of the walkers
		*/
		vector<Pos3i> _walkersStartPosition;

		/**
		* Binary texture defining the pore space (and solid parts) of the sample
		*/
		BinaryImage* _image;

		/**
		* The mask is a binary image that blocks the walkers displacemente without degrading its
		* energy (not used yet)
		*/
		BinaryImage* _mask;

		/**
		* Vector of magnetization decay values
		*/
		vector<rw::Step_Value> _decayValues;

		/**
		* Sequence of collision profiles during the RW simulation
		* It is not necessarily stored for each simulation. 
		* It is updated only if _updateProfileInterval > 0. This affects
		* GPU Random Walk performance. 
		*/
		vector<vector<scalar>*> _profileSequence;

		/**
		* Bulk relaxation time (T2b)
		*/
		scalar _bulkRelaxationTime;

		/**
		* Relaxation delta of the constan surface relaxivity. This value is normalized
		* in the interval [0,1]
		*/
		scalar _surfaceRelaxationRate;

		/**
		* Time step of the simulation (the time the particles take to walk a voxel distance). 
		*/
		scalar _timeStep;

		/**
		* Magnetization threshold to stop simulation
		*/
		scalar _stopEnergyThreshold;

		/**
		* TRUE if the walkers position is shown during the simulation, so their position is stored
		* and recovered in memory. For example, for GPU case, the memory position is copied from GPU
		* to CPU.
		*/
		bool _showWalkers;

		/**
		* TRUE if the walkers starting position has been set
		*/
		bool _walkersPlaced;

		/**
		* Update event that is triggered during simulation
		*/
		RandomWalkObserver* _updateEvent;

		/**
		* Length of the voxel size
		*/
		scalar _pixelSize;

		/**
		* This is a base random number generator to generated the seed for the entire simulation
		* It is a more robust and slower generator 
		*/
		static std::ranlux48 _randomSeedGenerator;

		/**
		* The linear random generator which is seeded by the _randomSeedGenerator
		* It can be used, for example, for a walker큦 decisions during their motion or for
		* a normalized random number used to uniformly distribute initial walker's position. 
		* It is a faster generator. 
		*/
		std::mt19937 _randomGenerator;

		/**
		* TRUE if the initial seed has been generated
		*/
		static bool _seedGenerated;

		/**
		* A mutex function is to synchronize several processes of the simulation 
		* For example in a Genetic algorithm optimization it could be necessary
		* to synchronize between threads, and this mutex blocks activity of all
		* threads. 
		*/
		tbb::spin_mutex _simulationMutex;

		/**
		* Field gradient to apply to the simulation.
		* It is always applied, even when it is zero (to avoid if inside the parallel loop). 
		*/
		Field3D _gradient;

		/**
		* Random walk implementation routines are encapsulated in this instance
		*/
		RandomWalkImplementor* _implementor;

		/**
		* @return Total number of iterations executed during simulation
		*/
		uint _totalIterations;

	protected:

		/**
		* Adds a decay step to the list of decay values
		*/
		void Add_Decay_Step_Value(rw::Step_Value& value);

		/**
		* @return TRUE if a Walk_Event is defined
		*/
		bool Has_Walk_Event() const;

		/**
		* @return A pointer to the walk event
		*/
		Plug::RandomWalkObserver* On_Walk_Event();

		/**
		* @return A reference to the walker indexed by 'i'
		*/
		Walker& Walking_Particle(int i);

		/**
		* Kills a walker, when this mode is enabled in _walkerMoveAndDegradeStrategy
		*/
		bool Kill_Walking_Particle(Walker& w, scalar decision);

		/**
		* Picks a random number based on the internal random number generator.
		* The number is normalized between 0 and 1. A Mersenne-Twister generator
		* is used
		*/
		scalar Pick_Random_Normalized_Number();

		/**
		* Deletes all collision profile information that has been stored internally
		*/
		void Clear_Collision_Profile();

		/**
		* This method calls the implementor, in a background thread, to execute random walk
		* simulation
		*/
		void Random_Walk_Procedure();

	public:
		Plug();

		/**
		* The copy constructor can copy walkers from w if copywalkers is true.
		* This may be necessary to generate parallel simulations in which all walkers
		* follow the same path, but with different surface relaxivities. 
		*/
		Plug(const Plug& e, bool copywalkers = true);

		/**
		* The random walk can be "simulated" using an instance of a rw::Simulator. 
		* Currently, this simulation is faster and is based on a collision profile. Therefore, the information
		* stored in Plug related to BinaryImage and Walker's position is not used. However, new kinds of simulators 
		* can be implemented using more information about the plug. 
		* This method picks the simulator and creates an instance of RandomWalkSimulatorImplementor, a random walk implementor that
		* iterates using the overloaded methods Simulator::Magnetization() and Simulator::Update(int itr). These two methods define
		* the simulator behavior. 
		* @param simulator Simulator to be used for the new simulation
		*/
		void Simulate_Random_Walk_Procedure(rw::Simulator& simulator);

		/**
		* Number of blocks to process random walk simulation
		*/
		uint Minimal_Walkers_Per_Thread() const;


		/**
		* Locks formation local mutex. It is used, for example, when a random walk simulation
		* is being executed
		*/
		void Lock();

		/**
		* Unlocks formation local mutex
		*/
		void UnLock();

		virtual ~Plug();

		/**
		* Defines the image formation in which the walkers are to be displaced
		*/
		void Set_Image_Formation(const BinaryImage& image);

		/**
		* @return Dimension of the formation. It can be 2 or 3. 
		*/
		uint Dimension() const;

		/**
		* Recharges walker magnetization to 1
		*/
		void Recharge_Walkers_Magnetization();

		/**
		* @return Binary image associated to the plug pore space description (a 3D or 2D texture)
		*/
		const BinaryImage& Plug_Texture() const;

		/**
		* Place walking particles inside the formation texture
		*/
		void Place_Walking_Particles();

		/**
		* @return The number of particles placed inside the texture formation
		*/
		uint Number_Of_Walking_Particles() const;

		/**
		* This event is triggered after several random walk iterations. The observer allows to
		* display grpahically walker's movement. 
		*/
		void Set_On_Walk_Event(RandomWalkObserver* evt);

		/**
		* Enables displaying walker큦 progress
		*/
		void Show_Walkers(bool enable);

		/**
		* @return TRUE if the walker큦 progress should be displayed
		*/
		bool Showing_Walkers() const;

		/**
		* Sets the number of walkers
		* @param N Number of walkers
		*/
		void Set_Number_Of_Walking_Particles(uint N);

		/**
		* Defines the bulk time in seconds
		* @aram rate Bulk time in seconds
		*/
		void Set_TBulk_Time_Seconds(scalar rate);

		/**
		* Sets the surface relaxivity factor, as a number between 0 and 1
		* @param rate Surfacer relaxivity factor
		*/
		void Set_Surface_Relaxivity_Delta(scalar rate);

		/**
		* Sets stopping criterion for the random walk simulation
		* @param perc Percentage of energy to stop the simulation
		*/
		void Set_Stop_Threshold(scalar perc);

		/**
		* Defines the time step between each iteration of the walking particles
		* @param tsc Time step in seconds
		*/
		void Set_Time_Step(scalar tsc);

		/**
		* Deallocates walkers so when the simulation is performed, the 
		* walkers acquire a new position
		*/
		void Deallocate_Walking_Particles();

		/**
		* @return Time step between each iteration of the simulation
		*/
		scalar Time_Step() const;

		/**
		* @return Energy threshold to stop the simulation
		*/
		scalar Stop_Threshold() const;

		/**
		* @return Bulk time in seconds
		*/
		scalar TBulk_Seconds() const;

		/**
		* @return Surface relaxivity factor, as a number between 0 and 1
		*/
		scalar Surface_Relaxivity_Delta() const;

		/**
		* @return TRUE if the walkers have been placed in a valid starting point
		*/
		bool Walking_Particles_Placed() const;

		/**
		* Number of samples in the decay
		*/
		uint Decay_Size() const;

		/**
		* Decay step indexed by "i"
		*/
		Plug::Step_Value Decay_Step_Value(uint i) const;

		/**
		* Starts random walk simulation. 
		*/
		void Start_Walk();

		/**
		* Activates gpu simulation
		* @param gpu TRUE if GPU mode is activated
		*/
		void Activate_GPU_Walk(bool gpu);


		/**
		* @return Walker indexed by i.
		*/
		const Walker& Walking_Particle(int i) const;

		/**
		* Clears decay vector
		*/
		void Clear_Decay_Steps();

		/**
		* @return Formation image porosity
		*/
		scalar Porosity() const;

		/**
		* Only places the walkers between min and max collision rate, at their starting 
		* positions.
		*/
		void Place_Walkers_In_Restricted_Collision_Rate(scalar min, scalar max, scalar factor);

		/**
		* @return TRUE if the formation has an associated image
		*/
		bool Has_Associated_Image() const;

		/**
		* @rteturn TRUE if the formation does not contain particles
		*/
		bool Empty_Particles() const;

		/**
		* Limits the maximal number of iterations that are to be executed in the simulation. 
		*/
		void Limit_Maximal_Number_Of_Iterations(uint maxit);

		/**
		* @return The starting position of a walker indexed by 
		*/
		Pos3i Walker_Starting_Position(int id) const;

		/**
		* Reallocates all walkers to their corresponding start position
		*/
		void Reassign_Walkers_To_Starting_Position();

		/**
		* Define the relaxivity distribution function that is assigned to the walkers
		* @param f The relaxivity distribution that is applied to the walkers
		*/
		void Set_Relaxivity_Distribution(RelaxivityDistribution* f, uint itr = 0);

		/**
		* @return TRUE if the walkers repeat their paths
		*/
		bool Repeating_Walkers_Paths() const;

		/**
		* Defines if the walkers repeat their paths. 
		* @param v Defines if the walkers repeat their paths or not
		* @param seed When the walkers repeat their paths, the same seed is assigned to
		* the random number generator. This seed is defined in this parameter. 
		*/
		void Repeat_Walkers_Paths(bool v, uint seed = 0);

		/**
		* @return Random number generator seed
		*/
		uint Seed_For_Random_Number_Generation() const;	

		/**
		* A profile collisin distribution function is obtained during the simulation, according to the number of 
		* steps defined in the parameter. The distribution is also reduced to the size defined in the
		* parameter.
		* @param steps Number of steps between each update
		* @param size Size of the collision rate distribution that is stored during the simulation.
		*/
		void Set_Collision_Rate_Profile_Interval(int steps, int size);

		/**
		* Number of voxels associated to the formation image,in the dimension idexed by "i"
		*/
		int Image_Size(int i) const;

		/**
		* Defines the internal gradient of the sample
		*/
		void Set_Internal_Gradient(const Field3D& gradient);

		/**
		* @return TRUE if the texture index idx is being masked, blocking any walker to occupy that position
		*/
		bool Masked(int idx) const;

		/**
		* @return TRUE if the formation is masked (walker큦 motion is restricted by another fluid)
		*/
		bool Masked() const;

		/**
		* Places walkers at their starting position. No reallocation is performed (their last positon cannot be recovered).
		*/
		void Init_Walkers_Position();

		/**
		* Sets a mask for the walkers. They cannot move in the solid phase of the mask
		* @param img Mask image
		*/
		void Set_Mask(const rw::BinaryImage& img);

		/**
		* @return TRUE if the simulation is executed at the GPU
		*/
		bool GPU_Walk() const;

		/**
		& @return TRUE if the surface relaxivity is varying according to pore size (collision rate).
		*/
		bool Varying_Surface_Relaxivity() const;

		/**
		* @return Maximal number of iterations allowed
		*/
		uint Max_Number_Of_Iterations() const;

		/**
		* Updates walker's collision rate during the simulation
		*/
		int Update_Varying_Relaxivity_Steps() const;

		/**
		* @return The number of steps between each surface relaxivity update according to the
		* walker's collision rate.
		*/
		int Update_Profile_Interval() const;

		/**
		* Updates the collision profile that is stored internally with the compression defined
		* in _profileCompressionSize.
		*/
		void Update_Collision_Profile(uint currentItr);
		
		/**
		* @return The 3D field gradient
		*/
		const Field3D& Gradient() const;

		/**
		* @return A reference to the random walk observer
		*/
		const Plug::RandomWalkObserver& Observer() const;

		/**
		* Sets the simulation parameters.
		* @param params Parameter array object
		*/
		void Set_Simulation_Parameter(const SimulationParams& params);

		/**
		* @return Current simulation parameters object
		*/
		const SimulationParams& Simulation_Parameters() const;

		/**
		* Generates a new seed for random number generation
		*/
		uint Pick_New_Seed();

		/**
		* @return The number of iterations executed during the simulation. 
		*/
		uint Total_Number_Of_Simulated_Iterations() const;

		/**
		* Sets the number of simulated iterations
		* @param itrs Number of iterations
		*/
		void Set_Total_Number_Of_Simulated_Iterations(uint itrs);
	};

	inline bool Plug::Masked() const
	{
		return(this->_mask != 0);
	}

	inline uint Plug::Number_Of_Walking_Particles() const
	{
		return((uint)this->_simParams.Get_Value(NO_OF_WALKERS));
	}

	inline bool Plug::Showing_Walkers() const
	{
		return(this->_showWalkers);
	}

	inline bool Plug::Walking_Particles_Placed() const
	{
		return(this->_walkersPlaced);
	}

	inline uint Plug::Decay_Size() const
	{
		return((uint)this->_decayValues.size());
	}

	inline Walker& Plug::Walking_Particle(int i)
	{
		return(this->_walkers[i]);
	}

	inline const Walker& Plug::Walking_Particle(int i) const
	{
		return(this->_walkers[i]);
	}

	inline uint Plug::Minimal_Walkers_Per_Thread() const
	{
		return((uint)this->_simParams.Get_Value(MIN_WALKERS_PER_THREAD));
	}

	inline bool Plug::Has_Associated_Image() const
	{
		return ((this->_image) && (this->_image->Length() > 0));
	}

	inline bool Plug::Empty_Particles() const
	{
		return(this->_walkers.size() == 0);
	}

	inline uint Plug::Seed_For_Random_Number_Generation() const
	{
		return(this->_simParams.Get_Bool(SEED));
	}
	
	inline bool Plug::Varying_Surface_Relaxivity() const
	{
		return(this->_simParams.Get_Bool(VARYING));
	}

	inline const BinaryImage& Plug::Plug_Texture() const
	{
		return(*this->_image);
	}

	inline uint Plug::Max_Number_Of_Iterations() const
	{
		return(this->_simParams.Get_Value(ITERATION_LIMIT));
	}

	inline int Plug::Update_Profile_Interval() const
	{
		return (this->_simParams.Get_Value(PROFILE_UPDATE));
	}

	inline int Plug::Update_Varying_Relaxivity_Steps() const
	{
		return(this->_simParams.Get_Value(PROFILE_UPDATE));
	}

	inline const Field3D& Plug::Gradient() const
	{
		return(this->_gradient);
	}

	inline uint Plug::Dimension() const
	{
		uint r = 3;
		if (this->_image->Depth() == 0)
		{
			r = 2;
		}
		return(r);
	}

	inline const RandomWalkObserver& Plug::Observer() const
	{
		return(*this->_updateEvent);
	}

	inline const SimulationParams& Plug::Simulation_Parameters() const
	{
		return(this->_simParams);
	}
}

#endif