#compile parameters

CC = g++
# NOTICE: we set all optimize option to -O2 instead of -O6, for fair comparison with others
CFLAGS = -std=c++11 -c  -Wall -O2 #-fprofile-arcs -ftest-coverage -coverage #-pg
EXEFLAG = -std=c++11 -Wall  -O2 #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
#CFLAGS = -std=c++11 -c -Wall -g #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
#EXEFLAG = -std=c++11 -Wall -g #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
	 
#add -lreadline -ltermcap if using readline or objs contain readline
library = #-lgcov -coverage
files=$(wildcard offline/GraphAdaptation/*) $(wildcard offline/Graph/*) $(wildcard offline/CommonUtility/*) util/*

all: turboBoosted.exe gshBoostISO.exe

turboBoosted.exe: main.cpp Graph.cpp Graph.h hyper.h util/Util.cpp util/Util.h
	$(CC) $(EXEFLAG) -o turboBoosted.exe main.cpp Graph.cpp util/Util.cpp $(library)

gshBoostISO.exe: $(files)
	$(CC) $(EXEFLAG) -o gshBoostISO.exe  $(files)

.PHONY: clean dist tarball test sumlines

clean:
	rm -f *.exe
dist: clean
	rm -f *.txt 
	rm -f core.*

sumline:
	bash script/sumline.sh

