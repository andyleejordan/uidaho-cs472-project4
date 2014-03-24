/* main.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 */

#include <chrono>
#include <ctime>
#include <iostream>
#include <tuple>

#include "algorithm/algorithm.hpp"
#include "individual/individual.hpp"
#include "options/options.hpp"
#include "random_generator/random_generator.hpp"
#include "trials/trials.hpp"

int main() {
  const options::Options options{options::get_data(), 64, 128, 3, 3};
  const int trials = 4;
  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::time_t time = std::time(nullptr);
  // begin timing trials
  start = std::chrono::system_clock::now();
  const std::tuple<int, individual::Individual> best = trials::run(options, trials, time);
  // end timing trials
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "Total elapsed time: " << elapsed_seconds.count() << "s\n"
	    << "Average time: " << elapsed_seconds.count() / trials << "s\n"
	    << "Best trial: " << time << "_"
	    << std::get<0>(best) << "\n"
	    << std::get<1>(best).print();
}
