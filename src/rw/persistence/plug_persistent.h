#ifndef SIM_H
#define SIM_H

#include <vector>
#include <map>
#include <random>
#include "math_la/mdefs.h"
#include "rw/plug.h"
#include "rw/field3d.h"
#include "rw/exponential_fitting.h"
#include "math_la/math_lac/full/vector.h"
#include "rw/relaxivity_optimizer.h"
#include "rw/random_walk_step_value.h"
#include "rw/binary_image/rgb_color.h"
#include "rw/sim_params.h"

namespace rw
{
	using std::vector;
	using std::map;
	class SimStore;
	class RelaxivityOptimizer;
	class Relaxivity_Simulator;
	class ProfileOptimizer;
	class PerSim;

	/**
	* This class handles all relevant information about a Random Walk simulation. This information can be stored in a file or can be used
	* to populate a rw::Formation. The walkers position, the RND seed, the last position of walkers, the Laplace transform and other
	* simulation values are stored with this class. All simulation parameters can be stored in file and can be used to reproduce the walker's path
	* for a genetic algorithm optimization. 
	*/
	class PlugPersistent
	{
	private:
		friend class rw::RelaxivityOptimizer;
		friend class rw::ExponentialFitting;
		friend class rw::SimStore;
		friend class rw::RelaxivityDistribution;
		friend class rw::Relaxivity_Simulator;
		friend class rw::ProfileOptimizer;
		friend class rw::PerSim;
		/**
		* Date time in which the simulation was executed
		*/
		uint _dateTime;
		/**
		* Associated simulations of the current simulation. The path is stored here. 
		*/
		vector<string> _associatedSims;

		/**
		* Path file of the image associated to the simulation
		*/
		string _imagePath;

		/**
		* Diffusion coefficient value of the staurating fluid. 
		*/
		scalar _diffusionCoefficient;

		/**
		* The diffusion coefficient units are defined separately as 0: nm^2/s, 1: um^2/s and 2: mm^2/s
		*/
		int _diffusionCoefficientUnits;

		/**
		* Surface relaxivity of the formation. Here it is assumed that this relaxivity is constant, however
		* varying surface relaxivities can be defined later before starting the simulation. So this is a default
		* value. 
		*/
		scalar _rho;

		/**
		* The units of the surface relaxivity. The correspondence is: 0: nm/s, 1: um/s  and 2: mm/s
		*/
		int _rhoUnits;

		/** 
		* The voxel size of the image which also defines the step size each walker moves and indirectly
		* the time step of the simulation samples. 
		*/
		scalar _voxelSize;

		/**
		* The units of the voxel size. The correspondence is: 0: nm, 1: um and 2: mm
		*/
		int _voxelSizeUnits;

		/**
		* Time step of the simulation
		*/
		scalar _dt;

		/**
		* Units of the time step. 0: ns, 1: us, 2: ms, 3: s
		*/
		int _dtUnits;

		/**
		* The relaxivity fraction associated to the surface relaxivity. This values does not have associated units.
		* The value of _delta is contained in the interval [0,1]. 
		*/
		scalar _delta;

		/**
		* T2 Bulk relaxation time, in seconds
		*/
		scalar _bulkTime;

		/**
		* Total number of iterations in the simulation
		*/
		int _totalIterations;

		/**
		* Enerrgy threshold criterion to stop the simulation
		*/
		scalar _magnetizationThreshold;

		/**
		* Noise absolute magnitude
		*/
		scalar _noiseAmplitude;

		/**
		* The set of walkers of the simulation
		*/
		vec(Walker) _walkers;

		/**
		* Starting walker positions
		*/
		vector <Pos3i> _walkersStartPosition;

		/**
		* Magnetization decay valyes
		*/
		vector <rw::Step_Value> _decayValues;

		/**
		* The sequence of colision rate along the simulation. This collision rate distribution evolves with time and this evolution
		* is captured in this sequence. The size of these vectors do not need to be of the same size of the total number of walkers
		*/
		vector <vector<scalar>*> _profileSequence;

