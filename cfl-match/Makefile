#compile parameters

CC = g++ -std=c++11
CFLAGS = -c -w  -O0 -g #-fprofile-arcs -ftest-coverage -coverage #-pg
EXEFLAG = -w  -O0 -g #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
#CFLAGS = -c -w -g #-fprofile-arcs -ftest-coverage -coverage #-pg
#EXEFLAG = -w -g  #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
	 
#add -lreadline -ltermcap if using readline or objs contain readline
library = #-lgcov -coverage

objdir = ./objs/
objfile = $(objdir)Util.o $(objdir)IO.o $(objdir)Graph.o $(objdir)CPI.o

all: cfl.exe

cfl.exe: main/run.cpp $(objfile)
	$(CC) $(EXEFLAG) -o cfl.exe main/run.cpp $(objfile) $(library)

$(objdir)Util.o: util/Util.cpp util/Util.h graph/Graph.h
	$(CC) $(CFLAGS) -o $(objdir)Util.o util/Util.cpp

$(objdir)Graph.o: graph/Graph.cpp graph/Graph.h util/Util.h
	$(CC) $(CFLAGS) -o $(objdir)Graph.o graph/Graph.cpp

$(objdir)IO.o: io/IO.cpp io/IO.h util/Util.h graph/Graph.h
	$(CC) $(CFLAGS) -o $(objdir)IO.o io/IO.cpp

$(objdir)CPI.o: cpi/CPI.cpp cpi/CPI.h util/Util.h graph/Graph.h
	$(CC) $(CFLAGS) -o $(objdir)CPI.o cpi/CPI.cpp

.PHONY: clean dist 

clean:
	rm -f $(objdir)*
dist: clean
	rm -f *.txt *.exe

