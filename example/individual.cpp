#include <iostream>
#include <cmath>
using namespace std;

#include "individual.hpp"

const int NUM_POINTS = 5; // test points
// inputs are x values, outputs are f(x) values
float inputs[NUM_POINTS] = {1,2,3,4,5};
float outputs[NUM_POINTS] = {1,4,9,16,25};

/* calls a recursive function to erase/free the tree.
 */
void indiv::erase(void){
  the_indiv -> erase();
}

/* Generates a random full tree.
 */
void indiv::generate(int max_depth){
  the_indiv = new node;
  the_indiv->full(0,max_depth,NULL);
}

/* Calculates the size of an individual's tree.*/
void indiv::calc_size(void){
  terms = 0;
  non_terms = 0;
  size = 0;			
  the_indiv-> calc_size(terms,non_terms);
  size += (terms + non_terms);
}

/* Calculates fitness.
   For the symbolic regression problem it has to reevaluate the expression 
   tree for each of the X points.  
   The square root of the sum of the squared errors at each of those
   points is the fitness.
*/
void indiv::evaluate(void){  
  fitness = 0;
  float output;
  for(int j=0; j < NUM_POINTS; j++){
    // evaluate function on each input point
    output = the_indiv-> evaluate(inputs[j]);
    fitness += (pow((output-outputs[j]),2));
    // outputs array holds correct values 
  }
  fitness = sqrt(fitness);
}

/* Evalautes the tree and prints the y value for each of the x values.*/
void indiv::evaluate_print(void){
  /* evaluates a tree and prints the outputs and 
     correct outputs */
  fitness = 0;
  float output;
  for(int j=0; j < NUM_POINTS; j++){
    output = the_indiv-> evaluate(inputs[j]);
    fitness += (pow((outputs[j]-output),2));
    // outputs array holds correct values 
    cout << inputs[j] << "," << output << "," << outputs[j] << " , " ;
  }
  fitness = sqrt(fitness); // square root of the sum of the squared errors
  cout << endl << "Fitness = " << fitness << endl;
}