		/**
		* Simulation name. It can be used as a unique identifier. 
		*/
		string _comments;

		/**
		* Number of space dimensions of the simulation
		*/
		uint _dimension;

		/**
		* Image index associated to the simulation (in case it is a 2D simulation) 
		*/
		int _imageIndex;

		/**
		* Simulation code that identifies the simulation uniquely
		*/
		int _simCode;

		/**
		* Laplace transform time domain
		*/
		math_la::math_lac::full::Vector _laplaceT;

		/**
		* Laplace transform bin domain
		*/
		math_la::math_lac::full::Vector _laplaceTransform;

		/**
		* Laplace transform smallest power n. The time starting value is 10^n
		*/
		int _laplaceT2min;

		/**
		* Laplace transform largest power n. The time starting value is 10^n
		*/
		int _laplaceT2max;

		/**
		* Number of bins in the Laplace transform
		*/
		uint _laplaceResolution;

		/**
		* Laplace regularizer
		*/
		scalar _laplaceRegularizer;

		/**
		* Laplace maximal value
		*/
		scalar _laplaceMax;

		/**
		* TRUE if Laplace transform has been applied to the simulation decay
		*/
		bool _laplaceApplied;

		/**
		* Simulation color
		*/
		RGBColor _simColor;

		/**
		* Historical evolution of walkers colissions. Domain
		*/
		math_la::math_lac::full::Vector _historicCollisionDomain;

		/**
		* Historical evolution of walkers colissions. Weights
		*/
		math_la::math_lac::full::Vector _historicCollisionWeight;

		/**
		* Magnetization normalizer. It is generally 1. 
		*/
		scalar _magnetization;

		/**
		* Simulation flags, used to store extra propoerties and keep file compatibility
		*/
		rw::SimulationParams _simParams;

		/**
		* TRUE if bulk relaxation is applied
		*/
		bool _bulkApplied;

		/**
		* Number of alive walkers
		*/
		int _aliveWalkers;

		/**
		* Random number generator to generate the simulation and image codes
		*/
		static std::mt19937 _randomMerseneGenerator;

		/**
		* TRUE if local seed has been generated
		*/
		static bool _randomSeedGenerated;

		/**
		* Internal gradient.
		*/
		Field3D _gradient;

		/**
		* Units of the field gradient. 0: G/um 1: T/um
		*/
		uint _gradientUnits;

		/**
		* Internal gradient gyromagnetic ratio. 
		*/
		scalar _gyromagneticRatio;

		/**
		* Units of the gyromagnetic ratio. 0: rad/(Gs) 1: rad/(Ts)
		*/

		uint _gyroUnits;

		/**
		* Timestep of the pulses in seconds
		*/
		scalar _timeStep;

		/**
		* Units of the time step between pulses for the gradient magnetic field. 0: ns 1: us 2:ms 3: s
		*/
		uint _timeStepUnits;

		/**
		* Number of samples to estimate Signal to noise ratio
		*/
		uint _samplesSNR;

		/**
		* Populater walker information based on the information collected in the Formation. 
		*/
		void Fill_Sim_Walkers(const rw::Plug& env);

		/**
		* Populates simulation values with the information collected in the Formation
		*/
		void Fill_Sim_Values(const rw::Plug& env);
	public:	
		PlugPersistent();
		~PlugPersistent();

		/**
		* Fills the simulation with the parameters of the formation env. 
		* @param env Formation containing data of the simulation
		*/
		void Get_Formation_Properties(const rw::Plug& f);

		/**
		* FIll the Formation object with the parameters collected in the simulation
		* @param plug Plug to be filled
		*/
		bool Fill_Plug_Paremeters(rw::Plug& plug) const;

		/**
		* Sets the noise magnitude, which is applied to the decay defining a noisy output. 
		* The value is normalized in the interval [0,1[. 
		* @param v Value of the noise magnitude.
		*/
		void Set_Noise_Distortion(scalar v);

		/**
		* @return Noise distortion that is applied to the decay
		*/
		scalar Noise_Distortion() const;

