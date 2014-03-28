/* individual.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for interface of an individual representing a
 * gentically generated solution function
 */

#ifndef _INDIVIDUAL_H_
#define _INDIVIDUAL_H_

#include <string>
#include <vector>
#include "../options/options.hpp"

namespace individual
{
  // Implemented functions for expression.
  enum Function
    {
      NIL,
      CONSTANT, INPUT,
      SQRT, SIN, COS, LOG, EXP,
      ADD, SUBTRACT, DIVIDE, MULTIPLY, POW,
      LESSER, GREATER
    };

  // Implemented initial population generation methods.
  enum Method { GROW, FULL };

  // Represents a tree's size in terms of internal and leaf nodes.
  struct Size { unsigned int internals = 0; unsigned int leafs = 0; };

  // Implements a recursive parse tree representing an expression.
  class Node
  {
    Function function = NIL; // Should never be in an expression.
    unsigned int arity = 0; // Space for time trade-off.
    double k = 0; // Excess for internal nodes.
    std::vector <Node> children;

    void set_constant (const double&, const double&);
    void mutate_self ();

  public:
    Node ();
    Node (const Method&, const unsigned int&, const double&,const double&);
    std::string print () const;
    std::string represent () const;
    double evaluate (const double&) const;
    const Size size () const;
    Node&  visit (const Size&, Size&);
    void mutate_tree (const double&);
  };

  class Individual
  {
    Node root;
    Size size;
    double fitness = 0;
    double adjusted = 0;
  public:
    Individual () {}
    Individual (const Method, const unsigned int, const double&,
		const double&, const options::pairs&);

    std::string print () const;
    std::string print_formula () const;

    unsigned int get_internals () const { return size.internals; }
    unsigned int get_leafs () const { return size.leafs; }
    unsigned int get_total () const { return size.internals + size.leafs; }
    double get_fitness () const { return fitness; }
    double get_adjusted () const { return adjusted; }

    Node&  operator[] (const Size&);
    void mutate (const double&);
    std::string evaluate (const options::pairs&, const double& penalty = 0,
			  const bool& print = false);
    friend void crossover (const double&, Individual&, Individual&);
  };
}

#endif /* _INDIVIDUAL_H_ */
