#include <iostream>
#include <fstream>
#include <algorithm>
#include "tbb/parallel_for.h"
#include "tbb/spin_mutex.h"
#include <wx/progdlg.h>
#include "math_la/txt/separator.h"
#include "math_la/txt/converter.h"
#include "plug_persistent.h"
#include "math_la/file/binary.h"
#include "math_la/math_lac/space/vec3.h"

namespace rw
{

	using file::Binary;
	using std::ofstream;
	using std::ifstream;
	using std::size;

	bool PlugPersistent::_randomSeedGenerated = false;
	std::mt19937 PlugPersistent::_randomMerseneGenerator;

	PlugPersistent::PlugPersistent()
	{
		this->_samplesSNR = 100;
		if (!PlugPersistent::_randomSeedGenerated)
		{
			PlugPersistent::_randomMerseneGenerator.seed(time(0));
			PlugPersistent::_randomSeedGenerated = true;
			PlugPersistent::_randomMerseneGenerator.discard(5192);
		}
		this->_magnetization = 1;
		this->_voxelSize = 0;
		this->_voxelSizeUnits = 1;
		this->_diffusionCoefficient = 1;
		this->_diffusionCoefficientUnits = 0;
		this->_delta = 1;
		this->_dimension = 1;
		this->_rho = 1;
		this->_rhoUnits = 0;
		this->_dt = 1;
		this->_dtUnits = 0;
		this->_bulkTime = 3;
		this->_dimension = 1;
		this->_imageIndex = 0;
		this->_laplaceApplied = false;
		this->_laplaceT2max = 1;
		this->_laplaceT2min = -3;
		this->_laplaceResolution = 128;
		this->_laplaceRegularizer = 1;
		this->_laplaceMax = 1;
		this->_simCode = -1;
		this->_noiseAmplitude = 0;
		int r = rand() % 100;
		int g = rand() % 100;
		int b = rand() % 100;
		this->_totalIterations = 0;
		this->_simColor = rw::RGBColor(154 + r, 154 + g, 154 + b);
		this->_bulkApplied = false;
		this->_gradientUnits = 0;
		this->_gyromagneticRatio = 0;
		this->_gyroUnits = 0;
		this->_timeStep = 0;
		this->_timeStepUnits = 0;
	}

	void PlugPersistent::Set_Laplace_Regularizer(scalar regularizer)
	{
		this->_laplaceRegularizer = regularizer;
	}

	PlugPersistent::~PlugPersistent()
	{
		for (int k = 0; k < (int)this->_profileSequence.size(); ++k)
		{
			vector<scalar>* profile = this->_profileSequence[k];
			delete profile;
		}
		this->_profileSequence.clear();
	}


	int PlugPersistent::Laplace_Resolution() const
	{
		return(this->_laplaceResolution);
	}

	void PlugPersistent::Set_Magnetization_Factor(scalar Magnetization_Factor)
	{
		this->_magnetization = Magnetization_Factor;
	}

	scalar PlugPersistent::Magnetization_Factor() const
	{
		return(this->_magnetization);
	}

	void PlugPersistent::Set_Decay_Reduction(int red)
	{
		this->_simParams.Set_Value(DECAY_REDUCTION, red);
	}

	int PlugPersistent::Decay_Reduction() const
	{
		return(this->_simParams.Get_Value(DECAY_REDUCTION));
	}


	scalar PlugPersistent::Laplace_T_Min() const
	{
		return(pow(10,this->_laplaceT2min));
	}

	scalar PlugPersistent::Laplace_T_Max() const
	{
		return(pow(10,this->_laplaceT2max));
	}

	scalar PlugPersistent::Regularizer() const
	{
		return(this->_laplaceRegularizer);
	}

	RGBColor PlugPersistent::Sim_Color() const
	{
		return(this->_simColor);
	}


	void PlugPersistent::Set_Sim_Color(const RGBColor& sim_color)
	{
		this->_simColor = sim_color;
	}

	scalar PlugPersistent::Magnetization_Threshold() const
	{
		return(this->_magnetizationThreshold);
	}

	void PlugPersistent::Set_Magnetization_Threshold(scalar mt)
	{
		this->_magnetizationThreshold = mt;
	}

