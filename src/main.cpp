/* main.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 */

#include "algorithm/algorithm.hpp"
#include "individual/individual.hpp"
#include "problem/problem.hpp"
#include "random_generator/random_generator.hpp"

  // int total_size = std::accumulate(population.begin(), population.end(), 0, [](const int & a, const Individual & b)->double const {return a + b.get_total();});

int main() {
  const problem::Problem problem{problem::get_data(), 512, 2048};
  algorithm::genetic(problem);
}
