#ifndef RANDOM_WALK_OBSERVER_H
#define RANDOM_WALK_OBSERVER_H

#include "math_la/mdefs.h"

namespace rw
{
	class RandomWalkImplementor;
	/**
	* The Observer allows to visualize the random walk process when it is being executed, generally in a
	* background thread.
	*/
	class RandomWalkObserver
	{
	private:
		friend class Plug;
		friend class RandomWalkImplementor;
		/**
		* Parent formation whose simulation execute the events
		*/
		Plug* _parentFormation;

		/**
		* This variable determines if the walkers are visible or not
		*/
		bool _showWalkers;

		/**
		* Number of seconds elapsed since the last time the event was triggered
		*/
		int _elapsedSeconds;
	public:
		RandomWalkObserver();
		/**
		* Parent formation that executes the event
		*/
		Plug& Parent_Formation();
		/**
		* Walking event, a procedure that is called at every walking frame. It allows to observe the walking
		* process
		*/
		virtual void Observe_Walk(scalar perc, scalar Magnetization_Factor, scalar Decay_Domain) = 0;

		/**
		* This is called when the simulation ends
		*/
		virtual void Walk_End() = 0;

		/**
		* @return TRUE if the walker position is shown in every frame
		*/
		bool Show_Walkers() const;

		/**
		* Elapsed seconds since the beginning of the simulation
		*/
		int Elapsed_Seconds();
	};

	inline Plug& RandomWalkObserver::Parent_Formation()
	{
		return(*this->_parentFormation);
	}

	inline bool RandomWalkObserver::Show_Walkers() const
	{
		return(this->_showWalkers);
	}

}

#endif