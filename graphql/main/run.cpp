#include "../util/Util.h"
#include "../io/IO.h"
#include "../graph/Graph.h"
#include "../match/Match.h"
//#define _PRINT_ANS
using namespace std;

//NOTICE:a pattern occurs in a graph, then support++(not the matching num in a graph), support/N >= minsup
vector<QueryGraph*> query_list;
long numofembeddings = 0, bound = 100000, ncalls = 0;
double max_mem = 0.0;
long TIME_LIMIT_SECONDS = 600;    //10 min

int main(int argc, const char * argv[]) 
{
	//int nei_radius=2;
    //NOTICE: generally, 1-hop neighborhood signature is enough.
    int nei_radius=1;
    //int global_refine_level=2;
    //NOTICE: use 4 for large graphs.
    int global_refine_level=4;
	int i, j, k;
	string output_path = "ans.txt";
	if(argc > 6 || argc < 3) {
		cerr<<"invalid arguments!"<<endl;
		return -1;
	}
	string data_path = argv[1];
	string query_path = argv[2];
    int pos1 = query_path.rfind("/");
    int pos2 = query_path.rfind(".");
    string qid = query_path.substr(pos1+1, pos2-pos1-1);
	if(argc >= 4)	output_path = argv[3];
	if(argc>=5)	nei_radius=atoi(argv[4]);
	if(argc>=6)	global_refine_level=atoi(argv[5]);
	//cerr<<"GQL args all got!"<<endl;
	long t1 = Util::get_cur_time();
	IO io = IO(query_path, data_path, output_path);
	//read query file and keep all queries in memory
	io.input_query_list(query_list);
	int qnum = query_list.size();
	//cerr<<"input ok!"<<endl;
	long t2 = Util::get_cur_time();   
	long sumt = 0;
	DataGraph* data_graph = NULL;
	int cnt = 0; 	// jiangyan add
	int data_graph_id=0;
	while(true) {
		if(!io.input_data(data_graph))	
			break;
		
		for(i = 0; i < qnum; ++i) {
			//printf("query %d\n", i);
            Util::timeLimit(TIME_LIMIT_SECONDS);
			long begin = Util::get_cur_time();
			Match m(query_list[i], data_graph, 0, 1000);
#ifdef PRINT_RESULT
			io.output(i);
#endif
			m.match(&io,nei_radius,global_refine_level);
			long end = Util::get_cur_time();
			sumt += (end-begin);
            Util::noTimeLimit();
#ifdef PRINT_RESULT
			io.output();
			io.flush();
#endif
		}
		delete data_graph;
		++data_graph_id;
	}
	//cerr<<"match ended!"<<endl;
	//cerr<<"part 1 used: "<<(t2-t1)<<"ms"<<endl;
	//cerr<<"part 2 used: "<<(t3-t2)<<"ms"<<endl;
	// cout << "GQL total time: "<<sumt<<"ms"<<endl;
	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%-8.0lf kB    %s\n", 
		"graphql: ", numofembeddings, ncalls, sumt, max_mem, qid.c_str());
	
	for(i = 0; i < qnum; ++i)
		delete query_list[i];
	io.flush();
	return 0;
}
