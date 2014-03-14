/* algorithm.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for algorithm namespace
 */

#include <ctime>
#include <iostream>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <sstream>
#include <thread>

#include "algorithm.hpp"
#include "../individual/individual.hpp"
#include "../problem/problem.hpp"
#include "../random_generator/random_generator.hpp"

namespace algorithm {
  using std::vector;
  using individual::Individual;
  using problem::Problem;
  using namespace random_generator;

  bool compare_fitness(const Individual & a, const Individual & b) {
    return (std::isnan(a.get_fitness())) ? false : a.get_fitness() < b.get_fitness();
  }

  std::ofstream open_log(const std::time_t & time) {
    std::stringstream time_string;
    time_string << std::put_time(std::localtime(&time), "%y%m%d_%H%M%S");
    std::ofstream log("logs/" + time_string.str(), std::ios_base::app);
    if (!log.is_open()) {
      std::cerr << "Log file logs/" << time_string.str() << " could not be opened!";
      std::exit(EXIT_FAILURE);
    }
    return log;
  }

  void log_info(const std::time_t & time, const Individual & best, const vector<Individual> & population) {
    double total_fitness = std::accumulate(population.begin(), population.end(), 0.,
					   [](const double & a, const Individual & b)->double const {return a + b.get_fitness();});
    int total_size = std::accumulate(population.begin(), population.end(), 0,
				     [](const int & a, const Individual & b)->double const {return a + b.get_total();});
    std::ofstream log = open_log(time);
    log << best.get_fitness() << "\t"
	<< total_fitness / population.size() << "\t"
	<< best.get_total() << "\t"
	<< total_size / population.size() << "\n";
    log.close();
  }

  vector<Individual> new_population(const Problem & problem) {
    vector<Individual> population;
    for (int i = 0; i < problem.population_size; ++i)
      population.emplace_back(Individual(problem));
    return population;
  }

  Individual selection(const Problem & problem, const vector<Individual> & population) {
    int_dist dis(0, problem.population_size - 1); // closed interval
    vector<Individual> contestants;
    // get contestants
    for (int i = 0; i < problem.tournament_size; ++i)
      contestants.emplace_back(population[dis(rg.engine)]);
    return *std::min_element(contestants.begin(), contestants.end(), compare_fitness);
  }

  const vector<Individual> get_children(const unsigned long & size, const vector<Individual> & population, const Problem & problem) {
    // select parents for children
    vector<Individual> nodes;
    while (nodes.size() != size) {
      Individual mother = selection(problem, population);
      Individual father = selection(problem, population);
      // crossover with probability
      real_dist dis(0, 1);
      if (dis(rg.engine) < problem.crossover_chance)
	crossover(problem.internals_chance, mother, father);
      // places mother and father in nodes
      nodes.emplace_back(mother);
      nodes.emplace_back(father);
    }
    for (Individual & child : nodes) {
      // mutate children
      child.mutate(problem.mutate_chance, problem.constant_min, problem.constant_max);
      // update fitness (and size)
      child.evaluate(problem.values);
    }
    return nodes;
  }

  vector<Individual> new_offspring(const Problem & problem, const vector<Individual> & population) {
    vector<Individual> offspring;
    offspring.reserve(population.size());
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long num_threads = hardware_threads != 0 ? hardware_threads : 2;
    const unsigned long block_size = population.size() / num_threads;
    vector<std::future<const vector<Individual>>> results;
    // spawn threads
    for (unsigned long i = 0; i < num_threads; ++i)
      results.emplace_back(std::async(std::launch::async, get_children, block_size, std::ref(population), problem));
    // gather results
    for (std::future<const vector<Individual>> & result : results) {
      const vector<Individual> nodes = result.get();
      offspring.insert(offspring.end(), nodes.begin(), nodes.end());
    }
    return offspring;
  }

  const individual::Individual genetic(const problem::Problem & problem) {
    // setup time and start log
    std::time_t time = std::time(nullptr);
    std::ofstream log = open_log(time);
    log << "# running a Genetic Program @ "
	<< std::ctime(&time)
	<< "# initial depth: " << problem.max_depth
	<< ", iterations: " << problem.iterations
	<< ", population size: " << problem.population_size
	<< ", tournament size: " << problem.tournament_size << "\n"
	<< "# best fitness - average fitness - size of best - average size\n";
    log.close();
    // start timing algorithm
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    // create initial popuation
    vector<Individual> population = new_population(problem);
    Individual best;
    // run algorithm to termination
    for (int iteration = 0; iteration < problem.iterations; ++iteration) {
      // find Individual with lowest "fitness" AKA error from populaiton
      best = *std::min_element(population.begin(), population.end(), compare_fitness);
      std::thread log_thread(log_info, time, best, population);
      // create replacement population
      vector<Individual> offspring = new_offspring(problem, population);
      // perform elitism
      int_dist dis(0, problem.population_size - 1);
      for (int i = 0; i < problem.elitism_size; ++i)
	offspring[dis(rg.engine)] = best;
      // replace current population with offspring
      population = offspring;
      log_thread.join();
    }
    // end timing algorithm
    end = std::chrono::system_clock::now();
    // log duration
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::ofstream log_after = open_log(time);
    log_after << "# finished computation @ " << std::ctime(&end_time)
	      << "# elapsed time: " << elapsed_seconds.count() << "s\n";
    log_after.close();
    return best;
  }
}
