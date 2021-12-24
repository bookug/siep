/*=============================================================================
# Filename:		run.cpp
# Author: bookug
# Mail: bookug@qq.com
# Last Modified:	2019-09-24 10:41
# Description: 
=============================================================================*/

#include "../util/Util.h"
#include "../io/IO.h"
#include "../graph/Graph.h"
#include "../match/Match.h"

#include "../../control.h"

using namespace std;

//NOTICE:a pattern occurs in a graph, then support++(not the matching num in a graph), support/N >= minsup
vector<Graph*> query_list;

long TIME_LIMIT_SECONDS = 600; //10 min
//long ncalls = 0, bound = 5000, numofembeddings = 0;
long ncalls = 0, bound = 100000, numofembeddings = 0;
double max_mem = 0.0;

int main(int argc, const char * argv[]) {
	int i, j, k;

	string output = "ans.txt";
	if(argc > 4 || argc < 3) {
		cerr<<"invalid arguments!"<<endl;
		return -1;
	}
	string data = argv[1];
	string query = argv[2];
    int pos1 = query.rfind("/");
    int pos2 = query.rfind(".");
    string qid = query.substr(pos1+1, pos2-pos1-1);
	if(argc == 4)
		output = argv[3];

	long t1 = Util::get_cur_time();

	IO io = IO(query, data, output);
	//read query file and keep all queries in memory
	io.input(query_list);
	int qnum = query_list.size();
	
	cerr<<"DAF input ok!"<<endl;
	long t2 = Util::get_cur_time();

    long sumt = 0;
	Graph* data_graph = NULL;
	while(true) {
		if(!io.input(data_graph))
			break;
		for(i = 0; i < qnum; ++i) {
			Util::timeLimit(TIME_LIMIT_SECONDS);
        	long begin = Util::get_cur_time();
			numofembeddings = 0;
			Match m(query_list[i], data_graph);
#ifdef PRINT_RESULT
			io.output(i);
#endif
			m.match(io);
			Util::noTimeLimit();
        	long end = Util::get_cur_time();
        	sumt += (end-begin);
#ifdef PRINT_RESULT
			io.output();
			io.flush();
#endif
		}

		delete data_graph;
	}

	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%8.0lf kB    %s\n", 
		"daf: ", numofembeddings, ncalls, sumt, max_mem, qid.c_str());

	/*
	cout << "DAF: " << endl;
	cout << "  nembeddings: " << numofembeddings << endl;
	cout << "  ncalls: " << ncalls << endl;
	cout << "  time: " << sumt << "ms" << endl;
	cout << "  max_mem: " << max_mem << "kB" << endl;
	*/
	//release all and flush cached writes
	for(i = 0; i < qnum; ++i)
		delete query_list[i];
	io.flush();

	return 0;
}