	void PlugPersistent::Save_To_File(const string& filename) const
	{
		Binary bfile(WRITE);
		bfile.Open(string(filename.c_str()));
		bfile.Write((uint)this->_imagePath.size());
		bfile.Write(std::string(this->_imagePath.c_str()), this->_imagePath.size());
		bfile.Write((uint)this->_associatedSims.size());
		for (int k = 0; k < this->_associatedSims.size(); ++k)
		{
			bfile.Write((uint)this->_associatedSims[k].size());
			bfile.Write(std::string(this->_associatedSims[k].c_str()), this->_associatedSims[k].size());
		}
		uint flags[PARAMS_SIZE];
		this->_simParams.Pick_Vector(flags);
		for (int k = 0; k < PARAMS_SIZE; ++k)
		{
			bfile.Write((uint)flags[k]);
		}
		bfile.Write((uint)this->_dateTime);
		bfile.Write((bool)this->_bulkApplied);
		bfile.Write((int)this->_simCode);
		bfile.Write((int)this->_aliveWalkers);
		bfile.Write((int)this->_imageIndex);
		bfile.Write((uint)this->_dimension);
		bfile.Write((double)this->_diffusionCoefficient);
		bfile.Write((double)this->_delta);
		bfile.Write((double)this->_voxelSize);
		bfile.Write((double)this->_rho);
		bfile.Write((double)this->_dt);
		bfile.Write((double)this->_bulkTime);
		bfile.Write((int)this->_diffusionCoefficientUnits);
		bfile.Write((int)this->_voxelSizeUnits);
		bfile.Write((int)this->_rhoUnits);
		bfile.Write((int)this->_dtUnits);
		bfile.Write((uint)this->_comments.size());
		bfile.Write(string(this->_comments.c_str()), this->_comments.size());
		int r = this->_simColor.Red();
		int g = this->_simColor.Green();
		int b = this->_simColor.Blue();
		bfile.Write((int)r);
		bfile.Write((int)g);
		bfile.Write((int)b);
		bfile.Write((double)this->_noiseAmplitude);
		bfile.Write((double)this->_magnetizationThreshold);
		uint tw = this->_simParams.Get_Value(NO_OF_WALKERS);
		for (uint i = 0; i < tw; ++i)
		{
			Walker w = this->_walkers[i];
			bfile.Write((int)i);
			bfile.Write((double)w.Magnetization());
			bfile.Write((double)w.Coordinate(eX));
			bfile.Write((double)w.Coordinate(eY));
			if (this->_dimension > 2)
			{
				bfile.Write((double)w.Coordinate(eZ));
			}
			else
			{
				bfile.Write((double)0);
			}
			bfile.Write((double)w.Rho());
			bfile.Write((uint)w.Hits_Dim_Flag());
			bfile.Write((double)1);
			bfile.Write((int)w.Dimension());
		}
		for (uint i = 0; i < tw; ++i)
		{
			bfile.Write((uint)this->_walkersStartPosition[i].x);
			bfile.Write((uint)this->_walkersStartPosition[i].y);
			bfile.Write((uint)this->_walkersStartPosition[i].z);
		}
		bfile.Write((uint)this->_decayValues.size());
		bfile.Write((uint)this->_totalIterations);
		for (uint i = 0; i < (uint)this->_decayValues.size(); ++i)
		{
			rw::Step_Value decay_step = this->_decayValues[i];
			bfile.Write((uint)decay_step.Iteration);
			bfile.Write((double)decay_step.Time);
			bfile.Write((double)decay_step.Magnetization);
		}
		bfile.Write((bool)this->_laplaceApplied);
		bfile.Write((int)this->_laplaceT2min);
		bfile.Write((int)this->_laplaceT2max);
		bfile.Write((uint)this->_laplaceResolution);
		bfile.Write((double)this->_laplaceRegularizer);
		bfile.Write((double)this->_laplaceMax);
		if (this->_laplaceApplied)
		{
			for (int i = 0; i < (int)this->_laplaceResolution; ++i)
			{
				bfile.Write((double)this->_laplaceT(i));
				bfile.Write((double)this->_laplaceTransform(i));
			}
		}
		bfile.Write((double)this->_magnetization);
		bfile.Write((double)this->_gradient.x);
		bfile.Write((double)this->_gradient.y);
		bfile.Write((double)this->_gradient.z);
		bfile.Write((double)this->_gyromagneticRatio);
		bfile.Write((double)this->_timeStep);
		bfile.Write((uint)this->_gradientUnits);
		bfile.Write((uint)this->_gyroUnits);
		bfile.Write((uint)this->_timeStepUnits);
		bfile.Write((uint)0);
		bfile.Close();
	}

	void PlugPersistent::Load_Header(const string& filename)
	{
		Binary bfile(READ);
		bfile.Open(filename);
		uint size = bfile.Read_UInt();
		this->_imagePath = bfile.Read_String(size);
		uint assocsize = bfile.Read_UInt();
		for (int k = 0; k < (int)assocsize; ++k)
		{
			uint ss = bfile.Read_UInt();
			string a = bfile.Read_String(ss);
			this->_associatedSims.push_back(a);
		}
		uint flags[PARAMS_SIZE];
		for (int k = 0; k < PARAMS_SIZE; ++k)
		{
			flags[k] = bfile.Read_UInt();
		}
		this->_simParams.Copy_From(flags);
		this->_dateTime = bfile.Read_UInt();
		this->_bulkApplied = bfile.Read_Bool();
		this->_simCode = bfile.Read_Int();
		this->_aliveWalkers = bfile.Read_Int();
		this->_imageIndex = bfile.Read_Int();
		this->_dimension = bfile.Read_UInt();
		this->_diffusionCoefficient = bfile.Read_Double();
		this->_delta = bfile.Read_Double();
		this->_voxelSize = bfile.Read_Double();
		this->_rho = bfile.Read_Double();
		this->_dt = bfile.Read_Double();
		this->_bulkTime = bfile.Read_Double();
		this->_diffusionCoefficientUnits = bfile.Read_Int();
		this->_voxelSizeUnits = bfile.Read_Int();
		this->_rhoUnits = bfile.Read_Int();
		this->_dtUnits = bfile.Read_Int();
		uint csize = bfile.Read_UInt();
		this->_comments = bfile.Read_String(csize);
		int r = bfile.Read_Int();
		int g = bfile.Read_Int();
		int b = bfile.Read_Int();
		this->_simColor = RGBColor(r, g, b);
		this->_noiseAmplitude = bfile.Read_Double();
		this->_magnetizationThreshold = bfile.Read_Double();
		bfile.Close();
	}

