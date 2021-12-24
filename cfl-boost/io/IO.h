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
	IO(std::string query, std::string data, std::string sc, std::string file);
	bool readData();
	bool input(std::vector<Graph*>& query_list);
	bool input(Graph*& data_graph);
	Graph *input(FILE *fp);
	Graph *datainput(FILE *fp);
	void SCRead(Graph *data_graph);

	bool output(int qid);
	bool output(int* m, int size);
	void outputtail();
	void flush();
	~IO();

	long tio;
	std::string line;
	int data_id, cachen, cachelb;
	FILE *qfp, *dfp, *pdfp, *ofp, *scfp;
};

#endif

