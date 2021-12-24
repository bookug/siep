
#ifndef _IO_IO_H
#define _IO_IO_H

#include "../util/Util.h"
#include "../graph/Graph.h"

class IO
{
public:
	IO();
	IO(std::string _query_path, std::string _data_se_path, std::string _data_sc_path, std::string _output_path);
	bool input_query_list(std::vector<QueryGraph*>& query_list);
	bool input_hyper_data_graph(Hyper_Data_Graph* _hyper_data_graph_ptr);
	QueryGraph* input_query_file(FILE* fp);

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
	std::string data_se_path;
	std::string data_sc_path;
	//output file pointer
	FILE* ofp;
};

#endif

