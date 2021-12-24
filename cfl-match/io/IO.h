/*=============================================================================
# Filename: IO.h
# Author: Jiang Yan (Frozenlight)
# Mail: 527507046@qq.com 
=============================================================================*/

#ifndef _IO_IO_H
#define _IO_IO_H

#include "../util/Util.h"
#include "../graph/Graph.h"
#include "../cpi/CPI.h"

class IO {
public:
	IO();
	IO(std::string query, std::string data, std::string file);
	bool input(std::vector<Graph*>& query_list);
	bool input(Graph*& data_graph);
	Graph *input(FILE *fp);
	bool output(int qid);
	bool output(int* m, int size);
	void outputtail();
	void flush();
	~IO();
	long tio;
public:
	std::string line;
	int data_id;
	FILE *qfp, *dfp, *ofp;
};

#endif

