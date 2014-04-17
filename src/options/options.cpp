/* options.cpp - CS 472 Project #3: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for options namespace
 */

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/program_options.hpp>

#include "options.hpp"

namespace options
{
  Position::Position(): x{0}, y{0}, direction{Direction::east} {}

  Map::Map(): width{0}, height{0}, ticks{0}, max_ticks{0}, score{0}, pieces{0},
	      position{Position{}} {}

  Map::Map(std::string filename, unsigned int ticks):
    width{0}, height{0}, ticks{0}, max_ticks{ticks}, score{0}, pieces{0},
    position{Position{}}
  {
    // Try to open the given file.
    std::ifstream data_file{filename};
    if (not data_file.is_open())
      {
	std::cerr << "File " << filename << " could not be read!\n";
	std::exit(EXIT_FAILURE);
      }

    // Parse file into map structure
    std::string line;
    while (data_file >> line)
      {
	std::vector<Cell> row;
	if (width != 0) row.reserve(width);
	for (const char& c : line)
	  {
	    if (c != '.' and c != 'x')
	      {
		std::cerr << "File " << filename << " had bad cells!\n"
			  << "They were: " << c << std::endl;
		std::exit(EXIT_FAILURE);
	      }
	    else
	      {
		Cell cell = (c == 'x') ? Cell{Cell::food} : Cell{Cell::blank};
		row.emplace_back(cell);
		if (cell == Cell::food) ++pieces;
	      }
	  }
	if (width == 0) width = row.size(); // Get initial width
	// Verify all lines are same width
	else if (row.size() != width)
	  {
	    std::cerr << "File " << filename << " had uneven lines!\n"
		      << "The width is: " << width
		      << " and the line was " << row.size() << std::endl;
	    std::exit(EXIT_FAILURE);
	  }
	rows.emplace_back(row);
      }
    height = rows.size();
  }

  bool
  Map::active() const
  {
    return ticks < max_ticks;
  }

  bool
  Map::look() const
  {
    Position ahead = position;
    switch (position.direction)
      {
      case Direction::north:
	ahead.y = (position.y - 1) % height; break;
      case Direction::west:
	ahead.x = (position.x - 1) % width; break;
      case Direction::south:
	ahead.y = (position.y + 1) % height; break;
      case Direction::east:
	ahead.y = (position.x + 1) % width; break;
      }
    return rows[ahead.y][ahead.x] == Cell::food;
  }

  void
  Map::forward()
  {
    switch (position.direction)
      {
      case Direction::north:
	position.y = (position.y - 1) % height; break;
      case Direction::west:
	position.x = (position.x - 1) % width; break;
      case Direction::south:
	position.y = (position.y + 1) % height; break;
      case Direction::east:
	position.y = (position.x + 1) % width; break;
      }
    // Increment score if moved onto food
    if (rows[position.y][position.x] == Cell::food) ++score;

    // Mark location on map as visitied
    rows[position.y][position.x] = Cell::marked;

    ++ticks;
  }

  void
  Map::left()
  {
    switch (position.direction)
      {
      case Direction::north:
	position.direction = Direction::west; break;
      case Direction::west:
	position.direction = Direction::south; break;
      case Direction::south:
	position.direction = Direction::east; break;
      case Direction::east:
	position.direction = Direction::north; break;
      }
    ++ticks;
  }

  void
  Map::right()
  {
    switch (position.direction)
      {
      case Direction::north:
	position.direction = Direction::east; break;
      case Direction::east:
	position.direction = Direction::south; break;
      case Direction::south:
	position.direction = Direction::west; break;
      case Direction::west:
	position.direction = Direction::north; break;
      }
    ++ticks;
  }

  unsigned int
  Map::fitness() const
  {
    return score;
  }

  unsigned int
  Map::max() const
  {
    return pieces;
  }

  std::string Map::print() const
  {
    std::stringstream out;
    out << "# 'x' is food and 'o' is ant trail\n";
    for (const std::vector<Cell>& row : rows)
      {
	for (const Cell& cell : row)
	  {
	    // Add blank, food, and marked locations
	    switch (cell)
	      {
	      case Cell::blank: out << '.'; break;
	      case Cell::food: out << 'x'; break;
	      case Cell::marked: out << 'o'; break;
	      }
	  }
	// Add newline after each row
	out << '\n';
      }
    return out.str();
  }

  // Validates options parameters; should instead be unit tests.
  void
  Options::validate() const
  {
    assert(trials > 0);
    assert(generations > 0);
    assert(pop_size > 0);
    assert(tourney_size > 0 and tourney_size <= pop_size);
    assert(fitter_size > 0 and fitter_size <= pop_size);
    assert(crossover_size == 2 or crossover_size == 0);
    assert(elitism_size <= pop_size);
    assert(penalty >= 0 and penalty <= 1);
    assert(grow_chance >= 0 and grow_chance <= 1);
    assert(over_select_chance >= 0 and over_select_chance <= 1);
    assert(mutate_chance >= 0 and mutate_chance <= 1);
    assert(crossover_chance >= 0 and crossover_chance <= 1);
    assert(internals_chance >= 0 and internals_chance <= 1);
  }

