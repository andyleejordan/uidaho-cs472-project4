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
    pairs values;
    int trials;
    int iterations;
    int population_size;
    int max_depth;
    int tournament_size;
    int crossover_size;
    int elitism_size;
    double constant_min;
    double constant_max;
    double penalty;
    double grow_chance;
    double mutate_chance;
    double crossover_chance;
    double internals_chance;
    std::string logs_dir;
    std::string plots_dir;
    void validate() const;
  };

  const Options parse(int argc, char** argv);
}

#endif /* _OPTIONS_H_ */