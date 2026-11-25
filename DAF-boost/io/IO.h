
#ifndef _IO_IO_H
#define _IO_IO_H

#include "../util/Util.h"
#include "../graph/Graph.h"

class IO
{
public:
	IO();
	IO(std::string query, std::string data, std::string file);
	bool input(std::vector<Graph*>& query_list);
	bool input(Graph*& data_graph);
	Graph* input(FILE* fp);
	bool output(int qid);
	bool output();
	bool output(int* m, int size);
	void flush();
	~IO();
	//output file pointer
	FILE* ofp;

private:
	std::string line;
	int data_id;
	//query file pointer
	FILE* qfp;
	//data file pointer
	FILE* dfp;
};

#endif

