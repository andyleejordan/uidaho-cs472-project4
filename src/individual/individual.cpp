/* indivudal.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for Individual
 */

#include <cmath>
#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

#include "individual.hpp"
#include "../problem/problem.hpp"
#include "../random/random_generator.hpp"

using namespace individual;
using namespace random_generator;
using problem::Problem;

Node::Node(const Problem & problem, const int & depth) {
  if (depth < problem.max_depth) {
    // Assign random internal type
    int_dist dist(0, internal_types - 1); // Closed interval
    type = Type(dist(engine));
    // Recursively create subtrees
    for (int i = 0; i < arity; i++)
      children.emplace_back(Node(problem, depth + 1));
  } else {
    // Reached max depth, assign random terminal type
    int_dist dist(internal_types, internal_types + terminal_types - 1); // Closed interval
    type = Type(dist(engine));
    // Setup constant type; input is provided on evaluation
    if (type == CONSTANT) {
      // Choose a random value between the problem's min and max
      real_dist dist(problem.constant_min, problem.constant_max);
      constant = dist(engine);
    }
  }
}

double Node::evaluate(const double & input) {
  double left, right;
  if (type != INPUT and type != CONSTANT) {
    left = children[0].evaluate(input);
    right = children[1].evaluate(input);
  }
  switch(type) {
  case ADD:
    return left + right;
  case SUBTRACT:
    return left - right;
  case MULTIPLY:
    return left * right;
  case DIVIDE:
    if (right == 0)
      return 1;
    return left / right;
  case CONSTANT:
    return constant;
  case INPUT:
    return input;
  }
}

Size Node::size() {
  // Recursively count children via pre-order traversal
  // Keep track of internals, leafs, and total
  Size size;
  for (auto child : children) {
    size.internals += child.size().internals;
    size.leafs += child.size().leafs;
  }
  if (children.size() == 0) ++size.leafs;
  else ++size.internals;
  size.total = size.leafs + size.internals;
  return size;
}

void Node::print(const int & depth) {
  // Post-order traversal print of expression in RPN/posfix notation
  std::cout << '(';
  for (auto child : children)
    child.print(depth + 1);
  switch(type) {
  case ADD:
    std:: cout << " + ";
    break;
  case SUBTRACT:
    std:: cout << " - ";
    break;
  case MULTIPLY:
    std:: cout << " * ";
    break;
  case DIVIDE:
    std:: cout << " / ";
    break;
  case CONSTANT:
    std:: cout << constant;
    break;
  case INPUT:
    std:: cout << "X";
    break;
  }
  std:: cout << ')';
}

Individual::Individual(const Problem & p): problem(p), root(Node(problem)) {
  size = root.size();
  fitness = evaluate();
}

double Individual::evaluate() {
  double fitness = 0;
  for (auto pair : problem.values) {
    double output = root.evaluate(std::get<0>(pair));
    fitness += std::pow(output - std::get<1>(pair), 2);
  }
  return std::sqrt(fitness);
}

void Individual::print() {
  std::cout << "Expression tree of size " << size.total
	    << " with " << size.internals << " internals"
	    << " and " << size.leafs << " leafs "
	    << " has the following formula: " << std::endl;
  root.print();
  std::cout << std::endl
	    << "Has a fitness of: " << evaluate() << std::endl;
}
