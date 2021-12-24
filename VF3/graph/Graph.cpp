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
}

void 
Graph::addEdge(VID _from, VID _to, LABEL _elb)
{
	this->vertices[_from].out.push_back(Neighbor(_to, _elb));
	//vector<Neighbor>& out = this->vertices[_from].out;
	//cout<<"size: "<<out.size()<<endl;
	//Neighbor tn(_to, _elb);
	//out.push_back(tn);
	this->vertices[_to].in.push_back(Neighbor(_from, _elb));
    //treated as undirected edge
	this->vertices[_to].out.push_back(Neighbor(_from, _elb));
	this->vertices[_from].in.push_back(Neighbor(_to, _elb));
}

void 
Graph::transform(const char* _file)
{
    FILE* fp = fopen(_file, "w+");
    if(fp == NULL)
    {
        cout<<"unable to open file "<<_file<<endl;
    }

	int vnum = this->vertices.size();
    fprintf(fp, "%d\n", vnum);
    for(int i = 0; i < vnum; ++i)
    {
        fprintf(fp, "%d %d\n", i, this->vertices[i].label);
    }
    for(int i = 0; i < vnum; ++i)
    {
        int eenum = this->vertices[i].in.size();
        fprintf(fp, "%d\n", eenum);
        for(int j = 0; j < eenum; ++j)
        {
            fprintf(fp, "%d %d\n", i, this->vertices[i].in[j].vid);
        }
    }

    fclose(fp);
}


