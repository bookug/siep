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
#include "../hyper.h"

#include "../../control.h"

using namespace std;


//NOTICE:a pattern occurs in a graph, then support++(not the matching num in a graph), support/N >= minsup
vector<Graph*> query_list;

bool CheckQ(Graph *q, HyperGraph *gsh)
{
	if (q->vertexLabelNum > gsh->LabelNum) return false;
	return true;
}

long TIME_LIMIT_SECONDS = 10 * 60;    //10 min
long ncalls = 0, nembeddings = 0, bound = 100000;
double max_mem = 0.0;

int main(int argc, const char * argv[]) {
	int i, j, k;

	string output = "ans.txt";
	if(argc > 4 || argc < 3)
	{
		cerr<<"invalid arguments!"<<endl;
		return -1;
	}
	string data = argv[1];
	string query = argv[2];
    int pos1 = query.rfind("/");
    int pos2 = query.rfind(".");
    string qid = query.substr(pos1+1, pos2-pos1-1);
	if(argc == 4)
	{
		output = argv[3];
	}

	//cerr<<"args all got!"<<endl;
	long t1 = Util::get_cur_time();

	IO io = IO(query, data, output);
	//read query file and keep all queries in memory
	io.input(query_list);
	int qnum = query_list.size();
	
	cerr<<"DAF-boost input ok!"<<endl;
	long t2 = Util::get_cur_time();

    long sumt = 0;
	int dgcnt = 0;
	Graph* data_graph = NULL;
	while(true)
	{
		if(!io.input(data_graph))
		{
			break;
		}
		//cout<<"data"<<dgcnt++<<endl;

		cerr.flush();

            ////offline computing SE and SC for BoostISO
			//string tmpFile = "temp.g";
            ////NOTICE: we need to output it to a single file, because the original data file may contain several graphs.
            //data_graph->outputGraph(tmpFile);
			//string cmd = "./gshBoostISO.exe " + tmpFile + " se.txt sc.txt";
            //system(cmd.c_str());
            //NOTICE: in se.txt sc.txt , the second line is always 0.
	        HyperGraph* gsh = new HyperGraph;
        	gsh->build("se.txt", "sc.txt");

        long begin = Util::get_cur_time();
		for(i = 0; i < qnum; ++i)
		{
    Util::timeLimit(TIME_LIMIT_SECONDS);
			nembeddings = 0;
			Match m(query_list[i], data_graph, gsh);
#ifdef PRINT_RESULT
			io.output(i);
#endif
			if(CheckQ(query_list[i], gsh))  //check the maximum label num
                m.match(io.ofp);
#ifdef PRINT_RESULT
			io.output();
			io.flush();
#endif
    Util::noTimeLimit();
		}
        long end = Util::get_cur_time();
        //cerr<<"match used: "<<(end-begin)<<" ms"<<endl;
        sumt += (end-begin);

		delete data_graph;
        delete gsh;
	}


	//output the time for contrast
	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%8.0lf kB    %s\n", 
		"dafBoosted: ", nembeddings, ncalls, sumt, max_mem, qid.c_str());

	/*
	cout << "DAF-boost:" << endl;
	cout << "  nembeddings: " << nembeddings  << endl;
	cout << "  ncalls: " << ncalls << endl;
	cout << "  time: "<< sumt << "ms" << endl;
	cout << "  max_mem: " << max_mem << endl;
	*/

	//release all and flush cached writes
	for(i = 0; i < qnum; ++i)
		delete query_list[i];
	io.flush();

	return 0;
}

