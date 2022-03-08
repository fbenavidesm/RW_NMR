#include <fstream>
#include <ostream>
#include <stdio.h>
#include <time.h>
#include <list>
#include <thread>
#include <string>
#include <mutex>
#include "tbb/parallel_for.h"
#include "population.h"
#include "rw/relaxivity_experiment.h"

#include "math_la/txt/parameters.h"
#include "math_la/txt/converter.h"


namespace math_la
{
	namespace math_lac
	{
		namespace genetic
		{

			using std::vector;
			using std::list;

			bool Population::Comparer::operator()(const Creature* c1, const Creature* c2) const
			{
				if (this->_parentFormation->_maxFitness)
				{
					return (c1->_fitness > c2->_fitness);
				}
				else
				{
					return (c1->_fitness < c2->_fitness);
				}
			};

			void Population::Regenerate()
			{
				this->_renewPopulation = true;
			}

			Population::Population(int size, int size_islands)
			{
				this->_eventHandler = 0;
				this->_renewPopulation = false;
				this->_generations = 64;
				this->_creatureComparer._parentFormation = this;
				this->_maxFitness = true;
				std::ranlux48 gen(time(0));
				int length = 2 * 112 + 16;
				vector<unsigned long long> seed_data(length);
				std::generate(seed_data.begin(), seed_data.end(),gen);
				std::seed_seq q(seed_data.begin(), seed_data.end());
				this->_randomNumberGenerator.seed(q);

				this->Set_Number_Of_Islands(size_islands);
				set<Creature*, Population::Comparer> isl(this->_creatureComparer);
				this->_creatureCollection = isl;
				this->_islandSize.resize(size_islands);
				this->_populationSize = size;
				scalar ns = (scalar)size;

				txt::Params prms;
				prms.Load_From_File("files\\genetic.conf");	
				this->_chunkSize = 12;
				if (prms.Exists("chunk"))
				{
					this->_chunkSize = txt::Converter::Convert_To_Int(prms["chunk"]);
				}
				this->_crossOverMethod = Population::CrossOver::WholeArithmetic;
				this->_crossOverCoeffMin = 0.0;
				this->_crossOverCoeffMax = 0.5;
				this->_mutationProbability = 0.1;
				this->_displacementMutation = 1;
				this->_populated = false;
				this->_probUniformCrossOver = 0.2;
				this->_migrationPercentage = 0.25;
				this->_incestDistance = 0.2;
				this->_migrationRatio = 1;
				this->_islandPreservation = 0.25;
				this->_minFitnessRenew = 0.99;
				this->_percentageFitnessRenew = 0.75;
				this->_migrationRate = 8;
			};

			Population::~Population()
			{
				if (this->_islands.size() > 0)
				{
					for (int i = 0; i < this->_islands.size(); ++i)
					{
						set<Creature*, Comparer>& island = this->_islands[i];
						set<Creature*, Comparer>::iterator ii = island.begin();
						while (ii != island.end())
						{
							Creature* c = *ii;
							delete c;
							++ii;
						}
					}
				}
			}


			void Population::Set_Phenotype_Size(int size)
			{
				this->_phenotypeSize = size;
				this->_genLimits.resize(size);
				this->_genPrecision.resize(size);
			}

			scalar Population::Translate_To_Scalar(int idx, int v) const
			{
				scalar vs = (scalar)v;
				scalar p = (scalar)this->_genPrecision[idx];
				return(vs / p);
			}

			int Population::Translate_To_Int(int idx, scalar v) const
			{
				scalar vt = v * this->_genPrecision[idx];
				int vi = (int)vt;
				return(vi);
			}

			void Population::Set_Max_Fitness(bool b)
			{
				this->_maxFitness = b;
			}

			void Population::Set_Gene_Limits(int indx, int flag, int size)
			{
				indx = indx%this->_phenotypeSize;
				this->_genLimits[indx].sx = std::min(flag, size);
				this->_genLimits[indx].s = std::max(flag, size);
			}

			void Population::Estimate_Gen_Precision(int idx, scalar valmin, scalar valmax)
			{
				int p1 = Population::getPrecision(valmin);
				int p2 = Population::getPrecision(valmax);
				this->_genPrecision[idx] = std::max(p1, p2);

			}

			void Population::Set_Precision(int idx, int precision)
			{
				this->_genPrecision[idx] = precision;
			}

