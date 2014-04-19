/* logging.cpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for logging namespace
 */

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "../individual/individual.hpp"
#include "../options/options.hpp"

namespace logging
{
  using individual::Individual;

  const unsigned int width(10);

  // Opens the appropriate log file for given time, trial, and folder.
  void
  open_log(std::ofstream& log, const std::time_t& time,
	   const unsigned int trial, const std::string& folder)
  {
    std::string filename = folder + std::to_string(time) + "_"
      + std::to_string(trial) + ".dat";

    log.open(filename, std::ios_base::app);
    if (not log.is_open())
      {
	std::cerr << "Log file " << filename << " could not be opened!\n";
	std::exit(EXIT_FAILURE);
      }
  }

  void start_log(std::ofstream& log, const std::time_t& time,
		 const options::Options& options)
  {
    using std::setw;
    log << "# running a Genetic Program @ "
	<< std::ctime(&time)
	<< "# generations: " << options.generations
	<< ", population size: " << options.pop_size
      	<< ", min depth: " << options.min_depth
	<< ", max depth: " << options.max_depth
      	<< ", depth limit: " << options.depth_limit
	<< ", tournament size: " << options.tourney_size
      	<< ", fitter size: " << options.fit_size
	<< ", crossover size: " << options.crossover_size
	<< ", elitism size: " << options.elitism_size
	<< ", fitness penalty: " << options.penalty << " * total size"
	<< ", grow chance: " << options.grow_chance
	<< ", over select chance: " << options.over_select_chance
	<< ", mutate chance: " << options.mutate_chance
	<< ", crossover chance: " << options.crossover_chance
      	<< ", internals chance: " << options.internals_chance
	<< std::left
	<< setw(width) << "\n# gen"
	<< setw(width) << "score"
	<< setw(width) << "best fit"
	<< setw(width) << "avg fit"
	<< setw(width) << "best size"
	<< setw(width) << "avg size"
	<< setw(width) << "best dep"
	<< setw(width) << "avg dep"
	<< std::endl;
    log.close();
  }

  /* Log a line of relevant algorithm information (best and average
     fitness and size plus adjusted best fitness). */
  void
  log_info(const unsigned int verbosity, const std::string& logs_dir,
	   const std::time_t& time, const unsigned int trial,
	   const unsigned int generation, const Individual& best,
	   const std::vector<Individual>& pop)
  {
    // Don't log if verbosity is zero, but still allow calls to this function.
    if (verbosity == 0)
      return;

    float total_fitness =
      std::accumulate(begin(pop), end(pop), 0.,
		      [](const float a, const Individual& b)
		      { return a + b.get_adjusted(); });

    unsigned int total_size =
      std::accumulate(begin(pop), end(pop), 0,
		      [](const unsigned int a, const Individual& b)
		      { return a + b.get_total(); });

    unsigned int total_depth =
      std::accumulate(begin(pop), end(pop), 0,
		      [](const unsigned int a, const Individual& b)
		      { return a + b.get_depth(); });

    std::ofstream log;
    using std::setw;
    open_log(log, time, trial, logs_dir);
    log << std::setprecision(4) << std::left
	<< setw(width) << generation
	<< setw(width) << best.get_score()
	<< setw(width) << best.get_adjusted()
	<< setw(width) << total_fitness / pop.size()
	<< setw(width) << best.get_total()
	<< setw(width) << static_cast<float>(total_size) / pop.size()
	<< setw(width) << best.get_depth()
	<< setw(width) << static_cast<float>(total_depth) / pop.size()
	<< std::endl;
    log.close();
  }
}
