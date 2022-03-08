#ifndef CREATURE_H
#define CREATURE_H

#include "math_la/mdefs.h"
#include <vector>
#include <random>

namespace math_la
{
	namespace math_lac
	{

		namespace genetic
		{

			using std::vector;

			class Population;

			class Creature
			{
			public:
				struct OffSpring
				{
					genetic::Creature* A;
					genetic::Creature* B;
				};
			private:
				friend class Population;
				/**
				* Set of genes
				*/
				vector<int> _genes;

				/**
				* Creature fitness
				*/
				scalar _fitness;

				/**
				* Creature container
				*/
				Population* _population;

				/**
				* Creature island
				*/
				int _island;
			protected:
				/**
				* Recombines with pp, producing a new offspring, using uniform crossover
				*/
				Creature::OffSpring Uniform_Cross_Over(const Creature* pp) const;

				/**
				* Recombines with pp, producing a new offspring, using whole arithmetic crossover
				*/
				Creature::OffSpring War_Cross_Over(const Creature* pp, scalar w = 0.5) const;

				/**
				* Recombines with pp
				*/
				virtual Creature::OffSpring Cross(const Creature* pp) const;

				/**
				* Defines the vector of genes, according to the parent
				*/
				void Dimension_Genome(const Creature* parent);

				/**
				* Defines the vector of genes
				*/
				void Dimension_Genome(int size);

				/**
				* Sets creature fitness
				*/
				void Set_Fitness(scalar fitness);
			public:
				Creature(Population* pop, int size);
				virtual ~Creature();

				/**
				* Creates individual using the current as basis
				*/
				virtual Creature* Create_Individual() const;

				/**
				* Preparation before execution (not parallelized)
				*/
				virtual void Prepare() {};

				/**
				* Execution (parallelized)
				*/
				virtual void Execute() {};

				/**
				* Updates fitness (non parallelized). The value is returned
				* in the parameter
				* @param fitness Returned fitness of the creature
				*/
				virtual void Update_Fitness(scalar& fitness) {};

				/**
				* @return Fitness
				*/
				scalar Fitness() const;

				/**
				* @return Gene indexed by indx
				*/
				int Gene(int indx) const;

				/**
				* @return Parent population
				*/
				Population* Parent() const;

				/**
				* The distance between two creatures is an integer
				* @return The distance between the present creature and creature c. 
				*/
				int Distance(const Creature* c) const;

				/**
				* The similarity coefficient is a number that measures how similar two creatures are
				* @return The similarity between the present creature and creature c.
				*/
				scalar Similarity_Coefficient(const Creature* c) const;

				/**
				* Sets gene id with the value
				* @param id Gene id
				* @param value Gene value
				*/
				void Set_Gene(int id, int value);

				/**
				* @return The island that contains the creature
				*/
				int Island() const;
			};

			inline scalar Creature::Fitness() const
			{
				return(this->_fitness);
			};

			inline int Creature::Gene(int indx) const
			{
				return(this->_genes[indx]);
			}

			inline Population* Creature::Parent() const
			{
				return(this->_population);
			}

			inline int Creature::Island() const
			{
				return(this->_island);
			}

		}
	}

}

#endif