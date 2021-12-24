#compile parameters

CC = g++ -std=c++11
CFLAGS = -c -Wall -O2 #-fprofile-arcs -ftest-coverage -coverage #-pg
EXEFLAG = -O2 #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
	 
#add -lreadline -ltermcap if using readline or objs contain readline
library = #-lgcov -coverage
files=$(wildcard offline/GraphAdaptation/*) $(wildcard offline/Graph/*) $(wildcard offline/CommonUtility/*) util/*

objdir = ./objs/
objfile = $(objdir)Util.o $(objdir)IO.o $(objdir)Match.o $(objdir)Graph.o $(objdir)Seq.o
prog=quicksiBoosted.exe


all: $(prog) gshBoostISO.exe

$(prog): main/run.cpp $(objfile)  hyper.h
	$(CC) $(EXEFLAG) -o $(prog) main/run.cpp $(objfile) $(library)

gshBoostISO.exe: $(files)
	$(CC) $(EXEFLAG) -o gshBoostISO.exe  $(files)

$(objdir)Util.o: util/Util.cpp util/Util.h
	$(CC) $(CFLAGS) util/Util.cpp -o $(objdir)Util.o

$(objdir)Graph.o: graph/Graph.cpp graph/Graph.h
	$(CC) $(CFLAGS) graph/Graph.cpp -o $(objdir)Graph.o

$(objdir)IO.o: io/IO.cpp io/IO.h
	$(CC) $(CFLAGS) io/IO.cpp -o $(objdir)IO.o

$(objdir)Match.o: match/Match.cpp match/Match.h
	$(CC) $(CFLAGS) match/Match.cpp -o $(objdir)Match.o

$(objdir)Seq.o: seq/Seq.cpp seq/Seq.h
	$(CC) $(CFLAGS) seq/Seq.cpp -o $(objdir)Seq.o


.PHONY: clean dist tarball test sumlines

clean:
	rm -f $(objdir)*
dist: clean
	rm -f *.txt dig

tarball:
	tar -czvf graphql.tar.gz main util match io graph Makefile README.md objs script vflib2

test: main/test.o $(objfile)
	$(CC) $(EXEFLAG) -o test main/test.cpp $(objfile) $(library)

sumline:
	bash script/sumline.sh