		/**
		* Sets the diffusion coefficient of the simulation. 
		* @param diffusion_coefficient Diffusion coefficient value
		* @param u Units. 0: nm^2/s, 1: um^2/s and 2: mm^2/s
		*/
		void Set_Diffusion_Coefficient(scalar diffusion_coefficient, int u);

		/**
		* @return Diffucion coefficient valye
		*/
		scalar Diffusion_Coefficient() const;

		/**
		* @return Diffucion coefficient unit index. 0: nm^2/s, 1: um^2/s and 2: mm^2/s
		*/
		int Diffusion_Coefficient_Units() const;

		/**
		* Sets surface relaxivity
		* @param P Surface relaxivity value
		* @param u Surface relaxivity unit index. 0: nm/s, 1: um/s  and 2: mm/s
		*/
		void Set_Surface_Relaxivity(scalar sr, int u);

		/**
		* @return Surface relaxivity value
		*/
		scalar Surface_Relaxivity() const;

		/**
		* @return Surface relaxivity units. 0: nm/s, 1: um/s  and 2: mm/s
		*/
		int Surface_Relaxivity_Units() const;

		/**
		* Sets voxel length. 
		* @param s Voxel length
		* @param u Voxel length units. 0: nm, 1: um and 2: mm
		*/
		void Set_Voxel_Length(scalar Voxel_Length, int u);

		/**
		* @return Voxel length
		*/
		scalar Voxel_Length() const;

		/**
		* @return Voxel length units. 0: nm, 1: um and 2: mm
		*/
		int Voxel_Length_Units() const;

		/**
		* Sets Bulk time. 
		* @param bulk_time Bulk time value
		* @param Time units. 0: ns 1: us 2:ms 3: s
		*/
		void Set_Bulk_Time(scalar bulk_time, int units = 3);

		/**
		* @return Bulk time in seconds
		*/
		scalar Bulk_Time() const;

		/**
		* Sets the simulation time step. 
		* @param dt Time step
		* @param dtu Units of the time step
		*/
		void Set_Pulse_Time_Step(scalar sim_time_step, int sim_time_step_units);

		/**
		* @return Simulation time step value in seconds
		*/
		scalar Pulse_Time_Step() const;

		/**
		* @return Simulation time step units. 0: ns, 1: us, 2: ms, 3: s
		*/
		int Sim_Time_Step_Units() const;

		/**
		* @return Number of walkers
		*/
		int Number_Of_Walkers() const;

		/**
		* Threshold of magnetization energy to stop the simulation
		*/
		scalar Magnetization_Threshold() const;

		/**
		* Sets magnetization stopping criteria for the simulation
		*/
		void Set_Magnetization_Threshold(scalar mt);

		/**
		* Magnetization scale (set to 1)
		*/
		void Set_Magnetization_Factor(scalar magnetization_factor);

		/**
		* @return Magnetization scale (set to 1)
		*/
		scalar Magnetization_Factor() const;

		/**
		* Sets the decay reduction to apply Laplace transform
		* @param red The number of samples to which the decay is reduced
		*/
		void Set_Decay_Reduction(int red);

		/**
		* @return Number of samples of the decay reduction
		*/
		int Decay_Reduction() const;

		/**
		* @return Number of bins of the Laplace transform
		*/
		int Laplace_Resolution() const;

		/**
		* Saves all sim information info
		*/
		void Save_To_File(const string& filename) const;

		/**
		* Recovers all sim information from a file
		*/
		void Load_From_File(const string& filename);

		/**
		* Reads only the simulation main information
		*/
		void Load_Header(const string& filename);

		/**
		* Defines the Laplace parameters to be applied in the Laplace inverse
		]* transform
		*/
		void Set_Laplace_Parameters(scalar tmin, scalar tmax, int res, scalar a);

		/**
		* Sets simulation unique comments
		*/
		void Set_Comments(const string& sim_comments);


		/**
		* Recovers simulation commenbts
		* @return Comments string
		*/
		string Sim_Comments() const;

		/**
		* Laplace domain vector
		*/
		math_la::math_lac::full::Vector Laplace_Time_Vector() const;

