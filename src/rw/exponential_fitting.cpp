#include <iostream>
#include <fstream>
#include "math_la/txt/separator.h"
#include "math_la/txt/converter.h"
#include "exponential_fitting.h"
#include "persistence/plug_persistent.h"
#include "tbb/parallel_for.h"

using std::ifstream;


namespace rw
{

	ExponentialFitting::ExponentialFitting()
	{
		this->_normalizeLaplaceRange = false;
		this->_kernelType = 1;
		this->_modKernel = false;
	}

	void ExponentialFitting::Normalize(bool nf)
	{
		this->_normalizeLaplaceRange = nf;
	}

	void ExponentialFitting::Modify_Kernel_Decay(math_la::math_lac::full::Vector& decay)
	{
		if (!this->_modKernel)
		{
			if (this->_kernelType == 0)
			{
				tbb::parallel_for(tbb::blocked_range<int>(0, (int)decay.Size(), BCHUNK_SIZE), [&decay](const tbb::blocked_range<int>& b)
				{
					for (int j = b.begin(); j != b.end(); ++j)
					{
						scalar v = decay(j);
						decay(j, ((scalar)1 - v )/ (scalar)2);
					}
				});
			}
			this->_modKernel = true;
		}
	}

	void ExponentialFitting::Load_Sim(const rw::PlugPersistent& sim)
	{
		uint size = sim.Decay_Vector_Size();
		this->_decayRange.Set_Size(size);
		this->_decayDomain.Set_Size(size);
		for (int j = 0; j < (int)size; ++j)
		{
			rw::Step_Value v = sim.Decay_Step_Value(j);
			this->_decayRange(j, v.Magnetization);
			this->_decayDomain(j, v.Time);
		}
		this->Modify_Kernel_Decay(this->_decayRange);
	}