	uint PlugPersistent::Associated_Sims() const
	{
		return((uint)this->_associatedSims.size());
	}

	const string PlugPersistent::Associated_Sim(uint idx) const
	{
		return(this->_associatedSims[idx].c_str());
	}


	void PlugPersistent::Load_From_File(const string& filename)
	{
		Binary bfile(READ);
		bfile.Open(filename);
		uint size = bfile.Read_UInt();
		this->_imagePath = bfile.Read_String(size);
		uint assocsize = bfile.Read_UInt();
		for (int k = 0; k < (int)assocsize; ++k)
		{
			uint ss = bfile.Read_UInt();
			string a = bfile.Read_String(ss);
			this->_associatedSims.push_back(a);
		}
		uint flags[PARAMS_SIZE];
		for (int k = 0; k < PARAMS_SIZE; ++k)
		{
			flags[k] = bfile.Read_UInt();
		}
		this->_simParams.Copy_From(flags);
		this->_dateTime = bfile.Read_UInt();
		this->_bulkApplied = bfile.Read_Bool();
		this->_simCode = bfile.Read_Int();
		this->_aliveWalkers = bfile.Read_Int();
		this->_imageIndex = bfile.Read_Int();
		this->_dimension = bfile.Read_UInt();
		this->_diffusionCoefficient = bfile.Read_Double();
		this->_delta = bfile.Read_Double();
		this->_voxelSize = bfile.Read_Double();
		this->_rho = bfile.Read_Double();
		this->_dt = bfile.Read_Double();
		this->_bulkTime = bfile.Read_Double();
		this->_diffusionCoefficientUnits = bfile.Read_Int();
		this->_voxelSizeUnits = bfile.Read_Int();
		this->_rhoUnits = bfile.Read_Int();
		this->_dtUnits = bfile.Read_Int();
		uint csize = bfile.Read_UInt();
		this->_comments = bfile.Read_String(csize);	
		int r = bfile.Read_Int();
		int g = bfile.Read_Int();
		int b = bfile.Read_Int();
		this->_simColor = RGBColor(r, g, b);
		this->_noiseAmplitude = bfile.Read_Double();
		this->_magnetizationThreshold = bfile.Read_Double();
		uint tw = this->_simParams.Get_Value(NO_OF_WALKERS);
		this->_walkers.reserve(tw);

		for (uint i = 0; i < tw; ++i)
		{
			Walker w;
			bfile.Read_Int();
			int x;
			int y;
			int z;
			scalar e = bfile.Read_Double();
			x = (int)bfile.Read_Double();
			y = (int)bfile.Read_Double();
			z = (int)bfile.Read_Double();
			scalar rho = bfile.Read_Double();
			uint strikesdim = bfile.Read_UInt();
			scalar speed = bfile.Read_Double();
			int dim = bfile.Read_Int();
			w.Set(dim, rho, 0);
			w.Set_Hits_Dim_Flag(strikesdim);
			w(0, x)(1, y)(2, z);
			w.Set_Magnetization(e);
			this->_walkers.push_back(w);
		}
		this->_walkersStartPosition.clear();
		for (uint i = 0; i < tw; ++i)
		{
			rw::Pos3i pp;
			pp.x = bfile.Read_UInt();
			pp.y = bfile.Read_UInt();
			pp.z = bfile.Read_UInt();
			this->_walkersStartPosition.push_back(pp);
		}
		uint vsize = bfile.Read_UInt();
		this->_decayValues.reserve(vsize);
		this->_totalIterations = bfile.Read_UInt();
		for (uint i = 0; i < vsize; ++i)
		{
			rw::Step_Value step_value;
			step_value.Iteration = bfile.Read_UInt();
			step_value.Time = bfile.Read_Double();
			step_value.Magnetization = bfile.Read_Double();
			this->_decayValues.push_back(step_value);
		}
		this->_laplaceApplied = bfile.Read_Bool();
		this->_laplaceT2min = bfile.Read_Int();
		this->_laplaceT2max = bfile.Read_Int();
		this->_laplaceResolution = bfile.Read_UInt();
		this->_laplaceRegularizer = bfile.Read_Double();
		this->_laplaceMax = bfile.Read_Double();
		if (this->_laplaceApplied)
		{
			this->_laplaceT = math_la::math_lac::full::Vector(this->_laplaceResolution);
			this->_laplaceTransform = math_la::math_lac::full::Vector(this->_laplaceResolution);
			for (int i = 0; i < (int)this->_laplaceResolution; ++i)
			{
				this->_laplaceT(i, bfile.Read_Double());
				this->_laplaceTransform(i, bfile.Read_Double());
			}
		}
		this->_magnetization = bfile.Read_Double();
		this->_gradient.x = bfile.Read_Double();
		this->_gradient.y = bfile.Read_Double();
		this->_gradient.z = bfile.Read_Double();
		this->_gyromagneticRatio = bfile.Read_Double();
		this->_timeStep = bfile.Read_Double();
		this->_gradientUnits = bfile.Read_UInt();
		this->_gyroUnits = bfile.Read_UInt();
		this->_timeStepUnits = bfile.Read_UInt();
		bfile.Read_UInt();
		bfile.Close();
	}

