/* logging.hpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Interface file for logging namespace
 */

#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <string>
#include <ctime>
#include <vector>

// Forward declarations
namespace individual { class Individual; }
namespace options { struct Options; }

namespace logging
{
  // Opens the appropriate log file for given time, trial, and folder.
  void
  open_log(std::ofstream&, const std::time_t&, int, const std::string&);

  // Logs initial parameters from options object.
  void
  start_log(std::ofstream&, const std::time_t&, const options::Options&);

  // Logs info about current population.
  void
  log_info(int, const std::string&, const std::time_t&, int, int,
	   const individual::Individual&,
	   const std::vector<individual::Individual>&);
}

#endif /* _LOGGING_H_ */
