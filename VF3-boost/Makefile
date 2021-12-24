# WARN: vf3 can not be run in several processes here, because it uses two fix-name txt files.

CXXFLAGS= -std=c++11 -O2 -w -Wno-deprecated -Iinclude
CXXFLAGS= -std=c++11 -g -Wall -Wno-deprecated -Iinclude
inc=include/match.hpp include/vf3_sub_state.hpp include/nodesorter.hpp
# Default target:
all: vf3Boosted.exe

#vf3.exe: src/main.cpp $(wildcard include/*) util/Util.cpp graph/Graph.cpp io/IO.cpp
vf3Boosted.exe: src/main.cpp ${inc} util/Util.cpp graph/Graph.cpp io/IO.cpp 
	$(CXX) $(CXXFLAGS) -o $@ $^
	
test: bin/vf3
	@bin/vf3  test/bvg1.grf test/bvg1.sub.grf

.PHONY: clean dist
clean:
	rm vf3Boosted.exe
dist: clean
	rm -rf ./doc/html
	rm -rf ./doc/latex

