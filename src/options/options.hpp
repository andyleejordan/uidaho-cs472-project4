/* options.hpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * header file for options namespace
 * provides program options and gathers test data
 */

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <string>
#include <tuple>
#include <vector>

namespace options
{
  enum class Cell {blank, food, marked};

  // toroidal map as vector of vector of bools
  class Map
  {
  public:
    Map();
    Map(const std::string&);
    void reset(const Map&);
    void left();
    void right();
    bool forward();
    bool look();
    std::string print();
  private:
    std::vector<std::vector<Cell>> map;
    void move(unsigned int, unsigned int);
    unsigned int width;
    unsigned int height;
    unsigned int x;
    unsigned int y;
  };

  // "singleton" struct with configured options for the algorithm
  // setup and returned by parse()
  struct Options
  {
    Map map;
    unsigned int trials;
    unsigned int iterations;
    unsigned int population_size;
    unsigned int max_depth;
    unsigned int tournament_size;
    unsigned int crossover_size;
    unsigned int elitism_size;
    double penalty;
    double grow_chance;
    double mutate_chance;
    double crossover_chance;
    double internals_chance;
    std::string logs_dir;
    std::string plots_dir;
    unsigned int verbosity;
    void validate() const;
  };

  // given argc and argv, returns a finished and validated Options object
  const Options parse(int argc, char* argv[]);
}

#endif /* _OPTIONS_H_ */
