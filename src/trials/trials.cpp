/* trials.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for the trials namespace
 */

#include <algorithm>
#include <cassert>
#include <ctime>
#include <future>
#include <tuple>

#include "trials.hpp"
#include "../algorithm/algorithm.hpp"
#include "../individual/individual.hpp"
#include "../options/options.hpp"

namespace trials {
  const std::tuple<int, individual::Individual> run(const options::Options & options, const int & trials, const std::time_t & time) {
    // spawn trials number of threads in blocks
    using individual::Individual;
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long blocks = hardware_threads != 0 ? hardware_threads : 2;
    assert(trials % blocks == 0);
    int trial = 0;
    std::vector<Individual> candidates;
    for (unsigned long t = 0; t < trials / blocks; ++t) {
      std::vector<std::future<const Individual>> results;
      for (unsigned long i = 0; i < blocks; ++i)
	results.emplace_back(std::async(std::launch::async, algorithm::genetic, options, time, ++trial));
      // gather results
      for (std::future<const Individual> & result : results)
	candidates.emplace_back(result.get());
    }
    std::vector<Individual>::iterator best = std::min_element(candidates.begin(), candidates.end(), algorithm::compare_fitness);
    int distance = std::distance(candidates.begin(), best) + 1; // filenames are not zero-indexed
    return std::make_tuple(distance, *best);
  }
}
