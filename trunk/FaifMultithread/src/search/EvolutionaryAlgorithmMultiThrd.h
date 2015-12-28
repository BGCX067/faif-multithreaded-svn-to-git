/*
 * EvolutionaryAlgorithmMultiThrd.h
 *
 *  Created on: 26 Apr 2011
 *      Author: Maciej
 */

#ifndef EVOLUTIONARYALGORITHMMULTITHRD_H_
#define EVOLUTIONARYALGORITHMMULTITHRD_H_

#include "EvolutionaryAlgorithm.hpp"
#include "../utils/Runnable.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
//TODO wywalić inkluda:
#include <iostream>

namespace faif {
    namespace search {

	template<typename Space,
           template <typename> class Mutation = MutationNone,
           template <typename> class Crossover = CrossoverNone,
           template <typename> class Selection = SelectionRanking,
           typename StopCondition = StopAfterNSteps<100>
           >
	class EvolutionaryAlgorithmMultiThrd;

		template<typename Space,
           template <typename> class Mutation = MutationNone,
           template <typename> class Crossover = CrossoverNone,
           template <typename> class Selection = SelectionRanking,
           typename StopCondition = StopAfterNSteps<100>
           >
    	class EvolutionaryThread: public Runnable {
            typedef typename Space::Individual Individual;
            typedef typename Space::Population2 Population2;
            typedef typename Space::Fitness Fitness;

    	private:
    		int firstIndex_;
    		int lastIndex_;
    		const Population2& population_;
    		boost::shared_ptr<Population2> result_;
    		EvolutionaryAlgorithmMultiThrd<Space, Mutation, Crossover, Selection, StopCondition>* ev_;
    		boost::barrier* barrier1_;
    		boost::barrier* barrier2_;

    	public:
    		bool running;

    		void run() {

    			while (running) {
    				//std::cout<<"Czekam na pierwszej bramce " <<this->getId()<< "\n"; std::cout.flush();
					barrier1_->wait();	// wait for data
					//std::cout<<"Wszedłem " <<this->getId()<< "\n"; std::cout.flush();
					result_ =  ev_->multiCoreMutation(population_, firstIndex_, lastIndex_);
					//result_ = population_;
					//std::cout<<firstIndex_ << "  " << lastIndex_ << "\n";
					barrier2_->wait();	// set main thread that processing finished
    			}
    		}

    		EvolutionaryThread(int firstIndex, int lastIndex,const Population2& population,
    				EvolutionaryAlgorithmMultiThrd<Space, Mutation, Crossover, Selection, StopCondition>* ev,
    	    		boost::barrier* barrier1, boost::barrier* barrier2):
    					ev_(ev), population_(population),
    				firstIndex_(firstIndex), lastIndex_(lastIndex), barrier1_(barrier1), barrier2_(barrier2)
    		{
    			running = true;
    		}

    		boost::shared_ptr<Population2> getResult() {
    			return result_;
    		}

    		void setPopulation(Population2& population) {
    			population_ = population;
    		}


    	};


    	template<typename Space>
    	class MultiCoreEvoAlgorithm
    	{
    		typedef typename Space::Population2 Population2;


    	};



    	template<typename Space,
               template <typename> class Mutation,
               template <typename> class Crossover,
               template <typename> class Selection,
               typename StopCondition
               >
		class EvolutionaryAlgorithmMultiThrd: public EvolutionaryAlgorithm<Space, Mutation, Crossover, Selection, StopCondition> {
		public:
			EvolutionaryAlgorithmMultiThrd() { }
			virtual ~EvolutionaryAlgorithmMultiThrd() { }

			static const int THREADS = 2;

            typedef typename Space::Individual Individual;
            typedef typename Space::Population2 Population2;
            typedef typename Space::Fitness Fitness;
//            typedef typename Space::Fitnesses Fitnesses;


