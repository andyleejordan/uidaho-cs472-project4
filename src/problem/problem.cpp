/* problem.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for problem namespace
 */

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "problem.hpp"

const problem::pairs problem::get_data(std::string file_name) {
  std::ifstream data_file(file_name);
  if (!data_file.is_open()) {
    std::cerr << "Data file " << file_name << " could not be read!";
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
    std::cerr << "Data file " << file_name << " was empty!";
    std::exit(EXIT_FAILURE);
  }
  return values;
}

problem::Problem::Problem(const pairs & v, const int & i, const int & p):
  values(v), iterations(i), population_size(p), max_depth(4), tournament_size(4), crossover_size(2),
  elitism_size(2), constant_min(0), constant_max(10), mutate_chance(0.01), crossover_chance(0.8) {}
