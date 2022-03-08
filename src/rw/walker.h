#ifndef FORMATION_WALKER_H
#define FORMATION_WALKER_H

#include <amp.h>
#include <amp_math.h>
#include "math_la/mdefs.h"

namespace rw
{

	#define WALKER_POS_MASK_X 0x0000FFFF
	#define WALKER_POS_MASK_Y 0xFFFF0000
	#define WALKER_POS_SHIFT 16
	#define WALKER_STRIKES_MASK 0xFFFFFFF8 
	#define WALKER_ALIVE_MASK 0x00000004
	#define WALKER_NOT_STRIKES_MASK 0x00000007
	#define WALKER_DIM_MASK 0x00000003
	#define SHIFT_STRIKES 3


	/**
	* The walker is the basic unit in the random walk simulator. It is a fluid particle that contains basic information
	* about magnetization decay.
	*/
	class Walker
	{
	private:
		/** An union variable that stores the number of strikes and the walker dimensions (2 or 3)*/
		int _hitsDim;
		/** The position of the walker, stored as integers. There can be 3 positions*/
		int _pos[2];
		/** The energy of the walker, normalized in the interval [0,1] */
		float _energy;
		/** The relaxation value the walker carries. This is necessary to simulate distributions*/
		float _delta;		
	public:
		/**
		*Default constructor
		*/
		Walker() GPU;
		/**
		* The copy constructor
		* @param w Walker to be copied
		*/
		Walker(const Walker& w) GPU;

		Walker& operator=(const Walker& w) GPU;

		/**
		* Establishes the walker paremeters
		* @param dim Dimension of the walker. It can be 2D or 3D
		* @param rho Relaxation value of the current walker
		* @param strikes Walker number of collisions
		*/
		void Set(int dim, scalar rho, int strikes) GPU;
		/**
		* Updates number of walker strikes. It is always used during the Random Walk simulation, every time the walker
		* hits a solid surface.
		* @param strikes. The walker number of strikes
		*/
		void Set_Strikes(int strikes) GPU;

		/**
		* Updates the energy of the walker
		* @param energy. Current energy of the walker
		*/

		void Set_Magnetization(scalar magnetization) GPU;

		/**
		* Updates the current possition of the walker
		* @param id Coordinate of the position to update
		* @param val New coordinate value of the position
		*/
		Walker& operator()(int id, int Decay_Step_Value) GPU;

		/**
		* Access the position of the walker whose coordinate is given by id
		* @param id Coordinate of the walker
		*/
		int operator()(int id) const GPU;

		/**
		* @return Position of the coordinate indx of the walker
		* @param indx Coordinate of the walker
		*/
		int Coordinate(int indx) const GPU;

		/**
		* @ereturn Dimension of the formation in which the walker resides. It can be 2 or 3.
		*/
		int Dimension() const GPU;

		/**
		* @return Fraction of energy the wealker decays when it hits a solid surface. This value is used internally by
		* the formation to simulate the NMR decay. The degradation can be applied as a continuous function or as a probability.
		*/
		scalar Rho() const GPU;

		/**
		* @param rho Establishes the degradation the walker suffers when it hits a solid surface.
		*/
		void Set_Rho(scalar rho) GPU;

		/**
		* @return The number of times the walker has colided during the simulation
		*/
		int Hits() const GPU;

		/**
		* @return The flag of colisions and dimension of the walker. Both variables are stored in a single "int" to save memory
		*/
		int Hits_Dim_Flag() const GPU;

		/**
		* Set the colision and dimension flag of the walker.
		* @param flag Collision and dimension flag
		* @see strikesDimFlag()
		*/
		void Set_Hits_Dim_Flag(int flag) GPU;

		/**
		* @return TRUE if the Walker is alive
		*/
		bool Alive() const GPU;

		/**
		* Kills the walker, setting its energy to zero
		*/
		void Kill() GPU;

		/**
		* @return The amount of energy of the walker
		*/
		scalar Magnetization() const GPU;

		/**
		* Degrades the internal magnetization of the walker, by multiplying its
		* current magnetization by the value of_delta.
		*/

		void Degrade() GPU;

		/**
		* Sets the 3D position of the walker
		* @param position The new position of the Walker
		*/
		void Set_Position(const rw::Pos3i& position) GPU;

		/**
		* @return The 3D position of the walker
		*/
		rw::Pos3i Position() const GPU;
	};

