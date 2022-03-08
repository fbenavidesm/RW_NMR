#ifndef SIM_PARAMS_H
#define SIM_PARAMS_H

#include "math_la/mdefs.h"

#define PARAMS_SIZE    512
#define DEGR           0x00000001
#define GPU_P          0x00000002
#define T2             0x00000004
#define DYN_RHO        0x00000008
#define VARYING        0x00000040
#define REPEAT         0x00000080

#define SEED						511
#define PROFILE_UPDATE				510
#define PROFILE_SIZE				509
#define ITERATION_LIMIT				508
#define NO_OF_WALKERS				507
#define MIN_WALKERS_PER_THREAD		506
#define DIM_X						503
#define DIM_Y						504
#define DIM_Z						505
#define DECAY_REDUCTION				502

namespace rw
{

	class SimulationParams
	{
	private:
		uint _field[PARAMS_SIZE];
	public:
		SimulationParams();
		SimulationParams(const uint* field);
		SimulationParams(const SimulationParams& params);
		void Copy_From(const SimulationParams& params);
		void Pick_Vector(uint* field) const;
		void Set_Bool(uint idx, bool value);
		bool Get_Bool(uint idx) const;
		void Set_Value(uint idx, uint value);
		uint Get_Value(uint idx) const;
		bool Kill() const;
		bool Degrade() const;
		bool Gpu() const;
		bool Cpu() const;
		bool T2_Relaxation() const;
		bool T1_Relaxation() const;
		void Fill_Array(uint* array) const;
	};

	inline SimulationParams::SimulationParams()
	{
		for (int i = 0; i < PARAMS_SIZE; ++i)
		{
			this->_field[i] = 0;
		}
		this->Set_Bool(DEGR, true);
	};

	inline SimulationParams::SimulationParams(const SimulationParams& params)
	{
		for (int i = 0; i < PARAMS_SIZE; ++i)
		{
			this->_field[i] = params._field[i];
		}
	}

	inline void SimulationParams::Copy_From(const SimulationParams& params)
	{
		for (int i = 0; i < PARAMS_SIZE; ++i)
		{
			this->_field[i] = params._field[i];
		}
	}

	inline SimulationParams::SimulationParams(const uint* field)
	{
		for (int i = 0; i < PARAMS_SIZE; ++i)
		{
			this->_field[i] = field[i];
		}
	};

	inline void SimulationParams::Pick_Vector(uint* field) const
	{
		for (int i = 0; i < PARAMS_SIZE; ++i)
		{
			field[i] = this->_field[i];
		}
	}

	inline void SimulationParams::Set_Bool(uint idx, bool value)
	{
		uint id = idx / PARAMS_SIZE;
		uint bit = idx - id * PARAMS_SIZE;
		uint mask = 0x01 << bit;
		mask = ~mask;
		this->_field[id] = this->_field[id] & mask;
		mask = ~mask;
		if (value)
		{
			this->_field[id] = this->_field[id] | mask;
		}
	};

	inline bool SimulationParams::Get_Bool(uint idx) const
	{
		uint id = idx / PARAMS_SIZE;
		uint bit = idx - id * PARAMS_SIZE;
		uint mask = 0x01 << bit;
		uint ret = this->_field[id] & mask;
		return(ret > 0);
	}
	
	inline bool SimulationParams::Degrade() const
	{
		return (this->Get_Bool(DEGR));
	}

	inline bool SimulationParams::Gpu() const
	{
		return(this->Get_Bool(GPU_P));
	}

	inline bool SimulationParams::Cpu() const
	{
		return(!this->Gpu());
	}

	inline bool SimulationParams::Kill() const
	{
		return(!this->Degrade());
	}

	inline void SimulationParams::Set_Value(uint idx, uint value)
	{
		this->_field[idx] = value;
	}

	inline uint SimulationParams::Get_Value(uint idx) const
	{
		return(this->_field[idx]);
	}


	inline bool SimulationParams::T2_Relaxation() const
	{
		return(this->Get_Bool(T2));
	}

	inline bool SimulationParams::T1_Relaxation() const
	{
		return(!this->Get_Bool(T2));
	}

	inline void SimulationParams::Fill_Array(uint* array) const
	{
		for (uint i = 0; i < PARAMS_SIZE; ++i)
		{
			array[i] = this->_field[i];
		}
	}

	
}

#endif
