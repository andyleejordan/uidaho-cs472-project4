/* main.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 */

#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>

#include "individual/individual.hpp"
#include "problem/problem.hpp"

int main() {
  using individual::Individual;
  using namespace problem;

  const pairs values = { {1, 1}, {2, 4}, {3, 9}, {4, 16}, {5, 25}, {6, 36},
			 {7, 49}, {8, 64}, {9, 81}, {10, 100}, {11, 121} };
  const Problem problem(values, 3);
  const int population_size = 256;
  std::vector<Individual> population;
  for (int i = 0; i < population_size; ++i)
    population.emplace_back(Individual(problem));
  Individual best = *std::min_element(population.begin(), population.end(), [](const Individual & a, const Individual & b)->bool {return a.get_fitness() < b.get_fitness();});
  double sum = std::accumulate(population.begin(), population.end(), 0., [](const double & a, const Individual & b)->double {return a + b.get_fitness();});
  best.print_formula();
  best.print_calculation();
  std::cout << "Average fitness: " << sum / population.size() << std::endl;
  int total_size = 0;
  total_size = std::accumulate(population.begin(), population.end(), 0, [](const int & a, const Individual & b)->double {return a + b.get_total();});
  std::cout << "Average size: " << total_size / population.size() << std::endl;
  return 0;
}
