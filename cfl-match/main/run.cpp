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
long numembeddings = 0, bound = 100000;
//long numembeddings = 0, bound = 5000;

int main(int argc, const char *argv[]) {
	long tb = Util::get_cur_time();
	string output = "ans.txt";
	if (argc > 5 || argc < 3) {
		cerr<<"invalid arguments!"<<endl;
		return -1;
	}
	double vsize, rss;
	string data = argv[1];
	string query = argv[2];
    int pos1 = query.rfind("/");
    int pos2 = query.rfind(".");
    string qid = query.substr(pos1+1, pos2-pos1-1);
	if (argc >= 4) output = argv[3];
	if (argc == 5) sscanf(argv[4], "%d", &bound);
	IO io = IO(query, data, output);

	// read query file and keep all queries in memory
	io.input(query_list);		// Read all query graphs from qfp
	int qnum = query_list.size();
	cerr << "cfl input ok!" << endl;
	//assert(query_list.size() <= 1);

	long end = 0, begin = 0, tottime = 0, totcalls = 0;
	Graph *data_graph = NULL;
	while (io.input(data_graph)) {	// Read one data graph from dfp
		Util::process_mem_usage(vsize, rss);
		printf("After data readin: %8.0lf\n", vsize);
		for (int i=0; i<qnum; i++) {
			begin = Util::get_cur_time();
			numembeddings = 0;
			Util::process_mem_usage(vsize, rss);
			printf("After query readin: %8.0lf\n", vsize);
			//Util::timeLimit(600);
			CPI *cpi = new CPI(query_list[i], data_graph, io.ofp);
#ifdef PRINT_RESULT
			io.output(i);
#endif
			Util::process_mem_usage(vsize, rss);
			printf("After cpi constructed: %8.0lf\n", vsize);

			if (cpi->CPI_Construct())		// Do match only if sln exists
				cpi->Core_match();
			Util::noTimeLimit();
			end = Util::get_cur_time();
			tottime += (end - begin);
#ifdef PRINT_RESULT
			io.outputtail();
			io.flush();
#endif

			totcalls += cpi->ncalls;
			delete cpi;
		}
		delete data_graph;
	}
	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%8.0lf kB    %s\n",
		"cfl: ", numembeddings, totcalls, tottime, max_mem, qid.c_str());
	//release all and flush cached writes
	for(int i = 0; i < qnum; ++i)
		delete query_list[i];
	
	return 0;
}