	void PlugPersistent::Fill_Sim_Walkers(const rw::Plug& env)
	{
		this->_dimension = env.Dimension();
		this->_walkers.clear();
		this->_walkers.reserve(env.Number_Of_Walking_Particles());
		for (int i = 0; i < (int)env.Number_Of_Walking_Particles(); ++i)
		{
			Walker w = env.Walking_Particle(i);
			this->_walkers.push_back(w);
		}
	}

	void PlugPersistent::Fill_Sim_Values(const rw::Plug& env)
	{
		this->_decayValues.clear();
		this->_decayValues.reserve(env.Decay_Size());
		for (int i = 0; i < (int)env.Decay_Size(); ++i)
		{
			this->_decayValues.push_back(env.Decay_Step_Value(i));
		}
	}

	void PlugPersistent::Get_Formation_Properties(const rw::Plug& plug)
	{
		this->Fill_Sim_Walkers(plug);
		this->Fill_Sim_Values(plug);
		uint flags[PARAMS_SIZE];
		plug.Simulation_Parameters().Fill_Array(flags);
		this->_simParams.Copy_From(flags);
		this->_delta = plug.Surface_Relaxivity_Delta();
		this->_totalIterations = plug.Total_Number_Of_Simulated_Iterations();
		this->_magnetizationThreshold = plug.Stop_Threshold();
		this->_dimension = plug.Dimension();	
		this->_walkersStartPosition = plug._walkersStartPosition;
		this->_totalIterations = plug.Total_Number_Of_Simulated_Iterations();
		for (int k = 0; k < plug._profileSequence.size(); ++k)
		{
			vector<scalar>* vsc = new vector<scalar>(*plug._profileSequence[k]);
			this->_profileSequence.push_back(vsc);
		}
	}


	void PlugPersistent::Fill_Relaxivity_Optimizer_Parameters(RelaxivityOptimizer& genalg) const
	{
		genalg._D = this->_diffusionCoefficient;
		genalg._DUnit = this->_diffusionCoefficientUnits;
		genalg._S = this->_voxelSize;
		genalg._SUnit = this->_voxelSizeUnits;
		genalg._laplaceT2min = this->_laplaceT2min;
		genalg._laplaceT2max = this->_laplaceT2max;
		genalg._laplaceResolution = this->_laplaceResolution;
		genalg._rhoUnit = this->_rhoUnits;
		genalg._lambda = this->_laplaceRegularizer;
		genalg._reductionT2 = this->_simParams.Get_Value(DECAY_REDUCTION);
		genalg._laplaceTransform = this->_laplaceTransform;
		for (int k = 0; k < this->_profileSequence.size(); ++k)
		{
			vector<scalar>* vsc = new vector<scalar>(*this->_profileSequence[k]);
			genalg._profileSequence.push_back(vsc);
		}
	}


	bool PlugPersistent::Fill_Plug_Paremeters(rw::Plug& plug) const
	{
		bool r = false;
		if (plug.Dimension() == this->Dimension())
		{
			r = true;
			rw::SimulationParams params(this->_simParams);			
			plug.Set_Simulation_Parameter(params);
			plug._walkers = this->_walkers;
			plug.Set_Surface_Relaxivity_Delta(this->_delta);
			plug.Set_TBulk_Time_Seconds(this->_bulkTime);
			plug.Set_Stop_Threshold(this->_magnetizationThreshold);
			plug._decayValues = this->_decayValues;
			plug._walkersStartPosition = this->_walkersStartPosition;
			plug._timeStep = this->Time_Step_Simulation();
			plug._gradient = this->Gradient();
			plug._totalIterations = this->_totalIterations;
			if (this->_profileSequence.size() > 0)
			{
				if (plug._profileSequence.size() > 0)
				{
					for (int k = 0; k < plug._profileSequence.size(); ++k)
					{
						vector<scalar>* profile = plug._profileSequence[k];
						delete profile;
					}
				}
				plug._profileSequence.clear();
				for (int k = 0; k < (int)this->_profileSequence.size(); ++k)
				{
					vector<scalar>* profile = new vector<scalar>(*this->_profileSequence[k]);
					plug._profileSequence.push_back(profile);
				}
			}
		}
		return(r);
	}

	int PlugPersistent::Profile_Process_Vector_Size() const
	{
		return((int)this->_profileSequence.size());
	}

	const vector<scalar>& PlugPersistent::Profile_Vector(int id) const
	{
		vector<scalar>* ptr = this->_profileSequence[id];
		return(*ptr);
	}


