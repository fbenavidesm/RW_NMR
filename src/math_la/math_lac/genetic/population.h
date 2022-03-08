#ifndef POPULATION_H
#define POPULATION_H

#include <random>
#include <set>
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <list>
#include "math_la/mdefs.h"
#include "creature.h"
#include "tbb/blocked_range.h"
#include "tbb/spin_mutex.h"

namespace math_la
{
	namespace math_lac
	{

		namespace genetic
		{

			class Creature;

			using std::queue;
			using std::set;
			using std::list;
			using tbb::blocked_range;
			using std::vector;
			using std::map;
			using std::string;

			/**
			* A population is a set of creatures whose genome is optimized according to a fittness function in a genetic
			* algorithm procedure. This class handles the optimization, dividing the corresponding simulation process
			* of each crature in parallel threads according to a fixed size. The class is configurable through inheritance
			* so the type of the creature can be defined and the fitness function.
			* All creatures must implement a simulation to update its fitness and it is supposed these simulations are
			* computationally costly.
			*/
			class Population
			{
				friend class Creature;
			public:
				/**
				* This class encloses the events called during the genetic optimization process.
				* As its name suggests, this class observes all events that occur during a genetic algorithm
				* optimization.
				*/
				class Observer
				{
				private:
					Population* _parentFormation;
				protected:
					friend class Population;
					/**
					* Triggered when the creature simulations are being executed
					*/
					virtual void Executing_Simulations(int curent, int totsize) = 0;

					/**
					* Triggered when an island is being diversified
					* @param i Index of the island that is being diversified
					*/
					virtual void Diversifying_Island(int i) = 0;

					/**
					* Triggered when creatures recombine and create a new offspring
					*/
					virtual void Create_Offspring(int it) = 0;

					/**
					* Triggered when a winner is selected
					*/
					virtual void Update_Winners(const Creature& winner) = 0;

					/**
					* Triggered when a creature fitness is being updated
					*/
					virtual void Fitting_Creature(const Creature& creature, scalar fitness) = 0;

					/**
					* Triggered when a set of survivors are selected
					*/
					virtual void Conclude(list<const Creature*> survivors) = 0;
				public:
					Observer()
					{
						this->_parentFormation = 0;
					}
					const Population* parent() const
					{
						return(this->_parentFormation);
					}
				};
				enum CrossOver
				{
					Uniform,
					WholeArithmetic,
					Whole_Uniform
				};

			private:
				/**
				* This class implements the comparison criteria of two creatures
				*/
				class Comparer
				{
				private:
					friend class Population;
					/**
					* Parent formation of the creatures
					*/
					Population* _parentFormation;
				public:
					bool operator()(const Creature* c1, const Creature* c2) const;
				};
				/**
				* A mutex to synchronize genetic algorithm optimization parameters
				*/
				std::mutex _mutexThread;
				friend class Comparer;

				/**
				* The object handles the events occured during simulation. If the pointer is
				* null, events are not triggered.
				*/
				Population::Observer* _eventHandler;

				/**
				* Size of the chunk of creatures that are simulated at the same time during
				* optimization in separate threads
				*/
				int _chunkSize;

				/**
				* The random number generator necessary to create genetic algorithm diversity
				*/
				std::mt19937 _randomNumberGenerator;

				/**
				* An instance of creature comparer used to sort creatures and select survivors
				*/
				Comparer _creatureComparer;

				/**
				* Set of creatures that produce offspring and are being optimized
				*/
				set<Creature*, Comparer> _creatureCollection;

				/**
				* The set of islands subdivide the creature population, avoiding them to recombine
				* outside the island. A periodic migration occurs to increase island diversity.
				*/
				vector<set <Creature*, Comparer>> _islands;

				/**
				* The fixed population size of each island
				*/
				vector<int> _islandSize;

				/**
				* TRUE if fitness is being maximized
				*/
				bool _maxFitness;

				/**
				* Size of the entire population
				*/
				int _populationSize;

				/**
				* Number of genes of each creature
				*/
				int _phenotypeSize;

				/**
				* Border limites for each gene
				*/
				vector<s_int2> _genLimits;

				/**
				* Number of digits of precision of each gene. The crossover is performed using integer
				* arithmetic
				*/
				vector<int> _genPrecision;

				/**
				* A flag to stop the simulation
				*/
				bool _stopOptimization;

				/**
				* Renews entire population
				*/
				bool _renewPopulation;

