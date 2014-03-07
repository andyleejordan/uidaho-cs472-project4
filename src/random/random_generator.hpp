/* Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for random generator setup
 */

#ifndef _RANDOM_GENERATOR_H_
#define _RANDOM_GENERATOR_H_

#include <random>

namespace RandomGenerator {
  std::random_device rd;
  std::mt19937_64 engine(rd());
  typedef std::uniform_real_distribution<double> real_dist;
  typedef std::uniform_int_distribution<> int_dist;
}

#endif /* _RANDOM_GENERATOR_H_ */
