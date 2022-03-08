#ifndef RANDOM_WALK_STEP_VALUE
#define RANDOM_WALK_STEP_VALUE


#include "math_la/mdefs.h"

namespace rw
{
	/**
	* A step value in the sequence of time steps of the simulation. Contains the most relevant information
	* about the magentization decay step.
	*/

	struct Step_Value
	{
		/**
		* Magnetization value
		*/
		scalar Magnetization;

		/**
		* Time value for the current magnetization
		*/
		scalar Time;

		/**
		* Number of iterations since the beginning of the simulatiopn
		*/
		uint Iteration;
	};
}


#endif