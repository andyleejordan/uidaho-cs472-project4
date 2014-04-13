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
  enum class Function {nil, prog2, prog3, iffoodahead, left, right, forward};

  using F = Function;
  // Vectors of same-arity function enums.
  vector<F> nullaries {F::left, F::right, F::forward};
  vector<F> binaries {F::prog2, F::iffoodahead};
  vector<F> trinaries {F::prog3};

  /* Vectors of available function enums.  Should be moved into
     Options struct. */
  vector<F> leaves = nullaries;
  vector<F> internals {F::prog2, F::prog3, F::iffoodahead};

  // Returns a random function from a given set of functions.
  Function
  get_function(const vector <Function>& functions)
  {
    int_dist dist{0, static_cast<int>(functions.size()) - 1}; // closed interval
    return functions[dist(rg.engine)];
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
    else if (contains(function, binaries))
      return 2;
    else if (contains(function, trinaries))
      return 3;
    assert(false);
  }

  // Default constructor for "empty" node
  Node::Node(): function{Function::nil}, arity{0} {}

  /* Recursively constructs a parse tree using the given method
     (either GROW or FULL). */
  Node::Node(const Method& method, const unsigned int& max_depth)
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
      }
    // Otherwise choose a non-terminal node.
    else
      {
	function = get_function(internals);
	arity = get_arity(function);
	// Recursively create subtrees.
	for (unsigned int i = 0; i < arity; ++i)
	  children.emplace_back(Node{method, max_depth - 1});
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
      case F::prog2:
	return "prog2";
      case F::prog3:
	return "prog3";
      case F::iffoodahead:
	return "if-food-ahead";
      case F::left:
	return "left";
      case F::right:
	return "right";
      case F::forward:
	return "forward";
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
  bool
  Node::evaluate(options::Map& map) const
  {
    switch (function)
      {
      case F::nil:
	assert(false); // Never evaluate empty node

      case F::left:
	return map.left(); // Terminal case

      case F::right:
	return map.right(); // Terminal case

      case F::forward:
	return map.forward(); // Terminal case

      case F::prog2:
	if (children[0].evaluate(map)) // Have more ticks, do next tree
	  return children[1].evaluate(map);
	else return false; // Max ticks reached, leave tree

      case F::prog3:
	if (children[0].evaluate(map)) // Have more ticks, do next tree
	  if (children[1].evaluate(map)) // Have more ticks, do next tree
	    return children[2].evaluate(map);
	  else return false; // Max ticks reached, leave tree
	else return false; // Max ticks reached, leave tree

      case F::iffoodahead:
	return (map.look()) // Do left or right depending on if food ahead
	  ? children[0].evaluate(map)
	  : children[1].evaluate(map);
      }
    assert(false); // Every node should be handled
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

  // TODO: implement mutations of individual nodes
  void
  Node::mutate_self()
  {
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
			 options::Map map): fitness{0}, adjusted{0}
  {
    // 50/50 chance to choose grow or full
    real_dist dist{0, 1};
    Method method = (dist(rg.engine) < chance) ? Method::grow : Method::full;
    root = Node{method, depth};
    /* The evaluate method updates the size and both raw and adjusted
       fitnesses. */
    evaluate(map);
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
  Individual::evaluate(options::Map map, const double& penalty,
		       const bool& print)
  {
    // Update size on evaluation because it's incredibly convenient.
    size = root.size();

    while (map.active())
      root.evaluate(map);

    fitness = map.fitness();
    adjusted = static_cast<double>(fitness) / map.max();

    string evaluation;
    if (print) evaluation = map.print();

    return evaluation;
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
