#ifndef _GRAPH_GRAPH_H
#define _GRAPH_GRAPH_H

#include "../util/Util.h"
using namespace std;

class Vertex
{
public:
	int vlabel;
	vector<int> neighbors;
	Vertex(int _vlabel);
};

class Edge
{
public:
	int left_vid;
	int right_vid;
	Edge(int _left_vid, int _right_vid);
};

class Graph
{
public:
	int vertex_num;
	int edge_num;
    int vertexLabelNum;
	vector<Vertex> vertices;
	vector<Edge> edges;
	void add_vertex(int _vlabel);
	void add_edge(int _left_v_id, int _right_v_id);
	Graph() 
    { 
        this->vertex_num = 0;
        this->edge_num = 0;
        this->vertexLabelNum = 0;
    }
    void outputGraph(string tmpFile)
    {
        FILE* fp = fopen(tmpFile.c_str(), "w+");
        if(fp == NULL)
        {
            cout<<"Error in outputGraph()"<<endl;
        }
        fprintf(fp, "t # 0\n");
        fprintf(fp, "%d %d %d\n", this->vertex_num, this->edge_num, this->vertexLabelNum);
        for(int i = 0; i < this->vertices.size(); ++i)
        {
            fprintf(fp, "v %d %d\n", i, this->vertices[i].vlabel);
        }
        for(int i = 0; i < this->edges.size(); ++i)
        {
            fprintf(fp, "e %d %d\n", edges[i].left_vid, edges[i].right_vid);
        }
        fprintf(fp, "t # -1\n");
        fclose(fp);
    }
};



class Weight_Vertex
{
public:
	int vlabel;
	int weight;
	int degree;
	vector<int> neighbors;
	Weight_Vertex(int _vlabel, int _weight, int degree);
};



class Weight_Edge
{
public:
	int left_vid;
	int right_vid;
	int weight;
	bool removed;
	Weight_Edge(int _left_vid, int _right_vid, int _weight, bool _removed);
};


class Weight_Graph
{
public:
	vector<Weight_Vertex> weight_vertices;
	vector<Weight_Edge> weight_edges;
	void add_vertex(int _vlabel, int _weight);
	void add_edge(int _left_vid, int _right_vid, int _weight, bool _removed);
	void remove_edge(int _eid);
	void initialize_degrees();
	void print_weight_graph();
};

class Candidate_Edge
{
	// candidate edge is directed
public:
	// left vertex in SEQ while right vertex not in SEQ
	int eid;
	int in_seq_vid;
	int outside_seq_vid;
	Candidate_Edge(int _eid,int _in_seq_vid, int _outside_seq_vid);
};

bool weight_edge_ptr_sort(Weight_Edge* _weight_e_ptr_a, Weight_Edge* _weight_e_ptr_b);

#endif

