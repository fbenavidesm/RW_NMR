#ifndef EXPERIMENT_REPORT
#define EXPERIMENT_REPORT

#include <string>
#include "relaxivity_experiment.h"

namespace rw
{
	class ExperimentReport
	{
	private:
		const rw::RelaxivityExperiment* _experiment;
		std::string _prefix;
	public:
		ExperimentReport(const rw::RelaxivityExperiment* e, const string& file_prefix);
		void Create_Decay_Report();
		void Create_Laplace_Report();
		void Create_Relaxivity_Distribution_Report();		

		/**
		* Recovers a volumetric distribution of rho values
		* @param rho_min Minimal rho value
		* @param rho_max Maximal rho value
		* @param step Step size between categories
		* @param tot_itrs Total number of iterations
		* @param walker_vector Vector of walkers
		* @param eval Surface relaxivity evaluator (Relaxivity distribution)
		* @param distribution Returned distribution
		*/
		static void Create_Volumetric_Relaxivity_Distribution(scalar rho_min, scalar rho_max, scalar step, int tot_itrs,
			const vec(Walker)& walker_vector, const rw::RelaxivityDistribution& eval, vector<scalar2>& distribution);

		void Create_Collision_Rate_Error_Report();

		~ExperimentReport() {};
	};
}

#endif
