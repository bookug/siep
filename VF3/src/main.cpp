#include "../util/Util.h"
#include "../io/IO.h"
#include "../graph/Graph.h"

#include "match.hpp"
#include "argloader.hpp"
#include "argraph.hpp"
#include "argedit.hpp"
#include "nodesorter.hpp"
#include "probability_strategy.hpp"
#include "nodesorter.hpp"
#include "nodeclassifier.hpp"

#include "../../control.h"


#ifndef VF3BIO
typedef int data_t;
#else
typedef string data_t;
#endif

#include "vf3_sub_state.hpp"
#define SUBSTATE_CLASS VF3SubState
#define PARAMETER_K_IF_NEEDED 


#define PRINT_SORTING	0
#define PRINT_SOLUTION   1
#define PRINT_NUM_STATES 0

using namespace std; 

template<> long long SUBSTATE_CLASS<data_t,data_t,Empty,Empty>::instance_count = 0;
static long long state_counter = 0;
double max_mem = 0.0;
long nembeddings = 0, bound = 100000;
long TIME_LIMIT_SECONDS = 600;

typedef struct visitor_data_s {
	unsigned long first_solution_time;
	long solutions;
}visitor_data_t;

FILE* ofp = NULL;
bool visitor(int n, node_id n1[], node_id n2[], void* state, void *usr_data) {
	#if (defined PRINT_NUM_STATES) && PRINT_NUM_STATES
	// Print number of traversed states for this solution
	AbstractVFState<int, int, Empty, Empty>* s = static_cast<AbstractVFState<int, int, Empty, Empty>*>(state);
	while (s) {
		if (!s->IsUsed()) {
			s->SetUsed();
			state_counter++;
		}
		s = s->GetParent();
	}
	#endif

#ifdef PRINT_RESULT
	#if (defined PRINT_SOLUTION) && PRINT_SOLUTION
	// Print found solution
	//cout<<"Solution Found:\n";
	for(int k = 0; k < n; k++) {
	if(n1[k] != NULL_NODE )	//cout<<n2[n1[k]]<<","<<n1[k]<<":";
		fprintf(ofp, "(%d, %d) ", n1[k], n2[n1[k]]);
	}
	fprintf(ofp, "\n");
	#endif
#endif

	// Count solutions and take time of first solution
	visitor_data_t* data = (visitor_data_t*)usr_data;
	data->solutions++;
	nembeddings++;
	if(data->solutions == 1) data->first_solution_time = clock();
	
	double vsize, rss;
	Util::process_mem_usage(vsize, rss);
	if (vsize > max_mem) max_mem = vsize;
	return false;
}

vector<Graph*> query_list;


size_t get_executable_path( char* processdir,char* processname, size_t len)
{
        char* path_end;
        if(readlink("/proc/self/exe", processdir,len) <=0)
                return -1;
        path_end = strrchr(processdir,  '/');
        if(path_end == NULL)
                return -1;
        ++path_end;
        strcpy(processname, path_end);
        *path_end = '\0';
        return (size_t)(path_end - processdir);
}

string
int2string(long n)
{
    string s;
    stringstream ss;
    ss<<n;
    ss>>s;
    return s;
}

int main(int argc, char** argv) {
//char path[PATH_MAX];
//char processname[1024];
//get_executable_path(path, processname, sizeof(path));
    //NOTICE: we add pid to file name, to avoid errors in multiprocessing.
    string pid = int2string(getpid());
    string pattern = pid+"pattern.txt";
    string target = pid+"target.txt";
	//char pattern[] = "pattern.txt";
	//char target[]  = "target.txt";

	state_counter = 0;
	int n = 0, i;

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
	long sumt = 0;
	long class_t = 0;
	long num_recursive_call = 0;

	IO io = IO(query, data, output);
	ofp = io.ofp;
	//read query file and keep all queries in memory
	io.input(query_list);
	int qnum = query_list.size();
	Graph* data_graph = NULL;

	cerr << "VF3 input OK" << endl;
	int cnt=0;
	//read each graph and transform into the format of vf3
	while(true) {
		//NOTICE: each undirected edge is added as two directed egdes
		if(!io.input(data_graph)) break;
		//cout << "data" << cnt++ << endl;
		data_graph->transform(target.c_str());
        delete data_graph;
		for(i = 0; i < qnum; ++i) {
			//cout << "q" << i << endl;
			//if (query_list[i]->vertices.size() > data_graph->vertices.size()) continue;
			//if (query_list[i]->nlb > data_graph->nlb) continue;
			
			//transform and input to v/f3
			query_list[i]->transform(pattern.c_str());
			visitor_data_t vis_data;
			ifstream graphInPat(pattern.c_str());
			ifstream graphInTarg(target.c_str());
			StreamARGLoader<data_t, Empty> pattloader(graphInPat);
			StreamARGLoader<data_t, Empty> targloader(graphInTarg);
			ARGraph<data_t, Empty> patt_graph(&pattloader);
			ARGraph<data_t, Empty> targ_graph(&targloader);

			int nodes1, nodes2;
			nodes1 = patt_graph.NodeCount();
			nodes2 = targ_graph.NodeCount();
			node_id *n1, *n2;
			n1 = new node_id[nodes1];
			n2 = new node_id[nodes2];
			NodeClassifier<data_t, Empty> classifier(&targ_graph);
			NodeClassifier<data_t, Empty> classifier2(&patt_graph, classifier);
			vector<int> class_patt = classifier2.GetClasses();
			vector<int> class_targ = classifier.GetClasses();
			if (classifier2.CountClasses() > classifier.CountClasses())
				continue;

#ifdef PRINT_RESULT
			io.output(i);
#endif
			long begin = Util::get_cur_time();
			Util::timeLimit(TIME_LIMIT_SECONDS);
			nembeddings = 0;
			
			//match process
			vis_data.solutions = 0;
			VF3NodeSorter<data_t, Empty, SubIsoNodeProbability<data_t, Empty> > sorter(&targ_graph);
			vector<node_id> sorted = sorter.SortNodes(&patt_graph);
			SUBSTATE_CLASS<data_t, data_t, Empty, Empty>s0(&patt_graph, &targ_graph, class_patt.data(), class_targ.data(), classifier.CountClasses(), sorted.data() PARAMETER_K_IF_NEEDED);
		
			//double vsize, rss;
			//Util::process_mem_usage(vsize, rss);
			//if (vsize > max_mem) max_mem = vsize;
			match<SUBSTATE_CLASS<data_t, data_t, Empty, Empty> >(s0, &n, n1, n2, visitor, &vis_data, num_recursive_call);
			Util::noTimeLimit();
			long end = Util::get_cur_time();
			sumt += (end-begin);
			delete []n1;
			delete []n2;
			//nembeddings += vis_data.solutions;
#ifdef PRINT_RESULT
			io.output();
			io.flush();
#endif
		}
		//delete data_graph;
	}

	// Print number of solutions, time for all solutions, time for first solution.
	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%8.0lf kB    %s\n",
		"vf3: ", nembeddings, num_recursive_call, sumt, max_mem, qid.c_str());
	
	for(i = 0; i < qnum; ++i) delete query_list[i];
	io.flush();

    //clear temporary files
    string cmd = "rm -f " + pattern + " " + target;
    system(cmd.c_str());

	return 0;
}

