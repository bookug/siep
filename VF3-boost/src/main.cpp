#include "../util/Util.h"
#include "../io/IO.h"
#include "../graph/Graph.h"
#include "../hyper.h"

#include "match.hpp"
#include "nodesorter.hpp"
#include "probability_strategy.hpp"
#include "nodesorter.hpp"

#include "vf3_sub_state.hpp"

#include "../../control.h"

using namespace std; 


typedef struct visitor_data_s {
	unsigned long first_solution_time;
	long solutions;
}visitor_data_t;
typedef int data_t;

// bool visitor(int n, node_id n1[], node_id n2[], void* state, void *usr_data) {
// 	#if (defined PRINT_NUM_STATES) && PRINT_NUM_STATES
// 	// Print number of traversed states for this solution
// 	AbstractVFState<int, int, Empty, Empty>* s = static_cast<AbstractVFState<int, int, Empty, Empty>*>(state);
// 	while (s) {
// 		if (!s->IsUsed()) {
// 			s->SetUsed();
// 			state_counter++;
// 		}
// 		s = s->GetParent();
// 	}
// 	#endif

// 	#if (defined PRINT_SOLUTION) && PRINT_SOLUTION
// 	// Print found solution
// 	//cout<<"Solution Found:\n";
// 	for(int k = 0; k < n; k++) {
// 		if(n1[k] != NULL_NODE)	//cout<<n2[n1[k]]<<","<<n1[k]<<":";
// 			fprintf(ofp, "(%d, %d) ", n1[k], n2[n1[k]]);
// 	}
// 	fprintf(ofp, "\n");
// 	#endif

// 	// Count solutions and take time of first solution
// 	visitor_data_t* data = (visitor_data_t*)usr_data;
// 	data->solutions++;
// 	if(data->solutions == 1) data->first_solution_time = clock();

// 	return false;
// }

vector<Graph*> query_list;

long numembeddings = 0, ncalls = 0, bound = 100000;
double max_mem = 0.0;

int main(int argc, char** argv) {

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
	if(argc == 4) output = argv[3];
	long class_t = 0;

	IO io = IO(query, data, output);
	//read query file and keep all queries in memory
	io.input(query_list);
	//assert(query_list.size() <= 1);
	volatile int qnum = query_list.size();
	Graph *data_graph = NULL;

	cerr << "VF3-boost input OK" << endl;
	//read each graph and transform into the format of vf3
	cerr.flush();
	long begin, end, tottime = 0, datanum = 0;

	while(io.input(data_graph)) {
        //offline computing SE and SC for BoostISO
		datanum++;

		//string tmpFile = "temp.g";
        ////NOTICE: output it to a single file, 
		//// because original data file may contain several graphs
        //data_graph->outputGraph(tmpFile);
		//string cmd = "./gshBoostISO.exe " + tmpFile + " se.txt sc.txt";
        //system(cmd.c_str());

	    HyperGraph *gsh = new HyperGraph;
	    gsh->build("se.txt", "sc.txt");

		for (int i=0; i<qnum; i++){
			numembeddings = 0;
			if (query_list[i]->vertexLabelNum > gsh->LabelNum)
				continue;
			query_list[i]->transform();
			begin = Util::get_cur_time();
			Util::timeLimit(600);
			int *usedTimes = new int[gsh->numVertex];
			VF3NodeSorter sorter;
			vector<node_id> sorted = sorter.SortNodes(query_list[0]);
#ifdef PRINT_RESULT
			io.output(i);
#endif
			VF3SubState s0(gsh, data_graph, query_list[i], query_list[i]->vLabel, 
				gsh->vList, gsh->LabelNum, sorted.data(), io.ofp);
			match(s0);

			Util::noTimeLimit();
			end = Util::get_cur_time();
			tottime += (end - begin);
			//io.output();
			//io.flush();
			delete []usedTimes;
		}
		delete gsh;
		delete data_graph;
	}
	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%8.0lf kB    %s\n", 
		"vf3Boosted: ", numembeddings, ncalls, tottime, max_mem, qid.c_str());
	
	for(int i = 0; i < qnum; ++i)
		delete query_list[i];
	return 0;
}

