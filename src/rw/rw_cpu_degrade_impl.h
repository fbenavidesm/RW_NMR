#ifndef RANDOM_WALK_CPU_DEGRADE_IMPLEMENTOR_H
#define RANDOM_WALK_CPU_DEGRADE_IMPLEMENTOR_H

#include "random_walk_implementor.h"
#include "rw/walker.h"
#include "rw/plug.h"

namespace rw
{
	class RandomWalkCPUDegradeImplementor : public RandomWalkImplementor
	{
	private:

		/**
		* The width of the 3D texture
		*/
		uint _textureWidth;

		/**
		* The height of the 3D texture
		*/
		uint _textureHeight;

		/**
		* Depth of the 3D texture
		*/
		uint _textureDepth;
		
		/**
		* The maximal number for the random number generation, required for every walker to make a decision
		*/
		uint _maxRnd;

		/**
		* The chunk size of the parallel block
		*/
		uint _chunkSize;


		/**
		* During the walking processes, some shared information may be updated across the threads. Each threads controls its own
		* WalkerHandling instance, but these children processes cannot free these shared data. This flag defines if the
		* instance is a child or a parent.
		*/
		bool _shared;

		/**
		* Magnetization processed by the instance
		*/
		scalar* _magnetization;

		/**
		* Vector of collisions, shared by the walkers
		*/
		int* _collisionDistribution;
	protected:
		void Set_Max_Rnd(uint maxrnd);

	public:
		RandomWalkCPUDegradeImplementor(rw::Plug* parent);
		RandomWalkCPUDegradeImplementor(RandomWalkCPUDegradeImplementor& p, split);
		~RandomWalkCPUDegradeImplementor();

		/**
		* Inits random number seeds, and the decision of all walkers
		*/
		void init();
		uint Max_Rnd() const;
		void operator()(const blocked_range<int>& r);

		/**
		* Joins two handlers, reducing the total magnetization
		*/
		void join(RandomWalkCPUDegradeImplementor& p);

		void operator()();

		virtual void Process_Walker(int id, rw::Walker& walker, int rnd, const Field3D& gradient);
		virtual void Execute();
		virtual void Set_Degrees_Of_Freedom();
	};

	inline uint RandomWalkCPUDegradeImplementor::Max_Rnd() const
	{
		return(this->_maxRnd);
	}
}


#endif
