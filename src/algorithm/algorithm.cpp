/* algorithm.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for algorithm namespace
 */

#include <ctime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "algorithm.hpp"
#include "../individual/individual.hpp"
#include "../problem/problem.hpp"
#include "../random_generator/random_generator.hpp"

using namespace algorithm;
using namespace random_generator;

bool algorithm::compare_fitness(const Individual & a, const Individual & b) {
  return (std::isnan(a.get_fitness())) ? false : a.get_fitness() < b.get_fitness();
}

Individual algorithm::selection(const Problem & problem, const vector<Individual> & population) {
  int_dist dis(0, problem.population_size - 1); // closed interval
  vector<Individual> contestants;
  // get contestants
  for (int i = 0; i < problem.tournament_size; ++i)
    contestants.emplace_back(population[dis(rg.engine)]);
  return *std::min_element(contestants.begin(), contestants.end(), compare_fitness);
}

void algorithm::genetic(const Problem & problem) {
  // setup time and start log
  std::time_t t = std::time(nullptr);
  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::stringstream time_string;
  time_string << std::put_time(std::localtime(&t), "%y%m%d_%H%M%S");
  std::ofstream log("logs/" + time_string.str());
  log << "# running a Genetic Program on "
      << std::ctime(&t) << "\n";
  // TODO add sizes
  // start timing algorithm
  start = std::chrono::system_clock::now();

  // run algorithm
  vector<Individual> population;
  for (int i = 0; i < problem.population_size; ++i)
    population.emplace_back(Individual(problem));
  Individual best;
  for (int i = 0; i < problem.iterations; ++i) {
    // find Individual with lowest "fitness" AKA error from populaiton
    best = *std::min_element(population.begin(), population.end(), compare_fitness);
    log << best.get_fitness();
    // log average fitness
    double total_fitness = std::accumulate(population.begin(), population.end(), 0., [](const double & a, const Individual & b)->double const {return a + b.get_fitness();});
    log << "\t" << total_fitness / problem.population_size << "\n";
    // create replacement population
    vector<Individual> offspring;
    while (offspring.size() != population.size()) {
      // select parents
      vector<Individual> children;
      for (int j = 0; j < problem.crossover_size; ++j)
	children.emplace_back(selection(problem, population));
      // crossover with probability
      real_dist dis(0, 1);
      if (dis(rg.engine) < problem.crossover_chance)
	crossover(problem.internals_chance, children[0], children[1]);
      for (Individual & child : children) {
	// mutate children
	child.mutate(problem.mutate_chance, problem.constant_min, problem.constant_max);
	// update fitness and size
	child.evaluate(problem.values);
	// save children
	offspring.emplace_back(child);
      }
    }
    int_dist dis(0, problem.population_size - 1);
    // perform elitism
    for (int j = 0; j < problem.elitism_size; ++j)
      offspring[dis(rg.engine)] = best;
    population.swap(offspring);
  }
  // end timing algorithm
  end = std::chrono::system_clock::now();
  log << best.print_formula();
  // log duration
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::time_t end_time = std::chrono::system_clock::to_time_t(end);
  log << "Finished computation at " << std::ctime(&end_time)
      << "Elapsed time: " << elapsed_seconds.count() << "s\n";
  log.close();
}
