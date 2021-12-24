# WARN: vf3 can not be run in several processes here, because it uses two fix-name txt files.

CXXFLAGS= -std=c++11 -O2 -Wall -Wno-deprecated -Iinclude
#CXXFLAGS= -std=c++11 -g -Wall -Wno-deprecated -Iinclude
inc=$(ls include/)
# Default target:
all: vf3.exe

#vf3.exe: src/main.cpp $(wildcard include/*) util/Util.cpp graph/Graph.cpp io/IO.cpp
vf3.exe: src/main.cpp ${inc} util/Util.cpp graph/Graph.cpp io/IO.cpp 
	$(CXX) $(CXXFLAGS) -o $@ $^
	
test: bin/vf3
	@bin/vf3  test/bvg1.grf test/bvg1.sub.grf

.PHONY: clean dist
clean:
	rm *.exe
dist: clean
	rm -rf ./doc/html
	rm -rf ./doc/latex

