/* random_generator.hpp - Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for random generator setup
 */

#ifndef _RANDOM_GENERATOR_H_
#define _RANDOM_GENERATOR_H_

#include <random>

namespace random_generator
{
  typedef std::uniform_int_distribution<int> int_dist;
  typedef std::uniform_int_distribution<std::size_t> size_dist;
  typedef std::normal_distribution<float> normal_dist;
  typedef std::bernoulli_distribution bool_dist;

  // "Singleton" class for an engine initialized with device
  class RandomGenerator
  {
  public:
    RandomGenerator();
    std::mt19937_64 engine;

  private:
    std::random_device rd;
  };

  extern RandomGenerator rg;
}

#endif /* _RANDOM_GENERATOR_H_ */
