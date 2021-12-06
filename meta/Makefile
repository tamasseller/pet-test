OUTPUT = pet-meta-test

# Library sources

# Test code

SOURCES += TestGroups.cpp
SOURCES += TestMock.cpp
SOURCES += TestNoGroup.cpp
SOURCES += TestFailureInjector.cpp
SOURCES += TestGenericSuite.cpp

# Test support

SOURCES += TestMain.cpp

# Includes

INCLUDE_DIRS += .
INCLUDE_DIRS += ../..

# Flags

CXXFLAGS += -std=c++17								# Use the c++ standard released in 2017
CXXFLAGS += -O0 									# Avoid optimizer screwing up coverage data
CXXFLAGS += -g3										# Include all available debug info
CXXFLAGS += -fno-inline								# Do not inline (coverage again)
CXXFLAGS += -fmax-errors=5							# Avoid blowing the console
CXXFLAGS += -rdynamic								# Allows meaningful backrtaces 
CXXFLAGS += --coverage								# Generate program flow data and inject profiling code
CXXFLAGS += -fno-exceptions
CXXFLAGS += -fdelete-null-pointer-checks

LIBS += gcov

LD=$(CXX) 

include ../../mod.mk
include ../ultimate-makefile/Makefile.ultimate	