    		// jedna iteracja mutacji - dostaje jako referencje juz wydzielona dla siebie populacje (np 1/4 calej populacji)
    		// i robi mutation i crossover i dokleja do konca tej populacji punkty
    		std::auto_ptr<Population2> multiCoreMutation(const Population2& mutationPoints, int firstIndex, int lastIndex)
    		{
				//int pop_size = mutationPoints.size();
    			// copy population part to mutate
    			std::auto_ptr<Population2> old_population(new Population2());
    			//old_population.reserve(lastIndex-firstIndex);
    			for (int i = firstIndex; i <=lastIndex; ++i) {
    				old_population->push_back(mutationPoints[i]);
    			}

				//mutation
				doMutation( *old_population, typename Mutation<Space>::TransformationCategory() );

				//crossover
				// bierze po dwa osobniki, krzyżuje je, i mam dwójkę dzieci do tego potrzebny cały wektor
				// żeby wylosować osobniki do mutacji i synchronizować zapis tego, więc to na razie olewam.
				doCrossover( *old_population, mutationPoints, typename Crossover<Space>::TransformationCategory() );

				typename Population2::iterator it;
				for (it = old_population->begin(); it != old_population->end(); ++it) {
					it->second = Space::fitness(it->first);
				}
//				for_each(population.begin(), population.end(),
//						boost::bind(Space::fitness, _1::first) );
//				boost::bind(boost::bind() , _1)

				//merge old_pop and mutated
				//std::copy(old_population.begin(), old_population.end(), back_inserter(*mutationPoints) );
				//mutationPoints.insert(mutationPoints.end(), old_population->begin(), old_population->end());
				return old_population;
    		}

            /** \brief the evolutionary algorithm - until stop repeat mutation, cross-over, selection, succession.
                Modifies the initial population.
            */
		    Individual& solve(Population2& init_population, StopCondition stop = StopCondition() ) {
                int pop_size = init_population.size();
                Population2& current = init_population;
                current.reserve(current.size() * 2);
                //StopCondition stop;

                // zrób wątki
                EvolutionaryThread<Space, Mutation, Crossover, Selection, StopCondition>* threads[THREADS];
                int indexesPerThread = pop_size / THREADS;

                int leftIndexes = pop_size % THREADS;

                boost::barrier barrier1(THREADS + 1);
                boost::barrier barrier2(THREADS + 1);

            	// rozdziel pracę
            	int firstIndex = 0;
            	int lastIndex = indexesPerThread-1;
            	if (leftIndexes > 0) {
            		++lastIndex;
            		--leftIndexes;
            	}
            	for(int i=0; i<THREADS; ++i) {
            		threads[i] = new EvolutionaryThread<Space, Mutation, Crossover, Selection, StopCondition>(firstIndex,
            				lastIndex, current, this, &barrier1, &barrier2);
            		firstIndex = lastIndex + 1;
            		lastIndex += indexesPerThread;
                	if (leftIndexes > 0) {
                		++lastIndex;
                		--leftIndexes;
                	}
            	}

            	// wystartuj
            	for(int i=0; i<THREADS; ++i) {
            		threads[i]->start();
            	}

                do {
                	// zwolnij barierę 1
                	barrier1.wait();
                	stop.update(current);
                	if (stop.isFinished())		// if this is last itearton than stop threads
        				for(int i=0; i<THREADS; ++i) {
        					threads[i]->running = false;
        				}

                	// policz fitness function dla podstawowego wektora w trakcie obliczeń innych wątków
    				typename Population2::iterator it;
    				for (it = current.begin(); it != current.end(); ++it) {
    					it->second = Space::fitness(it->first);
    				}

                	// zwolnij drugą barierę - oczekiwanie na policzenie przez wątki
                	barrier2.wait();



                	// połącz rozwiązania z pojedynczych wątków
                	for(int i=0; i<THREADS; ++i) {
                		Population2& result = *(threads[i]->getResult());
                		std::copy(result.begin(), result.end(), back_inserter(current) );
                	}

                	Selection<Space>::selection(current, pop_size);
                }
                while(! stop.isFinished() );	//	(--k > 0);


                //std::cout<< " Wylazłem z pętli \n";     				std::cout.flush();

				for(int i=0; i<THREADS; ++i) {
					threads[i]->running = false;
				}

            	// poczekaj na zakończenie
            	for(int i=0; i<THREADS; ++i) {
            		threads[i]->join();
            	}

                // clean
            	for(int i=0; i<THREADS; ++i) {
            		delete threads[i];
            	}

//                typename Population2::iterator best;
//                    std::max_element(current.begin(), current.end(),
//                                     boost::bind(Space::fitness, _1.second) < boost::bind(Space::fitness, _2.second) );
				typename Population2::iterator it;
				typename Population2::iterator best = current.begin();
				for (it = current.begin(); it != current.end(); ++it) {
					if (it->second > best->second) {
						best = it;
					}
				}

                //std::cout<< " Zwracam wynik \n";
                return best->first;

            }
		};

    }
}

#endif /* EVOLUTIONARYALGORITHMMULTITHRD_H_ */
