//============================================================================
// Name        : FaifMultithread.cpp
// Author      : Maciej S
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "search/EvolutionaryAlgorithm.hpp"
#include "search/EvolutionaryAlgorithmMultiThrd.h"
#include "tests/ExProblem.hpp"
#include "tests/Problem2D.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::posix_time;

int main() {
	static const int iteracje = 7000;

	Problem2D::Population pop2D;
	vector<double> tab1(2);
	tab1[0] = 1.0; tab1[1] = 2.0;
	pop2D.push_back(tab1);
	vector<double> tab2(2);
	tab2[0] = 5.0; tab2[1] = -3.0;
	pop2D.push_back(tab2);
	vector<double> tab3(2);
	tab3[0] = -1.0; tab3[1] = -2.0;
	pop2D.push_back(tab3);
	vector<double> tab4(2);
	tab4[0] = -9.0; tab4[1] = 4.1;
	pop2D.push_back(tab4);
	vector<double> tab5(2);
	tab5[0] = 2.3; tab5[1] = 0.99;
	pop2D.push_back(tab5);

	ExProblem::Population pop; //initial population
	pop.push_back(1.0);
	pop.push_back(-2.0);
	pop.push_back(-4.0);
	pop.push_back(8.0);
	pop.push_back(2.0);

	bool twoDimentional = true;

	if (twoDimentional) {
		cout << ">>>\t2D problem with crossover\t<<<\n";

		Problem2D::Population2 popCopy;
		Problem2D::Population::iterator it;
		for (it = pop2D.begin(); it!=pop2D.end(); ++it) {
		  std::pair<Problem2D::Individual, double> pair;
		  pair.first = *it;
		  pair.second = 0.0;
		  popCopy.push_back(pair);
		}

		  ptime firstMeasure = microsec_clock::local_time();

		  EvolutionaryAlgorithm< Problem2D, MutationCustom, CrossoverCustom, SelectionRanking, StopAfterNSteps<iteracje> > ea;
		  Problem2D::Individual best = ea.solve(pop2D); //run the ea

		  ptime secondMeasure = microsec_clock::local_time();

		  EvolutionaryAlgorithmMultiThrd<Problem2D, MutationCustom, CrossoverCustom, SelectionRanking, StopAfterNSteps<iteracje> > eamt;
		  Problem2D::Individual thebest = eamt.solve(popCopy);

		  ptime thirdMeasure = microsec_clock::local_time();
		  time_period simpleAlg(firstMeasure, secondMeasure);
		  time_period multiThrdAlg(secondMeasure, thirdMeasure);

		  cout << "\n" << "zwykły:\t\t" << best[0]<< " " << best[1] << "\tczas:\t" <<  to_simple_string(simpleAlg.length()) << endl;
		  cout << "\n" << "wielowątkowy:\t"<< thebest[0] << " " << thebest[1] << "\tczas:\t" << to_simple_string(multiThrdAlg.length()) << endl << " koniec ";
	} else {
		cout << ">>>\t1D problem without crossover\t<<<\n";
		  ExProblem::Population2 popCopy;
		  ExProblem::Population::iterator it;
		  for (it = pop.begin(); it!=pop.end(); ++it) {
			  std::pair<ExProblem::Individual, double> pair;
			  pair.first = *it;
			  pair.second = 0.0;
			  popCopy.push_back(pair);
		  }

		  ptime firstMeasure = microsec_clock::local_time();

		  EvolutionaryAlgorithm< ExProblem, MutationCustom, CrossoverNone, SelectionRanking, StopAfterNSteps<iteracje> > ea;
		  ExProblem::Individual best = ea.solve(pop); //run the ea

		  ptime secondMeasure = microsec_clock::local_time();

		  EvolutionaryAlgorithmMultiThrd<ExProblem, MutationCustom, CrossoverNone, SelectionRanking, StopAfterNSteps<iteracje> > eamt;
		  ExProblem::Individual thebest = eamt.solve(popCopy);

		  ptime thirdMeasure = microsec_clock::local_time();
		  time_period simpleAlg(firstMeasure, secondMeasure);
		  time_period multiThrdAlg(secondMeasure, thirdMeasure);

		  cout << "\n" << "zwykły:\t\t" << best << "\tczas:\t" <<  to_simple_string(simpleAlg.length()) << endl;
		  cout << "\n" << "wielowątkowy:\t"<< thebest << "\tczas:\t" << to_simple_string(multiThrdAlg.length()) << endl << " koniec ";
	}

	return 0;
}
