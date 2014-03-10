/* main.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 */

#include <cmath>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>

#include "individual/individual.hpp"
#include "problem/problem.hpp"

int test_runner() {
  // basic test runner
  using individual::Individual;
  using problem::pairs;
  using problem::Problem;
  std::ifstream test_file("test/cs472.dat");
  if (!test_file.is_open()) return 1;
  pairs values;
  while (!test_file.eof()) {
    // fill values vector with (x, y) pairs
    int x;
    double y;
    test_file >> x >> y;
    values.emplace_back(std::make_tuple(x, y));
  }
  // create Problem and population of Individuals
  const int population_size = 128;
  const int tree_depth = 3;
  const Problem problem(values, tree_depth);
  std::vector<Individual> population;
  for (int i = 0; i < population_size; ++i)
    population.emplace_back(Individual(problem));
  // find Individual with lowest "fitness" AKA error from populaiton
  Individual best = *std::min_element(population.begin(), population.end(), [](const Individual & a, const Individual & b)->bool {return a.get_fitness() < b.get_fitness();});
  // calculate sum of fitnesses
  double total_fitness = std::accumulate(population.begin(), population.end(), 0., [](const double & a, const Individual & b)->double const {return a + b.get_fitness();});
  // caclulate sum of tree sizes
  int total_size = std::accumulate(population.begin(), population.end(), 0, [](const int & a, const Individual & b)->double const {return a + b.get_total();});
  // print results
  best.print_formula();
  best.print_calculation();
  std::cout << "Average fitness: " << total_fitness / population.size() << std::endl;
  std::cout << "Average size: " << total_size / population.size() << std::endl;
  return 0;
}

int main() {
  return test_runner();
}
