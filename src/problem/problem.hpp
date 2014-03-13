/* problem.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for struct definition of a problem space
 */

#ifndef _PROBLEM_H_
#define _PROBLEM_H_

#include <string>
#include <tuple>
#include <vector>

namespace problem {
  typedef std::vector<const std::tuple<const double, const double>> pairs;
  
  struct Problem {
    const pairs values;
    const int max_depth;
    const double constant_min;
    const double constant_max;
    const double mutate_chance;
    const double crossover_chance;
    Problem(const pairs & v,
	    const int & depth = 10,
	    const double & min = 0,
	    const double & max = 10,
	    const double & mutate = 0.2,
	    const double & crossover = 0.8 ):
      values(v), max_depth(depth), constant_min(min), constant_max(max),
      mutate_chance(mutate), crossover_chance(crossover) {}
  };

  const pairs get_data(std::string file_name = "test/cs472.dat");
}

#endif /* _PROBLEM_H_ */
