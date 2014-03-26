/* algorithm.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for algorithm namespace
 */

#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#include <ctime>

#include "../individual/individual.hpp"
#include "../options/options.hpp"

namespace algorithm
{
  bool
  compare_fitness (const individual::Individual &,
		   const individual::Individual &);

  const individual::Individual
  genetic (const options::Options &options,
	   std::time_t time = std::time(nullptr), const int &trial = 0);
}

#endif /* _ALGORITHM_H_ */
