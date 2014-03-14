/* random_generator.cpp - Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for a singleton class which sets up std::random
 */

#include <random>

#include "random_generator.hpp"

namespace random_generator {
  RandomGenerator::RandomGenerator() {
    engine.seed(rd());
  }

  // Singleton RandomGenerator object for shared use
  RandomGenerator rg;
}
