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
  enum class Function;

  // Implemented initial population generation methods.
  enum class Method;

  // Represents a tree's size in terms of internal and leaf nodes.
  struct Size
  {
    unsigned int internals;
    unsigned int leafs;
    Size ();
  };

  // Implements a recursive parse tree representing an expression.
  class Node
  {
  public:
    Node ();
    Node (const Method&, const unsigned int&, const double&,const double&);
    std::string print () const;
    std::string represent () const;
    double evaluate (const double&) const;
    const Size size () const;
    Node& visit (const Size&, Size&);
    void mutate_tree (const double&);

  private:
    Function function;
    unsigned int arity;
    double value;
    std::vector <Node> children;

    void set_constant (const double&, const double&);
    void mutate_self ();
  };

  class Individual
  {
  public:
    Individual ();
    Individual (const Method, const unsigned int, const double&,
		const double&, const options::pairs&);

    std::string print () const;
    std::string print_formula () const;

    unsigned int get_internals () const;
    unsigned int get_leafs () const;
    unsigned int get_total () const;
    double get_fitness () const;
    double get_adjusted () const;

    Node&  operator[] (const Size&);
    void mutate (const double&);
    std::string evaluate (const options::pairs&, const double& penalty = 0,
			  const bool& print = false);
    friend void crossover (const double&, Individual&, Individual&);

  private:
    Node root;
    Size size;
    double fitness;
    double adjusted;
  };
}

#endif /* _INDIVIDUAL_H_ */
