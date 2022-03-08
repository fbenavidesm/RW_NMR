#include "creature.h"
#include "population.h"

namespace math_la
{
	namespace math_lac
	{

		namespace genetic
		{

			Creature::Creature(Population* pop, int size)
			{
				this->_genes.resize(size);
				this->_fitness = 0;
				this->_island = 0;
				this->_population = pop;
			};

			Creature::~Creature()
			{}

			Creature* Creature::Create_Individual() const
			{
				Creature* c = this->_population->Create_Individual(this);
				c->_population = this->_population;
				return(c);
			}

			void Creature::Dimension_Genome(const Creature* parent)
			{
				this->_genes.resize(parent->_genes.size());
				this->_fitness = 0;
			}

			void Creature::Dimension_Genome(int size)
			{
				this->_fitness = 0;
				this->_genes.resize(size);
			}


			void Creature::Set_Fitness(scalar fitness)
			{
				this->_fitness = fitness;
			}

			Creature::OffSpring Creature::Uniform_Cross_Over(const Creature* pp) const
			{
				OffSpring osp;
				osp.A = this->Create_Individual();
				osp.B = pp->Create_Individual();
				osp.A->Dimension_Genome(this);
				osp.B->Dimension_Genome(pp);
				Creature* A = 0;
				Creature* B = 0;
				int n = (int)this->_genes.size();
				for (int i = 0; i < n; ++i)
				{
					if (A == osp.B)
					{
						A = osp.A;
						B = osp.B;
					}
					else
					{
						A = osp.B;
						B = osp.A;
					}
					A->_genes[i] = this->_genes[i];
					B->_genes[i] = pp->_genes[i];
				}
				osp.A->_island = this->_island;
				osp.B->_island = pp->_island;
				return (osp);
			}


			Creature::OffSpring Creature::War_Cross_Over(const Creature* pp, scalar w) const
			{
				OffSpring osp;
				osp.A = this->Create_Individual();
				osp.B = pp->Create_Individual();
				osp.A->Dimension_Genome(this);
				osp.B->Dimension_Genome(pp);
				int n = (int)this->_genes.size();
				for (int i = 0; i < n; ++i)
				{
					scalar v1 = (scalar)this->_genes[i];
					scalar v2 = (scalar)pp->_genes[i];
					scalar va = w * v1 + (1 - w) * v2;
					scalar vb = (1 - w) * v1 + w * v2;
					int vai = (int)va;
					int vbi = (int)vb;

					osp.A->_genes[i] = vai;
					osp.B->_genes[i] = vbi;
				}
				osp.A->_island = this->_island;
				osp.B->_island = pp->_island;
				return(osp);
			}

			Creature::OffSpring Creature::Cross(const Creature* pp) const
			{
				OffSpring osp;
				osp.A = 0;
				osp.B = 0;
				return(osp);
			}

			int Creature::Distance(const Creature* c) const
			{
				int r = 0;
				for (int k = 0; k < this->_genes.size(); ++k)
				{
					r = r + abs(c->_genes[k] - this->_genes[k]);
				}
				return(r);
			}

			void Creature::Set_Gene(int id, int Decay_Step_Value)
			{
				this->_genes[id] = Decay_Step_Value;
			}

			scalar Creature::Similarity_Coefficient(const Creature* c) const
			{
				scalar num = 0;
				scalar den1 = 0;
				scalar den2 = 0;
				for (int k = 0; k < this->_genes.size(); ++k)
				{
					num = num + (scalar)abs(c->_genes[k] - this->_genes[k]);
					den1 = den1 + (scalar)c->_genes[k];
					den2 = den2 + (scalar)this->_genes[k];
				}
				scalar den = std::min(den1, den2);
				if (den == 0)
				{
					den = 1;
				}
				return(num / den);
			}

		}
	}
}