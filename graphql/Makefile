#compile parameters

CC = g++
CFLAGS = -c -w -O2  -std=c++11 #-fprofile-arcs -ftest-coverage -coverage #-pg
EXEFLAG = -O2 -w  -std=c++11#-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
#CFLAGS = -c -Wall -g #-fprofile-arcs -ftest-coverage -coverage #-pg
#EXEFLAG = -g #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
	 
#add -lreadline -ltermcap if using readline or objs contain readline
library = #-lgcov -coverage

objdir = ./objs/
objfile = $(objdir)Util.o $(objdir)IO.o $(objdir)Match.o $(objdir)Graph.o

all: graphql.exe

graphql.exe: main/run.cpp $(objfile)
	$(CC) $(EXEFLAG) -o graphql.exe main/run.cpp $(objfile) $(library)

$(objdir)Util.o: util/Util.cpp util/Util.h
	$(CC) $(CFLAGS) util/Util.cpp -o $(objdir)Util.o

$(objdir)Graph.o: graph/Graph.cpp graph/Graph.h
	$(CC) $(CFLAGS) graph/Graph.cpp -o $(objdir)Graph.o

$(objdir)IO.o: io/IO.cpp io/IO.h
	$(CC) $(CFLAGS) io/IO.cpp -o $(objdir)IO.o

$(objdir)Match.o: match/Match.cpp match/Match.h
	$(CC) $(CFLAGS) match/Match.cpp -o $(objdir)Match.o



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