	void PlugPersistent::Set_Laplace_Parameters(scalar stmin, scalar stmax, int res, scalar a)
	{
		stmin = fabs(stmin);
		stmax = fabs(stmax);
		scalar amin = std::min(stmin, stmax);
		scalar amax = std::max(stmin, stmax);
		stmin = amin;
		stmax = amax;
		int t2min = 18;
		int t2max = 18;
		scalar sdef; 
		bool transformedmax = false;
		bool transformedmin = false;
		int idx = 18;
		while (((!transformedmax) || (!transformedmin)) && (idx >= -18))
		{
			sdef = pow(10, idx);
			if ((sdef <= stmax+stmax/10)&&(!transformedmax))
			{
				t2max = idx;
				transformedmax = true;
			}
			if (sdef <= stmin+stmin/10)
			{
				t2min = idx;
				transformedmin = true;
			}
			--idx;
		}
		if ((this->_laplaceT2min != t2min) || (this->_laplaceT2max != t2max) || (this->_laplaceRegularizer != a)
			|| (this->_laplaceResolution != res))
		{
			this->_laplaceApplied = false;
			this->_laplaceT2min = t2min;
			this->_laplaceT2max = t2max;
			this->_laplaceResolution = res;
			this->_laplaceRegularizer = a;
		}	
	}

	bool PlugPersistent::Laplace_Applied() const
	{
		return(this->_laplaceApplied);
	}

	scalar PlugPersistent::Laplace_Range_Max_Value() const
	{
		return(this->_laplaceMax);
	}

	void PlugPersistent::Set_Comments(const string& Sim_Name)
	{
		this->_comments = Sim_Name;
	}

	void PlugPersistent::Set_Diffusion_Coefficient(scalar Diffusion_Coefficient, int u)
	{
		this->_diffusionCoefficient = Diffusion_Coefficient;
		this->_diffusionCoefficientUnits = u;
	}

	scalar PlugPersistent::Diffusion_Coefficient() const
	{
		return(this->_diffusionCoefficient);
	}

	int PlugPersistent::Diffusion_Coefficient_Units() const
	{
		return(this->_diffusionCoefficientUnits);
	}

	void PlugPersistent::Set_Surface_Relaxivity(scalar Surface_Relaxivity, int u)
	{
		this->_rho = Surface_Relaxivity;
		this->_rhoUnits = u;
	}

	scalar PlugPersistent::Surface_Relaxivity() const
	{
		return(this->_rho);
	}

	int PlugPersistent::Surface_Relaxivity_Units() const
	{
		return(this->_rhoUnits);
	}

	void PlugPersistent::Set_Voxel_Length(scalar Voxel_Length, int u)
	{
		this->_voxelSize = Voxel_Length;
		this->_voxelSizeUnits = u;
	}

	scalar PlugPersistent::Voxel_Length() const
	{
		return(this->_voxelSize);
	}

	int PlugPersistent::Voxel_Length_Units() const
	{
		return(this->_voxelSizeUnits);
	}

	void PlugPersistent::Set_Bulk_Time(scalar bulk_time, int units)
	{
		int diff = units-3;
		scalar converter = pow(10, 3*diff);
		this->_bulkTime = bulk_time*converter;
	}

	scalar PlugPersistent::Bulk_Time() const
	{
		return(this->_bulkTime);
	}

	void PlugPersistent::Set_Pulse_Time_Step(scalar time_step, int time_step_units)
	{
		this->_dt = time_step;
		this->_dtUnits = time_step_units;
	}

	scalar PlugPersistent::Pulse_Time_Step() const
	{
		return(this->_dt);
	}

	int PlugPersistent::Sim_Time_Step_Units() const
	{
		return(this->_dtUnits);
	}

	int PlugPersistent::Number_Of_Walkers() const
	{
		return(this->_simParams.Get_Value(NO_OF_WALKERS));
	}

	string PlugPersistent::Sim_Comments() const
	{
		return(string(this->_comments.c_str()));
	}

	math_la::math_lac::full::Vector PlugPersistent::Laplace_Time_Vector() const
	{
		return(this->_laplaceT);
	}

	math_la::math_lac::full::Vector PlugPersistent::Laplace_Bin_Vector() const
	{
		return(this->_laplaceTransform);
	}

	int PlugPersistent::Sim_Code() const
	{
		return(this->_simCode);
	}

	void PlugPersistent::Set_Noise_Distortion(scalar v)
	{
		if ((v > 1) || (v < 0))
		{
			v = 0;
		}
		this->_noiseAmplitude = v;
	}

	scalar PlugPersistent::Noise_Distortion() const
	{
		return(this->_noiseAmplitude);
	}

	rw::Step_Value PlugPersistent::Decay_Step_Value(int id) const
	{
		if (this->_noiseAmplitude == 0)
		{
			return(this->_decayValues[id]);
		}
		else
		{
			scalar nn = (scalar)PlugPersistent::_randomMerseneGenerator.max();
			scalar random = (scalar)PlugPersistent::_randomMerseneGenerator();
			scalar ns = random /nn - (scalar)0.5;
			ns = ns * this->_noiseAmplitude;
			rw::Step_Value v = this->_decayValues[id];
			scalar nms = std::max(this->_decayValues[0].Magnetization, (scalar)1.0);
			v.Magnetization = v.Magnetization + nms*ns;
			if (v.Magnetization > 1)
			{
				v.Magnetization = 1;
			}
			return(v);
		}
	}

	rw::Step_Value PlugPersistent::Noiseless_Decay_Step_Value(int id) const
	{
		return(this->_decayValues[id]);
	}

