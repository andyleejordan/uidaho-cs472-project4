/* main.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 */

#include <iostream>

#include "individual/individual.hpp"
#include "problem/problem.hpp"

int main() {
  using individual::Individual;
  using namespace problem;
  pairs values = { {1, 1}, {2, 4}, {3, 9}, {4, 16}, {5, 25}, {6, 36} };
  Problem problem(values, 3);
  Individual solution(problem);
  solution.print();
  return 0;
}