				/**
				* Crossover technique
				*/
				Population::CrossOver _crossOverMethod;

				/**
				* The crossover coefficient is chosen randomly. This is the minimal value
				*/
				scalar _crossOverCoeffMin;

				/**
				* The crossover coefficient is chosen randomly. This is the maximal value
				*/
				scalar _crossOverCoeffMax;

				/**
				* Mutation probability
				*/
				scalar _mutationProbability;

				/**
				* Largest displacement mutation
				*/
				scalar _displacementMutation;

				/**
				* Probability of uniform crossover recombination (when enabled)
				*/
				scalar _probUniformCrossOver;

				/**
				* Migration percentage between islands
				*/
				scalar _migrationPercentage;

				/**
				* TRUE when initial creatures have been created
				*/
				bool _populated;

				/**
				* Similarity coefficient between two creatures to be consideres incestual
				*/
				scalar _incestDistance;

				/**
				* The percentage of preservation when an island is extincted (diversified)
				*/
				scalar _islandPreservation;

				/**
				* Minimal fitness to determine if an island must be renewed (it became incestual)
				*/
				scalar _minFitnessRenew;

				/**
				* Percentage of fitness that define that an island must be renewed.  When the percentage
				* of number of creatures whose fitness is larger than _minFitnessRenew is larger than
				* _percentageFitnessRenew, then the islan is renewed.
				*/
				scalar _percentageFitnessRenew;

				/**
				* Number of generations between each island migration
				*/
				int _migrationRate;

				/**
				* Number of creatures that migrate between islands
				*/
				int _migrationRatio;

				/**
				* Number of generations
				*/
				int _generations;
				tbb::spin_mutex _eventLocker;

				/**
				* Map of fixed genes for every creature, with their corresponding integer values
				*/
				map<int, int> _fixedMap;

				/**
				* Set of islands that are being diversified
				*/
				set<int> _diversifiedIslands;
			protected:
				/**
				* @return A random number between 0 and 1
				*/
				scalar Pick_Random_Normalized_Number();

				/**
				* This method is called whenever an individual creature is created
				*/
				virtual Creature* Create_Individual(const Creature* parent) const = 0;

				/**
				* Populates creature genes with random values
				*/
				void Populate_Genes(Creature* c);

				/**
				* Creates a new set of creatures to complete island's population. The new set of creatures are
				* returned in the queue.
				* @return Newly created individuals
				*/
				queue<Creature*>* Fill_Islands();

				/**
				* Renews the entire population. The newly created individuals are returned in a queue
				* @return The set of renewed creatures
				*/
				queue<Creature*>* Renew();

				/**
				* Recombines all creatures (restricted to their islands) returning the newly created
				* individuals in the quere.
				* @return Children of the current population
				*/
				queue<Creature*>* Create_Offspring();

				/**
				* Sends the command to propagate to all population creatures
				*/
				bool Propagate_Offspring(queue<Creature*>* e, set <Creature*, Population::Comparer>& island);

				/**
				* Adds offspring to the corresponding islands, preserving their size destroying the poorly adapted
				* creatures
				*/
				void Add_Offspring(queue<Creature*>* offspring);

				/**
				* Updates global set of creatures, sorting them according to their fitness
				*/
				void Sort_Creature_List();

				/**
				* Executes a migration between islands
				*/
				void Migrate();

				/**
				* Checks island uniformity, returning a set of displaced creatures when this uniformity is too high
				*/
				bool Check_Island_Uniformity(queue<Creature*>* displaced, set <Creature*, Population::Comparer>& island);

				/**
				* Displaces a gene of the creature c
				* @param c Creature
				* @param i Gene index to displace
				*/
				void Displace_Gene(Creature* c, int i);

				/**
				* Mutates the creature
				* @param c Creature to mutate
				*/
				void Mutate(Creature* c);

				/**
				* This method is called just after a creature is created.
				* @param creature Creature whose properties are to be configured
				*/
				virtual void Shape_Creature(Creature* creature) {};

				/**
				* Tells if the entire population is too fit so a massive extinction is
				* necessary
				*/
				bool Population_Fit();

				/**
				* This method checks the creature previously fixed genes.
				*/
				virtual void Check_Creature_Fixed_Genes(Creature* c);

				/**
				* Starts optimization process
				*/
				void Propagate(const Population::CrossOver& mth, bool migrate, bool renew = false);