	int PlugPersistent::Decay_Vector_Size() const
	{
		return((int)this->_decayValues.size());
	}

	void PlugPersistent::Save_Laplace_CSV_File(const string& filename) const
	{
		if (this->_laplaceT.Size() > 0)
		{
			ofstream file;
			file.open(string(filename.c_str()));
			file << "T2,Normalized value, Factor,";
			file << this->_laplaceMax << "\n";
			for (int i = 0; i < this->_laplaceT.Size(); ++i)
			{
				scalar t2 = this->_laplaceT(i);
				scalar _laplaceRange = this->_laplaceTransform(i);
				file << t2 << "," << _laplaceRange << "\n";
			}
			file.close();
		}
	}

	int PlugPersistent::Image_ID() const
	{
		return(this->_imageIndex);
	}


	void PlugPersistent::Save_Decay_CSV_File(const string& filename) const
	{
		if (this->_decayValues.size() > 0)
		{
			ofstream file;
			file.open(string(filename.c_str()));
			file << "Iteration,Time,Alive Walkers,Energy(Noiseless), Noise , Energy \n";
			for (int i = 0; i < this->_decayValues.size(); ++i)
			{
				rw::Step_Value v = this->_decayValues[i];
				rw::Step_Value nv = this->Decay_Step_Value(i);
				file << v.Iteration << "," << v.Time << ",";
				file << "," << v.Magnetization - nv.Magnetization;
				file << "," << nv.Magnetization;
				file << "\n";
			}
			file.close();
		}
	}

	void PlugPersistent::Save_Collision_Rate_Distribution_CSV_File(const string& filename) const
	{
		map<int, int> count;
		map<int, vector<int>> idmap;
		for (int i = 0; i < this->_walkers.size(); ++i)
		{
			rw::Walker w = this->_walkers[i];
			int f = w.Hits();
			map<int, int>::iterator j = count.find(f);
			if (j == count.end())
			{
				count[f] = 1;
				vector<int> lst;
				idmap[f] = lst;
				idmap[f].push_back(i);
			}
			else
			{
				count[f] = j->second + 1;
				idmap[f].push_back(i);
			}
		}
		ofstream file;
		file.open(string(filename.c_str()));
		file << "Walker id, Number of collisions, Restricted trajectory fraction, Walker Global Id, LPos X, LPos Y, LPos Z, BPosX , BPos Y, BPos Z \n";
		map<int, int>::reverse_iterator i = count.rbegin();
		int xi = 1;
		while (i != count.rend())
		{
			for (int k = 0; k < i->second; ++k)
			{
				file << xi << "," << i->first << "," << (scalar)i->first / (scalar)this->_totalIterations << ",";
				int id = idmap[i->first][k];
				Walker w = this->_walkers[id];
				file << id << "," << w.Coordinate(eX) << "," << w.Coordinate(eY) << "," << w.Coordinate(eZ) << ",";
				file << this->_walkersStartPosition[id].x << "," << this->_walkersStartPosition[id].y << "," << this->_walkersStartPosition[id].z << "\n";
				++xi;
			}
			++i;
		}
		file.close();
	}

	scalar PlugPersistent::Build_Collision_Rate_Distribution()
	{
		return(this->Build_Collision_Rate_Distribution(this->_walkers, this->_historicCollisionDomain, this->_historicCollisionWeight, this->_totalIterations));
	}

	scalar PlugPersistent::Build_Collision_Rate_Distribution(math_la::math_lac::full::Vector& Diffusion_Coefficient, math_la::math_lac::full::Vector& W) const
	{
		return(this->Build_Collision_Rate_Distribution(this->_walkers, Diffusion_Coefficient, W, this->_totalIterations));
	}

	scalar PlugPersistent::Build_Collision_Rate_Distribution(const vec(rw::Walker)& walkers, 
		math_la::math_lac::full::Vector& D, math_la::math_lac::full::Vector& W, int tot)
	{
		map<int, int> count;
		for (int i = 0; i < walkers.size(); ++i)
		{
			rw::Walker w = walkers[i];
			int f = w.Hits();
			map<int, int>::iterator j = count.find(f);
			if (j == count.end())
			{
				count[f] = 1;
			}
			else
			{
				j->second = j->second + 1;
			}
		}
		map<int, int>::reverse_iterator j = count.rbegin();
		int ii = 0;
		int prop = 0;
		D = math_la::math_lac::full::Vector((int)count.size());
		W = math_la::math_lac::full::Vector((int)count.size());
		while (j != count.rend())
		{
			scalar w = (scalar)j->first;
			ii = ii + j->second;
			D(prop, (scalar)ii / (scalar)walkers.size());
			W(prop, w / (scalar)tot);
			++prop;
			++j;
		}
		if (W.Size() > 0)
		{
			return(W(0));
		}
		else
		{
			return(0);
		}
	}

