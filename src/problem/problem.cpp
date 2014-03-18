/* problem.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for problem namespace
 */

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "problem.hpp"

namespace problem {
  // reads in columnar X Y data from file, creates a vector of pairs
  const pairs get_data(std::string file_name) {
    std::ifstream data_file{file_name};
    if (!data_file.is_open()) {
      std::cerr << "Data file " << file_name << " could not be read!" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    pairs values;
    while (!data_file.eof()) {
      // fill values vector with (x, y) pairs
      int x;
      double y;
      data_file >> x >> y;
      values.emplace_back(std::make_tuple(x, y));
    }
    if (!values.size()) {
      std::cerr << "Data file " << file_name << " was empty!" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    return values;
  }

  Problem::Problem(const pairs & v, const int & i, const int & p, const int & d, const int & t):
    values{v}, iterations{i}, population_size{p}, max_depth{d},
    tournament_size{t}, crossover_size{2}, elitism_size{2},
    constant_min{0}, constant_max{10},
    growth_chance{0.5}, mutate_chance{0.01},
    crossover_chance{0.8}, internals_chance{0.9}
  {
    assert(values.size() > 0);
    assert(max_depth > 0);
  }
}
