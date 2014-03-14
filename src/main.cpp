/* main.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 */

#include <iostream>

#include "algorithm/algorithm.hpp"
#include "individual/individual.hpp"
#include "problem/problem.hpp"
#include "random_generator/random_generator.hpp"

int main() {
  using namespace problem;
  const Problem problem{get_data(), 64, 1024};
  individual::Individual best = algorithm::genetic(problem);
  std::cout << best.print_formula()
	    << best.print_calculation(problem.values);
}
