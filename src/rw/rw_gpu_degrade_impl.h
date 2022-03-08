#ifndef RANDOM_WALK_GPU_DEGRADE_IMPLEMENTOR_H
#define RANDOM_WALK_GPU_DEGRADE_IMPLEMENTOR_H

#include <amp.h>
#include <amp_math.h>
#include "math_la/mdefs.h"
#include "random_walk_implementor.h"
#include "walker.h"

namespace rw
{


	/**
	* This walker handling parallelizes the random walk movement in the GPU.
	* It is important to note that this particular walker handling demands several rules:
	*
	* 1) The number of walkers must be a multiple of 1000.
	*
	* 2) If the number of walkers is more than 128 000, then they wll have to be split
	* in several groups and the gain in performance is less noticeable.
	*
	*/

	class RandomWalkGPUDegradeImplementor : public RandomWalkImplementor
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
		* This vector of scalars stores the information of each set or group of walkers.
		* In order to boost performance, each walker is processed continously in several iterations
		* whose number is givan by TimeSize = 128. Therefore, before passing to another set of walkers
		* each walker set is processed for 128 iterations in time. The total magnetization of this
		* processing is stored in this vector.
		*/
		vec(uint) _imagnetization;

		/**
		* Block magnetization, storing the magnetization of the different walkers´ blocks
		*/
		vec(scalar) _blockMagnetization;

		/**
		* Not normalized value of the current magnetization
		*/

		vec(scalar) _magnetization;

		/**
		* The seeds for the random number generator of each walker. These seeds are used for an
		* internal GPU linear random generator, on which the walker takes its decisions.
		*/
		uint* _seeds;

		/**
		* The minimal block size in which the walkers are split
		*/
		int _blockSize;

		/**
		* The GPU container of the walker random number generator seeds.
		*/
		concurrency::array<uint, 1>* _rndSeeds;

		/**
		* The GPU container of the walkers
		*/
		concurrency::array<rw::Walker, 1>* _walkers;

		/**
		* The number of blocks in which the walkers areprocessed.
		* The total number of walkers must be a multiple of 1000
		*/
		int _noblocks;

		/**
		* The precision factor in which the magnetization is calculated.
		* We note this magnetization is calculated using integer arithmetic
		* so its precision is limited. By default a precision of 4 digits
		* (with _precision = 10 000)
		*/
		int _precision;

		concurrency::array<uint, 1>* _image;
		concurrency::array<uint, 1>* _mask;

	protected:
		/**
		* This method degrades a walker and is executed at the GPU.
		* @param id Unique identifier of the walker
		* @param timestep The local timestep that is being processed in the walker. The walkers are processed in groups of times of a fixed size, and timestep denotes
		* the current timestep group.
		* @param walker The walker that is bein processed
		* @param rnd The decision number
		* @param width Texture width
		* @param height Texture height
		* @param depth Texture depth
		* @param texture The 3D texture containing the image. Its values are accessed with the method BinaryImage::Accesor_Read
		* @param magnetization The vector of magnetization where all partial reductions are taken into account
		* @param precision The accumlative values of the magnetization are handled as integer numbers. This stores the number of digits of precision.
		* The common value is 4.
		*/
		static void Degrade_Walker_GPU_Cross_Periodic(int id, int timestep, rw::Walker& walker, uint rnd, int depth, int width, int height,
			const concurrency::array<uint, 1>& texture, concurrency::array<uint, 1>& magnetization, uint precision) GPUP;
		void Copy_Walkers_To_CPU();
	public:
		RandomWalkGPUDegradeImplementor(rw::Plug* parent);
		~RandomWalkGPUDegradeImplementor();
		void Init_Seeds(VSLStreamStatePtr& stream);
		void Init();
		void Execute_Walking_Step(VSLStreamStatePtr& stream);
		virtual void Execute();
		void operator()();
	};	
}



#endif