	void ExponentialFitting::Load_Formation(const rw::Plug& formation)
	{
		uint size = formation.Decay_Size();
		this->_decayRange.Set_Size(size);
		this->_decayDomain.Set_Size(size);
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)size, BCHUNK_SIZE), [this, &formation](const tbb::blocked_range<int>& b)
		{
			for (int j = b.begin(); j != b.end(); ++j)
			{
				rw::Step_Value v = formation.Decay_Step_Value(j);
				this->_decayRange(j, v.Magnetization);
				this->_decayDomain(j, v.Time);
			}
		});
		this->Modify_Kernel_Decay(this->_decayRange);
	}


	void ExponentialFitting::Load_Decay(const vector<rw::Step_Value>& values)
	{
		uint size = (uint)values.size();
		this->_decayRange.Set_Size(size);
		this->_decayDomain.Set_Size(size);
		for (int j = 0; j < (int)values.size(); ++j)
		{
			rw::Step_Value v = values[j];
			this->_decayRange(j, v.Magnetization);
			this->_decayDomain(j, v.Time);
		}
	}

	void ExponentialFitting::Set_Kernel_Type(int kernel_type)
	{
		this->_kernelType = kernel_type % 2;
	}

	void ExponentialFitting::Kernel_T2_Mount(scalar laplace_t_min, scalar laplace_t_max, int range_size, scalar lambda)
	{
		this->Modify_Kernel_Decay(this->_decayRange);
		this->_laplaceDomain.Set_Size(range_size);
		scalar Tmin = laplace_t_min;
		scalar Tmax = laplace_t_max;
		scalar sim_time_step = (Tmax - Tmin) / ((scalar)(range_size - 1));
		this->_laplaceDomain.Set_Size(range_size);
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)range_size, BCHUNK_SIZE), [this, Tmin, sim_time_step](const tbb::blocked_range<int>& b)
		{
			int beg = b.begin();
			int end = b.end();
			for (int k = beg; k < end; ++k)
			{
				scalar T = Tmin + ((scalar)k)*sim_time_step;
				T = (scalar)pow(10, T);
				this->_laplaceDomain(k, T);
			}
		});
		uint n = this->_decayDomain.Size();
		uint m = this->_laplaceDomain.Size();
		math_la::math_lac::full::Matrix A(n, m);
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)n, BCHUNK_SIZE), [&A, m, this](const tbb::blocked_range<int>& b)
		{
			for (int i = b.begin(); i < b.end(); ++i)
			{
				for (int j = 0; j < (int)m; ++j)
				{
					scalar s = this->_laplaceDomain(j);
					scalar v = exp(-this->_decayDomain(i) / this->_laplaceDomain(j));
					A(i, j, v);
				}
			}
		});

		if (lambda == 0)
		{
			this->_kernelMatrix << A;
		}
		else
		{
			lambda = fabs(lambda);
			this->_kernelMatrix << math_la::math_lac::full::Matrix(m + n, m);
			this->_kernelMatrix(0, 0, A);
			this->_kernelMatrix(n, 0, lambda* math_la::math_lac::full::Matrix::Identity(m));
			math_la::math_lac::full::Vector tt = this->_decayRange;
			this->_decayRange = math_la::math_lac::full::Vector(m + n);
			tbb::parallel_for(tbb::blocked_range<int>(0, n, BCHUNK_SIZE), [this, &tt](const tbb::blocked_range<int>& b)
			{
				for (int k = b.begin(); k < b.end(); ++k)
				{
					scalar v = tt(k);
					this->_decayRange(k, v);
				}
			});
		}
	}

	math_la::math_lac::full::Vector ExponentialFitting::Regularizer_Mount(scalar laplace_t_min, scalar laplace_t_max, int range_size,
		scalar lambda_min, scalar lambda_max, int lambda_resolution)
	{
		this->Modify_Kernel_Decay(this->_decayRange);
		math_la::math_lac::full::Vector dx(lambda_resolution);
		math_la::math_lac::full::Vector da(lambda_resolution);
		math_la::math_lac::full::Vector circ(lambda_resolution);
		scalar loglambdamin = log10(lambda_min);
		scalar loglambdamax = log10(lambda_max);
		scalar ds = (loglambdamax - loglambdamin) / ((scalar)lambda_resolution);
		math_la::math_lac::full::Matrix U;
		math_la::math_lac::full::Matrix V;
		math_la::math_lac::full::Matrix D;
		this->Kernel_T2_Mount(laplace_t_min, laplace_t_max, range_size, 0);
		this->_kernelMatrix.SVD_Zero_Shift(U, V, D);
		math_la::math_lac::full::Vector UY = U.Transposed()*this->_decayRange;
		for (int j = 0; j < lambda_resolution; ++j)
		{
			scalar lambda = loglambdamin + ds*((scalar)j);
			lambda = pow(10, lambda);
			scalar lambda2 = lambda*lambda;
			scalar e = 0;
			scalar n = 0;
			scalar np = 0;
			for (int i = 0; i < range_size; ++i)
			{
				scalar rho = D(i, i);
				scalar rho2 = rho*rho;
				if (rho > EPSILON)
				{
					scalar bi = UY(i);
					scalar fi = rho2 / (rho2 + lambda2);
					scalar fi2 = fi*fi;
					scalar bi2 = bi*bi;
					n = n + fi2*bi2 / (rho2);
					e = e + (1 - fi)*(1 - fi)*bi2;
					np = np - (4 / lambda)*(1 - fi)*fi2*bi2 / rho2;
				}
			}
			dx(j, sqrt(e));
			da(j, 0.5*log10(n));
			scalar cr = (-2 * n*e / np)*(lambda*lambda*np*e + 2 * lambda*n*e + lambda*lambda*lambda*lambda*n*np)
				/ (pow(lambda*lambda*n*n + e*e, 1.5));
			circ(j, cr);
		}
		this->_laplaceDomain = dx;
		this->_laplaceRange = da;
		return(circ);
	}

	math_la::math_lac::full::Vector ExponentialFitting::Laplace_Domain() const
	{
		return(this->_laplaceDomain);
	}

	math_la::math_lac::full::Vector ExponentialFitting::Laplace_Range() const
	{
		return(this->_laplaceRange);
	}

	void ExponentialFitting::Solve(math_la::math_lac::full::Vector& domain, math_la::math_lac::full::Vector& bins)
	{
		this->Modify_Kernel_Decay(this->_decayRange);
		this->_laplaceRange = math_la::math_lac::full::Matrix::NNLS(this->_kernelMatrix, this->_decayRange);
		domain = this->_laplaceDomain;
		bins = this->_laplaceRange;
		scalar n = 0;
		scalar nmax = 0;
		for (int i = 0; i < bins.Size(); ++i)
		{
			n = n + _laplaceRange(i);
			if (_laplaceRange(i) > nmax)
			{
				nmax = _laplaceRange(i);
			}
		}
		if (!this->_normalizeLaplaceRange)
		{
			this->_laplaceFactor = n;
		}
		else
		{
			this->_laplaceFactor = nmax;
			bins = ((scalar)1 / nmax)*bins;
		}
	}

	ExponentialFitting ExponentialFitting::Logarithmic_Reduction(int n)
	{
		if (n < this->_decayDomain.Size())
		{
			ExponentialFitting r;
			int M = this->_decayDomain.Size();
			scalar lM = (scalar)M;
			lM = log10(lM);
			scalar ds = lM / (scalar)(n - 1);
			math_la::math_lac::full::Vector flt(n);
			math_la::math_lac::full::Vector fly(n);

			tbb::parallel_for(tbb::blocked_range<int>(0, n, BCHUNK_SIZE), [&flt, &fly, ds, this](const tbb::blocked_range<int>& b)
			{
				for (int i = b.begin(); i < b.end(); ++i)
				{
					scalar si = (scalar)i;
					scalar lvs = si*ds;
					lvs = pow(10, lvs) - 1;
					if (lvs < si + 1)
					{
						flt(i, this->_decayDomain(i));
						fly(i, this->_decayRange(i));
					}
					else
					{
						int vl = i - 1;
						scalar sl = (scalar)(vl);
						scalar lls = sl*ds;
						lls = pow(10, lls) - 1;
						if (lls > sl + 1)
						{
							vl = (int)lls;
							vl = vl%this->_decayDomain.Size();
						}

						int vs = (int)lvs;
						vs = vs%this->_decayDomain.Size();
						scalar t = 0;
						scalar y = 0;
						for (int k = vl; k < vs; ++k)
						{
							t = t + this->_decayDomain(k);
							y = y + this->_decayRange(k);
						}
						t = t / ((scalar)vs - vl);
						y = y / ((scalar)vs - vl);
						flt(i, t);
						fly(i, y);
					}
				}
			});
			r._decayDomain = flt;
			r._decayRange = fly;
			return(r);
		}
		else
		{
			return(*this);
		}
	}

	ExponentialFitting ExponentialFitting::Sequential_Logarithmic_Reduction(int n)
	{
		if (n < this->_decayDomain.Size())
		{
			ExponentialFitting r;
			int M = this->_decayDomain.Size();
			scalar lM = (scalar)M;
			lM = log10(lM);
			scalar ds = lM / (scalar)(n - 1);
			math_la::math_lac::full::Vector flt(n);
			math_la::math_lac::full::Vector fly(n);
			for (int i = 0; i < n; ++i)
			{
				scalar si = (scalar)i;
				scalar lvs = si*ds;
				lvs = pow(10, lvs) - 1;
				if (lvs < si + 1)
				{
					flt(i, this->_decayDomain(i));
					fly(i, this->_decayRange(i));
				}
				else
				{
					int vl = i - 1;
					scalar sl = (scalar)(vl);
					scalar lls = sl*ds;
					lls = pow(10, lls) - 1;
					if (lls > sl + 1)
					{
						vl = (int)lls;
						vl = vl%this->_decayDomain.Size();
					}

					int vs = (int)lvs;
					vs = vs%this->_decayDomain.Size();
					scalar t = 0;
					scalar y = 0;
					for (int k = vl; k < vs; ++k)
					{
						t = t + this->_decayDomain(k);
						y = y + this->_decayRange(k);
					}
					t = t / ((scalar)vs - vl);
					y = y / ((scalar)vs - vl);
					flt(i, t);
					fly(i, y);
				}
			}
			r._decayDomain = flt;
			r._decayRange = fly;
			return(r);
		}
		else
		{
			return(*this);
		}
	}

	math_la::math_lac::full::Vector ExponentialFitting::Laplace_Filter(const math_la::math_lac::full::Vector& t2, 
		const math_la::math_lac::full::Vector& laplace, scalar tmin, scalar tmax, scalar& loss)
	{
		math_la::math_lac::full::Vector filtered(laplace.Size());
		math_la::math_lac::full::Vector smooth(laplace.Size());
		for (int i = 0; i < filtered.Size(); ++i)
		{
			if ((t2(i) > tmin) && (t2(i) < tmax))
			{
				filtered(i, laplace(i));
			}
		}
		loss = 0;
		for (int i = 4; i < (int)laplace.Size() - 4; ++i)
		{
			scalar v = 1 * filtered(i - 4) + 2 * filtered(i - 3) + 3 * filtered(i - 2) + 4 * filtered(i - 1) +
				+5 * filtered(i) + 4 * filtered(i + 1) + 3 * filtered(i + 2) + 2 * filtered(i + 3) + 1 * filtered(i + 4);
			v = v / (scalar)25;
			smooth(i, v);
			loss = loss + v;
		}
		for (int i = 0; i < 3; ++i)
		{
			smooth(i, laplace(i));
			smooth(laplace.Size() - 1 - i, laplace(laplace.Size() - i - 1));
		}
		smooth = ((scalar)1 / loss)*smooth;
		loss = 1 - loss;
		return(smooth);
	}

}