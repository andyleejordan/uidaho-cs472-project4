/* individual.hpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for interface of an individual representing a
 * gentically generated solution function
 */

#ifndef _INDIVIDUAL_H_
#define _INDIVIDUAL_H_

#include <string>
#include <vector>

namespace options { struct Options; class Map; }

namespace individual
{
  // Represents a tree's size in terms of internal and leaf nodes.
  struct Size
  {
    unsigned int internals;
    unsigned int leaves;
    unsigned int depth;
    Size();
  };

  // Available methods for tree creation.
  enum class Method {grow, full};

  // List of valid functions for an expression.
  enum class Function {nil, prog2, prog3, iffoodahead, left, right, forward};

  // Implements a recursive parse tree representing an expression.
  class Node
  {
    friend class Individual;

  public:
    Node();
    Node(const Method, const unsigned int);

  protected:
    std::string print() const;
    std::string represent() const;
    void evaluate(options::Map&) const;
    const Size size() const;
    Node& visit(const Size&, Size&);
    void mutate(const unsigned int, const unsigned int, const float);
    std::vector<Node> children;
    Function function;
    unsigned int arity;
  };

  // Implemented genetic operators for Individuals
  enum class Operator {shrink, hoist, subtree, replacement};

  class Individual
  {
  public:
    Individual();
    Individual(const options::Options&);

    std::string print() const;
    std::string print_formula() const;

    unsigned int get_internals() const;
    unsigned int get_leaves() const;
    unsigned int get_total() const;
    unsigned int get_depth() const;
    unsigned int get_score() const;
    float get_fitness() const;
    float get_adjusted() const;

    Node& operator[](const Size&);
    Node& at(const Size&);
    void mutate(const unsigned int, const unsigned int, const float);
    std::string evaluate(options::Map, const float penalty = 0,
			 const bool print = false);
    friend void crossover(const float, Individual&, Individual&);

  private:
    Node root;
    Size size;
    unsigned int score;
    float fitness;
    float adjusted;

    enum class Type {leaf, internal};
    Size get_node(const Type) const;
  };
}

#endif /* _INDIVIDUAL_H_ */
