#ifndef _GRAPH_GRAPH_H
#define _GRAPH_GRAPH_H

#include "../util/Util.h"

class Neighbor
{
public:
	int vid;
	// no elabel
	Neighbor();
	Neighbor(int _vid);
	bool operator<(const Neighbor& _nb) const
	{
		return this->vid < _nb.vid;
	}
};



class QueryVertex
{
public:
	
	int vid;
	int vlabel;
	std::vector<Neighbor> neighbors;
	std::set<int> candidates;
	
	QueryVertex();
    ~QueryVertex();
    QueryVertex(int _vid,int _vlabel);
};

class QueryGraph
{
public:
	std::vector<QueryVertex> vertices;
	QueryGraph() { }
	~QueryGraph();

	void add_vertex(int _vid,int  _vlb);
	void add_edge(int _from_vid, int _to_vid);

};

class DataVertex
{
public:
	
	int vid;
	int vlabel;
	std::vector<Neighbor> neighbors;

	DataVertex();
	~DataVertex();
	DataVertex(int _vid,int _vlabel);
};



class DataGraph
{
	public:
	std::vector<DataVertex> vertices;
	DataGraph() { }
	~DataGraph() { }

	void add_vertex(int _vid,int  _vlb);
	void add_edge(int _from_vid, int _to_vid);
};

// class Element
// {
// public:
// 	int label;
// 	int id;
// 	bool operator<(const Element& _ele) const
// 	{
// 		if(this->label == _ele.label)
// 		{
// 			return this->id <_ele.id;
// 		}
// 		else
// 		{
// 			return this->label < _ele.label;
// 		}
// 	}
// };

#endif

