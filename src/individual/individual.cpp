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
  vector<Function> nullaries{constant, input};
  vector<Function> unaries{sqrt, sin, cos, log, exp};
  vector<Function> binaries{add, subtract, multiply, divide, pow};
  vector<Function> quadnaries{lesser, greater};
  vector<Function> internals{add, subtract, multiply, divide, lesser, greater};

  template<typename I, typename S> bool contains(const I & item, const S & set) {
    return std::find(set.begin(), set.end(), item) != set.end();
  }

  Function get_internal() {
    int_dist dist{0, int(internals.size()) - 1}; // closed interval
    return Function(internals[dist(rg.engine)]);
  }

  Function get_leaf() {
    int_dist dist{0, int(nullaries.size()) - 1}; // closed interval
    return Function(nullaries[dist(rg.engine)]);
  }

  double get_constant(const double & min, const double & max) {
    real_dist dist{min, max};
    return dist(rg.engine);
  }

  int get_arity(const Function & function) {
    if (contains(function, nullaries)) return 0;
    else if (contains(function, unaries)) return 1;
    else if (contains(function, binaries)) return 2;
    else if (contains(function, quadnaries)) return 4;
    assert(false);
  }

  Node::Node(const Problem & problem, const int & depth) {
    real_dist dist{0, 1};
    if (dist(rg.engine) < problem.growth_chance)
      full(problem, depth);
    else
      growth(problem, depth);
    assert(int(children.size()) == arity);
    assert(function != null); // do not create null types
  }

  void Node::full(const Problem & problem, const int & depth) {
    if (depth < problem.max_depth) {
      // assign random internal function
      function = get_internal();
      assert(contains(function, internals));
      // determine node's arity
      arity = get_arity(function);
      assert(arity != 0);
      // recursively create subtrees
      for (int i = 0; i < arity; ++i)
	children.emplace_back(Node{problem, depth + 1});
    } else {
      // reached max depth, assign random terminal function
      function = get_leaf();
      assert(arity == 0);
      // setup constant function; input is provided on evaluation
      if (function == constant) {
	// choose a random value between the problem's min and max
	k = get_constant(problem.constant_min, problem.constant_max);
      }
    }
  }

  void Node::growth(const Problem & problem, const int & depth) {
    if (depth < problem.max_depth) {
      // assign with probability internal or leaf node
      // assign internal if root
      real_dist dist{0, 1};
      if (dist(rg.engine) < problem.growth_chance or depth == 0) {
	function = get_internal();
	arity = get_arity(function);
	assert(arity != 0);
	for (int i = 0; i < arity; ++i)
	  children.emplace_back(Node{problem, depth + 1});
      }
      else {
	function = get_leaf();
	assert(arity == 0);
      }
    } else {
      // maximum depth reached, must assign leaf node
      function = get_leaf();
      assert(arity == 0);
    }
    if (function == constant)
      // choose a random value between the problem's min and max
      k = get_constant(problem.constant_min, problem.constant_max);
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
      return "sqrt";
    case sin:
      return "sin";
    case cos:
      return "cos";
    case log:
      return "log";
    case exp:
      return "exp";
    case add:
      return "+";
    case subtract:
      return "-";
    case multiply:
      return "*";
    case divide:
      return "/";
    case pow:
      return "^";
    case lesser:
      return "<";
    case greater:
      return ">";
    }
    assert(false);
  }

  string Node::print() const {
    // Pre-order traversal print of expression in Polish/prefix notation
    if (children.size() == 0) return represent();
    string formula = "(" + represent();
    for (const Node & child : children)
      formula += " " + child.print();
    return formula + ")";
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
      assert(arity == 0);
      return k;
    case input:
      assert(arity == 0);
      return x;
    case sqrt:
      assert(arity == 1);
      return std::sqrt(std::abs(a)); // protected
    case sin:
      assert(arity == 1);
      return std::sin(a);
    case cos:
      assert(arity == 1);
      return std::cos(a);
    case log:
      assert(arity == 1);
      return (a == 0) ? 0 : std::log(std::abs(a)); // protected
    case exp:
      assert(arity == 1);
      return std::exp(a);
    case add:
      assert(arity == 2);
      return a + b;
    case subtract:
      assert(arity == 2);
      return a - b;
    case multiply:
      assert(arity == 2);
      return a * b;
    case divide:
      assert(arity == 2);
      return (b == 0) ? 1 : a / b; // protected
    case pow:
      assert(arity == 2);
      return std::pow(std::abs(a), std::abs(b)); // protected
    case lesser:
      assert(arity == 4);
      return (a < b) ? c : d;
    case greater:
      assert(arity == 4);
      return (a > b) ? c : d;
    }
    assert(false);
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
    for (Node & child : children) {
      // increase relevant count
      if (child.children.size() == 0) ++visiting.leafs;
      else ++visiting.internals;
      // return node reference if found
      if (visiting.internals == i.internals or visiting.leafs == i.leafs)
	return child;
      Node & temp = child.visit(i, visiting); // mark each node
      if (temp.function != null) return temp; // return found node
    }
    return empty; // need to indicate "not-found"
  }

  void Node::mutate_self() {
    // single node mutation to different function of same arity
    if (arity == 0) {
      // mutate constant to a value in its neighborhood, don't switch functions
      if (function == constant) {
	normal_dist dist{0, 1};
	k *= 1 + dist(rg.engine);
      }
    }
    else if (arity == 1) {
      int_dist dist{0, int(unaries.size()) - 1};
      Function prior = function;
      // ensure we're using a specified available function
      while (function == prior or not contains(function, internals))
	function = Function(unaries[dist(rg.engine)]);
    }
    else if (arity == 2) {
      int_dist dist{0, int(binaries.size()) - 1};
      Function prior = function;
      while (function == prior or not contains(function, internals))
	function = Function(binaries[dist(rg.engine)]);
    }
    else if (arity == 4) {
      int_dist dist{0, int(quadnaries.size()) - 1};
      Function prior = function;
      while (function == prior or not contains(function, internals))
	function = Function(quadnaries[dist(rg.engine)]);
    }
    assert(function != null);
  }

  void Node::mutate_tree(const double & chance) {
    // recursively mutate nodes with problem.mutate_chance probability
    real_dist dist{0, 1};
    for (Node & child : children) {
      if (dist(rg.engine) < chance) child.mutate_self();
      child.mutate_tree(chance);
    }
  }

  Individual::Individual(const Problem & problem): root{Node{problem}} {
    update_size();
    evaluate(problem.values);
  }

  string Individual::print() const {
    using std::to_string;
    string info = "Size " + to_string(get_total())
      + ", with " + to_string(get_internals())
      + " internals, and " + to_string(get_leafs()) + " leafs.\n"
      + "Raw fitness: " + to_string(get_fitness())
      + ", and adjusted: " + to_string(get_adjusted()) + ".\n";
    return info;
  }

  string Individual::print_formula() const {
    return "Formula: " + root.print() + "\n";
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
	+ ", y = " + to_string(get<1>(pair))
	+ ", E = " + to_string(error) + "\n";
    }
    calculation += "Residual sum of squares: " + to_string(fitness) + "\n"
      + "Adjusted fitness: " + to_string(1./(1+fitness)) + "\n";
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
    fitness = error;
    // update size on evaluation because it's incredibly convenient
    update_size();
  }

  void Individual::mutate(const double & chance) {
    // mutate each node with a problem.mutate_chance probability
    root.mutate_tree(chance);
  }

  Node & Individual::operator[](const Size & i) {
    assert(i.internals <= get_internals());
    assert(i.leafs <= get_leafs());
    Size visiting;
    return root.visit(i, visiting);
  }

  void crossover(const double & chance, Individual & a, Individual & b) {
    real_dist probability{0, 1};
    Size target_a, target_b;
    if (probability(rg.engine) < chance) {
      // choose an internal node
      int_dist dist{0, a.get_internals() - 1};
      target_a.internals = dist(rg.engine);
    } else {
      // otherwise we choose a leaf node
      int_dist dist{0, a.get_leafs() - 1};
      target_a.leafs = dist(rg.engine);
    }
    // do the same thing for the second individual
    if (probability(rg.engine) < chance) {
      int_dist dist{0, b.get_internals() - 1};
      target_b.internals = dist(rg.engine);
    } else {
      int_dist dist{0, b.get_leafs() - 1};
      target_b.leafs = dist(rg.engine);
    }
    // replace nodes
    std::swap(a[target_a], b[target_b]);
  }
}
