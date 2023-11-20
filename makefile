# Project:     DNS resolver
# @file        Makefile 
# @author Martina Hromádková <xhroma15>
 
CXX = g++
CFLAGS = -Wall
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lcatch

# Source file and executable name
SRC = dns.cpp argument_parser.cpp dns_functions.cpp
EXE = dns

all: $(EXE)

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXE)

test: tests.sh
	./tests.sh

clean:
	rm -f $(EXE) $(TEST_EXE)

.PHONY: all clean test