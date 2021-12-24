#include "Graph.h"
using namespace std;




Vertex::Vertex(int _vlabel)
{
	vlabel=_vlabel;
}



Edge::Edge(int _left_vid, int _right_vid)
{
	this->left_vid=_left_vid;
	this->right_vid=_right_vid;
}





void Graph::add_vertex(int _vlabel)
{
	vertices.push_back(Vertex(_vlabel));
}

void Graph::add_edge(int _left_vid, int _right_vid)
{
	edges.push_back(Edge(_left_vid,_right_vid));
	this->vertices[_left_vid].neighbors.push_back(_right_vid);
	this->vertices[_right_vid].neighbors.push_back(_left_vid);
}


Weight_Vertex::Weight_Vertex(int _vlabel,int _weight,int _degree)
{
	vlabel=_vlabel;
	weight=_weight;
	degree=_degree;
}

Weight_Edge::Weight_Edge(int _left_vid, int _right_vid, int _weight, bool _removed)
{
	this->left_vid=_left_vid;
	this->right_vid=_right_vid;
	weight=_weight;
	removed=_removed;
}

void Weight_Graph::add_vertex(int _vlabel, int _weight)
{
	weight_vertices.push_back(Weight_Vertex(_vlabel,_weight,0));
}

void Weight_Graph::add_edge(int _left_vid, int _right_vid, int _weight, bool _removed)
{
	weight_edges.push_back(Weight_Edge(_left_vid,_right_vid,_weight,_removed));
	this->weight_vertices[_left_vid].neighbors.push_back(_right_vid);
	this->weight_vertices[_right_vid].neighbors.push_back(_right_vid);
}

void Weight_Graph::initialize_degrees()
{
	int v_num=this->weight_vertices.size();
	int* degrees= new int[v_num];
	for(int vid=0; vid<v_num; ++vid)
	{
		degrees[vid]=0;
	}
	for(vector<Weight_Edge>::iterator iter=this->weight_edges.begin();
	iter!=this->weight_edges.end();++iter)
	{
		int left_vid=(*iter).left_vid;
		int right_vid=(*iter).right_vid;
		++degrees[left_vid];
		++degrees[right_vid];
	}
	for(int vid=0; vid<v_num; ++vid)
	{
		this->weight_vertices[vid].degree=degrees[vid];
	}
	delete []degrees;
}


void Weight_Graph::remove_edge(int _eid)
{
	this->weight_edges[_eid].removed=true;
	int left_vid=this->weight_edges[_eid].left_vid;
	int right_vid=this->weight_edges[_eid].right_vid;
	--this->weight_vertices[left_vid].degree;
	--this->weight_vertices[right_vid].degree;
}

void Weight_Graph::print_weight_graph()
{
	int weight_v_num=this->weight_vertices.size();
	int weight_e_num=this->weight_edges.size();
	printf("weight graph: %d vertices, %d edges\n", weight_v_num,weight_e_num);
	printf("vertex [vlabel,weight,degree]:\n");
	for(vector<Weight_Vertex>::iterator iter=this->weight_vertices.begin();
	iter!=this->weight_vertices.end();++iter)
	{
		int vlabel=(*iter).vlabel;
		int weight=(*iter).weight;
		int degree=(*iter).degree;
		printf("[%d,%d,%d] ",vlabel,weight,degree);
	}
	printf("\n");
	printf("edge [left_vid,right_vid,weight]:\n");
	for(vector<Weight_Edge>::iterator iter=this->weight_edges.begin();
	iter!=this->weight_edges.end();++iter)
	{
		int left_vid=(*iter).left_vid;
		int right_vid=(*iter).right_vid;
		int weight=(*iter).weight;
		printf("[%d,%d,%d] ",left_vid,right_vid,weight);
	}
	printf("\n");
}


Candidate_Edge::Candidate_Edge(int _eid, int _in_seq_vid, int _outside_seq_vid)
{
	this->eid=_eid;
	this->in_seq_vid=_in_seq_vid;
	this->outside_seq_vid=_outside_seq_vid;
}


bool weight_edge_ptr_sort(Weight_Edge* _weight_e_ptr_a, Weight_Edge* _weight_e_ptr_b)
{
    int edge_weight_a=_weight_e_ptr_a->weight;
    int edge_weight_b=_weight_e_ptr_b->weight;
    return edge_weight_a<edge_weight_b;
}