				/**
				* Sets the number of genes of all creatures
				* @param size Number of genes
				*/
				void Set_Phenotype_Size(int size);

				/**
				* Defines gene limits
				*/
				void Set_Gene_Limits(int index, int min, int max);
				/**
				* Estimates the gen precision necessary to represent given values. The precision of
				* the gene indexed by indx is updated
				* @param indx Gene index whose precision is estimated
				* @param valmin Minimal scalar of the gene interval
				* @param valmax Maximal scalar of the gene interval
				*/
				void Estimate_Gen_Precision(int indx, scalar valmin, scalar valmax = 0);

				/**
				* Translates the scalar value v to a gene code according to the
				* parameters of index idx
				*/
				int Translate_To_Int(int idx, scalar v) const;

				/**
				* Executes the optimization in a background thread
				*/
				void Exec();

				/**
				* Complements cratures creating a new one
				* @param ref Creature
				* @param c Complemented creature (a new one)
				*/
				void Complement(const Creature* ref, Creature* c);

			public:
				Population(int size, int size_islands = 3);
				~Population();

				/**
				* Sets the precision of the gene indx
				* @param indx Index of the gene to define
				* @param precision Number of precision digits
				*/
				void Set_Precision(int indx, int precision);


				/**
				* Gets the number of precision digits of the value
				*/
				static int getPrecision(scalar value);

				/**
				* Starts optimization
				*/
				void Start();

				/**
				* Sets the crossover rule between creatures
				*/
				void Set_Crossover_Rule(const Population::CrossOver& mth);

				/**
				* Stops simulation
				*/
				void Stop();

				/**
				* Fixes gene idexed by id with the code
				* @param id Gene index
				* @param code Code to fix
				*/
				void Fix(int id, int code);

				/**
				* @return TRUE if the gene id is fixed
				*/
				bool Fixed(int id) const;

				/**
				* @return The current population size. It can be different from the fixed population size set by
				* the user
				*/
				int Size() const;

				/**
				* Sets event handler
				*/
				void Set_Event_Handler(Population::Observer* evts);

				/**
				* Recovers the creature list
				*/
				void Pick_Creature_List(list<Creature*>& lst, scalar cut);

				/**
				* @return Phenotype size
				*/
				int Phenotype_Size() const;

				/**
				* @return Gene precision of the gene indexed by idx
				* @param idx Index of the gene
				*/
				int Gene_Precision(int idx) const;

				/**
				* Defines if the fitness is to be maximized or minimized
				*/
				void Set_Max_Fitness(bool b = true);

				/**
				* Locks mutex (for multithread)
				*/
				void Lock_Event_Trigger();

				/**
				* Unlocks mutex (for multithread)
				*/
				void UnLock_Event_Trigger();

				/**
				* @return Number of iterations between each migration
				*/
				int Migration_Rate() const;

				/**
				* Sets the number of iterations between each migration.
				* @param mr Number of iterations
				*/
				void Set_Migration_Rate(int mr);

				/**
				* @return The percentage of individuals that migrate in each migration
				*/
				scalar Migration_Percentage() const;

				/**
				* Sets hhe percentage of individuals that migrate in each migration
				* @param mp A number between 0 and 1 that defines the percentage
				*/
				void Set_Migration_Percentage(scalar mp);

				/**
				* @return The percentage of individuals that survive after and incestual condition
				*/
				scalar Incestual_Preservation() const;

				/**
				* Sets the percentage of individuals that survive after and incestual condition
				* @param ip The percentage, a number between 0 and 1. 
				*/
				void Set_Incestual_Preservation(scalar ip);

				/**
				* @return The relative error necessary to define an incestual condition
				*/
				scalar Incestual_Relative_Error() const;

				/**
				* When all individuals of an island are similar, an incestual event happens. All individuals are deleted
				* except for a small percentage. That percentage is defined here
				* @param ire Percentage, between 0 and 1
				*/
				void Set_Incestual_Relative_Error(scalar ire);

				/**
				* @return The crossover minimal weight. Generally 0
				*/
				scalar Crossover_Weight_Min() const;

				/**
				* @return The crossover maximal weight. Generally 1
				*/
				scalar Crossover_Weight_Max() const;

				/**
				* Sets the weights for the creatures crossover, a minimal and maximal value
				* @param min Minimal value, generally 0
				* @param max Maximal value, generally 0.5
				*/
				void Set_Weight_Interval(scalar min, scalar max);

