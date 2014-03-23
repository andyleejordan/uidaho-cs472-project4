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

#include "algorithm/algorithm.hpp"
#include "individual/individual.hpp"
#include "options/options.hpp"
#include "random_generator/random_generator.hpp"

int main() {
  using individual::Individual;
  using namespace options;
  const Options options{get_data(), 64, 128, 3, 3};
  const int trials = 4;
  int trial = 0;
  const unsigned long hardware_threads = std::thread::hardware_concurrency();
  const unsigned long blocks = hardware_threads != 0 ? hardware_threads : 2;
  assert(trials % blocks == 0);
  std::vector<Individual> candidates;
  std::chrono::time_point<std::chrono::system_clock> start, end;
  // begin timing trials
  std::time_t time = std::time(nullptr);
  start = std::chrono::system_clock::now();
  // spawn trials number of threads in blocks
  for (unsigned long t = 0; t < trials / blocks; ++t) {
    std::vector<std::future<const Individual>> results;
    for (unsigned long i = 0; i < blocks; ++i)
      results.emplace_back(std::async(std::launch::async, algorithm::genetic, options, time, ++trial));
    // gather results
    for (std::future<const Individual> & result : results)
      candidates.emplace_back(result.get());
  }
  // end timing trials
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::vector<Individual>::iterator best = std::min_element(candidates.begin(), candidates.end(), algorithm::compare_fitness);
  std::cout << "Total elapsed time: " << elapsed_seconds.count() << "s\n"
	    << "Average time: " << elapsed_seconds.count() / trials << "s\n"
	    << "Best trial: " << time << "_"
	    << std::distance(candidates.begin(), best) + 1 << "\n"
	    << best->print();
}
