/*=============================================================================
# Filename: Graph.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-10-24 23:00
# Description: 
=============================================================================*/

#ifndef _GRAPH_DGRAPH_H
#define _GRAPH_DGRAPH_H

#include <vector>
#include <set>
#include <algorithm>

#define xfree(x) free(x); x = NULL;

typedef int LABEL;
typedef int VID;
typedef int EID;
typedef int GID;
typedef long PID;

class DNeighbor
{
public:
	VID vid;
	LABEL elb;
	DNeighbor()
	{
		vid = -1;
		elb = -1;
	}
	DNeighbor(int _vid, int _elb)
	{
		vid = _vid;
		elb = _elb;
	}
	bool operator<(const DNeighbor& _nb) const
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
	std::vector<DNeighbor> in;
	std::vector<DNeighbor> out;
	Vertex()
	{
		label = -1;
	}
	Vertex(LABEL lb):label(lb)
	{
	}
};

class DGraph
{
public:
	std::vector<Vertex> vertices;
	DGraph() { }
	~DGraph() { }
	int vSize() const
	{
		return vertices.size();
	}
	void addVertex(LABEL _vlb);
	void addEdge(VID _from, VID _to, LABEL _elb);

	//CSR format: 4 pointers
	int vertex_num;
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
	void transformToCSR();

	bool isEdgeContained(VID from, VID to, LABEL label);
};

#endif

