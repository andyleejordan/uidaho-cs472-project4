/* algorithm.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for algorithm namespace
 */

#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#include "../individual/individual.hpp"
#include "../problem/problem.hpp"

namespace algorithm {
  using individual::Individual;
  using problem::Problem;
  using std::vector;

  bool compare_fitness(const Individual & a, const Individual & b);
  Individual selection(const Problem & problem, const vector<Individual> & population);
  void genetic(const Problem & problem);
}

#endif /* _ALGORITHM_H_ */
