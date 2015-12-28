/*
 * Problem2D.h
 *
 *  Created on: 31 May 2011
 *      Author: Maciej
 */

#ifndef PROBLEM2D_H_
#define PROBLEM2D_H_

#include "../search/EvolutionaryAlgorithm.hpp"
#include "../utils/Random.hpp"
#include <math.h>
#include <vector>

using namespace faif;
using namespace faif::search;

class Problem2D : public EvolutionaryAlgorithmSpace<std::vector<double > > {
public:
	//custom mutation
	static Individual& mutation(Individual& ind) {
		RandomDouble r; //returns the double from 0 to 1 with uniform distribution
		ind[0] = 4.0*(r() - 0.5)*ind[0];
		ind[1] = 4.0*(r() - 0.5)*ind[1];
		return ind;
	}

    static Individual& crossover(Individual& ind, Population& pop) {
    	RandomDouble r;
    	if (r() > 0.6) {
    		int secondParent = floor(r() * pop.size());
    		if (r() >=0.5)
    			ind[1] = pop[secondParent][1];
    		else
    			ind[0] = pop[secondParent][0];
    	}
        return ind;
    }

    static Individual& crossover(Individual& ind, const Population2& pop) {
    	RandomDouble r;
    	if (r() > 0.6) {
    		int secondParent = floor(r() * pop.size());
    		if (r() >=0.5)
    			ind[1] = pop[secondParent].first[1];
    		else
    			ind[0] = pop[secondParent].first[0];
    	}
        return ind;
    }



	//the fitness function
	static double fitness(const Individual& ind) {
		int cos = 0;
		for (int i = 0; i < 10000; ++i) {		// długie obliczenia
			++cos;
		}
		double x = ind[0] - 2;
		double y = ind[1] - 2;
		double result = -(x*x + y*y);
		return result;
	}
};

#endif /* PROBLEM2D_H_ */
