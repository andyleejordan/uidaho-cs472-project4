/* trials.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for the trials namespace
 */

#ifndef _TRIALS_H_
#define _TRIALS_H_

#include <ctime>
#include <tuple>

// Forward declaration
namespace individual { class Individual; }
namespace options { struct Options; }

namespace trials
{
  const std::tuple<int, individual::Individual>
  run(const options::Options&, const std::time_t&);
}

#endif /* _TRIALS_H_ */
