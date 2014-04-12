/* options.cpp - CS 472 Project #2: Genetic Programming
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

  Map::Map(const std::string& filename): width{0}, height{0}, ticks{0},
					 score{0}, position{Position{}}
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
	for (const char& c : line)
	  {
	    if (c != ' ' or c != 'x')
	      {
		std::cerr << "File " << filename << " had bad cells!\n";
		std::exit(EXIT_FAILURE);
	      }
	    else
	      row.emplace_back((c == ' ')
			       ? Cell{Cell::blank} : Cell{Cell::food});
	  }
	if (width == 0) width = row.size(); // Get initial width
	else if (row.size() != width)
	  {
	    // Verify all lines are same width
	    std::cerr << "File " << filename << " had uneven lines!\n";
	    std::exit(EXIT_FAILURE);
	  }
	else
	  rows.emplace_back(row);
      }
    height = rows.size();
    std::cout << "#Grid is " << width << " wide and " << height << " high\n."
	      << print();
  }

  bool
  Map::active() const
    {
      return ticks < 600; // TODO: implement options.ticks
    }

  std::string Map::print()
    {
      std::stringstream out;
      for (const auto& row : map)
	{
	  for (const Cell& cell : row)
	    {
	      // Add blank, food, and marked locations
	      if (cell == Cell::blank) out << ' ';
	      else if (cell == Cell::food) out << 'x';
	      else if (cell == Cell::marked) out << '*';
	      else
		{
		  std::cerr << "Map is malformed!\n";
		  std::exit(EXIT_FAILURE);
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
    assert(iterations > 0);
    assert(population_size > 0);
    assert(tournament_size > 0 and tournament_size <= population_size);
    assert(crossover_size == 2);
    assert(elitism_size <= population_size);
    assert(grow_chance >= 0 and grow_chance <= 1);
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
       default_value("test/cs472.dat"),
       "specify the location of the columnized test data \"X Y\"")

      ("trials,t", value<unsigned int>(&options.trials)->
       default_value(4),
       "set the number of trials to run")

      ("iterations,i",
       value<unsigned int>(&options.iterations)->
       default_value(128),
       "set the number of iterations for which to run each trial")

      ("population,p",
       value<unsigned int>(&options.population_size)->
       default_value(128),
       "set the size of each population")

      ("depth,d", value<unsigned int>(&options.max_depth)->
       default_value(4),
       "set the maximum depth for initial populations")

      ("tournament,T",
       value<unsigned int>(&options.tournament_size)->
       default_value(3),
       "set the tournment size to adjust selection pressure")

      ("crossover,C",
       value<unsigned int>(&options.crossover_size)->
       default_value(2),
       "set the crossover size(binary in current implementation)")

      ("elitism,e", value<unsigned int>(&options.elitism_size)->
       default_value(2),
       "set the number of elitism replacements to make each iteration")

      ("penalty,P", value<double>(&options.penalty)->
       default_value(0.1),
       "set the constant scalar of the size penalty for fitness")

      ("grow,g", value<double>(&options.grow_chance)->
       default_value(0.5),
       "set the probability that an initial tree will be made by the grow method")

      ("mutate,m", value<double>(&options.mutate_chance)->
       default_value(0.01),
       "set the probability that a single node will mutate")

      ("crossover_chance", value<double>(&options.crossover_chance)->
       default_value(0.8),
       "set the probability that a selected pair of invididuals will undergo crossover")

      ("internals,I", value<double>(&options.internals_chance)->
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
	exit(EXIT_FAILURE);
      }

    // Print options help and exit with EXIT_SUCCESS when finished.
    if (variables_map.count("help"))
      {
	std::cout << "Genetic Program implemented in C++ by Andrew Schwartzmeyer\n"
		  << "Code located at https://github.com/andschwa/uidaho-cs472-project2\n\n"
		  << "Logs saved to <logs>/<Unix time>.dat\n"
		  << "Plot data saved to <plots>/<Unix time>.dat\n"
		  << "GNUPlot PNG generation script './plot <plots>'\n\n"
		  << description << std::endl;
	exit(EXIT_SUCCESS);
      }

    // get values from given test file
    options.map = Map(filename);
    options.validate();

    return options;
  }
}