	math_la::math_lac::full::Vector PlugPersistent::Build_Collision_Rate_Distribution_Vector(const vec(rw::Walker)& walkers, int tot)
	{
		map<int, int> count;
		map<int, vector<int>> idmap;
		for (int i = 0; i < walkers.size(); ++i)
		{
			rw::Walker w = walkers[i];
			int f = w.Hits();
			map<int, int>::iterator j = count.find(f);
			if (j == count.end())
			{
				count[f] = 1;
				vector<int> lst;
				idmap[f] = lst;
				idmap[f].push_back(i);
			}
			else
			{
				count[f] = j->second + 1;
				idmap[f].push_back(i);
			}
		}
		math_la::math_lac::full::Vector r;
		r.Set_Size((int)walkers.size());
		map<int, int>::reverse_iterator i = count.rbegin();
		int Voxel_Length = 0;
		while (i != count.rend())
		{
			for (int k = 0; k < i->second; ++k)
			{
				r(Voxel_Length, (scalar)i->first / (scalar)tot);
				++Voxel_Length;
			}
			++i;
		}
		return(r);
	}



	math_la::math_lac::full::Vector PlugPersistent::Collision_Rate_Domain() const
	{
		return(this->_historicCollisionDomain);
	}

	math_la::math_lac::full::Vector PlugPersistent::Collision_Rate_Range() const
	{
		return(this->_historicCollisionWeight);
	}

	void PlugPersistent::Replace_Laplace_Vector(const math_la::math_lac::full::Vector& nl)
	{
		this->_laplaceTransform = nl;
	}

	scalar PlugPersistent::Surface_Relaxivity_Denormalized(scalar factor) const
	{
		int ddu = this->Diffusion_Coefficient_Units();
		int vox_length_units = this->Voxel_Length_Units();
		int dst = vox_length_units - ddu;
		int rhou = this->Surface_Relaxivity_Units();
		int dpt = rhou - ddu;
		scalar vox_length = this->Voxel_Length();
		vox_length = vox_length * pow(10, 3 * dst);
		scalar dd = this->Diffusion_Coefficient();
		scalar d = 1 - factor;
		scalar rho = d*3*dd/(2*vox_length);
		rho = rho * pow(10, 3 * dpt);
		return(rho);
	}

	scalar PlugPersistent::Surface_Relaxivity_Factor(scalar rho) const
	{
		int ddu = this->Diffusion_Coefficient_Units();
		int vox_length_units = this->Voxel_Length_Units();
		int dst = vox_length_units - ddu;
		int rhou = this->Surface_Relaxivity_Units();
		int dpt = rhou - ddu;
		scalar vox_length = this->Voxel_Length();
		vox_length = vox_length*pow(10, 3 * dst);
		rho = rho*pow(10, 3 * dpt);
		scalar dd = this->Diffusion_Coefficient();
		scalar d = 2 * rho*vox_length / (3 * dd);
		scalar r = (scalar)1 - d;
		return(r);
	}

	scalar PlugPersistent::Time_Step_Simulation() const
	{
		scalar diffusion_coefficient = this->Diffusion_Coefficient();
		scalar length = this->Voxel_Length();
		int du = this->Diffusion_Coefficient_Units();
		int ds = this->Voxel_Length_Units();
		int df = ds - du;
		scalar st = pow(10, 3 * df);
		length = length*st;
		scalar timestep = length*length / (6 * diffusion_coefficient);
		return(timestep);
	}

	void PlugPersistent::Set_Gradient_Parameters(scalar gyro, uint gyroUnits,
		const Field3D& gradient, uint gradientUnits, scalar dt, uint dtunits)
	{
		this->_gyromagneticRatio = gyro;
		this->_gyroUnits = gyroUnits;
		this->_gradient = gradient;
		this->_gradientUnits = gradientUnits;
		this->_timeStep = dt;
		this->_timeStepUnits = dtunits;
	}

	void PlugPersistent::Get_Gradient_Parameters(scalar& gyro, uint& gyroUnits,
		Field3D& gradient, uint& gradientUnits, scalar& dt, uint& dtunits) const
	{
		gyro = this->_gyromagneticRatio;
		gyroUnits = this->_gyroUnits;
		gradient = this->_gradient;
		gradientUnits = this->_gradientUnits;
		dt = this->_timeStep;
		dtunits = this->_timeStepUnits;
	}

	Field3D PlugPersistent::Gradient() const
	{
		Field3D grad;
		scalar diffusion = this->Diffusion_Coefficient();
		int dt = this->Diffusion_Coefficient_Units() - 1;
		diffusion = diffusion*pow(10, 6 * dt);
		int dtg = this->_gyroUnits;
		scalar gyro = this->_gyromagneticRatio*pow(10,-4*dtg);
		math_la::math_lac::space::Vec3 gradient(this->_gradient.x, this->_gradient.y, this->_gradient.z);
		int dtgg = this->_gradientUnits;
		gradient = gradient*pow(10, 4 * dtgg);
		scalar ddt = this->_timeStep;
		int tg = this->_timeStepUnits - 3;
		ddt = ddt*pow(10, 3 * tg);
		scalar tss = this->Time_Step_Simulation();
		scalar f = gyro*gyro*diffusion*ddt*tss*tss;
		grad.x = f*gradient(eX)*gradient(eX);
		grad.y = f*gradient(eY)*gradient(eY);
		grad.z = f*gradient(eZ)*gradient(eZ);
		return(grad);
	}

