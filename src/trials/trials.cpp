/* trials.cpp - CS 472 Project #3: Genetic Programming
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

namespace trials
{
  using individual::Individual;

  // Push results
  void
  push_results(const std::time_t& time, unsigned int& trial,
	       const unsigned int trials, std::vector<Individual>& candidates,
	       const options::Options& options)
  {
    // Spawn blocks number of async threads.
    std::vector<std::future<const Individual>> results;
    results.reserve(trials);

    auto task = [&time, &trial, &options]() mutable
      { return async(std::launch::async,
		     algorithm::genetic, time, ++trial, options); };
    generate_n(back_inserter(results), trials, task);
    // Gather future results.
    for (std::future<const Individual>& result : results)
      candidates.emplace_back(result.get());
  }

  // Spawn trials number of threads in blocks.
  const std::tuple<int, individual::Individual>
  run(const std::time_t& time, const options::Options& options)
  {
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long blocks = (hardware_threads != 0) ? hardware_threads : 2;

    unsigned int trial = 0;
    std::vector<Individual> candidates;

    // Spawn trials in chunks of size blocks.
    for (unsigned long t = 0; t < options.trials / blocks; ++t)
      push_results(time, trial, blocks, candidates, options);

    // Spawn remaining trials.
    push_results(time, trial, options.trials % blocks, candidates, options);

    // Retrieve best element.
    auto compare = [](const Individual& a, const Individual&b)
      { return a.get_score() > b.get_score(); };
    auto best = min_element(begin(candidates), end(candidates), compare);

    /* Get which trial was best.  Filenames are not zero-indexed so
       increase by one. */
    return std::make_tuple(distance(begin(candidates), best) + 1, *best);
  }
}
