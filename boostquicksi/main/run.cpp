
#include "../util/Util.h"
#include "../io/IO.h"
#include "../graph/Graph.h"
#include "../match/Match.h"
#include "../seq/Seq.h"
#include "../hyper.h"

using namespace std;

long bound = 100000, numofembeddings = 0, ncalls = 0;
double max_mem = 0.0;
long TIME_LIMIT_SECONDS = 600;    // 10 min

bool CheckQ(Graph *q, HyperGraph *gsh) {
 if (q->vertexLabelNum > gsh->LabelNum) return false;
	return true;
}

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
	//read query file and keep all queries in memory
	vector<Graph*> query_list;
	io.input_query_list(query_list);
	int qnum = query_list.size();
	
	cerr << "QSI-boost ok!"<<endl;
	long t2 = Util::get_cur_time();

    long sumt = 0;
	Graph* data_graph_ptr = NULL;
	while(true) {
		if(!io.input_data(data_graph_ptr))
			break;
		cerr.flush();

		////offline computing SE and SC for BoostISO
		//string tmpFile = "temp.g";
		////NOTICE: we need to output it to a single file, because the original data file may contain several graphs.
		//data_graph_ptr->outputGraph(tmpFile);
		//string cmd = "./gshBoostISO.exe " + tmpFile + " se.txt sc.txt";
		//system(cmd.c_str());

		//NOTICE: in se.txt sc.txt , the second line is always 0.
		HyperGraph* gsh = new HyperGraph;
		gsh->build("se.txt", "sc.txt");
		for(i = 0; i < qnum; ++i) {
			Util::timeLimit(TIME_LIMIT_SECONDS);
			long begin = Util::get_cur_time();
			Match m(query_list[i], data_graph_ptr,&io, gsh);
#ifdef PRINT_RESULT
			io.output(i);
#endif
			if(CheckQ(query_list[i], gsh))  //check the maximum label num
                m.match(io.ofp);
			Util::noTimeLimit();
			long end = Util::get_cur_time();
#ifdef PRINT_RESULT
			io.output();
			io.flush();
#endif
			sumt += (end - begin);
		}
		delete data_graph_ptr;
        delete gsh;
	}
	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%-8.0lf kB    %s\n", 
		"quicksiBoosted: ", numofembeddings, ncalls, sumt, max_mem, qid.c_str());

	//cout << "QSI-boost: " << endl;
	//cout << "  nembeddings: " << numofembeddings << endl;
	//cout << "  ncalls: " << ncalls << endl;
	//cout << "  time: " << sumt << endl;
	//cout << "  max_mem: " << sumt << "ms" << endl;

	//release all and flush cached writes
	for(i = 0; i < qnum; ++i)
		delete query_list[i];
	io.flush();

	return 0;
}

