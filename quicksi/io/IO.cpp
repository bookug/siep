

#include "IO.h"

using namespace std;

IO::IO()
{
	this->qfp = NULL;
	this->dfp = NULL;
	this->ofp = NULL;
	this->data_id = -1;
}

IO::IO(string _query_path, string _data_path, string _out_file_path)
{
	this->data_id = -1;
	this->line = "============================================================";
	qfp = fopen(_query_path.c_str(), "r");
	if(qfp == NULL)
	{
		cerr<<"input open error!"<<endl;
		return;
	}
	dfp = fopen(_data_path.c_str(), "r");
	if(dfp == NULL)
	{
		cerr<<"input open error!"<<endl;
		return;
	}
	ofp = fopen(_out_file_path.c_str(), "w+");
	if(ofp == NULL)
	{
		cerr<<"output open error!"<<endl;
		return;
	}
}

Graph* IO::input_query_file(FILE* _fp)
{
	char c1, c2;
	int id0, id1, id2, lb;
	bool flag = false;
	Graph* ng = NULL;

	while(true)
	{
		fscanf(_fp, "%c", &c1);
		if(c1 == 't')
		{
			if(flag)
			{
				fseek(_fp, -1, SEEK_CUR);
				return ng;
			}
			flag = true;
			fscanf(_fp, " %c %d\n", &c2, &id0);
			if(id0 == -1)
			{
				return NULL;
			}
			else
			{
				ng = new Graph;
			}
			//read vertex num, edge num, vertex label num, edge label num
			int numVertex, numEdge, vertexLabelNum;
			fscanf(_fp, " %d %d %d\n", &numVertex, &numEdge, &vertexLabelNum);
		}
		else if(c1 == 'v')
		{
			fscanf(_fp, " %d %d\n", &id1, &lb);
			ng->add_vertex(lb); 
		}
		else if(c1 == 'e')
		{
			fscanf(_fp, " %d %d\n", &id1, &id2);
			//NOTICE:we treat this graph as directed, each edge represents two
			//This may cause too many matchings, if to reduce, only add the first one
			ng->add_edge(id1, id2);
		}
		else 
		{
			cerr<<"ERROR in input() -- invalid char"<<endl;
			return NULL;
		}
	}
	return NULL;
}

Graph* IO::input_data_file(FILE* _fp)
{
	char c1, c2;
	int id0, id1, id2, lb;
	bool flag = false;
	Graph* ng = NULL;

	while(true)
	{
		fscanf(_fp, "%c", &c1);
		if(c1 == 't')
		{
			if(flag)
			{
				fseek(_fp, -1, SEEK_CUR);
				return ng;
			}
			flag = true;
			fscanf(_fp, " %c %d\n", &c2, &id0);
			if(id0 == -1)
			{
				return NULL;
			}
			else
			{
				ng = new Graph;
			}
			//read vertex num, edge num, vertex label num, edge label num
			int numVertex, numEdge, vertexLabelNum;
			fscanf(_fp, " %d %d %d\n", &numVertex, &numEdge, &vertexLabelNum);
		}
		else if(c1 == 'v')
		{
			fscanf(_fp, " %d %d\n", &id1, &lb);
			ng->add_vertex(lb); 
		}
		else if(c1 == 'e')
		{
			fscanf(_fp, " %d %d\n", &id1, &id2);
    
			//NOTICE:we treat this graph as directed, each edge represents two
			//This may cause too many matchings, if to reduce, only add the first one

			ng->add_edge(id1, id2);
			//ng->addEdge(id2, id1, lb);
		}
		else 
		{
			cerr<<"ERROR in input() -- invalid char"<<endl;
			return NULL;
		}
	}
	return NULL;
}

bool 
IO::input_data(Graph*& _data_graph)
{
	_data_graph = this->input_data_file(this->dfp);
	// whether all data graphs have been read
	if(_data_graph == NULL)
		return false;
	this->data_id++;
	//data_graph->transformToCSR();
	return true;
}

bool 
IO::input_query_list(vector<Graph*>& _query_list)
{
	Graph* query_ptr = NULL;
	while(true)
	{
		query_ptr = this->input_query_file(this->qfp);
		if(query_ptr == NULL) //to the end
			break;
		_query_list.push_back(query_ptr);
	}

	return true;
}

bool 
IO::output(int qid)
{
	fprintf(ofp, "query graph:%d    data graph:%d\n", qid, this->data_id);
	fprintf(ofp, "%s\n", line.c_str());
	return true;
}

bool
IO::output()
{
	fprintf(ofp, "\n\n\n");
	return true;
}

bool 
IO::output(int* m, int size)
{
	for(int i = 0; i < size; ++i)
	{
		fprintf(ofp, "(%d, %d) ", i, m[i]);
	}
	fprintf(ofp, "\n");
	return true;
}

void
IO::flush()
{
	fflush(this->ofp);
}

IO::~IO()
{
	fclose(this->qfp);
	this->qfp = NULL;
	fclose(this->dfp);
	this->dfp = NULL;
	fclose(this->ofp);
	this->ofp = NULL;
}

