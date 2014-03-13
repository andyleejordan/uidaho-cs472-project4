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
    const int iterations = 128;
    const int population_size = 64;
    const int max_depth = 3;
    const int tournament_size = 4;
    const int crossover_size = 2;
    const int elitism_size = 2;
    const double constant_min = 0;
    const double constant_max = 10;
    const double mutate_chance = 0.01;
    const double crossover_chance = 0.8;
    Problem(const pairs & v): values(v) {}
  };
}

#endif /* _PROBLEM_H_ */
