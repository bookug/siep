#compile parameters

CC = g++ --std=c++11
CFLAGS = -std=c++11 -c -Wall -O2 #-fprofile-arcs -ftest-coverage -coverage #-pg
EXEFLAG = -std=c++11 -O2 #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
#CFLAGS = -std=c++11 -c -g -Wall #-fprofile-arcs -ftest-coverage -coverage #-pg
#EXEFLAG = -std=c++11 -Wall -g #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
#CFLAGS = -c -Wall -g #-fprofile-arcs -ftest-coverage -coverage #-pg
#EXEFLAG = -g #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
	 
#add -lreadline -ltermcap if using readline or objs contain readline
library = #-lgcov -coverage
files=$(wildcard offline/GraphAdaptation/*) $(wildcard offline/Graph/*) $(wildcard offline/CommonUtility/*) util/*

objdir = ./objs/
objfile = $(objdir)Graph.o
prog=ceciBoosted.exe

all: $(prog) gshBoostISO.exe

$(prog): main.cpp Graph.cpp Graph.h hyper.h util/Util.cpp util/Util.h
	$(CC) $(EXEFLAG) -o $(prog) main.cpp Graph.cpp util/Util.cpp  $(library)

gshBoostISO.exe: $(files)
	$(CC) $(EXEFLAG) -o gshBoostISO.exe  $(files)

$(objdir)Graph.o: Graph.cpp Graph.h
	$(CC) $(CFLAGS) Graph.cpp -o $(objdir)Graph.o

.PHONY: clean dist tarball test sumlines

clean:
	rm -f $(objdir)* *.exe
dist: clean
	rm -f *.txt *.log core.* *.exe

#tarball:
	#tar -czvf vf2.tar.gz main util match io graph Makefile README.md objs script vflib2

#test: main/test.o $(objfile)
	#$(CC) $(EXEFLAG) -o test main/test.cpp $(objfile) $(library)

sumline:
	bash script/sumline.sh

