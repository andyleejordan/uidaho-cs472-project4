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
  // Represents a tree's size in terms of internal and leaf nodes.
  struct Size
  {
    unsigned int internals;
    unsigned int leaves;
    Size();
  };

  // Implemented initial population generation methods.
  enum class Method;

  // Implemented functions for expression.
  enum class Function;

  // Implements a recursive parse tree representing an expression.
  class Node
  {
  public:
    Node();
    Node(const Method&, const unsigned int&);
    std::string print() const;
    std::string represent() const;
    void evaluate(options::Map&) const;
    const Size size() const;
    Node& visit(const Size&, Size&);
    void mutate_tree(const double&);

  private:
    Function function;
    unsigned int arity;
    std::vector<Node> children;

    void mutate_self();
  };

  class Individual
  {
  public:
    Individual();
    Individual(const unsigned int, const double&, options::Map map);

    std::string print() const;
    std::string print_formula() const;

    unsigned int get_internals() const;
    unsigned int get_leaves() const;
    unsigned int get_total() const;
    int get_fitness() const;
    int get_score() const;
    double get_adjusted() const;

    Node& operator[](const Size&);
    void mutate(const double&);
    std::string evaluate(options::Map, const double& penalty = 0,
			 const bool& print = false);
    friend void crossover(const double&, Individual&, Individual&);

  private:
    Node root;
    Size size;
    int fitness;
    int score;
    double adjusted;
  };
}

#endif /* _INDIVIDUAL_H_ */
