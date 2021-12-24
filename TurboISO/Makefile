#compile parameters

CC = g++
CFLAGS = -c  -Wall -O2 #-fprofile-arcs -ftest-coverage -coverage #-pg
EXEFLAG = -O2 #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
#CFLAGS = -c -W -g  #-fprofile-arcs -ftest-coverage -coverage #-pg
#EXEFLAG = -g  #-fprofile-arcs -ftest-coverage -coverage #-pg #-O2
	 
#add -lreadline -ltermcap if using readline or objs contain readline
library = #-lgcov -coverage

objdir = ./objs/
objfile = $(objdir)Graph.o

all: turbo.exe

turbo.exe: main.cpp $(objfile) util.h
	$(CC) $(EXEFLAG) -o turbo.exe main.cpp $(objfile) $(library)

$(objdir)Graph.o: Graph.cpp Graph.h
	$(CC) $(CFLAGS) Graph.cpp -o $(objdir)Graph.o

.PHONY: clean dist tarball test sumlines

clean:
	rm -f $(objdir)*
dist: clean
	rm -f *.txt *.log

#tarball:
	#tar -czvf vf2.tar.gz main util match io graph Makefile README.md objs script vflib2

#test: main/test.o $(objfile)
	#$(CC) $(EXEFLAG) -o test main/test.cpp $(objfile) $(library)

#sumline:
	#bash script/sumline.sh