		/**
		* Laplace range vector
		*/
		math_la::math_lac::full::Vector Laplace_Bin_Vector() const;

		/**
		* @return Simulation unique identifier
		*/
		int Sim_Code() const;

		/**
		* @return Decay step value, with noise
		*/
		rw::Step_Value Decay_Step_Value(int id) const;

		/**
		* @return Noiseless decay step valye
		*/
		rw::Step_Value Noiseless_Decay_Step_Value(int id) const;

		/**
		* @return Number of iterations
		*/
		int Decay_Vector_Size() const;

		/**
		* @return Simulation color
		*/
		RGBColor Sim_Color() const;

		/**
		* Defines the simulation color
		* @param color Simulation color
		*/
		void Set_Sim_Color(const RGBColor& sim_color);

		/**
		* @return Minimal laplace domain value
		*/
		scalar Laplace_T_Min() const;

		/**
		* @return Maximal laplace domain value
		*/
		scalar Laplace_T_Max() const;

		/**
		* @return Regularizer value as stored in the simulation
		*/
		scalar Regularizer() const;

		/**
		* @return TRUE if Laplace vector is not empty
		*/
		bool Laplace_Applied() const;

		/**
		* Saves Laplace transform as a CSV file
		* @param filename File name
		*/
		void Save_Laplace_CSV_File(const string& filename) const;

		/**
		* Saves Decay as a CSV file
		*/
		void Save_Decay_CSV_File(const string& filename) const;

		/**
		* Saves collision histogram in a CSV file
		*/
		void Save_Collision_Rate_Distribution_CSV_File(const string& filename) const;

		/**
		* Applies Laplace transform using internal parameters. The result is also stored
		* internally.
		*/
		void Apply_Laplace();

		/**
		* Image identifier
		*/
		int Image_ID() const;


		/**
		* Builds collision rate distribution
		*/
		scalar Build_Collision_Rate_Distribution();

		/**
		* Builds collision rate distribution
		* @param proportion of walkers whose collision rate is larger than the corresponding weight
		* @param weight Collision rate value
		*/
		scalar Build_Collision_Rate_Distribution(math_la::math_lac::full::Vector& proportion, math_la::math_lac::full::Vector& weight) const;

		/**
		* Builds collision rate distribution
		* This is a static method that does not use information stored inside Sim. 
		*/
		static scalar Build_Collision_Rate_Distribution(const vec(Walker)& walkers, 
			math_la::math_lac::full::Vector& proportion_domain, math_la::math_lac::full::Vector& weight, int tot);

		/**
		* Creates collision rate distribution vector of weights
		*/
		static math_la::math_lac::full::Vector Build_Collision_Rate_Distribution_Vector(const vec(Walker)& walkers, int tot);

		/**
		* @return Collision rate proportion of walkers
		*/
		math_la::math_lac::full::Vector Collision_Rate_Domain() const;

		/**
		* @return Collision rate value according to the proportion 
		*/
		math_la::math_lac::full::Vector Collision_Rate_Range() const;

		/**
		* Replaces current laplace vector range with the values stored in the parameter
		* @parameter laplace
		*/
		void Replace_Laplace_Vector(const math_la::math_lac::full::Vector& laplace);

		/**
		* @return Maximal value in the range of the Laplace vector
		*/
		scalar Laplace_Range_Max_Value() const;

		/**
		* Fills the relaxivity optimizer with the parameters stored in Sim. 
		*/
		void Fill_Relaxivity_Optimizer_Parameters(RelaxivityOptimizer& genalg) const;

		/**
		* Transforms rho (whose units are defined by the _rhoUnits parameter)
		* from a surface relaxivity value to a relaxivity factor, normalized
		* in the interval [0,1[.
		* @param rho Surface relaxivity in the same units of _rhoUnits
		*/
		scalar Surface_Relaxivity_Factor(scalar rho) const;

		/**
		* Transforms rho (whose units are defined by the _rhoUnits parameter)
		* from a normalized surface relaxivity factor to a surface relaxivity 
		* using the same units defined in the persistent plug. 
		* @param factor Normalized surface relaxivity with no units
		*/
		scalar Surface_Relaxivity_Denormalized(scalar factor) const;
	
