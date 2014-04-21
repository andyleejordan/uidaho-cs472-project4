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

  // Prototypes
  vector<Individual>
  new_population(const Options& opts);

  vector<Individual>
  new_offspring(vector<Individual>& pop, int gen, const Options& opts);

  const Individual&
  select(int size, int start, int stop, const vector<Individual>& pop);

  void
  recombination(vector<Individual>& offspring, int gen, const Options& opts);

  // Definitions

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

    // Generate tournament size number of random indexes
    generate_n(back_inserter(group), size, [&pop, &dist]
	       { return dist(rg.engine); } );

    // Population is sorted, so choose lowest index of those generated
    return pop[*min_element(begin(group), end(group))];
  }

  void
  breed_pups(vector<Individual>::iterator& parent, int gen, const Options& opts)
  {
    // Brood selection, see Banzhaf section 6.5.1
    vector<Individual> brood;
    brood.reserve(opts.brood_count * opts.crossover_size);

    // Create N copies of the pair
    for (int i(0); i < opts.brood_count; ++i)
      {
	brood.push_back(*parent);
	brood.push_back(*next(parent));
      }

    // Crossover each pair of pups
    for (auto pup = begin(brood); pup != end(brood); advance(pup, 2))
      { crossover(opts.internals_chance, *pup, *next(pup)); }

    /* Evaluate pups with fewer ticks: minimum plus [0, 1] * 600
       where 0 is the first generation and 1 is the final
       generation (thus scaling the evaluation with the run's
       age). */
    options::Map map = opts.map;
    float scale = static_cast<float>(gen) / opts.generations;;
    int min = 0.1 * map.max_ticks;
    map.max_ticks = min + scale * (map.max_ticks - min);
    for (auto& pup : brood)
      { pup.evaluate(map, opts.penalty); }

    // Kill pups with too great a depth.
    auto remove = [&opts, &brood](const Individual& a)
      { return (a.get_depth() > opts.depth_limit); };
    brood.erase(remove_if(begin(brood), end(brood), remove), end(brood));

    // Replace parents with best pair of brood if available.
    sort(begin(brood), end(brood), compare_fitness());
    if (brood.size() >= 1) // Assign the first pup to the first parent
      { *parent = std::move(*begin(brood)); }
    if (brood.size() >= 2) // Assign the second pup to the second parent
      { *next(parent) = std::move(*next(begin(brood))); }
  }

  void
  recombination(vector<Individual>& offspring, int gen, const Options& opts)
  {
    bool_dist crossover_dist{opts.crossover_chance};
    for (auto iter = begin(offspring); iter != end(offspring); advance(iter, 2))
      {
	if (crossover_dist(rg.engine))
	  {
	    if (opts.brood_count == 0)
	      { crossover(opts.internals_chance, *iter, *next(iter)); }
	    else
	      { breed_pups(iter, gen, opts); }
	  }
      }
  }

  /* Return new offspring population.  The pop is not passed const as
     it must be sorted. */
  vector<Individual>
  new_offspring(vector<Individual>& pop, int gen, const Options& opts)
  {
    // Select parents for children.
    vector<Individual> offspring;
    offspring.reserve(opts.pop_size);

    /* Implements over-selection.  80% drawn from a fitter group of
       320, the other 20% drawn from the weaker group (past the first
       sorted 320).  See Eiben section 6.6. */
    sort(begin(pop), end(pop), compare_fitness());
    bool_dist select_dist(opts.over_select_chance);
    auto over_select = [&opts, &pop, &select_dist]
      {
	if (select_dist(rg.engine))
	  { return select(opts.tourney_size, 0, opts.fit_size, pop); }
	else
	  { return select(opts.tourney_size, opts.fit_size, opts.pop_size, pop); }
      };

    generate_n(back_inserter(offspring), opts.pop_size, over_select);

    // Binary crossover if enabled.
    if (opts.crossover_size == 2)
      { recombination(offspring, gen, opts); }

    // Mutate and evaluate children.
    bool_dist mutate_dist(opts.mutate_chance);
    for (auto& child : offspring)
      {
	if (mutate_dist(rg.engine))
	  { child.mutate(opts.min_depth, opts.max_depth, opts.grow_chance); }

	child.evaluate(opts.map, opts.penalty); // Evaluate all children
      }
    return offspring;
  }

  /* The actual genetic algorithm applied which (hopefully) produces a
     well-fit expression for a given dataset. */
  const individual::Individual
  genetic(const std::time_t& time, int trial, const Options& opts)
  {
    // Start logging
    std::ofstream log;
    if (opts.verbosity > 0)
      {
	logging::open_log(log, time, trial, opts.logs_dir);
	logging::start_log(log, time, opts);
      }

    // Begin timing algorithm.
    auto start = std::chrono::system_clock::now();

    // Create initial population.
    vector<Individual> pop = new_population(opts);
    Individual best;

    // Run algorithm to termination.
    for (int g(0); g < opts.generations; ++g)
      {
	// Find best Individual of current population.
	best = *min_element(begin(pop), end(pop), compare_fitness());

	// Launch background logging thread.
	auto log_thread =
	  async(std::launch::async, logging::log_info,
		opts.verbosity, opts.logs_dir, time, trial, g, best, pop);

	// Create replacement population.
	vector<Individual> offspring = new_offspring(pop, g, opts);

	// Perform elitism replacement of random individuals.
	int_dist dist{0, opts.pop_size - 1};
	for (int e(0); e < opts.elitism_size; ++e)
	  { offspring[dist(rg.engine)] = best; }

	// Replace current population with offspring.
	pop = move(offspring);

	// Sync with background logging thread.
	log_thread.wait();
      }

    // End timing algorithm.
    auto stop = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = stop - start;
    auto stop_time = std::chrono::system_clock::to_time_t(stop);

    // Log time information.
    if (opts.verbosity > 0)
      {
	logging::open_log(log, time, trial, opts.logs_dir);
	log << best.print() << best.print_formula()
	    << "# Finished computation @ " << ctime(&stop_time)
	    << "# Elapsed time: " << elapsed_seconds.count() << "s\n";
	log.close();
      }

    // Log evaluation plot data of best individual.
    std::ofstream plot;
    logging::open_log(plot, time, trial, opts.plots_dir);
    plot << best.evaluate(opts.map, opts.penalty, true);
    plot.close();

    return best;
  }
}
