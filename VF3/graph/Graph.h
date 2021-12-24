/*=============================================================================
# Filename: Graph.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-10-24 23:00
# Description: 
=============================================================================*/

#ifndef _GRAPH_GRAPH_H
#define _GRAPH_GRAPH_H

#include "../util/Util.h"

class Neighbor
{
public:
	VID vid;
	LABEL elb;
	Neighbor()
	{
		vid = -1;
		elb = -1;
	}
	Neighbor(int _vid, int _elb)
	{
		vid = _vid;
		elb = _elb;
	}
	bool operator<(const Neighbor& _nb) const
	{
		if(this->elb == _nb.elb)
		{
			return this->vid < _nb.vid;
		}
		else
		{
			return this->elb < _nb.elb;
		}
	}
};

class Element
{
public:
	int label;
	int id;
	bool operator<(const Element& _ele) const
	{
		if(this->label == _ele.label)
		{
			return this->id <_ele.id;
		}
		else
		{
			return this->label < _ele.label;
		}
	}
};

class Vertex
{
public:
	//VID id;
	LABEL label;
	//NOTICE:VID and EID is just used in this single graph
	std::vector<Neighbor> in;
	std::vector<Neighbor> out;
	Vertex()
	{
		label = -1;
	}
	Vertex(LABEL lb):label(lb)
	{
	}
};

class Graph
{
public:
	std::vector<Vertex> vertices;
	Graph() { }
	~Graph() { }
	int vSize() const
	{
		return vertices.size();
	}
	void addVertex(LABEL _vlb);
	void addEdge(VID _from, VID _to, LABEL _elb);

	//CSR format: 4 pointers
	int vertex_num;
	int nlb;
	int* vertex_value;

	int* row_offset_in;  //range is 0~vertex_num, the final is a border(not valid vertex)
	int* edge_value_in;
	int* edge_offset_in;
	int* column_index_in;

	int* row_offset_out;
	int* edge_value_out;
	int* edge_offset_out;
	int* column_index_out;

	//Inverse Label List
	int label_num;
	int* inverse_label;
	int* inverse_offset;
	int* inverse_vertex;
	void transform(const char* _file);
};

#endif

