#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([CS 472: Genetic Programming], [0.1], [andrew@schwartzmeyer.com])
AM_INIT_AUTOMAKE([-Wall -Werror -Woverride foreign subdir-objects])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CXX([g++-4.8])

# Checks for libraries.
AX_BOOST_BASE([1.55])
AX_BOOST_PROGRAM_OPTIONS

# Checks for and enables pthread.
AX_PTHREAD

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for compiler warnings.
AX_CXXFLAGS_WARN_ALL

# Checks for C++11.
AX_CXX_COMPILE_STDCXX_11

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
