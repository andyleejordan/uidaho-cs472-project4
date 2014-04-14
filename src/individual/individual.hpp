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
    void mutate_tree(const float&);

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
    Individual(const unsigned int, const float&, options::Map map);

    std::string print() const;
    std::string print_formula() const;

    unsigned int get_internals() const;
    unsigned int get_leaves() const;
    unsigned int get_total() const;
    unsigned int get_score() const;
    float get_fitness() const;
    float get_adjusted() const;

    Node& operator[](const Size&);
    void mutate(const float&);
    std::string evaluate(options::Map, const float& penalty = 0,
			 const bool& print = false);
    friend void crossover(const float&, Individual&, Individual&);

  private:
    Node root;
    Size size;
    unsigned int score;
    float fitness;
    float adjusted;
  };
}

#endif /* _INDIVIDUAL_H_ */
