/* indivudal.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for Individual
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

#include "individual.hpp"
#include "../problem/problem.hpp"
#include "../random_generator/random_generator.hpp"

using namespace individual;
using namespace random_generator;
using problem::Problem;

namespace individual {
  using std::vector;
  vector<Function> terminals { constant, input };
  vector<Function> unaries { sqrt, sin, log, exp };
  vector<Function> binaries { add, subtract, divide, multiply, pow };
  vector<Function> quadnaries { cond };
  vector<Function> internals {log, sqrt, sin, add, subtract, divide, multiply, cond};
}

Node::Node(const Problem & problem, const int & depth) {
  using std::find;
  if (depth < problem.max_depth) {
    // assign random internal type
    int_dist dist(0, internals.size() - 1); // closed interval
    type = Function(internals[dist(rg.engine)]);
    int arity = 0;
    if (find(unaries.begin(), unaries.end(), type) != unaries.end()) arity = 1;
    else if (find(binaries.begin(), binaries.end(), type) != binaries.end()) arity = 2;
    else if (find(quadnaries.begin(), quadnaries.end(), type) != quadnaries.end()) arity = 4;
    assert(arity != 0);
    // recursively create subtrees
    for (int i = 0; i < arity; i++)
      children.emplace_back(Node(problem, depth + 1));
  } else {
    // reached max depth, assign random terminal type
    int_dist dist(0, terminals.size() - 1); // closed interval
    type = Function(terminals[dist(rg.engine)]);
    // setup constant type; input is provided on evaluation
    if (type == constant) set_constant(problem);
  }
}

void Node::set_constant(const Problem & problem) {
  // choose a random value between the problem's min and max
  real_dist dist(problem.constant_min, problem.constant_max);
  k = dist(rg.engine);
}

double Node::evaluate(const double & x) const {
  // depth-first post-order recursive evaluation tree
  // slight overhead with find() to make calls a) clearer and b) easy
  // to protect without excess recursive calls
  double a, b, c, d;
  if (find(unaries.begin(), unaries.end(), type) != unaries.end())
    a = children[0].evaluate(x);
  if (find(binaries.begin(), binaries.end(), type) != binaries.end()) {
    a = children[0].evaluate(x);
    b = children[1].evaluate(x);
  }
  if (find(quadnaries.begin(), quadnaries.end(), type) != quadnaries.end()) {
    a = children[0].evaluate(x);
    b = children[1].evaluate(x);
    c = children[2].evaluate(x);
    d = children[3].evaluate(x);
  }
  // calculate the result
  switch(type) {
  case constant:
    return k;
  case input:
    return x;
  case sqrt:
    return std::sqrt(std::abs(a)); // protected
  case sin:
    return std::sin(a);
  case log:
    return (a == 0) ? 0 : std::log(std::abs(a)); // protected
  case exp:
    return std::exp(a);
  case add:
    return a + b;
  case subtract:
    return a - b;
  case multiply:
    return a * b;
  case divide:
    return (b == 0) ? 1 : a / b; // protected
  case pow:
    return std::pow(std::abs(a), std::abs(b)); // protected
  case cond:
    return (a < b) ? c : d;
  }
}

const Size Node::size() const {
  // Recursively count children via post-order traversal
  // Keep track of internals and leafs via Size struct
  Size size;
  for (const Node & child : children) {
    Size temp = child.size(); // is this micro-optimizing?
    size.internals += temp.internals;
    size.leafs += temp.leafs;
  }
  if (children.size() == 0) ++size.leafs;
  else ++size.internals;
  return size;
}

void Node::mutate(const Problem & problem) {
  // single node mutation to different function of same arity
  using std::find;
  if (find(terminals.begin(), terminals.end(), type) != terminals.end()) {
    int_dist dist(0, terminals.size() - 1);
    type = Function(terminals[dist(rg.engine)]);
    if (type == constant) set_constant(problem);
  }
  else if (find(unaries.begin(), unaries.end(), type) != unaries.end()) {
    int_dist dist(0, unaries.size() - 1);
    type = Function(unaries[dist(rg.engine)]);
  }
  else if (find(binaries.begin(), binaries.end(), type) != binaries.end()) {
    int_dist dist(0, binaries.size() - 1);
    type = Function(binaries[dist(rg.engine)]);
  }
  else if (find(quadnaries.begin(), quadnaries.end(), type) != quadnaries.end()) {
    int_dist dist(0, quadnaries.size() - 1);
    type = Function(quadnaries[dist(rg.engine)]);
  }
}

std::string Node::represent() const {
  switch(type) {
  case constant:
    return std::to_string(k);
  case input:
    return "x";
  case sqrt:
    return " sqrt";
  case sin:
    return " sin";
  case log:
    return " log";
  case exp:
    return " exp";
  case add:
    return " +";
  case subtract:
    return " -";
  case multiply:
    return " *";
  case divide:
    return " /";
  case pow:
    return " ^";
  case cond:
    return " a < b ? c : d";
  }
}

std::string Node::print() const {
  // Post-order traversal print of expression in RPN/posfix notation
  std::string formula = "(";
  for (auto child : children)
    formula += child.print();
  return formula + represent() + ")";
}

Individual::Individual(const Problem & p): problem(p), root(Node(problem)) {
  size = root.size();
  fitness = evaluate();
}

double Individual::evaluate() const {
  double fitness = 0;
  for (auto pair : problem.values) {
    double output = root.evaluate(std::get<0>(pair));
    fitness += std::pow(output - std::get<1>(pair), 2);
  }
  return std::sqrt(fitness);
}

void Individual::print_formula() const {
  std::cout << "Expression tree of size " << get_total()
	    << " with " << get_internals() << " internals"
	    << " and " << get_leafs() << " leafs"
	    << " has the following formula: " << std::endl
	    << root.print() << std::endl;
}

void Individual::print_calculation() const {
  double fitness = 0;
  for (auto pair : problem.values) {
    double output = root.evaluate(std::get<0>(pair));
    double error = std::pow(output - std::get<1>(pair), 2);
    fitness += error;
    std::cout << "f(" << std::get<0>(pair) << ", " << std::get<1>(pair)
	      << ") = " << output
	      << ", error = " << error << ".\n";
  }
  std::cout << "Total fitness: " << std::sqrt(fitness) << ".\n";
}
