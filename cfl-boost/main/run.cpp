/*=============================================================================
# Filename: main.cpp
# Author: Jiang Yan (Frozenlight)
# Mail: 527507046@qq.com 
=============================================================================*/

#include "../util/Util.h"
#include "../io/IO.h"
#include "../graph/Graph.h"
#include "../cpi/CPI.h"

using namespace std;

template<class IT>
void Print(IT F, IT L){
	for(; F != L; ++F)
		cout << *F << " ";
	cout<<endl;
}


vector<Graph*> query_list;
bool DBmode = false;

double max_mem = 0.0;
long bound = 100000, nmatchings = 0;

int main(int argc, const char *argv[]) {
	long tb = Util::get_cur_time();
	string output = "ans.txt";
	if (argc > 5 || argc < 3) {
		cerr<<"invalid arguments!"<<endl;
		return -1;
	}
	string data = argv[1];
	string query = argv[2];
    int pos1 = query.rfind("/");
    int pos2 = query.rfind(".");
    string qid = query.substr(pos1+1, pos2-pos1-1);
	if (argc >= 4) output = argv[3];
	IO io = IO(query, "se.txt", "sc.txt", output);

	// Precondition of data graph
	//string cmd = "./gshBoostISO.exe " + data + " se.txt sc.txt";
	//system(cmd.c_str());

	//read query file and keep all queries in memory
	io.input(query_list);		// Read all query graphs from qfp
	int qnum = query_list.size();
	cerr<<"cfl-boost input ok!"<<endl;
	//assert(query_list.size() <= 1);

    //BETTER: to be consistent with other algs, also read original data graph.

	long end = 0, begin = 0, tottime = 0, totmatchings = 0, totcalls = 0;
	Graph *data_graph = NULL;
	while (io.readData()) {
		io.input(data_graph);	
        //compute the vertex num of original data graph
        int real_vertex_num = 0;
        for(int i = 0; i < data_graph->n; ++i)
        {
		    real_vertex_num += data_graph->ctains[i].size();
        }

		for (int i=0; i<qnum; i++) {
			begin = Util::get_cur_time();
			Util::timeLimit(600);
			nmatchings = 0;
			CPI *cpi = new CPI(query_list[i], data_graph, io.ofp, real_vertex_num);
#ifdef PRINT_RESULT
			io.output(i);
#endif
			if (cpi->CPI_Construct())		// Do match only if sln exists
				cpi->Core_match();
			Util::noTimeLimit();
#ifdef PRINT_RESULT
			io.outputtail();
			io.flush();
#endif

			end = Util::get_cur_time();
			tottime += (end - begin);
			totcalls += cpi->ncalls;
			delete cpi;
		}
		delete data_graph;
	}
	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%8.0lf kB    %s\n", 
		"cflBoosted: ", nmatchings, totcalls, tottime, max_mem, qid.c_str());

	//release all and flush cached writes
	for(int i = 0; i < qnum; ++i)
		delete query_list[i];

	return 0;
}