		/**
		* Uses internal data to recover the Time_Step corresponding to the simulation. 
		* @return Time_Step associated to the simulation parameters in seconds
		*/
		scalar Time_Step_Simulation() const;

		/**
		* Parameters for the internal gradient
		*/
		void Set_Gradient_Parameters(scalar gyro, uint gyroUnits,
			const Field3D& gradient, uint gradientUnits, scalar dt, uint dtunits);

		/**
		* Populates the variables with the gradient parameters
		*/

		void Get_Gradient_Parameters(scalar& gyro, uint& gyroUnits,
			Field3D& gradient, uint& gradientUnits, scalar& dt, uint& dtunits) const;

		/**
		* @return Internal gradient in (1/s). It can be applied as a relaxation factor using
		* the procedure Exponential_Factor of the returned Field3D. 
		*/
		Field3D Gradient() const;

		/**
		* Signal to noise ratio estimate
		*/
		scalar Signal_To_Noise_Ratio() const;

		/**
		* Sets formation associated image file name
		*/
		void Set_Image_Path(const string& path);
				
		/**
		* Formation dimension (2 or 3)
		*/
		uint Dimension() const;

		/**
		* @return Profile process size through the simulation
		*/
		int Profile_Process_Vector_Size() const;

		/**
		* @return Profile vectoir indexed by id
		*/
		const vector<scalar>& Profile_Vector(int id) const;

		/**
		* Sets the time in which the simulation was executed
		*/
		void Set_Date_Time(uint date_time);

		/**
		* @return Date Time in which the simulation was executed
		*/
		uint Date_Time() const;

		/**
		* Sets Laplace regularizer
		* @param regularizer New regularizer
		*/
		void Set_Laplace_Regularizer(scalar regularizer);

		/**
		* Image path of the associated texture
		*/
		string Image_Path() const;

		/**
		* Replaces decay with vectors
		* @param domain Time domain of the decay
		* @param decay Range of the decay
		*/
		void Replace_Decay(const vector<scalar>& domain, const vector<scalar>& decay);

		/**
		* @return Walker vector associated to the simulation
		*/
		const vec(Walker)& Walker_Vector() const;

		/**
		* @return The number of associated simulations associated to the current simulation
		*/
		uint Associated_Sims() const;

		/**
		* @return The associated simulation indexed by idx
		* @param idx Index of the requested associated simulation
		*/
		const string Associated_Sim(uint idx) const;

		/**
		* Creates a collision rate distribution, estimating a pore size distribution based con the collision rate
		* of all walkers. 
		* @param distribution The returned distribution of scalar weights. It contains a unique identifier (the pore
		* radius) and two defining scalars. The firs entry is the weight (as a reason over the number of walkers) and
		* the second entry is the corresponding xi factor associated to the radius.
		*/

		void Collision_Rate_Weights(map<scalar, scalar2>& distribution) const;

		/**
		* Creates a volumetric pore size distribution file, using only the walkers  colision rate
		* @param filename Name of the file to store de data
		*/

		void Save_Collision_Rate_CSV_Weight_File(const string& filename) const;
		
		void Set_Simulation_Parameters(const SimulationParams& params);
		const SimulationParams& SimulationParams() const;

		uint Max_Iterations() const;
		uint Total_Number_Of_Simulated_Iterations() const;
	};

	inline const vec(Walker)& PlugPersistent::Walker_Vector() const
	{
		return(this->_walkers);
	}

	inline string PlugPersistent::Image_Path() const
	{
		return(this->_imagePath);
	}

	inline uint PlugPersistent::Dimension() const
	{
		return(this->_dimension);
	}

	inline uint PlugPersistent::Max_Iterations() const
	{
		return(this->_simParams.Get_Value(ITERATION_LIMIT));
	}

	inline uint PlugPersistent::Total_Number_Of_Simulated_Iterations() const
	{
		return(this->_totalIterations);
	}

}

#endif