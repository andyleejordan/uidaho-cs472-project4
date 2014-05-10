/* trials.cpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for the trials namespace
 */

#include <algorithm>
#include <ctime>
#include <fstream>
#include <future>
#include <iomanip>
#include <tuple>

#include "trials.hpp"
#include "../individual/individual.hpp"
#include "../logging/logging.hpp"
#include "../options/options.hpp"

// Forward declaration of algorithm::genetic
namespace algorithm
{
  const result_t
  genetic(const std::time_t&, int, const options::Options&);
}

namespace trials
{
  using individual::Individual;


  /* Delegate for run.  Given time, trial number, total trials, vector
     of candidate Individual solutions, and options, spawn the
     numbered trials in asynchronous threads, pushing results to
     candidates. */
  void
  push_results(const std::time_t&, int&, int, std::vector<algorithm::result_t>&,
	       const options::Options&);

  /* Given time and options, spawn trials in blocks of the appropriate
     size for hardware (determined at runtime, defaulting to two), and
     spawn any remaining trials.  Return the best Individual result
     from the trials along with its trial number as a tuple. */
  const std::tuple<int, algorithm::result_t>
  run(const std::time_t& time, const options::Options& opts)
  {
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long blocks = (hardware_threads != 0) ? hardware_threads : 2;

    int trial = 0;
    std::vector<algorithm::result_t> candidates;

    // Run the genetic algorithm (program).
    if (opts.trials == 1) // Spawn single non-threaded trial.
      { candidates.push_back(algorithm::genetic(time, trial, opts)); }
    else // Spawn trials in separate threads.
      {
	// Spawn trials in chunks of size blocks.
	for (unsigned long t = 0; t < opts.trials / blocks; ++t)
	  { push_results(time, trial, blocks, candidates, opts); }

	// Spawn remaining trials.
	push_results(time, trial, opts.trials % blocks, candidates, opts);
      }

    // Log trials
    std::ofstream log;
    logging::open_log(log, time, 0, opts.logs_dir);
    using std::setw;
    using std::endl;
    const int width{12};
    log << setw(width) << "# Score" << setw(width) << "Time" << endl;
    int score_sum{0};
    float time_sum{0};
    for (const auto& result : candidates)
      {
	Individual solution;
	std::chrono::duration<double> elapsed_time;
	std::tie(solution, elapsed_time) = result;
	score_sum += solution.get_score();
	time_sum += elapsed_time.count();

	log << setw(width) << solution.get_score()
	    << setw(width) << elapsed_time.count()
	    << endl;
      }
    log << setw(width) << "# " << score_sum / opts.trials
	<< setw(width) << time_sum / opts.trials
	<< setw(width) << (score_sum / time_sum) / opts.trials
	<< endl;
    log.close();

    // Retrieve best element.
    auto compare = [](const algorithm::result_t& a, const algorithm::result_t& b)
      { return std::get<0>(a).get_score() > std::get<0>(b).get_score(); };
    auto best = min_element(begin(candidates), end(candidates), compare);

    /* Get which trial was best.  Filenames are not zero-indexed so
       increment by one. */
    return std::make_tuple(distance(begin(candidates), best) + 1, *best);
  }

  void
  push_results(const std::time_t& time, int& trial, int trials,
	       std::vector<algorithm::result_t>& candidates,
	       const options::Options& opts)
  {
    // Spawn blocks number of async threads.
    std::vector<std::future<const algorithm::result_t>> results;
    results.reserve(trials);

    auto task = [&time, &trial, &opts]() mutable
      { return async(std::launch::async,
		     algorithm::genetic, time, ++trial, opts); };

    generate_n(back_inserter(results), trials, task);

    // Gather future results.
    for (auto& result : results)
      { candidates.push_back(result.get()); }
  }
}
