bin_PROGRAMS = search
search_SOURCES = \
	src/main.cpp \
	src/algorithm/algorithm.cpp \
	src/individual/individual.cpp \
	src/options/options.cpp \
	src/random_generator/random_generator.cpp \
	src/trials/trials.cpp

AM_CPPFLAGS = ${BOOST_CPPFLAGS} ${PTHREAD_CFLAGS}
AM_LDFLAGS = ${BOOST_LDFLAGS} ${PTHREAD_LIBS}
LDADD = ${BOOST_PROGRAM_OPTIONS_LIB}
