/* algorithm.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for algorithm namespace
 */

#include <algorithm>
#include <cassert>
#include <ctime>
#include <iostream>
#include <fstream>
#include <future>
#include <thread>

#include "algorithm.hpp"
#include "../individual/individual.hpp"
#include "../options/options.hpp"
#include "../random_generator/random_generator.hpp"

namespace algorithm {
  using std::vector;
  using individual::Individual;
  using options::Options;
  using namespace random_generator;

  bool compare_fitness(const Individual & a, const Individual & b) {
    // returns true if a is closer to 0 than b and is normal
    return (std::isnormal(a.get_fitness())) ? a.get_fitness() < b.get_fitness() : false;
  }

  void open_log(std::ofstream & log, const std::time_t & time, const int & trial, const std::string & folder) {
    std::string file_name = folder + std::to_string(time) + "_" + std::to_string(trial) + ".dat";
    log.open(file_name, std::ios_base::app);
    if (not log.is_open()) {
      std::cerr << "Log file " << file_name << " could not be opened!\n";
      std::exit(EXIT_FAILURE);
    }
  }

  void log_info(const std::string & logs_dir, const std::time_t & time, const int & trial, const int & iteration, const Individual & best, const vector<Individual> & population) {
    double total_fitness = std::accumulate(population.begin(), population.end(), 0.,
					   [](const double & a, const Individual & b)->double const {return a + b.get_adjusted();});
    int total_size = std::accumulate(population.begin(), population.end(), 0,
				     [](const int & a, const Individual & b)->double const {return a + b.get_total();});
    std::ofstream log;
    open_log(log, time, trial, logs_dir);
    log << iteration << "\t"
	<< best.get_fitness() << "\t"
	<< best.get_adjusted() << "\t"
	<< total_fitness / population.size() << "\t"
	<< best.get_total() << "\t"
	<< total_size / population.size() << "\n";
    log.close();
  }

  vector<Individual> new_population(const Options & options) {
    vector<Individual> population;
    int_dist depth_dist{0, int(options.max_depth)}; // ramped
    real_dist dist{0, 1}; // half-and-half
    for (unsigned int i = 0; i < options.population_size; ++i)
      // pass options, random method (grow or full), and random max depth in given range
      population.emplace_back(Individual{options, individual::Method(dist(rg.engine) < options.grow_chance), depth_dist(rg.engine)});
    return population;
  }

  Individual selection(const int & size, const vector<Individual> & population) {
    int_dist dist{0, int(population.size()) - 1}; // closed interval
    vector<Individual> contestants;
    // get contestants
    for (int i = 0; i < size; ++i)
      contestants.emplace_back(population[dist(rg.engine)]);
    return *std::min_element(contestants.begin(), contestants.end(), compare_fitness);
  }

  const vector<Individual> get_children(const unsigned long & size, const vector<Individual> & population, const Options & options) {
    // select parents for children
    vector<Individual> nodes;
    while (nodes.size() != size) {
      Individual mother = selection(options.tournament_size, population);
      Individual father = selection(options.tournament_size, population);
      // crossover with probability
      real_dist dist{0, 1};
      if (dist(rg.engine) < options.crossover_chance)
	crossover(options.internals_chance, mother, father);
      // places mother and father in nodes
      nodes.emplace_back(mother);
      nodes.emplace_back(father);
    }
    for (Individual & child : nodes) {
      // mutate children
      child.mutate(options.mutate_chance);
      // update fitness (and size)
      child.evaluate(options.values, options.penalty);
    }
    return nodes;
  }

  vector<Individual> new_offspring(const Options & options, const vector<Individual> & population) {
    vector<Individual> offspring;
    offspring.reserve(population.size());
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long num_threads = hardware_threads != 0 ? hardware_threads : 2;
    assert(population.size() % num_threads == 0);
    const unsigned long block_size = population.size() / num_threads;
    vector<std::future<const vector<Individual>>> results;
    // spawn threads
    for (unsigned long i = 0; i < num_threads; ++i)
      results.emplace_back(std::async(std::launch::async, get_children, block_size, std::ref(population), options));
    // gather results
    for (std::future<const vector<Individual>> & result : results) {
      const vector<Individual> nodes = result.get();
      offspring.insert(offspring.end(), nodes.begin(), nodes.end());
    }
    assert(offspring.size() == population.size());
    return offspring;
  }

  const individual::Individual genetic(const Options & options, const std::time_t time, const int & trial) {
    // start log
    std::ofstream log;
    open_log(log, time, trial, options.logs_dir);
    log << "# running a Genetic Program @ "
	<< std::ctime(&time)
	<< "# initial depth: " << options.max_depth
	<< ", iterations: " << options.iterations
	<< ", population size: " << options.population_size
	<< ", tournament size: " << options.tournament_size << "\n"
	<< "# raw fitness - best (adjusted) fitness - average (adjusted) fitness - size of best - average size\n";
    log.close();
    // start timing algorithm
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    // create initial popuation
    vector<Individual> population = new_population(options);
    Individual best;
    // run algorithm to termination
    for (unsigned int iteration = 0; iteration < options.iterations; ++iteration) {
      // find Individual with lowest "fitness" AKA error from populaiton
      best = *std::min_element(population.begin(), population.end(), compare_fitness);
      auto log_thread = std::async(std::launch::async, log_info, options.verbosity, options.logs_dir, time, trial, iteration, best, population);
      // create replacement population
      vector<Individual> offspring = new_offspring(options, population);
      // perform elitism
      int_dist dist{0, int(options.population_size) - 1};
      for (unsigned int i = 0; i < options.elitism_size; ++i)
	offspring[dist(rg.engine)] = best;
      // replace current population with offspring
      population = offspring;
      log_thread.wait();
    }
    // end timing algorithm
    end = std::chrono::system_clock::now();
    // log duration
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    open_log(log, time, trial, options.logs_dir);
    log << "# finished computation @ " << std::ctime(&end_time)
	<< "# elapsed time: " << elapsed_seconds.count() << "s\n";
    log.close();
    std::ofstream plot;
    open_log(plot, time, trial, options.plots_dir);
    plot << best.evaluate(options.values, options.penalty, true);
    plot.close();
    return best;
  }
}
