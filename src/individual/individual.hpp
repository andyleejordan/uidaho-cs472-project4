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
  using std::string;

  enum Function {
    null,
    constant, input,
    sqrt, sin, cos, log, exp,
    add, subtract, divide, multiply, pow,
    lesser, greater };

  struct Size {
    int internals;
    int leafs;
    Size(const int & i = 0, const int & l = 0): internals(i), leafs(l) {}
  };

  class Node {
  private:
    Function function = null;
    int arity = 0; // space for time trade-off
    double k = 0; // excess for internal nodes
    std::vector<Node> children;
    void set_constant(const double &, const double &);
    void mutate_self(const double &, const double &);
  public:
    Node() {};
    Node(const Problem &, const int & depth = 0);
    string print() const;
    string represent() const;
    double evaluate(const double &) const;
    const Size size() const;
    Node & visit(const Size &, Size &);
    void mutate_tree(const double &, const double &, const double &);
  };

  class Individual {
  private:
    Size size;
    double fitness;
    Node root;
    void update_size();
  public:
    Individual() {}
    Individual(const Problem &);
    string print_formula() const;
    string print_calculation(const problem::pairs &) const;
    int get_internals() const {return size.internals;}
    int get_leafs() const {return size.leafs;}
    int get_total() const {return size.internals + size.leafs;}
    double get_fitness() const {return fitness;}
    void evaluate(const problem::pairs &);
    void mutate(const double &, const double &, const double &);
    Node & operator[](const Size &);
    friend void crossover(const double &, Individual &, Individual &);
  };
}

#endif /* _INDIVIDUAL_H_ */
