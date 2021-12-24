/*=============================================================================
# Filename: Graph.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-10-24 23:01
# Description: 
=============================================================================*/

#include "Graph.h"

using namespace std;

void 
Graph::addVertex(LABEL _vlb)
{
	this->vertices.push_back(Vertex(_vlb));
    this->freq[_vlb]++;
}

void 
Graph::addEdge(VID _from, VID _to, LABEL _elb)
{
	this->vertices[_from].out.push_back(Neighbor(_to, _elb, edge_num));
	//vector<Neighbor>& out = this->vertices[_from].out;
	//cout<<"size: "<<out.size()<<endl;
	//Neighbor tn(_to, _elb);
	//out.push_back(tn);
	this->vertices[_to].in.push_back(Neighbor(_from, _elb, edge_num));
    //treated as undirected edge
    //if(_elb != 2)
    if(!directed)
    {
        this->vertices[_to].out.push_back(Neighbor(_from, _elb, edge_num));
        this->vertices[_from].in.push_back(Neighbor(_to, _elb, edge_num));
    }
    //NOTICE: if elb is 2, then this is a directed edge.
    this->edge_num++;
}

void 
Graph::transform()
{
	this->vertex_num = this->vertices.size();
	for(int i = 0; i < this->vertex_num; ++i)
	{
		//sort on label, when label is identical, sort on VID
		sort(this->vertices[i].in.begin(), this->vertices[i].in.end());
		sort(this->vertices[i].out.begin(), this->vertices[i].out.end());
    }
}



