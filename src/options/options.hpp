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

  struct Options {
    const pairs values;
    const int trials;
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
    Options(const pairs & values,
	    const int & trials,
	    const int & iterations,
	    const int & population_size,
	    const int & max_depth,
	    const int & tournament_size,
	    const int & crossover_size,
	    const int & elitism_size,
	    const double & constant_min,
	    const double & constant_max,
	    const double & penalty,
	    const double & grow_chance,
	    const double & mutate_chance,
	    const double & crossover_chance,
	    const double & internals_chance);
  };

  const Options parse(int argc, char** argv);
}

#endif /* _OPTIONS_H_ */