	scalar PlugPersistent::Signal_To_Noise_Ratio() const
	{
		scalar snr = 0;
		if (this->_decayValues.size() > 2*this->_samplesSNR)
		{
			scalar us = 0;
			scalar rs = 0;
			scalar un = 0;
			scalar rn = 0;
			for (int k = 0; k < (int)this->_samplesSNR; ++k)
			{
				scalar vs = this->Decay_Step_Value(k).Magnetization;
				us = us + fabs(vs);
				rs = rs + vs*vs;
				scalar vn = this->Decay_Step_Value(this->_decayValues.size()-1-k).Magnetization;
				if (this->_simParams.T2_Relaxation())
				{
					un = un + fabs(vn);
					rn = rn + vn * vn;
				}
				else
				{
					scalar a = (scalar)1 - vn;
					un = un + fabs(a);
					rn = rn + a * a;
				}
			}
			scalar n = (scalar)this->_samplesSNR;
			us = us / n;
			rs = rs / n;
			un = un / n;
			rn = rn / n;
			scalar den = rn;
			if (den <= 0)
			{
				snr = 1e18;
			}
			else
			{
				snr = (rs - us*us) / den;
			}
		}
		return(snr);
	}

	void PlugPersistent::Apply_Laplace()
	{
		ExponentialFitting fit;
		uint tt = 1;
		if (this->SimulationParams().T1_Relaxation())
		{
			tt = 0;
		}
		fit.Set_Kernel_Type(tt);	
		fit.Load_Sim(*this);
		fit = fit.Logarithmic_Reduction(this->_simParams.Get_Value(DECAY_REDUCTION));
		fit.Kernel_T2_Mount(this->_laplaceT2min, this->_laplaceT2max, 
			this->_laplaceResolution,this->_laplaceRegularizer);
		fit.Solve(this->_laplaceT, this->_laplaceTransform);	
		this->_laplaceApplied = true;
	}

	void PlugPersistent::Set_Image_Path(const string& path)
	{
		this->_imagePath = path;
	}

	void PlugPersistent::Set_Date_Time(uint date_time)
	{
		this->_dateTime = date_time;
	}

	uint PlugPersistent::Date_Time() const
	{
		return(this->_dateTime);
	}

	void PlugPersistent::Replace_Decay(const vector<scalar>& domain, const vector<scalar>& decay)
	{
		this->_decayValues.clear();
		for (int k = 0; k < domain.size(); ++k)
		{
			rw::Step_Value step;
			step.Time = domain[k];
			step.Magnetization = decay[k];
			step.Iteration = k;
			this->_decayValues.push_back(step);
		}
	}

	void PlugPersistent::Collision_Rate_Weights(map<scalar, scalar2>& distribution) const
	{
		distribution.clear();
		scalar vl = this->Voxel_Length();
		scalar vsize = vl * pow(10, 3 * (1 - this->Voxel_Length_Units()));
		scalar rad = 0;
		scalar step = vsize;
		scalar rad_max = 1000 * step;

		scalar2 s;
		s.x = 0;
		s.y = 1;
		distribution.insert(std::pair<scalar, scalar2>(0, s));
		rad = rad + step;
		while (rad <= rad_max)
		{
			scalar2 s;
			s.x = 0;
			s.y = (3 * vsize) / (4 * rad);
			distribution.insert(std::pair<scalar,scalar2>(rad,s));
			rad = rad + step;		
		}
		tbb::spin_mutex mtx;
		scalar tot = (scalar)this->Total_Number_Of_Simulated_Iterations();
		tbb::parallel_for(tbb::blocked_range<uint>(0, (uint)this->_walkers.size(), BCHUNK_SIZE), 
			[this,vsize,step,&distribution,&mtx,tot](const tbb::blocked_range<uint>& b)
		{
			for (uint i = b.begin(); i < b.end(); ++i)
			{
				const rw::Walker& w = this->_walkers[i];
				scalar xi = (scalar)w.Hits() / tot;
				if (xi <= 0)
				{
					xi = 1e-6;
				}
				scalar rad = (3 * vsize) / (4 * xi);
				rad = rad + step/2;
				map < scalar, scalar2 >::iterator ii = distribution.upper_bound(rad);
				if (ii == distribution.end())
				{
					map < scalar, scalar2 >::reverse_iterator il = distribution.rbegin();
					mtx.lock();
					il->second.x = il->second.x + 1;
					mtx.unlock();
				}
				else
				{
					--ii;
					mtx.lock();
					ii->second.x = ii->second.x + 1;
					mtx.unlock();
				}			
			}
		});
	}

	void PlugPersistent::Save_Collision_Rate_CSV_Weight_File(const string& filename) const
	{
		map<scalar, scalar2> distribution;
		this->Collision_Rate_Weights(distribution);
		std::ofstream file;
		file.open(string(filename.c_str()));
		file << "RADIUS, XI, VOLUME_WEIGHT \n";
		map<scalar, scalar2>::const_iterator itr = distribution.begin();
		while (itr != distribution.end())
		{
			file << itr->first << "," << itr->second.y << ",";
			file << itr->second.x / (scalar)this->_walkers.size() << "\n";
			++itr;
		}
		file.close();
	}

	void PlugPersistent::Set_Simulation_Parameters(const rw::SimulationParams& params)
	{
		this->_simParams.Copy_From(params);
	}

	const SimulationParams& PlugPersistent::SimulationParams() const
	{
		return(this->_simParams);
	}

}
