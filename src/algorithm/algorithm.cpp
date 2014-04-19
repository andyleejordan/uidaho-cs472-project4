/* algorithm.cpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for algorithm namespace
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ctime>
#include <fstream>
#include <future>
#include <thread>

#include "algorithm.hpp"
#include "../individual/individual.hpp"
#include "../logging/logging.hpp"
#include "../options/options.hpp"
#include "../random_generator/random_generator.hpp"

namespace algorithm
{
  using std::vector;
  using individual::Individual;
  using options::Options;
  using namespace random_generator;

  // Returns true if "a" is ordered before "b", i.e. more fit
  bool
  compare_fitness::operator()(const Individual& a, const Individual& b)
  {
    return std::isnormal(a.get_fitness())
      ? (a.get_fitness() > b.get_fitness()) : false;
  }

  /* Create an initial population using "ramped half-and-half" (half
     full trees, half randomly grown trees, all to random depths
     between 0 and maximum depth). */
  vector<Individual>
  new_population(const Options& opts)
  {
    vector<Individual> pop;
    pop.reserve(opts.pop_size);

    generate_n(back_inserter(pop), opts.pop_size, [&opts]
	       { return Individual{opts}; });

    return pop;
  }

  /* Return best candidate from size number of contestants randomly
     drawn from population.  Assume population is sorted. */
  const Individual&
  select(int size, int start, int stop, const vector<Individual>& pop)
  {
    int_dist dist{start, stop - 1}; // closed interval
    vector<unsigned int> group;
    group.reserve(size);

    generate_n(back_inserter(group), size, [&dist, &pop]() mutable
	       { return dist(rg.engine); } );
    // Population is sorted, so choose lowest index of random three
    return pop[*min_element(begin(group), end(group))];
  }

  void crossover(const Options& opts, vector<Individual>& offspring)
  {
    real_dist dist{0, 1};
    for (auto iter = begin(offspring); iter != end(offspring); advance(iter, 2))
      if (dist(rg.engine) < opts.crossover_chance)
	{
	  /* Copy the pair, crossover N times, evaluate with
	     ticks=100, replace with best two. */
	  vector<Individual> brood;
	  brood.reserve(opts.brood_count * opts.crossover_size);
	  // Create N copies of the pair
	  for (unsigned int i(0); i < opts.brood_count; ++i)
	    {
	      brood.push_back(*iter);
	      brood.push_back(*next(iter));
	    }
	  // Crossover each pair of pups
	  for (auto pup = begin(brood); pup != end(brood); advance(pup, 2))
	    crossover(opts.internals_chance, *pup, *next(pup));
	  // Evaluate pups with fewer ticks
	  options::Map map = opts.map;
	  map.max_ticks = 60;
	  for (auto& pup : brood)
	    pup.evaluate(map, opts.penalty);
	  sort(begin(brood), end(brood), compare_fitness());
	  // Replace parents with best pair of brood
	  *iter = *begin(brood);
	  *next(iter) = *next(begin(brood));
	}
  }

  /* Return new offspring population.  The pop is not passed const as
     it must be sorted. */
  vector<Individual>
  new_offspring(vector<Individual>& pop, const Options& opts)
  {
    // Select parents for children.
    vector<Individual> offspring;
    offspring.reserve(pop.size());

    sort(begin(pop), end(pop), compare_fitness());

    /* Implements over-selection.  80% drawn from a fitter group of
       320, the other 20% drawn from the weaker group (past the first
       sorted 320). */
    unsigned int strong_size = opts.over_select_chance * opts.pop_size;
    generate_n(back_inserter(offspring), strong_size, [&opts, &pop]
	       { return select(opts.tourney_size, 0, opts.fitter_size, pop); });

    unsigned int weak_size = opts.pop_size - strong_size;
    generate_n(back_inserter(offspring), weak_size, [&opts, &pop]
	       { return select(opts.tourney_size, opts.fitter_size, pop.size(), pop); });
    // Binary crossover if enabled.
    if (opts.crossover_size == 2) crossover(opts, offspring);

    // Mutate and evaluate children.
    real_dist dist{0, 1};
    for (auto& child : offspring)
      {
	// Mutate children conditionally.
	if (dist(rg.engine) < opts.mutate_chance)
	  child.mutate(opts.min_depth, opts.max_depth, opts.grow_chance);
	// Evaluate all children
	child.evaluate(opts.map, opts.penalty); // Evaluate children
      }
    return offspring;
  }

  /* The actual genetic algorithm applied which (hopefully) produces a
     well-fit expression for a given dataset. */
  const individual::Individual
  genetic(const std::time_t& time, const unsigned int trial,
	  const Options& options)
  {
    // Start logging
    std::ofstream log;
    if (options.verbosity > 0)
      {
	logging::open_log(log, time, trial, options.logs_dir);
	logging::start_log(log, time, options);
      }
    // Begin timing algorithm.
    auto start = std::chrono::system_clock::now();

    // Create initial population.
    vector<Individual> pop = new_population(options);
    Individual best;

    // Run algorithm to termination.
    for (unsigned int g(0); g < options.generations; ++g)
      {
	// Find best Individual of current population.
	best = *min_element(begin(pop), end(pop), compare_fitness());

	// Launch background logging thread.
	auto log_thread = async(std::launch::async, logging::log_info,
				options.verbosity, options.logs_dir, time,
				trial, g, best, pop);

	// Create replacement population.
	vector<Individual> offspring = new_offspring(pop, options);

	// Perform elitism replacement of random individuals.
	int_dist dist{0, static_cast<int>(options.pop_size) - 1};
	for (unsigned int e(0); e < options.elitism_size; ++e)
	  offspring[dist(rg.engine)] = best;

	// Replace current population with offspring.
	pop = move(offspring);

	// Sync with background logging thread.
	log_thread.wait();
      }
    // End timing algorithm.
    auto stop = std::chrono::system_clock::now();
    auto elapsed_seconds = stop - start;
    auto stop_time = std::chrono::system_clock::to_time_t(stop);

    // Log time information.
    if (options.verbosity > 0)
      {
	logging::open_log(log, time, trial, options.logs_dir);
	log << best.print() << best.print_formula()
	    << "# Finished computation @ " << ctime(&stop_time)
	    << "# Elapsed time: " << elapsed_seconds.count() << "s\n";
	log.close();
      }
    // Log evaluation plot data of best individual.
    std::ofstream plot;
    logging::open_log(plot, time, trial, options.plots_dir);
    plot << best.evaluate(options.map, options.penalty, true);
    plot.close();

    return best;
  }
}
