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


namespace individual {
  using std::vector;
  using std::string;
  using problem::Problem;
  using namespace random_generator;

  // vectors of same-arity function enums
  vector<Function> terminals {constant, input};
  vector<Function> unaries {sqrt, sin, cos, log, exp};
  vector<Function> binaries {add, subtract, multiply, divide, pow};
  vector<Function> quadnaries {lesser, greater};
  vector<Function> internals {add, subtract, multiply, divide, lesser, greater};

  template<typename I, typename S> bool contains(const I & item, const S & set) {
    return std::find(set.begin(), set.end(), item) != set.end();
  }

  Node::Node(const Problem & problem, const int & depth) {
    if (depth < problem.max_depth) {
      // assign random internal function
      int_dist dist(0, internals.size() - 1); // closed interval
      function = Function(internals[dist(rg.engine)]);
      assert(contains(function, internals));
      // determine node's arity
      if (contains(function, unaries)) arity = 1;
      else if (contains(function, binaries)) arity = 2;
      else if (contains(function, quadnaries)) arity = 4;
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
      if (function == constant) set_constant(problem.constant_min, problem.constant_max);
    }
    assert(function != null); // do not create null types
  }

  string Node::represent() const {
    switch(function) {
    case null:
      assert(false); // never represent empty node
    case constant:
      return std::to_string(k);
    case input:
      return "x";
    case sqrt:
      return " sqrt";
    case sin:
      return " sin";
    case cos:
      return " cos";
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
    case lesser:
      return " a < b ? c : d";
    case greater:
      return " a > b ? c : d";
    }
  }

  string Node::print() const {
    // Post-order traversal print of expression in RPN/postfix notation
    string formula = "(";
    for (auto child : children)
      formula += child.print();
    return formula + represent() + ")";
  }

  void Node::set_constant(const double & min, const double & max) {
    // choose a random value between the problem's min and max
    real_dist dist(min, max);
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
    case cos:
      return std::cos(a);
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
    // recursively count children via post-order traversal
    // keep track of internals and leafs via Size struct
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

  Node empty;

  Node & Node::visit(const Size & i, Size & visiting) {
    // depth-first search for taget node, either internal or leaf
    if (visiting.internals == i.internals) return *this;
    else if (visiting.leafs == i.leafs) return *this;
    for (Node & child : children) {
      if (child.children.size() == 0) ++visiting.leafs;
      else ++visiting.internals;
      Node & temp = child.visit(i, visiting); // mark each node
      if (temp.function != null) return temp; // return found node
    }
    return empty; // need to indicate "not-found"
  }

  void Node::mutate_self(const double & min, const double & max) {
    // single node mutation to different function of same arity
    if (arity == 0) {
      int_dist dist(0, terminals.size() - 1);
      Function prior = function;
      while (function == prior)
	function = Function(terminals[dist(rg.engine)]);
      if (function == constant) set_constant(min, max);
    }
    else if (arity == 1) {
      int_dist dist(0, unaries.size() - 1);
      Function prior = function;
      // ensure we're using a specified available function
      while (function == prior or not contains(function, internals))
	function = Function(unaries[dist(rg.engine)]);
    }
    else if (arity == 2) {
      int_dist dist(0, binaries.size() - 1);
      Function prior = function;
      while (function == prior or not contains(function, internals))
	function = Function(binaries[dist(rg.engine)]);
    }
    else if (arity == 4) {
      int_dist dist(0, quadnaries.size() - 1);
      Function prior = function;
      while (function == prior or not contains(function, internals))
	function = Function(quadnaries[dist(rg.engine)]);
    }
  }

  void Node::mutate_tree(const double & chance, const double & min, const double & max) {
    // recursively mutate nodes with problem.mutate_chance probability
    real_dist dis(0, 1);
    for (Node & child : children) {
      if (dis(rg.engine) < chance) child.mutate_self(min, max);
      child.mutate_tree(chance, min, max);
    }
  }

  Individual::Individual(const Problem & problem): root{Node{problem}} {
    update_size();
    evaluate(problem.values);
  }

  string Individual::print_formula() const {
    using std::to_string;
    string formula = "Expression tree of size " + to_string(get_total())
      + " with " + to_string(get_internals()) + " internals"
      + " and " + to_string(get_leafs()) + " leafs"
      + " has the following formula: " + "\n"
      + root.print() + "\n";
    return formula;
  }

  string Individual::print_calculation(const problem::pairs & values) const {
    using std::to_string;
    using std::get;
    double fitness = 0;
    string calculation;
    for (auto pair : values) {
      double output = root.evaluate(get<0>(pair));
      double error = std::pow(output - get<1>(pair), 2);
      fitness += error;
      calculation += + "f(" + to_string(get<0>(pair))
	+ ") = " + to_string(output)
	+ ", expected " + to_string(get<1>(pair))
	+ ", error = " + to_string(error) + "\n";
    }
    calculation += "Total fitness: " + to_string(std::sqrt(fitness)) + ".\n";
    return calculation;
  }

  void Individual::update_size() {
    size = root.size();
  }

  void Individual::evaluate(const problem::pairs & values) {
    double error = 0;
    for (auto pair : values) {
      double output = root.evaluate(std::get<0>(pair));
      assert(not std::isnan(output) and not std::isinf(output));
      error += std::pow(output - std::get<1>(pair), 2);
    }
    // update size on evaluation because it's incredibly convenient
    update_size();
    fitness = std::sqrt(error);
  }

  void Individual::mutate(const double & chance, const double & min, const double & max) {
    // mutate each node with a problem.mutate_chance probability
    root.mutate_tree(chance, min, max);
  }

  Node & Individual::operator[](const Size & i) {
    assert(i.internals <= get_internals());
    assert(i.leafs <= get_leafs());
    Size visiting;
    return root.visit(i, visiting);
  }

  void crossover(const double & chance, Individual & a, Individual & b) {
    real_dist probability(0, 1);
    Size target_a, target_b;
    if (probability(rg.engine) < chance) {
      // choose an internal
      int_dist dis(0, a.get_internals() - 1);
      target_a.internals = dis(rg.engine);
    } else {
      // otherwise we choose a leaf
      int_dist dis(0, a.get_leafs() - 1);
      target_a.leafs = dis(rg.engine);
    }
    // do the same thing for the second individual
    if (probability(rg.engine) < chance) {
      int_dist dis(0, b.get_internals() - 1);
      target_b.internals = dis(rg.engine);
    } else {
      int_dist dis(0, b.get_leafs() - 1);
      target_b.leafs = dis(rg.engine);
    }
    std::swap(a[target_a], b[target_b]);
  }
}
