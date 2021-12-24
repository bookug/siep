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
using namespace std;

class Neighbor {
public:
	VID vid;
	LABEL elb;
	Neighbor(): vid(-1), elb(-1) { }
	Neighbor(int _vid, int _elb): vid(_vid), elb(_elb) { }
	bool operator<(const Neighbor& _nb) const {
		return vid < _nb.vid;
	}
};


class Vertex {
public:
	//VID id;
	LABEL label;
	//NOTICE:VID and EID is just used in this single graph
	vector<Neighbor> neigh;
	Vertex(): label(-1) { }
	Vertex(LABEL lb):label(lb) {}
};

class Graph {
public:
	vector<Vertex> vertices;
	Graph():vertex_num(0), edge_num(0), vertexLabelNum(0), directed(false), vLabel(NULL) { }
	~Graph() { if(vLabel) delete[] vLabel; }
	int vSize() const { return vertices.size(); }
	void addVertex(LABEL _vlb);
	void addEdge(VID _from, VID _to, LABEL _elb);

	//CSR format: 4 pointers
	int vertex_num;

	
	int *vLabel;
	void transform();
	bool directed;
	int edge_num;
    int vertexLabelNum;
    vector<int> freq;

	bool isLeaf(int v) {
        return (vertices[v].neigh.size()==1);
    }

    void outputGraph(string tmpFile) {
        FILE* fp = fopen(tmpFile.c_str(), "w+");
        if(fp == NULL)
            cout<<"Error in outputGraph()"<<endl;
        fprintf(fp, "t # 0\n");
        fprintf(fp, "%d %d %d\n", vertex_num, edge_num, vertexLabelNum);
        for(int i = 0; i < vertices.size(); ++i)
            fprintf(fp, "v %d %d\n", i, vertices[i].label);
        for(int i = 0; i < vertices.size(); ++i) {
            //NOTICE: one undirected edge should be output once.
            for(int j = 0; j < vertices[i].neigh.size(); ++j) {
                int id = vertices[i].neigh[j].vid;
                if(i < id)
                    fprintf(fp, "e %d %d\n", i, id);
            }
        }
        fprintf(fp, "t # -1\n");
        fclose(fp);
    }
};

#endif

