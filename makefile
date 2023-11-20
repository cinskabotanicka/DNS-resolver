# Project:     DNS resolver
# @file        Makefile 
# @author Martina Hromádková <xhroma15>
 
CXX = g++
CFLAGS = -Wall
CXXFLAGS = -Wall -std=c++11

# Source file and executable name
SRC = dns.cpp argument_parser.cpp dns_functions.cpp
EXE = dns

TEST_SRC = test.cpp
TEST_EXE = test

all: $(EXE)

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXE) $(LDFLAGS)

test: $(TEST_EXE)

$(TEST_EXE): $(TEST_SRC) $(SRC)
	$(CXX) $(CXXFLAGS) -DTEST $(SRC) $(TEST_SRC) -o $(TEST_EXE) $(LDFLAGS)

clean:
	rm -f $(EXE)

.PHONY: all clean