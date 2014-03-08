/* individual.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for interface of an individual representing a
 * gentically generated solution function
 */

#ifndef _INDIVIDUAL_H_
#define _INDIVIDUAL_H_

#include <vector>
#include <memory>
#include "../problem/problem.hpp"

namespace individual {
  using problem::Problem;

  enum Type { ADD, SUBTRACT, DIVIDE, MULTIPLY, COND, CONSTANT, INPUT };
  const int binary_types = 4;
  const int internal_types = 5;
  const int terminal_types = 2;

  struct Size {
    int total = 0;
    int internals = 0;
    int leafs = 0;
  };

  class Node {
  protected:
    Type type;
    double constant = 0; // excess for internal nodes
    std::vector<Node> children;
  public:
    Node(const Problem & problem, const int & depth = 0);
    double evaluate(const double & input = 1) const;
    void print(const int & depth = 0) const;
    Size size() const;
  };

  class Individual {
  private:
    const Problem problem;
    Size size;
    double fitness;
    Node root;
  public:
    Individual(const Problem &);
    double evaluate() const;
    void print_formula() const;
    void print_calculation() const;
    double get_fitness() const {return fitness;}
    int get_total() const {return size.total;}
    int get_internals() const {return size.internals;}
    int get_leafs() const {return size.leafs;}
  };
}

#endif /* _INDIVIDUAL_H_ */
