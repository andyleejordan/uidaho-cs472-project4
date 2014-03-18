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
  bool compare_fitness(const Individual & a, const Individual & b);
  const Individual genetic(const problem::Problem & problem);
}

#endif /* _ALGORITHM_H_ */
