#include <fstream>
#include "experiment_report.h"
#include "sigmoid.h"
#include "rw/persistence/plug_persistent.h"

namespace rw
{
	ExperimentReport::ExperimentReport(const rw::RelaxivityExperiment* e, const string& file_prefix)
	{
		this->_experiment = e;
		this->_prefix = file_prefix;
	}

	void ExperimentReport::Create_Decay_Report()
	{
		std::ofstream tfile;
		tfile.open(string(this->_prefix)+string("_Function_Decay.csv"));
		const rw::RelaxivityDistribution* dst = &this->_experiment->Relaxivity_Distribution();
		const rw::Sigmoid* sigm = dynamic_cast<const rw::Sigmoid*>(dst);
		if (sigm)
		{
			tfile << "Function parameters \n";
			tfile << "Function ID,K,A,xi,B \n";
			for (int k = 0; k < sigm->Size(); ++k)
			{
				tfile << k << "," << sigm->K(k) << "," << sigm->A(k) << ",";
				tfile << sigm->XI(k) << "," << sigm->Slope(k) << "\n";
			}
		}
		const rw::Hat* hat = dynamic_cast<const rw::Hat*>(dst);
		if (hat)
		{
			tfile << "Function parameters \n";
			tfile << "Function ID,K,Xmin,Xmax \n";
			for (int k = 0; k < hat->Size(); ++k)
			{
				tfile << k << "," << hat->K(k) << "," << hat->X_Min(k) << ",";
				tfile << hat->X_Max(k) << "\n";
			}
		}
		tfile << "\n";
		tfile << "DECAY \n";
		tfile << "TIME, MAGNETIZATION \n";
		for (int k = 0; k < this->_experiment->Number_Of_Decay_Samples(); ++k)
		{
			rw::Step_Value vv = this->_experiment->Sample(k);
			tfile << vv.Time << "," << vv.Magnetization << "\n";
		}
		tfile.close();
	}

	void ExperimentReport::Create_Laplace_Report()
	{
		const rw::RelaxivityExperiment* e = this->_experiment;
		std::ofstream lfile;
		lfile.open(string(this->_prefix) + string("_Laplace.csv"));
		math_la::math_lac::full::Vector ltime = e->Laplace_Domain();
		math_la::math_lac::full::Vector lbin = e->Laplace_Bins();
		lfile << "Inverse laplace transform \n";
		lfile << "T2,BIN\n";
		for (int k = 0; k < ltime.Size(); ++k)
		{
			lfile << ltime(k) << "," << lbin(k) << "\n";
		}
		lfile.close();
	}

	void ExperimentReport::Create_Volumetric_Relaxivity_Distribution(scalar rho_min, scalar rho_max, scalar step, int tot_itrs, 
								const vec(Walker)& walker_vector, const rw::RelaxivityDistribution& eval, vector<scalar2>& distribution)
	{
		distribution.clear();
		scalar rho = rho_min;
		map<scalar, scalar> table;
		while (rho <= rho_max)
		{
			table.insert(std::pair<scalar, scalar>(rho, 0));
			rho = rho + step;
		}
		for (int k = 0; k < walker_vector.size(); ++k)
		{
			scalar xi = (scalar)walker_vector[k].Hits() / (scalar)tot_itrs;
			scalar rho = eval.Evaluate(xi);
			map<scalar, scalar>::iterator itr = table.lower_bound(rho);
			if ((itr != table.begin()) && (itr != table.end()))
			{
				--itr;
				if ((rho >= itr->first) && (rho < itr->first + step))
				{
					itr->second = itr->second + 1;
				}
			}
		}
		distribution.clear();
		map<scalar, scalar>::iterator itr = table.begin();
		scalar nn = (scalar)walker_vector.size();
		while (itr != table.end())
		{
			scalar rho = itr->first + step / 2;
			scalar2 dd;
			dd.x = rho;
			dd.y = itr->second / nn;
			distribution.push_back(dd);
			++itr;
		}
	}

	void ExperimentReport::Create_Relaxivity_Distribution_Report()
	{
		const rw::RelaxivityExperiment* e = this->_experiment;
		const rw::RelaxivityDistribution& dd = e->Relaxivity_Distribution();
		const rw::PlugPersistent& mapsim = dd.Mapping_Simulation();
		vector<scalar2> distribution;
		ExperimentReport::Create_Volumetric_Relaxivity_Distribution(0, 2000, 1, 
			mapsim.Total_Number_Of_Simulated_Iterations(), mapsim.Walker_Vector(), dd, distribution);
		std::ofstream lfile;
		lfile.open(string(this->_prefix) + string("_Volume_Relaxivity_Distribution.csv"));
		lfile << "Relaxivity_Value,Volumetric_Weight([0,1]\n";
		for (int k = 0; k < distribution.size(); ++k)
		{
			scalar2 v = distribution[k];
			lfile << v.x << "," << v.y << "\n";
		}
		lfile.close();
	}

	struct error_xi
	{
		scalar error;
		scalar xi_sim;
		scalar xi_base;
		bool operator <(const error_xi& ref) const
		{
			return(this->error < ref.error);
		}
	};

	void ExperimentReport::Create_Collision_Rate_Error_Report()
	{
		const rw::RelaxivityExperiment* e = this->_experiment;
		if (!e->Simulated())
		{
			const rw::RelaxivityDistribution& dd = e->Relaxivity_Distribution();
			const rw::PlugPersistent& mapsim = dd.Mapping_Simulation();
			const rw::Plug& currsim = e->Plug_Formation();
			uint n = currsim.Number_Of_Walking_Particles();
			const vec(Walker)& map_walkers = mapsim.Walker_Vector();
			uint n_curr = currsim.Total_Number_Of_Simulated_Iterations();
			uint n_base = mapsim.Total_Number_Of_Simulated_Iterations();
			std::list<error_xi> error_list;
			for (uint k = 0; k < n; ++k)
			{
				const rw::Walker& wsim = currsim.Walking_Particle(k);
				const rw::Walker& wbase = map_walkers[k];
				scalar xsim = (scalar)wsim.Hits() / (scalar)n_curr;
				scalar xbase = (scalar)wbase.Hits() / (scalar)n_base;
				scalar error = abs(xsim - xbase);
				error_xi v;
				v.xi_base = xbase;
				v.xi_sim = xsim;
				v.error = error;
				error_list.push_back(v);
			}
			error_list.sort();
			std::ofstream lfile;
			lfile.open(string(this->_prefix) + string("_Collision_Rate_Error.csv"));
			lfile << "Walker_Id(sorted),Collision_Rate\n";
			std::list<error_xi>::const_iterator itr = error_list.begin();
			uint id = 1;
			while (itr != error_list.end())
			{
				error_xi ex = *itr;
				lfile << id << "," << ex.xi_base << "," << ex.xi_sim << "," << ex.error << "\n";
				++id;
				++itr;
			}
			lfile.close();
		}
	}
}