
#ifndef _IO_IO_H
#define _IO_IO_H

#include "../util/Util.h"
#include "../graph/Graph.h"

class IO
{
public:
	IO();
	IO(string _query_path, string _data_path, string _out_file_path);
	bool input_query_list(std::vector<Graph*>& _weight_query_list);
	bool input_data(Graph*& data_graph);
	Graph* input_query_file(FILE* fp);
	Graph* input_data_file(FILE* fp);
	bool output(int qid);
	bool output();
	bool output(int* m, int size);
	void flush();
	~IO();

	std::string line;
	int data_id;
	//query file pointer
	FILE* qfp;
	//data file pointer
	FILE* dfp;
	//output file pointer
	FILE* ofp;
};

#endif