	inline Walker::Walker() GPU
	{
		this->_pos[0] = 0;
		this->_pos[1] = 0;
		this->_hitsDim = WALKER_ALIVE_MASK;
		this->_delta = 0;
		this->_energy = 1;
	}

	inline scalar Walker::Magnetization() const GPU
	{
		return((scalar)this->_energy);
	}

	inline void Walker::Set_Rho(scalar rho) GPU
	{
		this->_delta = (float)rho;
	}

	inline Walker::Walker(const Walker& w) GPU
	{
		this->_pos[0] = w._pos[0];
		this->_pos[1] = w._pos[1];
		this->_hitsDim = w._hitsDim;
		this->_delta = w._delta;
		this->_energy = w._energy;	
	}

	inline Walker& Walker::operator=(const Walker& w) GPU
	{
		this->_pos[0] = w._pos[0];
		this->_pos[1] = w._pos[1];
		this->_hitsDim = w._hitsDim;
		this->_delta = w._delta;
		this->_energy = w._energy;
		return(*this);
	}

	inline Walker& Walker::operator()(int id, int value) GPU
	{
		id = id % 3;
		uint pp = ((uint)value) << WALKER_POS_SHIFT;
		(id == 0) ? this->_pos[0] = (this->_pos[0] & WALKER_POS_MASK_Y) | value
			: (id == 1) ? this->_pos[0] = (this->_pos[0] & WALKER_POS_MASK_X) | pp
			: this->_pos[1] = value;
		return(*this);
	}

	inline void Walker::Set_Position(const rw::Pos3i& position) GPU
	{
		this->_pos[0] = (uint)position.x | ((uint)position.y << WALKER_POS_SHIFT);
		this->_pos[1] = (uint)position.z;
	}

	inline rw::Pos3i Walker::Position() const GPU
	{
		rw::Pos3i pp;
		pp.x = this->_pos[0] & WALKER_POS_MASK_X;
		pp.y = (this->_pos[0] & WALKER_POS_MASK_Y) >> WALKER_POS_SHIFT;
		pp.z = this->_pos[1];
		return(pp);
	}

	inline int Walker::operator()(int id) const GPU
	{
		return(this->Coordinate(id));
	}

	inline void Walker::Set_Strikes(int strikes) GPU
	{
		this->_hitsDim = (this->_hitsDim & WALKER_NOT_STRIKES_MASK) | (strikes << SHIFT_STRIKES);
	}

	inline void Walker::Set_Magnetization(scalar magnetization) GPU
	{
		this->_energy = (float)magnetization;
	}

	inline void Walker::Kill() GPU
	{
		int ss = this->_hitsDim & WALKER_DIM_MASK;
		this->_hitsDim = ((this->_hitsDim >> SHIFT_STRIKES) << SHIFT_STRIKES) | ss;
	}

	inline bool Walker::Alive() const GPU
	{
		return((this->_hitsDim & WALKER_ALIVE_MASK) > 0);
	}

	inline void Walker::Set(int dim, scalar rho, int strikes) GPU
	{
		this->_delta = (float)rho;
		this->_hitsDim = (this->_hitsDim | dim) | (strikes << SHIFT_STRIKES);
	}


	inline int Walker::Coordinate(int indx) const GPU
	{
		indx = indx % 3;
		return((indx == 0) ? this->_pos[0] & WALKER_POS_MASK_X :
			(indx == 1) ? (this->_pos[0] & WALKER_POS_MASK_Y) >> WALKER_POS_SHIFT
			: this->_pos[1]);
	}

	inline int Walker::Dimension() const GPU
	{
		return(this->_hitsDim & WALKER_DIM_MASK);
	}

	inline scalar Walker::Rho() const GPU
	{
		return(this->_delta);
	}

	inline int Walker::Hits() const GPU
	{
		return(this->_hitsDim >> SHIFT_STRIKES);
	}

	inline int Walker::Hits_Dim_Flag() const GPU
	{
		return(this->_hitsDim);
	}

	inline void Walker::Set_Hits_Dim_Flag(int flag) GPU
	{
		this->_hitsDim = flag;
	}

	inline void Walker::Degrade() GPU
	{
		this->_energy = this->_energy * this->_delta;
		this->Set_Strikes(this->Hits() + 1);
	}


}

#endif
