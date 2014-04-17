/* main.cpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 */

#include <cstdlib>
#include <chrono>
#include <ctime>
#include <iostream>
#include <tuple>

#include "algorithm/algorithm.hpp"
#include "individual/individual.hpp"
#include "options/options.hpp"
#include "random_generator/random_generator.hpp"
#include "trials/trials.hpp"

int
main(int argc, char* argv[])
{
  // Retrieve program options.
  const options::Options options = options::parse(argc, argv);

  // Chrono start, end, and Unix time variables.
  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::time_t time = std::time(nullptr);

  // Begin timing trials.
  start = std::chrono::system_clock::now();

  // Run trials and save best Individual.
  const std::tuple<int, individual::Individual> best =
    trials::run(time, options);

  // End timing trials.
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;

  // Print total time info and which trial was best.
  std::cout << "Total elapsed time: " << elapsed_seconds.count() << "s\n"
	    << "Average time: " << elapsed_seconds.count() / options.trials
	    << "s\nBest trial: " << time << "_"
	    << std::get<0>(best) << "\n"
	    << std::get<1>(best).print();

  return EXIT_SUCCESS;
}