				/**
				* @return Crossover rule between individuals
				*/
				Population::CrossOver Crossover_Rule() const;

				/**
				* @return The current population size. This can be larger than the fixed population size, depending on how it
				* is being handled
				*/
				int Population_Size() const;

				/**
				* Sets the population size (number of individuals)
				*/
				void Set_Population_Size(int size);

				/**
				* @return The number of islands in which the individuals are separated
				*/
				int Number_Of_Islands() const;

				/**
				* Sets the number of islands in which the individuals are separated
				*/
				void Set_Number_Of_Islands(int size);

				/**
				* When an island is renewed (after an incest or an apocalyptic event), this is the percentage of preservation of current individuals 
				* @return The percentage of individuals that are preserved in each island
				*/
				scalar Island_Uniform_Percentage() const;

				/**
				* Sets the percentage of preservation of current individuals after an apocalyptic or incestual event
				* @param perc The percentage of individuals that are preserved in each island
				*/	
				void Set_Island_Uniform_Percentage(scalar perc);

				/**
				* @return The mutation probability in each crossover (between 0 and 1)
				*/
				scalar Mutation_Probability() const;

				/**
				* Sets yhe mutation probability in each crossover (between 0 and 1)
				* @param prob Probability distribution
				*/
				void Set_Mutation_Probability(scalar prob);

				/**
				* @return The factor of displacement that affects a children when it suffers a mutation
				*/
				scalar Mutation_Displacement_Factor() const;

				/**
				* Sets the factor of displacement that affects a children when it suffers a mutation
				* @param md A number between 0 and 1 that determines how the value of a certain gene is modified
				*/
				void Set_Mutation_Displacement_Factor(scalar md);

				/**
				* Whan all creatures are fitted then alternative solutions can be found if almost all of them are replaced
				* by less fitted creatures. 
				* @param fitness The value of fitness of all creatures in the island to be destroyed in an apocalypsis
				*/
				void Set_Apocalyptic_Fitness(scalar fitness);

				/**
				* @return Apocalyptic fitness. 
				*/
				scalar Apocalyptic_Fitness() const;

				/**
				* Sets the percentage of idnviduals that survive an apocalyptic event
				* @param per Percenteage of survivers
				*/
				void Set_Apocalyptic_Percentage(scalar per);

				/**
				* return The percentage of individuals that survive apocalypsis
				*/
				scalar Apocalyptic_Percentage() const;

				/**
				* Sets the maximal numbers of iterations of the genetic optimization
				* @param gen Number of generations
				*/
				void Set_Maximal_Number_Of_Generations(int gen);

				/**
				*@return The total number of iterations that will be executed during optimization
				*/
				int Maximal_Number_Of_Generations() const;

				/**
				* Sets the percentage of individuals that will make crossover using a non-arithmeric rule, but a permutation of
				* their coefficients.
				* @param nar Percentage of individuals between 0 and 1
				*/
				void Set_Non_Arithmetic_Rate(scalar nar);

				/**
				* @return The percentage of individuals that will make crossover using a non-arithmeric rule, but a permutation of
				* their coefficients.
				*/

				scalar Non_Arithmetic_Rate() const;


				/**
				* Locks thread performing optimization (for example, to display results on screen)
				*/
				void Lock_Working_Thread();

				/**
				* Unlocks thread performing optimization (for example, to display results on screen)
				*/
				void Unlock_Working_Thread();

				/**
				* Sends a message to renew population in the next iteration. All but a percentage of individuals are destroyed and are replaced
				* by random ones. The percentage of survivors is specified in the method Set_Apocalyptic_Percentage. 
				*/
				void Regenerate();

				/**
				* Translates the code v to the scalar value using the gene configuration
				* of gene idx
				*/
				scalar Translate_To_Scalar(int idx, int v) const;

				/**
				* Size of the simulations that are performed in parallel. 
				*/
				void Set_Chunk_Size(uint size);
			};

			inline int Population::Phenotype_Size() const
			{
				return(this->_phenotypeSize);
			}

			inline int Population::Gene_Precision(int idx) const
			{
				return(this->_genPrecision[idx]);
			}

			inline int Population::Size() const
			{
				return((int)this->_creatureCollection.size());
			}

			inline void Population::Set_Chunk_Size(uint size)
			{
				this->_chunkSize = size;
			}
		}
	}
}

#endif