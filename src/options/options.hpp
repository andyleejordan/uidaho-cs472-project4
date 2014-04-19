/* options.hpp - CS 472 Project #3: Genetic Programming
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
  enum class Cell { blank, food, marked };

  enum class Direction { north, south, east, west };

  struct Position
  {
    int x;
    int y;
    Direction direction;
    Position();
  };

  // toroidal map as vector of vector of bools
  class Map
  {
  public:
    Map();
    Map(const std::string&, int);
    bool active() const;
    bool look() const;
    void forward();
    void left();
    void right();
    int fitness() const;
    int max() const;
    std::string print() const;
    int max_ticks;

  private:
    int ticks;
    std::size_t width;
    std::size_t height;
    int score;
    int pieces;
    Position position;
    std::vector<std::vector<Cell>> rows;
  };

  // "singleton" struct with configured options for the algorithm
  // setup and returned by parse()
  struct Options
  {
    Map map;
    int trials;
    int generations;
    int pop_size;
    int min_depth;
    int max_depth;
    int depth_limit;
    int tourney_size;
    int fit_size;
    int brood_count;
    int crossover_size;
    int elitism_size;
    float penalty;
    float grow_chance;
    float over_select_chance;
    float mutate_chance;
    float crossover_chance;
    float internals_chance;
    std::string logs_dir;
    std::string plots_dir;
    int verbosity;
    void validate() const;
  };

  // given argc and argv, returns a finished and validated Options object
  const Options parse(int argc, char* argv[]);
}

#endif /* _OPTIONS_H_ */
