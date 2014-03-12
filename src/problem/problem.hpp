/* problem.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Header file for struct definition of a problem space
 */

#ifndef _PROBLEM_H_
#define _PROBLEM_H_

#include <vector>
#include <tuple>

namespace problem {
  typedef std::vector<const std::tuple<const double, const double>> pairs;
  
  struct Problem {
    const pairs values;
    const int max_depth;
    const double constant_min;
    const double constant_max;
    Problem(const pairs & v,
	    const int & depth = 10,
	    const double & min = 0,
	    const double & max = 10):
      values(v), max_depth(depth), constant_min(min), constant_max(max) {}
  };
}

#endif /* _PROBLEM_H_ */
