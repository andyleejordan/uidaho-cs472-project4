/* indivudal.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for individual namespace
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

#include "individual.hpp"
#include "../options/options.hpp"
#include "../random_generator/random_generator.hpp"

namespace individual
{
  using std::vector;
  using std::string;
  using namespace random_generator;

  // Default Size struct constructor.
  Size::Size(): internals{0}, leaves{0} {}

  // Available methods for tree creation.
  enum class Method {grow, full};

  // List of valid functions for an expression.
  enum class Function {nil, constant, input, sqrt, sin, cos, log, exp,
      add, subtract, divide, multiply, pow, lesser, greater};

  using F = Function;
  // Vectors of same-arity function enums.
  vector<F> nullaries {F::constant, F::input};
  vector<F> unaries {F::sqrt, F::sin, F::cos, F::log, F::exp};
  vector<F> binaries {F::add, F::subtract, F::multiply, F::divide, F::pow};
  vector<F> quadnaries {F::lesser, F::greater};

  /* Vectors of available function enums.  Should be moved into
     Options struct. */
  vector<F> leaves {F::constant, F::input};
  vector<F> internals {F::sin, F::cos, F::add, F::subtract,
      F::multiply, F::divide, F::lesser, F::greater};

  // Returns a random function from a given set of functions.
  Function
  get_function(const vector <Function>& functions)
  {
    int_dist dist{0, static_cast<int>(functions.size()) - 1}; // closed interval
    return functions[dist(rg.engine)];
  }

  // Returns a random double between min and max.
  double
  get_constant(const double& min, const double& max)
  {
    real_dist dist{min, max};
    return dist(rg.engine);
  }

  // Returns bool of whether or not the item is in the set.
  template<typename I, typename S> bool
  contains(const I& item, const S& set)
  {
    return std::find(set.begin(), set.end(), item) != set.end();
  }

  // Returns the appropriate arity for a given function.
  unsigned int
  get_arity(const Function& function)
  {
    if (contains(function, nullaries))
      return 0;
    else if (contains(function, unaries))
      return 1;
    else if (contains(function, binaries))
      return 2;
    else if (contains(function, quadnaries))
      return 4;
    assert(false);
  }

  // Default constructor for "empty" node
  Node::Node(): function{Function::nil}, arity{0}, value{0} {}

  /* Recursively constructs a parse tree using the given method
     (either GROW or FULL). */
  Node::Node(const Method& method, const unsigned int& max_depth,
	     const double& constant_min, const double& constant_max)
  {
    // Create terminal node if at the max depth or randomly (if growing).
    real_dist dist{0, 1};
    double grow_chance =
      static_cast<double>(leaves.size()) / (leaves.size() + internals.size());
    if (max_depth == 0
	or (method == Method::grow and dist(rg.engine) < grow_chance))
      {
	function = get_function(leaves);
	arity = get_arity(function);
	// Setup constant function; input is provided on evaluation.
	if (function == Function::constant)
	  value = get_constant(constant_min, constant_max);
      }
    // Otherwise choose a non-terminal node.
    else
      {
	function = get_function(internals);
	arity = get_arity(function);
	// Recursively create subtrees.
	for (unsigned int i = 0; i < arity; ++i)
	  children.emplace_back(Node{
	      method, max_depth - 1, constant_min, constant_max});
      }
    assert(function != Function::nil); // do not create null types
    assert(children.size() == arity); // ensure arity
  }

  // Returns a string visually representing a particular node.
  string
  Node::represent() const
  {
    switch (function)
      {
      case F::nil:
	assert(false); // Never represent empty node.
      case F::constant:
	return std::to_string(value);
      case F::input:
	return "x";
      case F::sqrt:
	return "sqrt";
      case F::sin:
	return "sin";
      case F::cos:
	return "cos";
      case F::log:
	return "log";
      case F::exp:
	return "exp";
      case F::add:
	return "+";
      case F::subtract:
	return "-";
      case F::multiply:
	return "*";
      case F::divide:
	return "%";
      case F::pow:
	return "^";
      case F::lesser:
	return "<";
      case F::greater:
	return ">";
      }
    assert(false); // Every node should have been matched.
  }

  /* Returns string representation of expression in Polish/prefix
     notation using a pre-order traversal. */
  string
  Node::print() const
  {
    if (children.size() == 0)
      return represent();

    string formula = "(" + represent();

    for (const Node& child : children)
      formula += " " + child.print();

    return formula + ")";
  }

  /* Returns a double as the result of a depth-first post-order
     recursive evaluation of a parse tree. */
  double
  Node::evaluate(const double& x) const
  {
    double a, b;

    // Evaluate children a and possibly b.
    if (arity == 1)
      a = children[0].evaluate(x);
    else if (arity == 2 or arity == 4)
      {
	a = children[0].evaluate(x);
	b = children[1].evaluate(x);
	// Children c and d are evaluated conditionally.
      }

    // Calculate the result for the given function.
    switch (function)
      {
      case F::nil:
	assert(false); // Never calculate empty node.
      case F::constant:
	assert(arity == 0);
	return value;
      case F::input:
	assert(arity == 0);
	return x;
      case F::sqrt:
	assert(arity == 1);
	return std::sqrt(std::abs(a)); // Protected via abs.
      case F::sin:
	assert(arity == 1);
	return std::sin(a);
      case F::cos:
	assert(arity == 1);
	return std::cos(a);
      case F::log:
	assert(arity == 1);
	return (a == 0) ? 0 : std::log(std::abs(a)); // Protected via abs.
      case F::exp:
	assert(arity == 1);
	return std::exp(a);
      case F::add:
	assert(arity == 2);
	return a + b;
      case F::subtract:
	assert(arity == 2);
	return a - b;
      case F::multiply:
	assert(arity == 2);
	return a * b;
      case F::divide:
	assert(arity == 2);
	return (b == 0) ? 1 : a / b; // Protected divide by zero: return 1.
      case F::pow:
	assert(arity == 2);
	return std::pow(std::abs(a), std::abs(b)); // Protected via abs.
      case F::lesser:
	assert(arity == 4);
	return (a < b) ? children[2].evaluate(x) : children[3].evaluate(x);
      case F::greater:
	assert(arity == 4);
	return (a > b) ? children[2].evaluate(x) : children[3].evaluate(x);
      }
    assert(false);
  }

  /* Recursively count children via post-order traversal.  Keep track
     of internals and leaves via Size struct */
  const Size
  Node::size() const
  {
    Size size;

    for (const Node& child : children)
      {
	Size temp = child.size();
	size.internals += temp.internals;
	size.leaves += temp.leaves;
      }

    if (children.size() == 0)
      ++size.leaves;
    else
      ++size.internals;

    return size;
  }

  // Used to represent "not-found" (similar to a NULL pointer).
  Node empty;

  /* Depth-first search for taget node.  Must be seeking either
     internal or leaf, cannot be both. */
  Node&
  Node::visit(const Size& i, Size& visiting)
  {
    for (Node& child : children) {
      // Increase relevant count.
      if (child.children.size() == 0)
	++visiting.leaves;
      else
	++visiting.internals;

      // Return node reference if found.
      if (visiting.internals == i.internals or visiting.leaves == i.leaves)
	return child;
      else
	{
	  Node& temp = child.visit(i, visiting); // Recursive search.
	  if (temp.function != Function::nil)
	    return temp;
	}
    }
    return empty;
  }

  // Single node mutation to different function of same arity.
  void
  Node::mutate_self()
  {
    /* Mutate constant to a value in its neighborhood.  Here we do
       not switch functions (between CONSTANT and INPUT), as it
       introduces too much volatility into the terminals.  TODO:
       review the above and possibly change. */
    if (arity == 0)
      {
	if (function == Function::constant)
	  {
	    normal_dist dist{0, 1};
	    value *= 1 + dist(rg.engine);
	  }
      }
    /* Otherwise mutate to internal function of same arity.  We do
       this by saving the old function and arity, then repeatedly
       drawing a new random function until the arity matches and it is
       not the original function. */
    else
      {
	Function old_function = function;
	unsigned int old_arity = arity;
	while (function == old_function or arity != old_arity)
	  {
	    function = get_function(internals);
	    arity = get_arity(function);
	  }
	assert(function != old_function and arity == old_arity);
      }
    assert(function != Function::nil);
  }

  // Recursively mutate nodes with given probability.
  void
  Node::mutate_tree(const double& chance)
  {
    real_dist dist{0, 1};
    for (Node& child : children)
      {
	if (dist(rg.engine) < chance)
	  child.mutate_self();
	child.mutate_tree(chance);
      }
  }

  // Default constructor for Individual
  Individual::Individual() {}

  /* Create an Individual tree by having a root node (to which the
     actual construction is delegated). The depth is passed by value
     as its creation elsewhere is temporary. */
  Individual::Individual(const unsigned int depth, const double& chance,
			 const double& min, const double& max,
			 const options::pairs& values): fitness{0}, adjusted{0}
  {
    // 50/50 chance to choose grow or full
    real_dist dist{0, 1};
    Method method = (dist(rg.engine) < chance) ? Method::grow : Method::full;
    root = Node{method, depth, min, max};
    /*The evaluate method updates the size and both raw and adjusted
      fitnesses. */
    evaluate(values);
  }

  // Return string representation of a tree's size and fitness.
  string
  Individual::print() const
  {
    using std::to_string;

    string info = "# Size " + to_string(get_total())
      + ", with " + to_string(get_internals())
      + " internals, and " + to_string(get_leaves()) + " leaves.\n"
      + "# Raw fitness: " + to_string(get_fitness())
      + ", and adjusted: " + to_string(get_adjusted()) + ".\n";

    return info;
  }

  // Return string represenation of tree's expression (delegated).
  string
  Individual::print_formula() const
  {
    return "# Formula: " + root.print() + "\n";
  }

  // Read-only "getters" for private data

  unsigned int
  Individual::get_internals() const
  {
    return size.internals;
  }

  unsigned int
  Individual::get_leaves() const
  {
    return size.leaves;
  }

  unsigned int
  Individual::get_total() const
  {
    return size.internals + size.leaves;
  }

  double
  Individual::get_fitness() const
  {
    return fitness;
  }

  double
  Individual::get_adjusted() const
  {
    return adjusted;
  }

  /* Evaluate Individual for given values and calculate size.  Update
     Individual's size and fitness accordingly. Return non-empty
     string if printing. */
  string
  Individual::evaluate(const options::pairs& values, const double& penalty,
		       const bool& print)
  {
    using std::to_string;
    using std::get;

    string calculation = "# x - y - expected - error\n";

    // Update size on evaluation because it's incredibly convenient.
    size = root.size();

    // Reset fitness and apply scaled size penalty.
    fitness = penalty * get_total();

    // Evalute for given values.
    for (auto pair : values)
      {
	double x = std::get<0>(pair);
	double y = root.evaluate(x);
	assert(not std::isnan(y) and not std::isinf(y));

	double expected = std::get<1>(pair);
	double error = std::pow(y - expected, 2);
	fitness += error;

	if (print) // Concatenate information if printing.
	  {
	    calculation += to_string(x) + "\t"
	      + to_string(y) + "\t"
	      + to_string(expected) + "\t"
	      + to_string(error) + "\n";
	  }
      }
    // remove penalty when adjusting
    adjusted = static_cast<double>(1) / (1 + fitness - penalty * get_total());

    return calculation;
  }

  // Mutate each node with given probability.
  void
  Individual::mutate(const double& chance)
  {
    root.mutate_tree(chance);
  }

  // Safely return reference to desired node.
  Node&
  Individual::operator[](const Size& i)
  {
    assert(i.internals <= get_internals());
    assert(i.leaves <= get_leaves());

    Size visiting;
    // Return root node if that's what we're seeking.
    if (i.internals == 0 and i.leaves == 0)
      return root;
    else
      return root.visit(i, visiting);
  }

  /* Swap two random subtrees between Individuals "a" and "b",
     selecting an internal node with chance probability.  TODO: DRY */
  void
  crossover(const double& chance, Individual& a, Individual& b)
  {
    real_dist probability{0, 1};
    Size target_a, target_b;

    // Guaranteed to have at least 1 leaf, but may have 0 internals.
    if (a.get_internals() != 0 and probability(rg.engine) < chance)
      {
	// Choose an internal node.
	int_dist dist{0, static_cast<int>(a.get_internals()) - 1};
	target_a.internals = dist(rg.engine);
      }
    else
      {
	// Otherwise choose a leaf node.
	int_dist dist{0, static_cast<int>(a.get_leaves()) - 1};
	target_a.leaves = dist(rg.engine);
      }
    // Totally repeating myself here for "b".
    if (b.get_internals() != 0 and probability(rg.engine) < chance)
      {
	int_dist dist{0, static_cast<int>(b.get_internals()) - 1};
	target_b.internals = dist(rg.engine);
      }
    else
      {
	int_dist dist{0, static_cast<int>(b.get_leaves()) - 1};
	target_b.leaves = dist(rg.engine);
      }
    std::swap(a[target_a], b[target_b]);
  }
}
