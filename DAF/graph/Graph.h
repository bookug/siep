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
    int eid;
	Neighbor()
	{
		vid = -1;
		elb = -1;
        eid = -1;
	}
	Neighbor(int _vid, int _elb, int _eid)
	{
		vid = _vid;
		elb = _elb;
        eid = _eid;
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
    bool directed;
	Graph() 
    { 
        this->vertex_num = 0;
        this->edge_num = 0;
        this->vertexLabelNum = 0;
        this->directed = false;
    }
	Graph(bool _directed) 
    { 
        this->vertex_num = 0;
        this->edge_num = 0;
        this->vertexLabelNum = 0;
        this->directed = _directed;
    }
    void setDirected(bool _directed)
    {
        this->directed = _directed;
    }
	~Graph() { }
	int vSize() const
	{
		return vertices.size();
	}

	int vertex_num;
	int edge_num;
    int vertexLabelNum;
    std::vector<int> freq;
    void initLabelFreq(int _label_num)
    {
        this->vertexLabelNum = _label_num;
        for(int i = 0; i <= _label_num; ++i)
        {
            freq.push_back(0);
        }
    }
    int getLabelFreq(int l)
    {
        if(l > vertexLabelNum)
        {
            return 0;
        }
        return this->freq[l];
    }
    void transform();
	void addVertex(LABEL _vlb);
	void addEdge(VID _from, VID _to, LABEL _elb);
    bool isLeaf(int v)
    {
        if(directed)
        {
            return (this->vertices[v].in.size() + this->vertices[v].out.size()==1);
        }
        else
        {
            return (this->vertices[v].in.size()==1);
        }
    }
};

#endif

