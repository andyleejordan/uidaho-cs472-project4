/* trials.hpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for the trials namespace
 */

#ifndef _TRIALS_H_
#define _TRIALS_H_

#include <ctime>

#include "../algorithm/algorithm.hpp"

// Forward declaration
namespace options { struct Options; }

namespace trials
{
  const std::tuple<int, algorithm::result_t>
  run(const std::time_t&, const options::Options&);
}

#endif /* _TRIALS_H_ */
