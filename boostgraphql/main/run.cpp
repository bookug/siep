
#include "../util/Util.h"
#include "../io/IO.h"
#include "../graph/Graph.h"
#include "../match/Match.h"

using namespace std;

//NOTICE:a pattern occurs in a graph, then support++(not the matching num in a graph), support/N >= minsup
vector<QueryGraph*> query_graph_ptr_list;

long bound = 100000, numofembeddings = 0, ncalls = 0;
double max_mem = 0.0;
long TIME_LIMIT_SECONDS = 600;   //10 min

int
main(int argc, const char * argv[])
{
	//int nei_radius=2;
	int nei_radius=1;
	//int global_refine_level=2;
	int global_refine_level=4;

	int i, j, k;

	string output_path = "ans.txt";
	if(argc > 4 || argc < 3)
	{
		cerr<<"invalid arguments!"<<endl;
		return -1;
	}
	
	string data_path = argv[1];
	string query_path = argv[2];
    int pos1 = query_path.rfind("/");
    int pos2 = query_path.rfind(".");
    string qid = query_path.substr(pos1+1, pos2-pos1-1);
	// string data_sc_path=argv[3];
	if(argc >= 4)
	{
		output_path = argv[3];
	}

	//string cmd = "./gshBoostISO.exe " + data_path + " se.txt sc.txt";
    //system(cmd.c_str());

	string data_se_path="se.txt";
	string data_sc_path="sc.txt";
	// if(argc>=6)
	// {
	// 	nei_radius=atoi(argv[5]);
	// }
	// if(argc>=7)
	// {
	// 	global_refine_level=atoi(argv[6]);
	// }

	//cerr<<"args all got!"<<endl;
	long t1 = Util::get_cur_time();

	IO io = IO(query_path,data_se_path,data_sc_path,output_path);
	//read query file and keep all queries in memory
	io.input_query_list(query_graph_ptr_list);
	int qnum = query_graph_ptr_list.size();
	//printf("there are %d query graphs\n",qnum);
	
	// cerr<<"input ok!"<<endl;
	long t2 = Util::get_cur_time();

    long sumt = 0;
	// DataGraph* data_graph = NULL;

	// if(!io.input_data(data_graph))
	// {
	// 	break;
	// }
	Hyper_Data_Graph* hyper_data_graph_ptr = new Hyper_Data_Graph;
	//cout<<"input data hyper graph:"<<endl;
	io.input_hyper_data_graph(hyper_data_graph_ptr);
	//cout<<"get a hyper data graph"<<endl;
	
	long begin = Util::get_cur_time();
	for(int q_g_id = 0; q_g_id < qnum; ++q_g_id)
	{
        Util::timeLimit(TIME_LIMIT_SECONDS);
		Match m(query_graph_ptr_list[q_g_id], hyper_data_graph_ptr);
#ifdef PRINT_RESULT
		io.output(q_g_id);
#endif
		m.match(&io,nei_radius,global_refine_level);
#ifdef PRINT_RESULT
		io.output();
		io.flush();
#endif
        Util::noTimeLimit();
	}
	long end = Util::get_cur_time();
	//cerr<<"match used: "<<(end-begin)<<" ms"<<endl;
	sumt += (end-begin);

	delete hyper_data_graph_ptr;


	cerr<<"match ended!"<<endl;
	long t3 = Util::get_cur_time();

	//output the time for contrast
	//cerr<<"part 1 used: "<<(t2-t1)<<"ms"<<endl;
	//cerr<<"part 2 used: "<<(t3-t2)<<"ms"<<endl;
	//cerr<<"total time used: "<<sumt<<"ms"<<endl;

	//release all and flush cached writes
	for(i = 0; i < qnum; ++i)
	{
		delete query_graph_ptr_list[i];
	}
	io.flush();

	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%-8.0lf kB    %s\n", 
		"graphqlBoosted: ", numofembeddings, ncalls, sumt, max_mem, qid.c_str());

	return 0;
}

