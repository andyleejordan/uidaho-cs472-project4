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
  using options::Options;
  using namespace random_generator;

  // Vectors of same-arity function enums.
  vector <Function> nullaries { CONSTANT, INPUT };
  vector <Function> unaries { SQRT, SIN, COS, LOG, EXP };
  vector <Function> binaries { ADD, SUBTRACT, MULTIPLY, DIVIDE, POW };
  vector <Function> quadnaries { LESSER, GREATER};

  /* Vectors of available function enums.  Should be moved into
     Options struct. */
  vector <Function> leafs { CONSTANT, INPUT };
  vector <Function> internals { SIN, COS, ADD, SUBTRACT, MULTIPLY, DIVIDE,
      LESSER, GREATER};

  // Returns bool of whether or not the item is in the set.
  template <typename I, typename S> bool
  contains (const I &item, const S &set)
  {
    return std::find (set.begin (), set.end (), item) != set.end ();
  }

  // Returns a random function from a given set of functions.
  Function
  get_function (const vector <Function> &functions)
  {
    int_dist dist { 0, (int) functions.size () - 1 }; // closed interval
    return functions[dist (rg.engine)];
  }

  // Returns a random double between min and max.
  double
  get_constant (const double &min, const double &max)
  {
    real_dist dist { min, max };
    return dist (rg.engine);
  }

  // Returns the appropriate arity for a given function.
  unsigned int
  get_arity (const Function &function)
  {
    if (contains (function, nullaries))
      return 0;
    else if (contains (function, unaries))
      return 1;
    else if (contains (function, binaries))
      return 2;
    else if (contains (function, quadnaries))
      return 4;
    assert (false);
  }

  /* Recursively constructs a parse tree using the given method
     (either GROW or FULL).  TODO: replace options with necessary data
     (min and max). */
  Node::Node (const Options &options, const Method &method,
	      const unsigned int &max_depth)
  {
    // Create terminal node if at the max depth or randomly (if growing).
    real_dist dist { 0, 1 };
    double grow_chance = (double) leafs.size () / leafs.size () + leafs.size ();
    if (max_depth == 0 or (method == GROW and dist (rg.engine) < grow_chance))
      {
	function = get_function (leafs);
	// Setup constant function; input is provided on evaluation.
	if (function == CONSTANT)
	  k = get_constant (options.constant_min, options.constant_max);
      }
    // Otherwise choose a non-terminal node.
    else
      {
	function = get_function (internals);
	// Determine node's arity.
	arity = get_arity (function);
	// Recursively create subtrees.
	for (unsigned int i = 0; i < arity; ++i)
	  children.emplace_back (Node { options, method, max_depth - 1 });
      }
    assert (function != NIL); // do not create null types
    assert (children.size () == arity); // ensure arity
  }

  // Returns a string visually representing a particular node.
  string
  Node::represent () const
  {
    switch (function)
      {
      case NULL:
	assert (false); // Never represent empty node.
      case CONSTANT:
	return std::to_string (k);
      case INPUT:
	return "x";
      case SQRT:
	return "sqrt";
      case SIN:
	return "sin";
      case COS:
	return "cos";
      case LOG:
	return "log";
      case EXP:
	return "exp";
      case ADD:
	return "+";
      case SUBTRACT:
	return "-";
      case MULTIPLY:
	return "*";
      case DIVIDE:
	return "%";
      case POW:
	return "^";
      case LESSER:
	return "<";
      case GREATER:
	return ">";
      }
    assert (false); // Every node should have been matched.
  }

  /* Returns string representation of expression in Polish/prefix
     notation using a pre-order traversal. */
  string
  Node::print () const
  {
    if (children.size () == 0)
      return represent ();

    string formula = "(" + represent ();

    for (const Node &child : children)
      formula += " " + child.print ();

    return formula + ")";
  }

  /* Returns a double as the result of a depth-first post-order
     recursive evaluation of a parse tree. */
  double
  Node::evaluate (const double &x) const
  {
    double a, b;

    // Evaluate children a and possibly b.
    if (arity == 1)
      a = children[0].evaluate (x);
    else if (arity == 2 or arity == 4)
      {
	a = children[0].evaluate (x);
	b = children[1].evaluate (x);
	// Children c and d are evaluated conditionally.
      }

    // Calculate the result for the given function.
    switch (function)
      {
      case NIL:
	assert (false); // Never calculate empty node.
      case CONSTANT:
	assert (arity == 0);
	return k;
      case INPUT:
	assert (arity == 0);
	return x;
      case SQRT:
	assert (arity == 1);
	return std::sqrt (std::abs (a)); // Protected via abs.
      case SIN:
	assert (arity == 1);
	return std::sin (a);
      case COS:
	assert (arity == 1);
	return std::cos (a);
      case LOG:
	assert (arity == 1);
	return (a == 0) ? 0 : std::log (std::abs (a)); // Protected via abs.
      case EXP:
	assert (arity == 1);
	return std::exp (a);
      case ADD:
	assert (arity == 2);
	return a + b;
      case SUBTRACT:
	assert (arity == 2);
	return a - b;
      case MULTIPLY:
	assert (arity == 2);
	return a * b;
      case DIVIDE:
	assert (arity == 2);
	return (b == 0) ? 1 : a / b; // Protected divide by zero: return 1.
      case POW:
	assert (arity == 2);
	return std::pow (std::abs (a), std::abs (b)); // Protected via abs.
      case LESSER:
	assert (arity == 4);
	return (a < b) ? children[2].evaluate (x) : children[3].evaluate (x);
      case GREATER:
	assert (arity == 4);
	return (a > b) ? children[2].evaluate (x) : children[3].evaluate (x);
      }

    assert (false);
  }

  /* Recursively count children via post-order traversal.  Keep track
     of internals and leafs via Size struct */
  const Size
  Node::size () const
  {
    Size size;

    for (const Node &child : children)
      {
	Size temp = child.size ();
	size.internals += temp.internals;
	size.leafs += temp.leafs;
      }

    if (children.size () == 0)
      ++size.leafs;
    else
      ++size.internals;

    return size;
  }

  // Used to represent "not-found" (similar to a NULL pointer).
  Node empty;

  /* Depth-first search for taget node.  Must be seeking either
     internal or leaf, cannot be both. */
  Node &
  Node::visit (const Size &i, Size &visiting)
  {
    for (Node &child : children) {
      // Increase relevant count.
      if (child.children.size () == 0)
	++visiting.leafs;
      else
	++visiting.internals;

      // Return node reference if found.
      if (visiting.internals == i.internals or visiting.leafs == i.leafs)
	return child;
      else
	{
	  Node &temp = child.visit (i, visiting); // Recursive search.
	  if (temp.function != NIL)
	    return temp;
	}
    }
    return empty;
  }

  // Single node mutation to different function of same arity.
  void
  Node::mutate_self ()
  {
    if (arity == 0)
      {
	/* Mutate constant to a value in its neighborhood.  Here we do
	   not switch functions (between CONSTANT and INPUT), as it
	   introduces too much volatility into the terminals.  TODO:
	   review the above and possibly change. */
	if (function == CONSTANT)
	  {
	    normal_dist dist { 0, 1 };
	    k *= 1 + dist (rg.engine);
	  }
      }
    else if (arity == 1)
      {
	int_dist dist { 0, (int) unaries.size() - 1 };
	Function prior = function;
	/* Here we ensure that we're using a specified available
	   function, and not duplicating the old one.  TODO: This (and
	   the next two blocks) can be majorly refactored using
	   get_function () and simply checking to maintain the proper
	   arity. */
	while (function == prior or not contains (function, internals))
	  function = (Function) unaries[dist (rg.engine)];
      }
    else if (arity == 2)
      {
	int_dist dist { 0, (int) binaries.size() - 1 };
	Function prior = function;
	while (function == prior or not contains (function, internals))
	  function = (Function) binaries[dist (rg.engine)];
      }
    else if (arity == 4)
      {
	int_dist dist { 0, (int) quadnaries.size() - 1 };
	Function prior = function;
	while (function == prior or not contains (function, internals))
	  function = (Function) quadnaries[dist (rg.engine)];
      }
    assert (function != NIL);
  }

  // Recursively mutate nodes with given probability.
  void
  Node::mutate_tree (const double &chance)
  {
    real_dist dist { 0, 1 };
    for (Node &child : children)
      {
	if (dist (rg.engine) < chance)
	  child.mutate_self ();
	child.mutate_tree (chance);
      }
  }

  /* Create an Individual tree by having a root node (to which the
     actual construction is delegated).  TODO: replace options with
     necessary data (min, max, and values). */
  Individual::Individual (const Options &options, const Method method,
			  const unsigned int depth)
    : root { Node { options, method, depth } }
  {
    evaluate(options.values);
  }

  // Return string representation of a tree's size and fitness.
  string
  Individual::print () const
  {
    using std::to_string;

    string info = "# Size " + to_string (get_total ())
      + ", with " + to_string (get_internals ())
      + " internals, and " + to_string (get_leafs ()) + " leafs.\n"
      + "# Raw fitness: " + to_string (get_fitness ())
      + ", and adjusted: " + to_string (get_adjusted ()) + ".\n";

    return info;
  }

  // Return string represenation of tree's expression (delegated).
  string
  Individual::print_formula () const
  {
    return "# Formula: " + root.print () + "\n";
  }

  /* Evaluate Individual for given values and calculate size.  Update
     Individual's size and fitness accordingly. Return non-empty
     string if printing. */
  string
  Individual::evaluate (const options::pairs &values, const double &penalty,
			const bool &print)
  {
    using std::to_string;
    using std::get;

    string calculation;

    // Update size on evaluation because it's incredibly convenient.
    size = root.size ();

    // Reset fitness and apply scaled size penalty.
    fitness = penalty * get_total ();

    // Evalute for given values.
    for (auto pair : values)
      {
	double x = std::get <0> (pair);
	double y = root.evaluate (x);
	assert (not std::isnan (y) and not std::isinf (y));

	double expected = std::get <1> (pair);
	double error = std::pow (y - expected, 2);
	fitness += error;

	if (print) // Concatenate information if printing.
	  {
	    calculation += "# x - y - expected - error\n"
	      + to_string (x) + "\t"
	      + to_string (y) + "\t"
	      + to_string (expected) + "\t"
	      + to_string (error) + "\n";
	  }
      }
    return calculation;
  }

  // Mutate each node with given probability.
  void
  Individual::mutate (const double &chance)
  {
    root.mutate_tree (chance);
  }

  // Safely return reference to desired node.
  Node &
  Individual::operator[] (const Size &i)
  {
    assert (i.internals <= get_internals ());
    assert (i.leafs <= get_leafs ());

    Size visiting;
    // Return root node if that's what we're seeking.
    if (i.internals == 0 and i.leafs == 0)
      return root;
    else
      return root.visit (i, visiting);
  }

  /* Swap two random subtrees between Individuals "a" and "b",
     selecting an internal node with chance probability.  TODO: DRY */
  void
  crossover (const double &chance, Individual &a, Individual &b)
  {
    real_dist probability { 0, 1 };
    Size target_a, target_b;

    // Guaranteed to have at least 1 leaf, but may have 0 internals.
    if (a.get_internals () != 0 and probability (rg.engine) < chance)
      {
	// Choose an internal node.
	int_dist dist { 0, (int) a.get_internals () - 1 };
	target_a.internals = dist(rg.engine);
      }
    else
      {
	// Otherwise choose a leaf node.
	int_dist dist { 0, (int) a.get_leafs () - 1 };
	target_a.leafs = dist(rg.engine);
      }
    // Totally repeating myself here for "b".
    if (b.get_internals () != 0 and probability (rg.engine) < chance)
      {
	int_dist dist { 0, (int) b.get_internals () - 1 };
	target_b.internals = dist (rg.engine);
      }
    else
      {
	int_dist dist { 0, (int) b.get_leafs () - 1 };
	target_b.leafs = dist (rg.engine);
      }
    std::swap (a[target_a], b[target_b]);
  }
}
