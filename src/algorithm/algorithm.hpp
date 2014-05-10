/* algorithm.hpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for algorithm namespace
 */

#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#include <chrono>
#include <ctime>
#include <tuple>

// Forward declarations
namespace options { struct Options; }
namespace individual { class Individual; }

namespace algorithm
{
  // Functor to compare two individuals by fitness
  struct compare_fitness
  {
    bool
    operator()(const individual::Individual&, const individual::Individual&);
  };

  typedef std::tuple<individual::Individual, std::chrono::duration<double>> result_t;

  const result_t
  genetic(const std::time_t&, int, const options::Options&);
}

#endif /* _ALGORITHM_H_ */
