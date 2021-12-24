
#include "../util/Util.h"
#include "../io/IO.h"
#include "../graph/Graph.h"
#include "../match/Match.h"
#include "../seq/Seq.h"
#include "../hyper.h"

using namespace std;


long TIME_LIMIT_SECONDS = 10 * 60;    //10 min

bool CheckQ(Graph *q, HyperGraph *gsh)
{
	if (q->vertexLabelNum > gsh->LabelNum) return false;
	return true;
}



int
main(int argc, const char * argv[])
{


	int i, j, k;

	
	string data_path = argv[1];
	string query_path = argv[2];
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
	printf("there are %d query graphs\n",qnum);
	
	// cerr<<"input ok!"<<endl;
	long t2 = Util::get_cur_time();

    long sumt = 0;
	Graph* data_graph_ptr = NULL;
	while(true)
	{
		if(!io.input_data(data_graph_ptr))
		{
			break;
		}

		cerr.flush();
        	//offline computing SE and SC for BoostISO
	        string tmpFile = "temp.g";
            //NOTICE: we need to output it to a single file, because the original data file may contain several graphs.
        	data_graph_ptr->outputGraph(tmpFile);
	        string cmd = "./gshBoostISO.exe " + tmpFile + " se.txt sc.txt";
        	system(cmd.c_str());
            //NOTICE: in se.txt sc.txt , the second line is always 0.
	        HyperGraph* gsh = new HyperGraph;
        	gsh->build("se.txt", "sc.txt");

		//cout<<"get a data graph"<<endl;
         long begin = Util::get_cur_time();
		for(i = 0; i < qnum; ++i)
		{
    Util::timeLimit(TIME_LIMIT_SECONDS);
			Match m(query_list[i], data_graph_ptr,&io, gsh);
			// printf("Match object created\n");
			io.output(i);
			//printf("match begin\n");
			if(CheckQ(query_list[i], gsh))  //check the maximum label num
                m.match(io.ofp);
			io.output();
			io.flush();
    Util::noTimeLimit();
		}
         long end = Util::get_cur_time();
        //cerr<<"match used: "<<(end-begin)<<" ms"<<endl;
         sumt += (end-begin);

		delete data_graph_ptr;
        delete gsh;
	}

	cerr<<"match ended!"<<endl;
	long t3 = Util::get_cur_time();

	//output the time for contrast
	//cerr<<"part 1 used: "<<(t2-t1)<<"ms"<<endl;
	//cerr<<"part 2 used: "<<(t3-t2)<<"ms"<<endl;
	cerr<<"total time used: "<<sumt<<"ms"<<endl;

	//release all and flush cached writes
	for(i = 0; i < qnum; ++i)
	{
		delete query_list[i];
	}
	io.flush();

	return 0;
}

