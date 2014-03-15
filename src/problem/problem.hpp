/* problem.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for struct definition of a problem space
 */

#ifndef _PROBLEM_H_
#define _PROBLEM_H_

#include <string>
#include <tuple>
#include <vector>

namespace problem {
  typedef std::vector<const std::tuple<const double, const double>> pairs;
  
  const pairs get_data(std::string file_name = "test/cs472.dat");

  struct Problem {
    const pairs values;
    const int iterations;
    const int population_size;
    const int max_depth;
    const int tournament_size;
    const int crossover_size;
    const int elitism_size;
    const double constant_min;
    const double constant_max;
    const double growth_chance;
    const double mutate_chance;
    const double crossover_chance;
    const double internals_chance;
    Problem(const pairs &, const int & i = 128, const int & p = 1024,
	    const int & d = 5, const int & t = 3);
  };
}

#endif /* _PROBLEM_H_ */
