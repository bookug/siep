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
	vector<Vertex> vertices;
	vector<Edge> edges;
	void add_vertex(int _vlabel);
	void add_edge(int _left_v_id, int _right_v_id);
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

