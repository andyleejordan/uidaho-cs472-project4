/* algorithm.hpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for algorithm namespace
 */

#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#include <ctime>

// Forward declarations
namespace options { struct Options; }
namespace individual { class Individual; }

namespace algorithm
{
  struct compare_fitness
  {
    bool
    operator()(const individual::Individual&,
	       const individual::Individual&);
  };

  const individual::Individual
  genetic(const options::Options& options,
	  std::time_t time = std::time(nullptr), const int& trial = 0);
}

#endif /* _ALGORITHM_H_ */
