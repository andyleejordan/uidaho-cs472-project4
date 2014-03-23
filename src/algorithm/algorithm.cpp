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
#include "../problem/problem.hpp"
#include "../random_generator/random_generator.hpp"

namespace algorithm {
  using std::vector;
  using individual::Individual;
  using problem::Problem;
  using namespace random_generator;

  bool compare_fitness(const Individual & a, const Individual & b) {
    // returns true if a is closer to 0 than b and is normal
    return (std::isnormal(a.get_fitness())) ? a.get_fitness() < b.get_fitness() : false;
  }

  void open_log(std::ofstream & log, const std::time_t & time, const int & trial, const std::string & folder = "logs/") {
    std::string file_name = folder + std::to_string(time) + "_" + std::to_string(trial) + ".dat";
    log.open(file_name, std::ios_base::app);
    if (not log.is_open()) {
      std::cerr << "Log file " << file_name << " could not be opened!";
      std::exit(EXIT_FAILURE);
    }
  }

  void log_info(const std::time_t & time, const int & trial, const int & iteration, const Individual & best, const vector<Individual> & population) {
    double total_fitness = std::accumulate(population.begin(), population.end(), 0.,
					   [](const double & a, const Individual & b)->double const {return a + b.get_adjusted();});
    int total_size = std::accumulate(population.begin(), population.end(), 0,
				     [](const int & a, const Individual & b)->double const {return a + b.get_total();});
    std::ofstream log;
    open_log(log, time, trial);
    log << iteration << "\t"
	<< best.get_fitness() << "\t"
	<< best.get_adjusted() << "\t"
	<< total_fitness / population.size() << "\t"
	<< best.get_total() << "\t"
	<< total_size / population.size() << "\n";
    log.close();
  }

  vector<Individual> new_population(const Problem & problem) {
    vector<Individual> population;
    int_dist depth_dist{0, problem.max_depth}; // ramped
    real_dist dist{0, 1}; // half-and-half
    for (int i = 0; i < problem.population_size; ++i)
      // pass problem, random method (grow or full), and random max depth in given range
      population.emplace_back(Individual{problem, individual::Method(dist(rg.engine) < problem.grow_chance), depth_dist(rg.engine)});
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

  const vector<Individual> get_children(const unsigned long & size, const vector<Individual> & population, const Problem & problem) {
    // select parents for children
    vector<Individual> nodes;
    while (nodes.size() != size) {
      Individual mother = selection(problem.tournament_size, population);
      Individual father = selection(problem.tournament_size, population);
      // crossover with probability
      real_dist dist{0, 1};
      if (dist(rg.engine) < problem.crossover_chance)
	crossover(problem.internals_chance, mother, father);
      // places mother and father in nodes
      nodes.emplace_back(mother);
      nodes.emplace_back(father);
    }
    for (Individual & child : nodes) {
      // mutate children
      child.mutate(problem.mutate_chance);
      // update fitness (and size)
      child.evaluate(problem.values, problem.penalty);
    }
    return nodes;
  }

  vector<Individual> new_offspring(const Problem & problem, const vector<Individual> & population) {
    vector<Individual> offspring;
    offspring.reserve(population.size());
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long num_threads = hardware_threads != 0 ? hardware_threads : 2;
    assert(population.size() % num_threads == 0);
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
    assert(offspring.size() == population.size());
    return offspring;
  }

  const individual::Individual genetic(const Problem & problem, const std::time_t time, const int & trial) {
    // start log
    std::ofstream log;
    open_log(log, time, trial);
    log << "# running a Genetic Program @ "
	<< std::ctime(&time)
	<< "# initial depth: " << problem.max_depth
	<< ", iterations: " << problem.iterations
	<< ", population size: " << problem.population_size
	<< ", tournament size: " << problem.tournament_size << "\n"
	<< "# raw fitness - best (adjusted) fitness - average (adjusted) fitness - size of best - average size\n";
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
      auto log_thread = std::async(std::launch::async, log_info, time, trial, iteration, best, population);
      // create replacement population
      vector<Individual> offspring = new_offspring(problem, population);
      // perform elitism
      int_dist dist{0, problem.population_size - 1};
      for (int i = 0; i < problem.elitism_size; ++i)
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
    open_log(log, time, trial);
    log << "# finished computation @ " << std::ctime(&end_time)
	<< "# elapsed time: " << elapsed_seconds.count() << "s\n";
    log.close();
    std::ofstream plot;
    open_log(plot, time, trial, "logs/plots/");
    plot << best.evaluate(problem.values, true);
    plot.close();
    return best;
  }
}