  /* Given main's argc and argv, this will parse command-line options
     and the optional config file to return a built-up Options struct
     with all values, default or explicitly set. */
  const Options
  parse(int argc, char* argv[])
  {
    using std::string;
    using namespace boost::program_options;

    string filename;
    unsigned int ticks;
    Options options;

    positional_options_description positionals;
    variables_map variables_map;
    options_description description{"Allowed options"};

    // sets up all available CLI options
    description.add_options()
      ("help,h", "produce help message")

      ("config,c", value<string>()->
       default_value("search.cfg"),
       "specify the configuration file")

      ("file,f", value<string>(&filename)->
       default_value("test/santa-fe-trail.dat"),
       "specify the location of the columnized test data \"X Y\"")

      ("trials,t", value<unsigned int>(&options.trials)->
       default_value(4),
       "set the number of trials to run")

      ("generations,g",
       value<unsigned int>(&options.generations)->
       default_value(128),
       "set the number of iterations for which to run each trial")

      ("population,p",
       value<unsigned int>(&options.pop_size)->
       default_value(1024),
       "set the size of each population")

      ("min-depth", value<unsigned int>(&options.min_depth)->
       default_value(2),
       "set the minimum depth for initial populations")

      ("max-depth,d", value<unsigned int>(&options.max_depth)->
       default_value(6),
       "set the maximum depth for initial populations")

      ("depth-limit,l", value<unsigned int>(&options.depth_limit)->
       default_value(14),
       "set the depth limit for individuals")

      ("tournament-size,T",
       value<unsigned int>(&options.tourney_size)->
       default_value(3),
       "set the tournment size to adjust selection pressure")

      ("fitter-size,F",
       value<unsigned int>(&options.fitter_size)->
       default_value(320),
       "set the population size of fitter group")

      ("crossover-size",
       value<unsigned int>(&options.crossover_size)->
       default_value(2),
       "set the crossover size(binary in current implementation)")

      ("elitism-size,E", value<unsigned int>(&options.elitism_size)->
       default_value(3),
       "set the number of elitism replacements to make each iteration")

      ("ticks", value<unsigned int>(&ticks)->
       default_value(600),
       "set the number of moves the ant may move")

      ("penalty,P", value<float>(&options.penalty)->
       default_value(0),
       "set the constant scalar of the size penalty for fitness")

      ("grow-chance", value<float>(&options.grow_chance)->
       default_value(0.5),
       "set the probability that an initial tree will be made by the grow method")

      ("over-select-chance,O", value<float>(&options.over_select_chance)->
       default_value(0.8),
       "set the probability that a selection is made from the fitter group")

      ("mutate-chance,M", value<float>(&options.mutate_chance)->
       default_value(0.05),
       "set the probability that a single node will mutate")

      ("crossover-chance,C", value<float>(&options.crossover_chance)->
       default_value(0.8),
       "set the probability that crossover will be performed")

      ("internals-chance", value<float>(&options.internals_chance)->
       default_value(0.9),
       "set the probability that a crossover target node will be an internal node")

      ("logs", value<string>(&options.logs_dir)->
       default_value("logs/"),
       "set the save directory for log files")

      ("plots", value<string>(&options.plots_dir)->
       default_value("plots/"),
       "set the save directory for plot data files")

      ("verbosity,v", value<unsigned int>(&options.verbosity)->
       default_value(1),
       "set the verbosity: 0 - no logging; 1 - normal logging; 2 - debug output");

    /* Stores CLI and config file options. Will catch exception and
       exit with EXIT_FAILURE if given bad input. */
    try
      {
	store(parse_command_line(argc, argv, description), variables_map);

	std::ifstream config{variables_map["config"].as<string>()};
	if (config.is_open())
	  store(parse_config_file(config, description), variables_map);

	notify(variables_map);
      }
    catch (std::exception& e)
      {
	std::cerr << e.what() << std::endl;
	std::exit(EXIT_FAILURE);
      }

    // Print options help and exit with EXIT_SUCCESS when finished.
    if (variables_map.count("help"))
      {
	std::cout << "Genetic Program implemented in C++ by Andrew Schwartzmeyer\n"
		  << "Code located at https://github.com/andschwa/uidaho-cs472-project3\n\n"
		  << "Logs saved to <" << options.logs_dir << ">/<Unix time>.dat\n"
		  << "Ant maps saved to <" << options.plots_dir << ">/<Unix time>.dat\n"
		  << "GNUPlot PNG generation scripts in <tools>'\n\n"
		  << description << std::endl;
	std::exit(EXIT_SUCCESS);
      }

    // get values from given test file
    options.map = Map(filename, ticks);
    options.validate();

    return options;
  }
}
