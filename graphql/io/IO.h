
#ifndef _IO_IO_H
#define _IO_IO_H

#include "../util/Util.h"
#include "../graph/Graph.h"

class IO
{
public:
	IO();
	IO(std::string query, std::string data, std::string file);
	bool input_query_list(std::vector<QueryGraph*>& query_list);
	bool input_data(DataGraph*& data_graph);
	QueryGraph* input_query_file(FILE* fp);
	DataGraph* input_data_file(FILE* fp);
	bool output(int qid);
	bool output();
	bool output(std::vector<int>& m, int size);
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

