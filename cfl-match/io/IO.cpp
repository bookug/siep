/*=============================================================================
# Filename: IO.cpp
# Author: Jiang Yan (Frozenlight)
# Mail: 527507046@qq.com 
=============================================================================*/

#include "IO.h"
using namespace std;

IO::IO(): data_id(-1), qfp(NULL), dfp(NULL), ofp(NULL){}

IO::IO(string query, string data, string file): data_id(-1) {
	this->line = "============================================================";
	qfp = fopen(query.c_str(), "r");
	if(qfp == NULL) {
		cerr<<"input open error!"<<endl;
		exit(1);
	}
	dfp = fopen(data.c_str(), "r");
	if(dfp == NULL) {
		cerr<<"input open error!"<<endl;
		exit(1);
	}
	ofp = fopen(file.c_str(), "w+");
	if(ofp == NULL) {
		cerr<<"output open error!"<<endl;
		exit(1);
	}
}

Graph *IO::input(FILE *fp) {
	char c1, c2;
	int id0, id1, id2, lb;
	bool isReading = false;
	Graph* ng = NULL;

	while(true) {	// Read until meet t, ignore #vetices and #edges
		fscanf(fp, "%c", &c1);
		if(c1 == 't') {
			if(isReading) {
				/* (c1==t && isReading) mark the end of a single graph
				 * Move ptr backward for reading the next graph 
				 */
				fseek(fp, -1, SEEK_CUR);
				return ng;
			}
			isReading = true;
			fscanf(fp, " %c %d\n", &c2, &id0);
			if(id0 == -1)
				/* The end of a graph file */
				return NULL;	
			else{
				//read vertex_num, edge_num, vertex_label_num
				int numVertex, numEdge, vertexLabelNum;
				fscanf(fp, " %d %d %d\n", &numVertex, &numEdge, &vertexLabelNum);
				/* Begin a new graph */
				ng = new Graph(numVertex, numEdge, vertexLabelNum);
			
			}
		}
		else if(c1 == 'v') {
			fscanf(fp, " %d %d\n", &id1, &lb);
			ng->addVertex(id1, lb); 
		}
		else if(c1 == 'e') {
			fscanf(fp, " %d %d\n", &id1, &id2);
			ng->addEdge(id1, id2);
		}
		else  {
			cerr<<"ERROR in input() -- invalid char"<<endl;
			exit(0);
		}
	}
	return NULL;
}

bool IO::input(Graph *&data_graph) {
	long t1 = Util::get_cur_time();
	data_graph = input(dfp);
	if (data_graph == NULL)
		return false;
	data_graph->Precond();
	data_id++;
	long t2 = Util::get_cur_time();
	tio = t2-t1;
	return true;
}

bool IO::input(vector<Graph*> &query_list) {
	Graph *graph = NULL;
	int cnt = 0;
	while((graph = input(qfp)) != NULL){	// Read all query graphs into query_list
		query_list.push_back(graph);
		graph->decompose();
		graph->Precond();
		cnt++;
	}
	return true;
}

bool IO::output(int qid) {
	fprintf(ofp, "query graph:%d    data graph:%d\n", qid, data_id);
	fprintf(ofp, "%s\n", line.c_str());
	return true;
}

void IO::outputtail(){
	fprintf(ofp, "\n\n\n");
}

bool IO::output(int *m, int size) {
	for(int i = 0; i < size; ++i)
		fprintf(ofp, "(%d, %d) ", i, m[i]);
	fprintf(ofp, "\n");
	return true;
}

void IO::flush() {
	fflush(this->ofp);
}

IO::~IO() {
	fclose(qfp); 
	qfp = NULL;
	fclose(dfp); 
	dfp = NULL;
	fclose(ofp); 
	ofp = NULL;
}

