/* options.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for options namespace
 */

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <string>
#include <tuple>
#include <vector>

namespace options {
  typedef std::vector<std::tuple<double, double>> pairs;
  
  const pairs get_data(std::string file_name = "test/cs472.dat");

  struct Options {
    const pairs values;
    const int iterations;
    const int population_size;
    const int max_depth;
    const int tournament_size;
    const int crossover_size;
    const int elitism_size;
    const double constant_min;
    const double constant_max;
    const double penalty;
    const double grow_chance;
    const double mutate_chance;
    const double crossover_chance;
    const double internals_chance;
    Options(const pairs &, const int & i = 128, const int & p = 1024,
	    const int & d = 5, const int & t = 3);
  };
}

#endif /* _OPTIONS_H_ */
