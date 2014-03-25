/* options.cpp - CS 472 Project #2: Genetic Programming
 * Copyright 2014 Andrew Schwartzmeyer
 *
 * Source file for options namespace
 */

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

#include "options.hpp"

namespace options {
  // reads in columnar X Y data from file, creates a vector of pairs
  const pairs get_data(std::string file_name) {
    std::ifstream data_file{file_name};
    if (!data_file.is_open()) {
      std::cerr << "Data file " << file_name << " could not be read!" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    pairs values;
    while (!data_file.eof()) {
      // fill values vector with (x, y) pairs
      int x;
      double y;
      data_file >> x >> y;
      values.emplace_back(std::make_tuple(x, y));
    }
    if (!values.size()) {
      std::cerr << "Data file " << file_name << " was empty!" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    return values;
  }

  void Options::validate() const {
    assert(values.size() > 0);
    assert(trials > 0);
    assert(iterations > 0);
    assert(population_size > 0);
    assert(tournament_size > 0 and tournament_size <= population_size);
    assert(crossover_size == 2);
    assert(elitism_size <= population_size);
    assert(constant_min < constant_max);
    assert(grow_chance >= 0 and grow_chance <= 1);
    assert(mutate_chance >= 0 and mutate_chance <= 1);
    assert(crossover_chance >= 0 and crossover_chance <= 1);
    assert(internals_chance >= 0 and internals_chance <= 1);
  }

  const Options parse(int argc, char** argv) {
    namespace po = boost::program_options;
    std::string test_file;
    Options options;
    po::positional_options_description positionals;
    po::variables_map variables_map;
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "produce help message")
      ("file,f", po::value<std::string>(&test_file)->default_value("test/cs472.dat"),
      ("config,c", po::value<string>()->default_value("search.cfg"),
       "specify the configuration file")
       "specify the location of the columnized test data \"X Y\"")
      ("trials,t", po::value<unsigned int>(&options.trials)->default_value(4),
       "set the number of trials to run")
      ("iterations,i", po::value<unsigned int>(&options.iterations)->default_value(128),
       "set the number of iterations for which to run each trial")
      ("population,p", po::value<unsigned int>(&options.population_size)->default_value(128),
       "set the size of each population")
      ("depth,d", po::value<unsigned int>(&options.max_depth)->default_value(4),
       "set the maximum depth for initial populations")
      ("tournament,T", po::value<unsigned int>(&options.tournament_size)->default_value(3),
       "set the tournment size to adjust selection pressure")
      ("crossover,c", po::value<unsigned int>(&options.crossover_size)->default_value(2),
       "set the crossover size (binary in current implementation)")
      ("elitism,e", po::value<unsigned int>(&options.elitism_size)->default_value(2),
       "set the number of elitism replacements to make each iteration")
      ("min", po::value<double>(&options.constant_min)->default_value(0),
       "set the minimum value for random constants in an expression")
      ("max", po::value<double>(&options.constant_max)->default_value(10),
       "set the maximum value for random constants in an expression")
      ("penalty,P", po::value<double>(&options.penalty)->default_value(0.1),
       "set the constant scalar of the size penalty for fitness")
      ("grow,g", po::value<double>(&options.grow_chance)->default_value(0.5),
       "set the probability that an initial tree will be made by the grow method")
      ("mutate,m", po::value<double>(&options.mutate_chance)->default_value(0.01),
       "set the probability that a single node will mutate")
      ("crossover_chance", po::value<double>(&options.crossover_chance)->default_value(0.8),
       "set the probability that a selected pair of invididuals will undergo crossover")
      ("internals,I", po::value<double>(&options.internals_chance)->default_value(0.9),
       "set the probability that a target node for crossover will be an internal node")
      ("logs", po::value<std::string>(&options.logs_dir)->default_value("logs/"),
       "set the save directory for log files")
      ("plots", po::value<std::string>(&options.plots_dir)->default_value("plots/"),
       "set the save directory for plot data files")
      ("verbosity,v", po::value<unsigned int>(&options.verbosity)->default_value(1),
       "set the verbosity: 0 - no logging; 1 - normal logging; 2 - debug output");
    try {
      po::store(po::command_line_parser(argc, argv).options(desc).positional(positionals).run(), variables_map);
      po::notify(variables_map);
    } catch(std::exception & e) {
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    }
    // print help
    if (variables_map.count("help")) {
      std::cout << "Genetic Program implemented in C++ by Andrew Schwartzmeyer\n"
		<< "Code located at https://github.com/andschwa/uidaho-cs472-project2\n\n"
		<< "Logs saved to <logs>/<Unix time>.dat\n"
		<< "Plot data saved to <plots>/<Unix time>.dat\n"
		<< "GNUPlot PNG generation script './plot <plots>'\n\n"
		<< desc << std::endl;
      exit(EXIT_SUCCESS);
    }
	std::ifstream config(variables_map["config"].as<string>());
	if (config.is_open())
	  po::store(po::parse_config_file(config, description), variables_map);
    // get values from given test file
    options.values = get_data(test_file);
    options.validate();
    return options;
  }
}
