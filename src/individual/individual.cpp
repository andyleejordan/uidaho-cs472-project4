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
  // vectors of same-arity function enums
  vector<Function> terminals {constant, input};
  vector<Function> unaries {sqrt, sin, log, exp};
  vector<Function> binaries {add, subtract, divide, multiply, pow};
  vector<Function> quadnaries {lesser, greater};
  vector<Function> internals {log, sqrt, sin, add, subtract, divide, multiply, lesser, greater};
}

Node::Node(const Problem & problem, const int & depth) {
  using std::find;
  if (depth < problem.max_depth) {
    // assign random internal function
    int_dist dist(0, internals.size() - 1); // closed interval
    function = Function(internals[dist(rg.engine)]);
    // determine node's arity
    if (find(unaries.begin(), unaries.end(), function) != unaries.end()) arity = 1;
    else if (find(binaries.begin(), binaries.end(), function) != binaries.end()) arity = 2;
    else if (find(quadnaries.begin(), quadnaries.end(), function) != quadnaries.end()) arity = 4;
    assert(arity != 0);
    // recursively create subtrees
    for (int i = 0; i < arity; i++)
      children.emplace_back(Node(problem, depth + 1));
  } else {
    // reached max depth, assign random terminal function
    int_dist dist(0, terminals.size() - 1); // closed interval
    function = Function(terminals[dist(rg.engine)]);
    assert(arity == 0);
    // setup constant function; input is provided on evaluation
    if (function == constant) set_constant(problem);
  }
}

void Node::set_constant(const Problem & problem) {
  // choose a random value between the problem's min and max
  real_dist dist(problem.constant_min, problem.constant_max);
  k = dist(rg.engine);
}

double Node::evaluate(const double & x) const {
  // depth-first post-order recursive evaluation tree
  double a, b, c, d;
  if (arity == 1)
    a = children[0].evaluate(x);
  else if (arity == 2) {
    a = children[0].evaluate(x);
    b = children[1].evaluate(x);
  }
  else if (arity == 4) {
    a = children[0].evaluate(x);
    b = children[1].evaluate(x);
    c = children[2].evaluate(x);
    d = children[3].evaluate(x);
  }
  // calculate the result
  switch(function) {
  case null:
    assert(false); // never calculate empty node
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
  case lesser:
    return (a < b) ? c : d;
  case greater:
    return (a > b) ? c : d;
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

namespace individual {
  Node empty;
}

Node & Node::visit(const int & i, int & visiting) {
  // depth-first search for taget node
  if (visiting == i) return *this; // return reference to found node
  for (Node & child : children) {
    Node & temp = child.visit(i, ++visiting); // mark each node
    if (temp.function != null) return temp; // return found node
  }
  return empty; // need to indicate "not-found"
}

void Node::mutate_self(const Problem & problem) {
  // single node mutation to different function of same arity
  if (arity == 0) {
    int_dist dist(0, terminals.size() - 1);
    Function prior = function;
    while (function == prior)
      function = Function(terminals[dist(rg.engine)]);
    if (function == constant) set_constant(problem);
  }
  else if (arity == 1) {
    int_dist dist(0, unaries.size() - 1);
    Function prior = function;
    while (function == prior)
      function = Function(unaries[dist(rg.engine)]);
  }
  else if (arity == 2) {
    int_dist dist(0, binaries.size() - 1);
    Function prior = function;
    while (function == prior)
      function = Function(binaries[dist(rg.engine)]);
  }
  else if (arity == 4) {
    int_dist dist(0, quadnaries.size() - 1);
    Function prior = function;
    while (function == prior)
      function = Function(quadnaries[dist(rg.engine)]);
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
void Node::mutate_tree(const Problem & problem) {
  // recursively mutate nodes with problem.mutate_chance probability
  real_dist dis(0, 1);
  for (Node & child : children) {
    if (dis(rg.engine) < problem.mutate_chance) {
      std::cout << "Mutating child\n";
      child.mutate_self(problem);
    }
    child.mutate_tree(problem);
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
    std::cout << "f(" << std::get<0>(pair)
	      << ") = " << output
	      << ", expected " << std::get<1>(pair)
	      << ", error = " << error << "\n";
  }
  std::cout << "Total fitness: " << std::sqrt(fitness) << ".\n";
}
