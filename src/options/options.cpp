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

  Options::Options(const pairs & values,
		   const int & trials,
		   const int & iterations,
		   const int & population_size,
		   const int & max_depth,
		   const int & tournament_size,
		   const int & crossover_size,
		   const int & elitism_size,
		   const double & constant_min,
		   const double & constant_max,
		   const double & penalty,
		   const double & grow_chance,
		   const double & mutate_chance,
		   const double & crossover_chance,
		   const double & internals_chance) : values{values}, trials{trials}, iterations{iterations}, population_size{population_size}, max_depth{max_depth}, tournament_size{tournament_size}, crossover_size{crossover_size}, elitism_size{elitism_size}, constant_min{constant_min}, constant_max{constant_max}, penalty{penalty}, grow_chance{grow_chance}, mutate_chance{mutate_chance}, crossover_chance{crossover_chance}, internals_chance{internals_chance} {
		     assert(values.size() > 0);
		     assert(trials > 0);
		     assert(iterations > 0);
		     assert(population_size > 0);
		     assert(max_depth >= 0);
		     assert(tournament_size > 0 and tournament_size <= population_size);
		     assert(crossover_size == 2);
		     assert(elitism_size >= 0 and elitism_size <= population_size);
		     assert(constant_min < constant_max);
		     assert(grow_chance >= 0 and grow_chance <= 1);
		     assert(mutate_chance >= 0 and mutate_chance <= 1);
		     assert(crossover_chance >= 0 and crossover_chance <= 1);
		     assert(internals_chance >= 0 and internals_chance <= 1);
		   }

  const Options parse(int argc, char* argv[]) {
    namespace po = boost::program_options;
    std::string filename;
    int trials;
    int iterations;
    int population_size;
    int max_depth;
    int tournament_size;
    int crossover_size;
    int elitism_size;
    double constant_min;
    double constant_max;
    double penalty;
    double grow_chance;
    double mutate_chance;
    double crossover_chance;
    double internals_chance;
    po::positional_options_description positionals;
    po::variables_map variables_map;
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "produce help message")
      ("file,f", po::value<std::string>(&filename)->default_value("test/cs472.dat"),
       "specify the location of the columnized test data \"X Y\"")
      ("trials,t", po::value<int>(&trials)->default_value(4),
       "set the number of trials to run")
      ("iterations,i", po::value<int>(&iterations)->default_value(128),
       "set the number of iterations for which to run each trial")
      ("population,p", po::value<int>(&population_size)->default_value(128),
       "set the size of each population")
      ("depth,d", po::value<int>(&max_depth)->default_value(4),
       "set the maximum depth for initial populations")
      ("tournament,T", po::value<int>(&tournament_size)->default_value(3),
       "set the tournment size to adjust selection pressure")
      ("crossover,c", po::value<int>(&crossover_size)->default_value(2),
       "set the crossover size (binary in current implementation)")
      ("elitism,e", po::value<int>(&elitism_size)->default_value(2),
       "set the number of elitism replacements to make each iteration")
      ("min", po::value<double>(&constant_min)->default_value(0),
       "set the minimum value for random constants in an expression")
      ("max", po::value<double>(&constant_max)->default_value(10),
       "set the maximum value for random constants in an expression")
      ("penalty,P", po::value<double>(&penalty)->default_value(0.1),
       "set the constant scalar of the size penalty for fitness")
      ("grow,g", po::value<double>(&grow_chance)->default_value(0.5),
       "set the probability that an initial tree will be made by the grow method")
      ("mutate,m", po::value<double>(&mutate_chance)->default_value(0.01),
       "set the probability that a single node will mutate")
      ("crossover_chance,C", po::value<double>(&crossover_chance)->default_value(0.8),
       "set the probability that a selected pair of invididuals will undergo crossover")
      ("internals,I", po::value<double>(&internals_chance)->default_value(0.9),
       "set the probability that a target node for crossover will be an internal node");
    try {
      po::store(po::command_line_parser(argc, argv).
		options(desc).positional(positionals).run(), variables_map);
      if (variables_map.count("help")) {
	std::cout << "Genetic Program implemented in C++ by Andrew Schwartzmeyer\n"
		  << "Code located at https://github.com/andschwa/uidaho-cs472-project2\n\n"
		  << "Logs saved to logs/<Unix time>.dat\n"
		  << "Plot data saved to logs/plot/<Unix time>.dat\n"
		  << "GNUPlot PNG generation script `./plot.plg`\n\n"
		  << desc << std::endl;
	exit(EXIT_SUCCESS);
      }
      po::notify(variables_map);
    }
    catch(std::exception& e) {
      std::cout << e.what() << '\n';
      exit(EXIT_FAILURE);
    }
    return Options{get_data(filename),
	trials,
	iterations,
	population_size,
	max_depth,
	tournament_size,
	crossover_size,
	elitism_size,
	constant_min,
	constant_max,
	penalty,
	grow_chance,
	mutate_chance,
	crossover_chance,
	internals_chance};
  }
}
