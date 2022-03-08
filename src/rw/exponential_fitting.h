#ifndef EXPFIT_H
#define EXPFIT_H

#include <string>
#include "rw/plug.h"
#include "math_la/math_lac/full/matrix.h"
#include "math_la/math_lac/full/vector.h"

using std::string;

namespace rw
{

	class PlugPersistent;

	/**
	* This class handles all aspects related to the T2 distribution
	*/

	class ExponentialFitting
	{
	private:
		/**
		* Decay values corresponding to its range
		*/
		math_la::math_lac::full::Vector _decayRange;

		/**
		* Kernel matrix to obtaind the T2 distribution
		*/
		math_la::math_lac::full::Matrix _kernelMatrix;

		/**
		* Decay values corresponding to its domain
		*/
		math_la::math_lac::full::Vector _decayDomain;

		/**
		* Time values for the Laplace transform
		*/
		math_la::math_lac::full::Vector _laplaceDomain;

		/**
		* Range values for the Laplace transform
		*/
		math_la::math_lac::full::Vector _laplaceRange;
	
		/**
		* Regularizer
		*/
		scalar _laplaceFactor;

		/**
		* TRUE if Laplace range is normalized
		*/
		bool _normalizeLaplaceRange;

		/**
		* Kernel type: T1 or T2
		*/
		int _kernelType; 

		/**
		* Modifies the kernel according to the kernel type. TRUE for T1.
		*/
		bool _modKernel;

		/**
		* This method is necessary to adapy a T2 or T1 distribution. 
		* @param decay Range of the decay to modify
		*/
		void Modify_Kernel_Decay(math_la::math_lac::full::Vector& decay);
	public:
		ExponentialFitting();

		/**
		* Loads T2 distribution parameters from a simulation
		* @param sim Simulation to pick parameters from
		*/
		void Load_Sim(const PlugPersistent& sim);

		/**
		* Loads T2 distribution parameters from a formation
		* @param formation Formation to pick parameters from
		*/
		void Load_Formation(const rw::Plug& formation);

		/**
		* Loads vector of decays to the fitting device
		* @param values Decay values
		*/
		void Load_Decay(const vector<rw::Step_Value>& values);

		/**
		* Mounts the kernel matrix associated to the T2 distribution. 
		* @param laplace_t_min Minimal laplace time (in logarithmic scale). For example, use -2 instead of 0.01
		* @param laplace_t_max Maximal laplace time (in logarithmic scale). For example, use 2 instead of 100
		* @param range_size Number of bins of the T2 distribution
		* @param lambda Regularizer value
		*/
		void Kernel_T2_Mount(scalar laplace_t_min, scalar laplace_t_max, int range_size, scalar lambda = 0);

		/**
		* Mounts the kernel matrix associated to the T2 distribution and the regularizer estimate
		* @param laplace_t_min Minimal laplace time (in logarithmic scale). For example, use -2 instead of 0.01
		* @param laplace_t_max Maximal laplace time (in logarithmic scale). For example, use 2 instead of 100
		* @param range_size Number of bins of the T2 distribution
		* @param lambda_min Minimal regularizer value
		* @param lambda_max Maximal regularizer value
		* @param lambda_resolution Precision with which the regularizer is estimated (number of points)
		*/

		math_la::math_lac::full::Vector Regularizer_Mount(scalar laplace_t_min, scalar laplace_t_max, int range_size,
			scalar lambda_min, scalar lambda_max, int lambda_resolution);
		/**
		* Solves the T2 distribution 
		* @param domain Time domain of the T2 distribution. Non logarithmic scale
		* @param range Bins of the T2 distribution
		*/
		void Solve(math_la::math_lac::full::Vector& domain, math_la::math_lac::full::Vector& range);

		/**
		* @return Number of bins of the T2 distribution
		*/
		int Range_Size() const;

		/**
		* @return Maximal value of the T2 distribution
		*/
		scalar Laplace_Factor() const;

		/**
		* Reduces the exponential decay of the exponential fitting, returning a logarithmically compressed decay
		* @param n Number of samples of the new reduced decay
		* @return A new reduced exponential fitting object
		*/
		ExponentialFitting Logarithmic_Reduction(int n);

		/**
		* Same as ExponentialFitting::Logarithmic_Reduction but the process is not paralelized
		*/
		ExponentialFitting Sequential_Logarithmic_Reduction(int n);

		/**
		* @return Time vector for the T2 distribution
		*/
		math_la::math_lac::full::Vector Laplace_Domain() const;

		/**
		* @return T2 distribution (bins)
		*/
		math_la::math_lac::full::Vector Laplace_Range() const;

		/**
		* Normalizes laplace transform by magnitude according to the bool
		* @param nf TRUE if normalizing. FALSE otherwise
		*/
		void Normalize(bool nf);

		/**
		* Sets to zero all laplace entries outside the interval [tmin,tmax] in non logarithmic scale. A smooth T2 distribution is returned
		* @param t2 Time T2 distribution vector
		* @param laplace Bins of the T2 distribution
		* @param tmin Minimal time
		* @param tmax Maximal time
		* @param loss Returns the area loss due to the smoothing process
		*/
		static math_la::math_lac::full::Vector Laplace_Filter(const math_la::math_lac::full::Vector& t2, 
			const math_la::math_lac::full::Vector& laplace, 
			scalar tmin, scalar tmax, scalar& loss);

		/**
		* Sets kernel type, T1 or T2
		*/
		void Set_Kernel_Type(int kernel_type);
	};

	inline int ExponentialFitting::Range_Size() const
	{
		return((int)this->_laplaceRange.Size());
	}

	inline scalar ExponentialFitting::Laplace_Factor() const
	{
		return(this->_laplaceFactor);
	}

}


#endif