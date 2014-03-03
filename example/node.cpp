/* Copyright 2014 Terry Soule, all rights reserved */

#include <iostream>
#include <cstdlib>

using namespace std;

#include "node.hpp"

const double CONST_LIMIT = 10.0;

void node::copy(node *source){
  type = source->type;
  const_value = source->const_value;
  if(type<NUM_NON_TERMS){
    for(int i =0; i < MAX_ARITY; i++){
      if(source->branches[i] != NULL){
	branches[i] = new node;
	branches[i]->copy(source->branches[i]);
      }
      else{
	branches[i] = NULL;
      }
    }
  }
}

void node::erase(void){
  if(type<NUM_NON_TERMS){
    for(int i =0; i < MAX_ARITY; i++)
      if(branches[i] != NULL)
	branches[i]->erase();
  }
  delete this;
}

void node::full(int depth,int max,node *p){
  /* creates full trees for the initial population */
  parent = p;  // point to parent node
  if(depth >= max) // if at max depth put in terminal
    type = NUM_NON_TERMS + rand()%NUM_TERMS;
  else{    // else add non-terminal
    type=rand()%NUM_NON_TERMS;
    switch (type){
    case add:
    case subtract:
      for(int i =0; i < 2; i++){
	branches[i] = new node;
	branches[i]->full(depth+1,max,this);
      }
      for(int i=2;i<MAX_ARITY;i++)  // initially make all branches NULL
	branches[i] = NULL;
      break;
    default:
      cout << "Unknown case in grow"  << endl;
    }
  }
  if(type == constt)  // if a constant include a value
    const_value = double(drand48() * 2.0 * CONST_LIMIT) - (CONST_LIMIT/2.0);
}

/*  Evaluates a tree.  Just a recursive function that evaluates
    branches and adds, subtracts, etc. their values.  For a more
    complex problem it may need to manipulate some global values.
    E.g. for a 'forward' terminal the position (stored as a global) of
    a robot is changed.
*/
double node::evaluate(double X){
  /* evaluates the tree , for a given X. */
  double l,r;
  switch(type){
  case add: // +
    l=branches[0] -> evaluate(X);
    r=branches[1] -> evaluate(X);
    return(l+r);
  case subtract: // -
    l=branches[0] -> evaluate(X);
    r=branches[1] -> evaluate(X);
    return(l-r);
  case inputX: // X
    return(X);
  case constt:
    return(const_value);
  default:
    cout << "Error, unknown instruction " << type << endl;
  }
}

/* Recursively counts terminal and non-terminals. */
void node::calc_size(int &terms, int &non_terms){
  if(type >= NUM_NON_TERMS){	// count size
    terms++;
    return;
  }
  else{
    non_terms++;
    for(int i=0;i<MAX_ARITY;i++){
      if(branches[i] != NULL){
	branches[i]->calc_size(terms,non_terms);
      }
    }
  }
}
