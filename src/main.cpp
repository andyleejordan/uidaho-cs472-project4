/* main.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 */

#include <algorithm>
#include <cassert>
#include <chrono>
#include <ctime>
#include <future>
#include <iostream>
#include <thread>
#include <tuple>

#include "algorithm/algorithm.hpp"
#include "individual/individual.hpp"
#include "options/options.hpp"
#include "random_generator/random_generator.hpp"

namespace trials {
  std::tuple<int, individual::Individual> run(options::Options options, int trials, std::time_t time) {
    // spawn trials number of threads in blocks
    using individual::Individual;
    int trial = 0;
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long blocks = hardware_threads != 0 ? hardware_threads : 2;
    assert(trials % blocks == 0);
    std::vector<Individual> candidates;
    for (unsigned long t = 0; t < trials / blocks; ++t) {
      std::vector<std::future<const Individual>> results;
      for (unsigned long i = 0; i < blocks; ++i)
	results.emplace_back(std::async(std::launch::async, algorithm::genetic, options, time, ++trial));
      // gather results
      for (std::future<const Individual> & result : results)
	candidates.emplace_back(result.get());
    }
    auto best = std::min_element(candidates.begin(), candidates.end(), algorithm::compare_fitness);
    int distance = std::distance(candidates.begin(), best) + 1; // filenames are not zero-indexed
    return std::make_tuple(distance, *best);
  }
}

int main() {
  using namespace options;
  const Options options{get_data(), 64, 128, 3, 3};
  const int trials = 4;
  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::time_t time = std::time(nullptr);
  // begin timing trials
  start = std::chrono::system_clock::now();
  auto best = trials::run(options, trials, time);
  // end timing trials
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "Total elapsed time: " << elapsed_seconds.count() << "s\n"
	    << "Average time: " << elapsed_seconds.count() / trials << "s\n"
	    << "Best trial: " << time << "_"
	    << std::get<0>(best) << "\n"
	    << std::get<1>(best).print();
}
