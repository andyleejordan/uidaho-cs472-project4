/* Copyright 2014 Terry Soule, all rights reserved */

/* Defines the indiv class.  Each individual has a fitness,
   size, number of terminals (terms), number of non-terminals,
   and a pointer to the root node of the actual tree.

   Many of the functions simply call functions in the node class.
*/

#include "node.hpp"

class indiv{
public:
  float get_fitness(void){return fitness;}
  float get_size(void){return size;}
  void evaluate(void);
  void evaluate_print(void);
  void calc_size(void);
  void generate(int);
  void erase(void);
private:
  float fitness;  // fitness of the individual
  int size;       // total number of nodes in the tree
  int terms;      // total number of terminals in the tree
  int non_terms;  // total number of non-terminals in the tree
  node *the_indiv;    // pointer to the root of the 'program' tree 
};

