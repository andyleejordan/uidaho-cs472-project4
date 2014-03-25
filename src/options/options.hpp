/* options.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * header file for options namespace
 * provides program options and gathers test data
 */

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <string>
#include <tuple>
#include <vector>

namespace options
{
  // a vector of (X, Y) pairs of doubles as tuples
  typedef std::vector <std::tuple <double, double>> pairs;

  // "singleton" struct with configured options for the algorithm
  // setup and returned by parse()
  struct Options
  {
    pairs values;
    unsigned int trials;
    unsigned int iterations;
    unsigned int population_size;
    unsigned int max_depth;
    unsigned int tournament_size;
    unsigned int crossover_size;
    unsigned int elitism_size;
    double constant_min;
    double constant_max;
    double penalty;
    double grow_chance;
    double mutate_chance;
    double crossover_chance;
    double internals_chance;
    std::string logs_dir;
    std::string plots_dir;
    unsigned int verbosity;
    void validate () const;
  };

  // given argc and argv, returns a finished and validated Options object
  const Options parse (int argc, char *argv[]);
}

#endif /* _OPTIONS_H_ */