			int Population::getPrecision(scalar Decay_Step_Value)
			{
				int prec = 1;
				int vt = (int)Decay_Step_Value;
				while ((Decay_Step_Value - (scalar)vt > EPSILON) && (prec < 10000000))
				{
					prec = prec * 10;
					Decay_Step_Value = Decay_Step_Value * 10;
					vt = (int)Decay_Step_Value;
				}
				return(prec);
			}


			void Population::Populate_Genes(Creature* c)
			{
				for (int i = 0; i < this->_phenotypeSize; ++i)
				{
					int min = this->_genLimits[i].sx;
					int max = this->_genLimits[i].s;
					scalar v = (scalar)min;
					scalar r = this->Pick_Random_Normalized_Number();
					v = v + r*((scalar)(max - min));
					int code = (int)v;
					c->_genes[i] = code;		
				}
			}


			scalar Population::Pick_Random_Normalized_Number()
			{
				scalar n = (scalar)(this->_randomNumberGenerator() - this->_randomNumberGenerator.min());
				scalar nrm = (scalar)this->_randomNumberGenerator.max();
				return(n/nrm);
			}

			queue<Creature*>* Population::Renew()
			{
				queue<Creature*>* eval = new queue<Creature*>();
				vector<int> newSizes;
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					newSizes.push_back(0);
				}
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					int n = (int)(((scalar)this->_islandSize[i])*this->_islandPreservation);
					while ((int)this->_islands[i].size() > n)
					{
						Creature* winner = *this->_islands[i].begin();
						Creature* clone = this->Create_Individual(0);
						int ni = (int)(((scalar)this->_islands.size())*this->Pick_Random_Normalized_Number());
						clone->_island = ni;
						newSizes[ni] = newSizes[ni] + 1;
						for (int ff = 0; ff < this->_phenotypeSize; ++ff)
						{
							clone->_genes[ff] = winner->_genes[ff];
						}		
						eval->push(clone);
						this->_islands[i].erase(winner);
						delete winner;
					}
					set<Creature*, Population::Comparer>::iterator ii = this->_islands[i].begin();
					while (ii != this->_islands[i].end())
					{
						Creature* cd = *ii;
						delete cd; 
						++ii;
					}
					this->_islands[i].clear();
				}
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					int n = this->_islandSize[i];
					int isize = newSizes[i];
					while (isize < n)
					{
						Creature* c = this->Create_Individual(0);
						this->Populate_Genes(c);
						this->Shape_Creature(c);
						this->Check_Creature_Fixed_Genes(c);
						c->_island = i;
						eval->push(c);
						isize = isize + 1;
					}
				}
				this->_diversifiedIslands.clear();
				return(eval);
			}


			queue<Creature*>* Population::Fill_Islands()
			{
				int ns = 0;
				queue<Creature*>* eval = new queue<Creature*>();
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					this->_islandSize[i] = 0;
					for (int Voxel_Length = 0; Voxel_Length < this->_populationSize / (int)this->_islands.size(); ++Voxel_Length)
					{
						Creature* c = this->Create_Individual(0);
						this->Populate_Genes(c);
						this->Shape_Creature(c);
						this->Check_Creature_Fixed_Genes(c);
						c->_island = i;
						++ns;
						eval->push(c);			
						this->_islandSize[i] = this->_islandSize[i] + 1;
					}
				}
				if (ns < this->_populationSize)
				{
					while (ns < this->_populationSize)
					{
						Creature* c = this->Create_Individual(0);			
						this->Populate_Genes(c);
						this->Shape_Creature(c);
						this->Check_Creature_Fixed_Genes(c);
						c->_island = 0;
						++ns;
						eval->push(c);
						this->_islandSize[0] = this->_islandSize[0] + 1;
					}
				}
				this->_populated = true;
				return(eval);
			}

			void Population::Complement(const Creature* ref, Creature* c)
			{
				for (int i = 0; i < this->_phenotypeSize; ++i)
				{
					c->_genes[i] = this->_genLimits[i].s - ref->_genes[i];
					if (c->_genes[i] < this->_genLimits[i].sx)
					{
						c->_genes[i] = this->_genLimits[i].sx;
					}
					this->Shape_Creature(c);
					this->Check_Creature_Fixed_Genes(c);
				}
			}

			void Population::Displace_Gene(Creature* c, int i)
			{
				i = i%this->_phenotypeSize;
				scalar d = ((scalar)2)*this->_displacementMutation;
				int code = c->_genes[i];
				scalar pd = this->Pick_Random_Normalized_Number();
				scalar dk = pd - (scalar)0.5;
				dk = dk*d;
				code = code + (int)(dk*(scalar)code);
				code = std::max(this->_genLimits[i].sx, code);
				code = std::min(this->_genLimits[i].s, code);
				c->_genes[i] = code;
				c->_fitness = 0;
				this->Shape_Creature(c);
				this->Check_Creature_Fixed_Genes(c);
			}

			void Population::Mutate(Creature* c)
			{
				int attempts = 0;
				scalar cg = this->Pick_Random_Normalized_Number();
				int g = (int)(cg*(scalar)this->_phenotypeSize);
				while ((this->Fixed(g))&&(attempts < this->Phenotype_Size()))
				{
					cg = this->Pick_Random_Normalized_Number();
					g = (int)(cg*(scalar)this->_phenotypeSize);
					++attempts;
				}
				if (attempts < this->Phenotype_Size())
				{
					scalar d = ((scalar)2)*this->_displacementMutation;
					int Sim_Code = c->_genes[g];
					scalar pd = this->Pick_Random_Normalized_Number();
					scalar dk = pd - (scalar)0.5;
					dk = dk*d;
					Sim_Code = std::max(this->_genLimits[g].s - (int)(dk*(scalar)Sim_Code),this->_genLimits[g].sx);
					Sim_Code = std::min(this->_genLimits[g].s, Sim_Code);
					c->_genes[g] = Sim_Code;
					this->Shape_Creature(c);
					this->Check_Creature_Fixed_Genes(c);
				}
			}

			void Population::Fix(int i, int Decay_Step_Value)
			{
				std::pair<int, int> pp;
				pp.first = i;
				pp.second = Decay_Step_Value;
				this->_fixedMap.insert(pp);
			}

			bool Population::Fixed(int id) const
			{
				bool r = false;
				map<int, int>::const_iterator ii = this->_fixedMap.find(id);
				if (ii != this->_fixedMap.end())
				{
					r = true;
				}
				return(r);
			}

			void Population::Check_Creature_Fixed_Genes(Creature* c)
			{
				for (int i = 0; i < this->Phenotype_Size(); ++i)
				{
					map<int, int>::const_iterator ii = this->_fixedMap.find(i);
					if (ii != this->_fixedMap.end())
					{
						c->_genes[i] = ii->second;
					}
					if (c->_genes[i] < this->_genLimits[i].sx)
					{
						c->_genes[i] = this->_genLimits[i].sx;
					}
					if (c->_genes[i] > this->_genLimits[i].s)
					{
						c->_genes[i] = this->_genLimits[i].s;
					}
				}
			}

			bool Population::Check_Island_Uniformity(queue<Creature*>* e, set <Creature*, Population::Comparer>& island)
			{
				bool r = false;
				if (island.size() > 1)
				{
					Creature* cf = *island.rbegin();
					Creature* cb = *island.begin();
					int id = cb->_island;
					scalar d = cf->Similarity_Coefficient(cb);
					if (d < this->_incestDistance)
					{
						r = true;
						int n = (int)(((scalar)this->_islandSize[id])*this->_islandPreservation);
						int i = 0;
						while ((int)island.size() > n)
						{
							Creature* td = *island.rbegin();
							Creature* clone = this->Create_Individual(0);
							for (int ff = 0; ff < this->_phenotypeSize; ++ff)
							{
								clone->_genes[ff] = td->_genes[ff];
							}
							clone->_island = id;
							island.erase(td);
							while (this->Fixed(i))
							{
								i = i + 1;
								i = i%this->Phenotype_Size();
							}
							this->Displace_Gene(clone, i);
							this->Shape_Creature(clone);
							this->Check_Creature_Fixed_Genes(clone);
							e->push(clone);
							i = i + 1;
							delete td;
						}
						Creature* c = this->Create_Individual(0);
						c->_island = id;
						this->Complement(cb, c);
						this->Shape_Creature(c);
						this->Check_Creature_Fixed_Genes(c);
						e->push(c);

						c = this->Create_Individual(0);
						Creature* best = *island.begin();
						c->_island = id;
						for (int ii = 0; ii < this->_phenotypeSize; ++ii)
						{
							c->_genes[ii] = best->_genes[ii];
							if (this->_fixedMap.find(ii) == this->_fixedMap.end())
							{
								this->Displace_Gene(c, ii);
							}
						}
						this->Shape_Creature(c);
						this->Check_Creature_Fixed_Genes(c);
						e->push(c);
					}
				}
				return(r);
			}

			bool Population::Propagate_Offspring(queue<Creature*>* e, set <Creature*, Population::Comparer>& island)
			{
				bool diverse = !this->Check_Island_Uniformity(e, island);	
				if (diverse)
				{
					list<Creature*> parents;
					set<Creature*, Population::Comparer>::const_iterator itr = island.begin();
					while (itr != island.end())
					{
						Creature* c = *itr;
						parents.push_back(c);
						++itr;
					}
					Creature* c1 = 0;
					Creature* c2 = 0;
					scalar n = (scalar)parents.size();
					Population::CrossOver mth = this->_crossOverMethod;
					while (!parents.empty())
					{
						scalar pst = 0;
						list<Creature*>::iterator fi = parents.begin();
						c1 = *fi;
						c2 = 0;
						parents.pop_front();
						n = n - (scalar)1;
						if (!parents.empty())
						{
							fi = parents.begin();
							scalar pb = this->_incestDistance;
							scalar step = this->_incestDistance / (scalar)n;
							while (!c2)
							{
								pst = pst + ((scalar)1) / n;
								scalar pch = this->Pick_Random_Normalized_Number();
								if (pch < pst)
								{
									scalar incdec = this->Pick_Random_Normalized_Number();
									if ((incdec < this->_incestDistance) || (c1->Similarity_Coefficient(*fi) > pb))
									{
										c2 = *fi;
										parents.erase(fi);
										n = n - (scalar)1;
									}
									pb = pb - step;
								}
								else
								{
									list<Creature*>::iterator aux = fi;
									++fi;
									if (fi == parents.end())
									{
										c2 = *aux;
										parents.erase(aux);
									}
								}
							}
							Creature::OffSpring offsp;
							if (mth == Population::CrossOver::Uniform)
							{
								offsp = c1->Uniform_Cross_Over(c2);
							}
							else if (mth == Population::CrossOver::WholeArithmetic)
							{
								scalar w = this->_crossOverCoeffMin + (this->_crossOverCoeffMax - this->_crossOverCoeffMin)*this->Pick_Random_Normalized_Number();
								offsp = c1->War_Cross_Over(c2, w);
							}
							else if (mth == Population::CrossOver::Whole_Uniform)
							{
								scalar pr = this->Pick_Random_Normalized_Number();
								if (pr > this->_probUniformCrossOver)
								{
									scalar w = this->_crossOverCoeffMin + (this->_crossOverCoeffMax - this->_crossOverCoeffMin)*this->Pick_Random_Normalized_Number();
									offsp = c1->War_Cross_Over(c2, w);
								}
								else
								{
									offsp = c1->Uniform_Cross_Over(c2);
								}
							}
							else
							{
								offsp = c1->Cross(c2);
							}
							scalar pmut = this->Pick_Random_Normalized_Number();
							if (pmut > ((scalar)1) - this->_mutationProbability)
							{
								Creature* c = 0;
								scalar dec = this->Pick_Random_Normalized_Number();
								if (dec >= 0.5)
								{
									c = offsp.A;
								}
								else
								{
									c = offsp.B;
								}
								this->Mutate(c);
							}
							this->Shape_Creature(offsp.A);
							this->Check_Creature_Fixed_Genes(offsp.A);
							this->Shape_Creature(offsp.B);				
							this->Check_Creature_Fixed_Genes(offsp.B);
							e->push(offsp.A);
							e->push(offsp.B);
						}
					}
				}
				return(diverse);
			}

			queue<Creature*>* Population::Create_Offspring()
			{
				queue<Creature*>* e = new queue<Creature*>();
				this->_diversifiedIslands.clear();
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					bool diverse = this->Propagate_Offspring(e,this->_islands[i]);
					if (!diverse)
					{
						this->_diversifiedIslands.insert(i);
						if (this->_eventHandler)
						{
							this->_eventHandler->Diversifying_Island(i);
						}
					}
				}
				for (int k = 0; k < this->_islands.size(); ++k)
				{
					if (this->_islands[k].size() > 0)
					{
						Creature* c = 0;
						scalar pd = this->Pick_Random_Normalized_Number();
						if (pd > 0.5)
						{
							c = *this->_islands[k].rbegin();
						}
						else
						{
							c = *this->_islands[k].begin();
						}
						Creature* dc = this->Create_Individual(c);
						this->Complement(c, dc);
						dc->_island = k;
						this->Shape_Creature(dc);
						this->Check_Creature_Fixed_Genes(dc);
						e->push(dc);
					}
					else
					{
						int Voxel_Length = (int)this->_islands[k].size();			
						while (Voxel_Length < this->_islandSize[k])
						{
							Creature* dc = this->Create_Individual(0);
							this->Populate_Genes(dc);
							dc->_island = k;
							this->Shape_Creature(dc);
							this->Check_Creature_Fixed_Genes(dc);
							e->push(dc);
							++Voxel_Length;
						}
					}
				}
				return(e);
			}

			void Population::Add_Offspring(queue<Creature*>* offspring)
			{
				while (!offspring->empty())
				{
					Creature* c = offspring->front();
					offspring->pop();
					int i = c->_island;
					while (this->_islands[i].find(c) != this->_islands[i].end())
					{
						if (this->_maxFitness)
						{
							c->_fitness = c->_fitness - this->Pick_Random_Normalized_Number() * 1000000 * EPSILON;
						}
						else
						{
							c->_fitness = c->_fitness + this->Pick_Random_Normalized_Number() * 1000000 / EPSILON;
						}
					}
					this->_islands[i].insert(c);
				}
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					while (this->_islands[i].size() > this->_islandSize[i])
					{
						Creature* c = *this->_islands[i].rbegin();
						this->_islands[i].erase(c);
						delete c;
					}
				}
			}

			void Population::Sort_Creature_List()
			{
				this->_creatureCollection.clear();
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					set<Creature*, Population::Comparer>& island = this->_islands[i];
					set<Creature*, Population::Comparer>::iterator itr = island.begin();
					while (itr != island.end())
					{
						Creature* c = *itr;
						this->_creatureCollection.insert(c);
						++itr;
					}
				}
			}

			void Population::Propagate(const Population::CrossOver& mth, bool migrate, bool renew)
			{
				queue<Creature*>* eval = 0;
				if (!this->_populated)
				{
					eval = this->Fill_Islands();
				}
				else
				{
					if (!renew)
					{
						eval = this->Create_Offspring();
					}
					else
					{
						eval = this->Renew();
					}
				}
				queue<Creature*> clst = *eval;
				int ssize = (int)clst.size();
				while (!clst.empty())
				{
					int n = 0;
					if (this->_chunkSize < (int)clst.size())
					{
						n = this->_chunkSize;
					}
					else
					{
						n = (int)clst.size();
					}
					vector<Creature*> chunk(n);
					for (int i = 0; i < n; ++i)
					{
						chunk[i] = clst.front();
						clst.pop();
						Creature* c = chunk[i];
						c->Prepare();
					}
					if (this->_eventHandler)
					{
						this->_eventHandler->Executing_Simulations(ssize - (int)clst.size() - (int)chunk.size(),ssize);
					}
					tbb::parallel_for(tbb::blocked_range<int>(0, n, 1), [this, &chunk](const tbb::blocked_range<int>& b)
					{
						for (int i = b.begin(); i < b.end(); ++i)
						{
							Creature* c = chunk[i];
							c->Execute();
						}
					});
					for (int i = 0; i < n; ++i)
					{
						scalar fitness;
						chunk[i]->Update_Fitness(fitness);
						chunk[i]->Set_Fitness(fitness);
						if (this->_eventHandler)
						{
							this->_eventHandler->Fitting_Creature(*chunk[i], fitness);
						}
					}
				}
				this->Add_Offspring(eval);
				this->Sort_Creature_List();
				if ((migrate)&&(!renew))
				{
					this->Migrate();
				}
				delete eval;
			}

			void Population::Migrate()
			{
				vector<queue<Creature*>> migrants;
				migrants.resize(this->_islands.size());
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					int n = (i + this->_migrationRatio) % ((int)this->_islands.size());
					int tot = (int)(this->_migrationPercentage*((scalar)this->_islands[i].size()));
					if (tot == 0)
					{
						tot = 1;
					}
					int Voxel_Length = 0;
					while (Voxel_Length < tot)
					{
						Creature* c = *this->_islands[i].begin();
						this->_islands[i].erase(c);
						migrants[n].push(c);
						++Voxel_Length;
					}
				}
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					queue<Creature*>& cmigr = migrants[i];
					while (!cmigr.empty())
					{
						Creature* c = cmigr.front();
						cmigr.pop();
						if (this->_islands[i].find(c) == this->_islands[i].end())
						{
							this->_islands[i].insert(c);
							c->_island = i;
						}
						else
						{
							delete c;
						}
					}
					this->_islandSize[i] = (int)this->_islands[i].size();
				}
				this->_migrationRatio = this->_migrationRatio + 1;
				if (this->_migrationRatio == (int)this->_islands.size())
				{
					this->_migrationRatio = 1;
				}
			}

			void Population::Lock_Event_Trigger()
			{
				this->_eventLocker.lock();
			}

			void Population::UnLock_Event_Trigger()
			{
				this->_eventLocker.unlock();
			}

			void Population::Set_Crossover_Rule(const Population::CrossOver& mth)
			{
				this->_crossOverMethod = mth;
			}

			void Population::Pick_Creature_List(list<Creature*>& lst, scalar cut)
			{
				lst.clear();
				set<Creature*, Comparer>::iterator i = this->_creatureCollection.begin();
				bool reached = false;
				while ((i != this->_creatureCollection.end())&&(!reached))
				{
					Creature* c = *i;
					if (this->_maxFitness)
					{
						if (c->_fitness >= cut)
						{
							lst.push_back(c);
						}
						else
						{
							reached = true;
						}
					}
					else
					{
						if (c->_fitness <= cut)
						{
							lst.push_back(c);
						}
						else
						{
							reached = true;
						}
					}
					++i;
				}
			}

			bool Population::Population_Fit()
			{
				bool r = false;
				int counter = 0;
				set<Creature*, Population::Comparer>::iterator i = this->_creatureCollection.begin();
				while (i != this->_creatureCollection.end())
				{
					Creature* c = *i;
					if (this->_maxFitness)
					{
						if (c->_fitness > this->_minFitnessRenew)
						{
							counter = counter + 1;
						}
					}
					else
					{
						if (c->_fitness < this->_minFitnessRenew)
						{
							counter = counter + 1;
						}
					}
					++i;
				}
				scalar p = (scalar)counter / (scalar)this->_creatureCollection.size();
				if (p > this->_percentageFitnessRenew)
				{
					r = true;
				}
				return(r);
			}

			void Population::Set_Apocalyptic_Fitness(scalar fitness)
			{
				this->_minFitnessRenew = fitness;
			}

			scalar Population::Apocalyptic_Fitness() const
			{
				return(this->_minFitnessRenew);
			}

			void Population::Set_Apocalyptic_Percentage(scalar per)
			{
				this->_percentageFitnessRenew = per;
			}

			scalar Population::Apocalyptic_Percentage() const
			{
				return(this->_percentageFitnessRenew);
			}

			void Population::Exec()
			{
				bool dec = true;
				int iteration = 0;
				this->_stopOptimization = false;
				while ((!this->_stopOptimization) && (iteration < this->_generations))
				{
					bool renewed = false;
					bool migrate = false;
					if ((iteration % this->_migrationRate == 0) && (iteration > 1))
					{
						migrate = true;
					}
					if (this->_eventHandler)
					{
						this->_eventHandler->Create_Offspring(iteration);
					}
					if ((this->Population_Fit())||(this->_renewPopulation))
					{
						this->Propagate(this->_crossOverMethod, false, true);
						renewed = true;
						this->_renewPopulation = false;
					}
					else
					{
						this->Propagate(this->_crossOverMethod, migrate);
					}
					if ((this->_eventHandler)&&(!this->_creatureCollection.empty()))
					{
						Creature* winner = *this->_creatureCollection.begin();
						this->_eventHandler->Update_Winners(*winner);
					}
					++iteration;
				}
				if (this->_eventHandler)
				{
					set<Creature*, Comparer>::iterator i = this->_creatureCollection.begin();
					list<const Creature*> rtc;
					while (i != this->_creatureCollection.end())
					{
						rtc.push_back(*i);
						++i;
					}
					if (this->_eventHandler)
					{
						this->_mutexThread.lock();
						this->_eventHandler->Conclude(rtc);			
						this->_mutexThread.unlock();
					}
				}
			}



			void Population::Stop()
			{
				this->_stopOptimization = true;
			}

			void Population::Start()
			{
				this->_stopOptimization = false;
				std::thread t(&Population::Exec, this);
				t.detach();
			}

			int Population::Migration_Rate() const
			{
				return(this->_migrationRate);
			}

			void Population::Set_Migration_Rate(int mr)
			{
				this->_migrationRate = mr;
			}

			scalar Population::Migration_Percentage() const
			{
				return(this->_migrationPercentage);
			}

			void Population::Set_Migration_Percentage(scalar mp)
			{
				this->_migrationPercentage = mp;
			}

			scalar Population::Incestual_Preservation() const
			{
				return(this->_islandPreservation);
			}

			void Population::Set_Incestual_Preservation(scalar ip)
			{
				this->_islandPreservation = ip;
			}

			scalar Population::Incestual_Relative_Error() const
			{
				return(this->_incestDistance);
			}

			void Population::Set_Incestual_Relative_Error(scalar ire)
			{
				this->_incestDistance = ire;
			}
			scalar Population::Crossover_Weight_Min() const
			{
				return(this->_crossOverCoeffMin);
			}
			scalar Population::Crossover_Weight_Max() const
			{
				return(this->_crossOverCoeffMax);
			}

			void Population::Set_Weight_Interval(scalar flag, scalar size)
			{
				if (flag > size)
				{
					scalar r = flag;
					flag = size;
					size = r;
				}
				this->_crossOverCoeffMin = flag;
				this->_crossOverCoeffMax = size;
			}

			Population::CrossOver Population::Crossover_Rule() const
			{
				return (this->_crossOverMethod);
			}

			int Population::Population_Size() const
			{
				return(this->_populationSize);
			}

			void Population::Set_Population_Size(int size)
			{
				this->_populationSize = size;
			}

			int Population::Number_Of_Islands() const
			{
				return((int)this->_islands.size());
			}

			void Population::Set_Number_Of_Islands(int size)
			{
				for (int i = 0; i < this->_islands.size(); ++i)
				{
					set<Creature*, Population::Comparer>& isl = this->_islands[i];
					set<Creature*, Population::Comparer>::iterator ii = isl.begin();
					while (ii != isl.end())
					{
						Creature* cc = *ii;
						++ii;
						isl.erase(cc);
						delete cc;
					}		
				}
				this->_islands.clear();
				this->_islands.reserve(size);
				for (int i = 0; i < size; ++i)
				{
					set<Creature*, Population::Comparer> isl(this->_creatureComparer);
					this->_islands.push_back(isl);
				}
				this->_creatureCollection = set<Creature*, Population::Comparer>(this->_creatureComparer);
				this->_islandSize.resize(size);
			}

			scalar Population::Island_Uniform_Percentage() const
			{
				return (this->_islandPreservation);
			}

			void Population::Set_Island_Uniform_Percentage(scalar perc)
			{
				this->_islandPreservation = perc;
			}

			scalar Population::Mutation_Probability() const
			{
				return(this->_mutationProbability);
			}

			void Population::Set_Mutation_Probability(scalar perc)
			{
				this->_mutationProbability = perc;
			}

			scalar Population::Mutation_Displacement_Factor() const
			{
				return (this->_displacementMutation);
			}

			void Population::Set_Mutation_Displacement_Factor(scalar md)
			{
				this->_displacementMutation = md;
			}

			void Population::Set_Maximal_Number_Of_Generations(int gen)
			{
				this->_generations = gen;
			}

			int Population::Maximal_Number_Of_Generations() const
			{
				return(this->_generations);
			}

			void Population::Set_Non_Arithmetic_Rate(scalar nar)
			{
				this->_probUniformCrossOver = nar;
			}

			scalar Population::Non_Arithmetic_Rate() const
			{
				return(this->_probUniformCrossOver);
			}

			void Population::Set_Event_Handler(Population::Observer* evts)
			{
				this->_eventHandler = evts;
				this->_eventHandler->_parentFormation = this;
			}

			void Population::Lock_Working_Thread()
			{
				this->_mutexThread.lock();
			}

			void Population::Unlock_Working_Thread()
			{
				this->_mutexThread.unlock();
			}
		}
	}
}