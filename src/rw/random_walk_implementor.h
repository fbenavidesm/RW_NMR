#ifndef RANDOM_WALK_IMPLEMENTOR
#define RANDOM_WALK_IMPLEMENTOR

#include "math_la/mdefs.h"
#include "tbb/parallel_for.h"
#include "rw/plug.h"
#include "rw/random_walk_step_value.h"
#include "rw/relaxivity_distribution.h"
#include "rw/random_walk_observer.h"
#include "rw/walker.h"

namespace rw
{

	/**
	* A RandomWalkImplementor provides an interfase to implement several rules for the random motion of the walkers.
	* It is a pure abstract class, and each implementation must overload the operator(). 
	* The idea of the implementor is to provide a base class for all random walk implementations which can be defined 
	* with different methodologies. For example, two different instances of this class will define the rules for a 2D
	* or a 3D random walk, or for a CPU or GPU parallelization. 
	*/
	class RandomWalkImplementor
	{
	private:
		/**
		* The parent formation of the implementator
		*/
		rw::Plug* _parentFormation;
	protected:
		rw::Plug& Plug();
	public:	
		/**
		* An implementor must be always associated to a rw::Formation
		* @param parent Parent formation of the implementator
		*/
		RandomWalkImplementor(rw::Plug* parent);

		/**
		* The copy constrcutr inherits the associated formation
		*/
		RandomWalkImplementor(RandomWalkImplementor& parent);
		virtual ~RandomWalkImplementor();

		/**
		* Pushes a sequential step value to the Formation decay list. 
		* @param value The new value to be inserted
		*/
		void Push_Seq_Step_Value(const Step_Value& value);

		/**
		* Reserves memory space for the decay values. This is called generally just before
		* the random walk simulation.
		* @param size Size of the memory space
		*/
		void Reserve_Values_Memory_Space(uint size);

		/**
		* Sets the total number of iterations to zero
		*/
		void Init_Iterations();

		/**
		* Pickas randomly a new seed for the random number generation
		*@return New seed value
		*/
		uint Pick_New_Seed();		

		/**
		* Updates the current random walk observer (if it exists)
		* @param iteration Current number of iteration
		* @param magnetization Current magnetization value
		*/
		void Observe(uint iteration, scalar magnetization);

		/**
		* This method executes a post-processing when the experiment is a T1 essay. In this case, the exponential function
		* is increasing from a-0.5 to approximately 0.5.
		*/
		void Check_T1_Experiment();

		/**
		* The end of the walk, informing the event about the number of seconds that have passed.
		* @param secs Number of seconds
		*/
		void Walk_End(int secs);

		/**
		* @param id Walker id
		* @return The walker whose id is given by the parameter. It returns a reference, which can be modofied
		*/
		rw::Walker& Walker(uint id);


		virtual void operator()() = 0;

		BinaryImage& Image();
		BinaryImage& Mask();

		bool Masked() const;

		vec(rw::Walker)& Walkers();

		void Set_Seed(uint seed);

		bool Has_Walk_Event() const;
	};

	inline RandomWalkImplementor::~RandomWalkImplementor()
	{
		this->_parentFormation = 0;
	}

	inline vec(rw::Walker)& RandomWalkImplementor::Walkers()
	{
		return(this->_parentFormation->_walkers);
	}

	inline bool RandomWalkImplementor::Masked() const
	{
		return(this->_parentFormation->_mask != 0);
	}

	inline BinaryImage& RandomWalkImplementor::Mask()
	{
		return(*this->_parentFormation->_mask);
	}

	inline RandomWalkImplementor::RandomWalkImplementor(RandomWalkImplementor& parent)
	{
		this->_parentFormation = parent._parentFormation;
	}

	inline RandomWalkImplementor::RandomWalkImplementor(rw::Plug* parent)
	{
		this->_parentFormation = parent;
	}

	inline void RandomWalkImplementor::Push_Seq_Step_Value(const Step_Value& value)
	{
		this->_parentFormation->_decayValues.push_back(value);
	};

	inline rw::Plug& RandomWalkImplementor::Plug()
	{
		return(*this->_parentFormation);
	};

	inline void RandomWalkImplementor::Reserve_Values_Memory_Space(uint size)
	{
		this->_parentFormation->_decayValues.clear();
		this->_parentFormation->_decayValues.reserve(size);
	}

	inline void RandomWalkImplementor::Init_Iterations()
	{
		this->_parentFormation->_decayValues.clear();
	}

	inline uint RandomWalkImplementor::Pick_New_Seed()
	{
		uint seed = this->_parentFormation->Pick_New_Seed();
		return(seed);
	}

	inline void RandomWalkImplementor::Observe(uint iteration, scalar energy)
	{
		if (this->_parentFormation->Has_Walk_Event())
		{
			scalar perc = (scalar)1 - (log10(this->_parentFormation->Stop_Threshold()) - log10(energy)) / (log10(this->_parentFormation->Stop_Threshold()));
			this->_parentFormation->On_Walk_Event()->Observe_Walk(perc, energy, ((scalar)iteration)*this->_parentFormation->Time_Step());
		}
	}

	inline bool RandomWalkImplementor::Has_Walk_Event() const
	{
		return(this->_parentFormation->Has_Walk_Event());
	}

	inline void RandomWalkImplementor::Walk_End(int secs)
	{
		if (this->_parentFormation->_updateEvent)
		{
			this->_parentFormation->_updateEvent->_elapsedSeconds = secs;
			this->_parentFormation->_updateEvent->Walk_End();
		}
	}

	
	inline void RandomWalkImplementor::Check_T1_Experiment()
	{
		if (this->_parentFormation->Simulation_Parameters().T1_Relaxation())
		{
			tbb::parallel_for(tbb::blocked_range<int>(0, (int)this->_parentFormation->_decayValues.size(), BCHUNK_SIZE), 
				[this](const tbb::blocked_range<int>& b)
			{
				for (int j = b.begin(); j != b.end(); ++j)
				{
					scalar v = this->_parentFormation->_decayValues[j].Magnetization;
					this->_parentFormation->_decayValues[j].Magnetization = (scalar)1 - (scalar)2 * v;
				}
			});
		}
	}

	inline rw::Walker& RandomWalkImplementor::Walker(uint id)
	{
		return(this->_parentFormation->_walkers[id]);
	}

	inline BinaryImage& RandomWalkImplementor::Image()
	{
		return(*this->_parentFormation->_image);
	}

	inline void RandomWalkImplementor::Set_Seed(uint seed)
	{
		this->_parentFormation->_simParams.Set_Value(SEED,seed);
	}
}

#endif