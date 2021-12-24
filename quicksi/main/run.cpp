
#include "../util/Util.h"
#include "../io/IO.h"
#include "../graph/Graph.h"
#include "../match/Match.h"
#include "../seq/Seq.h"

using namespace std;

long bound = 100000, nembeddings = 0, ncalls = 0;
double max_mem = 0.0;

int main(int argc, const char * argv[]) {


	int i, j, k;
	
	string data_path = argv[1];
	string query_path = argv[2];
    int pos1 = query_path.rfind("/");
    int pos2 = query_path.rfind(".");
    string qid = query_path.substr(pos1+1, pos2-pos1-1);
	string output_path;
	if (argc == 4)	output_path = argv[3];
	else if (argc == 3) output_path = "ans.txt";
	else {
		printf("argument error\n");
		exit(0);
	}
	// if(argc >= 4)
	// {
	// 	output_path = argv[3];
	// }
	// if(argc>=5)
	// {
	// 	nei_radius=atoi(argv[4]);
	// }
	// if(argc>=6)
	// {
	// 	global_refine_level=atoi(argv[5]);
	// }

	// cerr<<"args all got!"<<endl;
	// long t1 = Util::get_cur_time();

	IO io = IO(query_path, data_path, output_path);
	// printf("io object created\n");
	//read query file and keep all queries in memory
	vector<Graph*> query_list;
	io.input_query_list(query_list);
	int qnum = query_list.size();
	
	cerr << "QSI ok!" << endl;
	long t2 = Util::get_cur_time();

    long sumt = 0;
	Graph* data_graph_ptr = NULL;
	while(true) {
		if(!io.input_data(data_graph_ptr))
			break;
		for(i = 0; i < qnum; ++i) {
			long begin = Util::get_cur_time();
			nembeddings = 0;
			Util::timeLimit(600);
			Match m(query_list[i], data_graph_ptr,&io);			
#ifdef PRINT_RESULT
			io.output(i);
#endif
			m.match();
			Util::noTimeLimit();
			long end = Util::get_cur_time();
#ifdef PRINT_ANS
			io.output();
			io.flush();
#endif
			sumt += (end - begin);
		}
		delete data_graph_ptr;
	}

	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%8.0lf kB    %s\n", 
		"quicksi: ", nembeddings, ncalls, sumt, max_mem, qid.c_str());
/*	
	cout << "QSI: " << endl;
	cout << "  nembeddings: " << nembeddings << endl;
	cout << "  ncalls: " << ncalls << endl;
	cout << "  time: " << sumt << "ms" << endl;
	cout << "  max_mem: " << max_mem << endl;
*/	
	for(i = 0; i < qnum; ++i)
		delete query_list[i];
	io.flush();

	return 0;
}

