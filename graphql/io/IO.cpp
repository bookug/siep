/*=============================================================================
# Filename: IO.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-10-24 22:55
# Description: 
=============================================================================*/

#include "IO.h"

using namespace std;

IO::IO()
{
	this->qfp = NULL;
	this->dfp = NULL;
	this->ofp = NULL;
	this->data_id = -1;
}

IO::IO(string query, string data, string file)
{
	this->data_id = -1;
	this->line = "============================================================";
	qfp = fopen(query.c_str(), "r");
	if(qfp == NULL)
	{
		cerr<<"input open error!"<<endl;
		return;
	}
	dfp = fopen(data.c_str(), "r");
	if(dfp == NULL)
	{
		cerr<<"input open error!"<<endl;
		return;
	}
	ofp = fopen(file.c_str(), "w+");
	if(ofp == NULL)
	{
		cerr<<"output open error!"<<endl;
		return;
	}
}

QueryGraph* IO::input_query_file(FILE* fp)
{
	char c1, c2;
	int id0, id1, id2, lb;
	bool flag = false;
	QueryGraph* ng = NULL;

	while(true)
	{
		fscanf(fp, "%c", &c1);
		if(c1 == 't')
		{
			if(flag)
			{
				fseek(fp, -1, SEEK_CUR);
				return ng;
			}
			flag = true;
			fscanf(fp, " %c %d\n", &c2, &id0);
			if(id0 == -1)
			{
				return NULL;
			}
			else
			{
				ng = new QueryGraph;
			}
			//read vertex num, edge num, vertex label num, edge label num
			int numVertex, numEdge, vertexLabelNum;
			fscanf(fp, " %d %d %d\n", &numVertex, &numEdge, &vertexLabelNum);
		}
		else if(c1 == 'v')
		{
			fscanf(fp, " %d %d\n", &id1, &lb);
			ng->add_vertex(id1,lb); 
		}
		else if(c1 == 'e')
		{
			fscanf(fp, " %d %d\n", &id1, &id2);
            // lb = 1;
			//NOTICE:we treat this graph as directed, each edge represents two
			//This may cause too many matchings, if to reduce, only add the first one
			//cout<<"check: "<<id1<<" "<<id2<<" "<<lb<<endl;
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

DataGraph* IO::input_data_file(FILE* fp)
{
	char c1, c2;
	int id0, id1, id2, lb;
	bool flag = false;
	DataGraph* ng = NULL;

	while(true)
	{
		fscanf(fp, "%c", &c1);
		if(c1 == 't')
		{
			
			if(flag)
			{
				fseek(fp, -1, SEEK_CUR);
				return ng;
			}
			flag = true;
			fscanf(fp, " %c %d\n", &c2, &id0);
			cout<<c2<<" "<<id0<<endl;
			if(id0 == -1)
			{
				return NULL;
			}
			else
			{
				ng = new DataGraph;
			}
			//read vertex num, edge num, vertex label num, edge label num
			int numVertex, numEdge, vertexLabelNum;
			fscanf(fp, " %d %d %d\n", &numVertex, &numEdge, &vertexLabelNum);
		}
		else if(c1 == 'v')
		{
			fscanf(fp, " %d %d\n", &id1, &lb);
			ng->add_vertex(id1,lb); 
		}
		else if(c1 == 'e')
		{
			fscanf(fp, " %d %d\n", &id1, &id2);
            // lb = 1;
			//NOTICE:we treat this graph as directed, each edge represents two
			//This may cause too many matchings, if to reduce, only add the first one
			//cout<<"check: "<<id1<<" "<<id2<<" "<<lb<<endl;
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
IO::input_data(DataGraph*& data_graph)
{
	data_graph = this->input_data_file(this->dfp);
	if(data_graph == NULL)
		return false;
	this->data_id++;
	//data_graph->transformToCSR();
	return true;
}

bool 
IO::input_query_list(vector<QueryGraph*>& query_list)
{
	QueryGraph* graph = NULL;
	while(true)
	{
		graph = this->input_query_file(qfp);
		if(graph == NULL) //to the end
			break;
		//graph->transformToCSR();
		query_list.push_back(graph);
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
IO::output(vector<int>& m, int size)
{
	for(int i = 0; i < size; ++i)
	{
		fprintf(ofp, "(%d) ",m[i]);
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
