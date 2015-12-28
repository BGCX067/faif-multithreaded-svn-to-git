/*
 * ExProblem.h
 *
 *  Created on: 28 Mar 2011
 *      Author: Maciej
 */

#ifndef EXPROBLEM_H_
#define EXPROBLEM_H_

#include "../search/EvolutionaryAlgorithm.hpp"
#include "../utils/Random.hpp"
#include <math.h>

using namespace faif;
using namespace faif::search;

class ExProblem : public EvolutionaryAlgorithmSpace<double> {
public:
	//custom mutation
	static Individual& mutation(Individual& ind) {
		RandomDouble r; //returns the double from 0 to 1 with uniform distribution
		//ind = r() * r() * r() * r() * r() * r() * r()*ind; //
		ind = 4.0*(r() - 0.5)*ind;
		return ind;
	}
	//the fitness function
	static double fitness(const Individual& ind) {
		Individual x = ind + 1;
		for (int i = 0; i < 10000; ++i) {		// długie obliczenia

		}
		return -x*x; //sqrt(x*x) * sqrt(x*x);
	}

};


#endif /* EXPROBLEM_H_ */
