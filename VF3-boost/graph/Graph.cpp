/*=============================================================================
# Filename: Graph.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-10-24 23:01
# Description: 
=============================================================================*/

#include "Graph.h"

using namespace std;

void Graph::addVertex(LABEL _vlb) {
	vertices.push_back(Vertex(_vlb));
    //freq[_vlb]++;
}

void Graph::addEdge(VID _from, VID _to, LABEL _elb) {
	vertices[_from].neigh.push_back(Neighbor(_to, _elb));
	vertices[_to].neigh.push_back(Neighbor(_from, _elb));
    //treated as undirected edge
}

void Graph::transform() {	// 对各邻居依据节点 id 排序
	vertex_num = vertices.size();
	vLabel = new int[vertex_num];
	for(int i = 0; i < vertex_num; ++i) {
		//sort on label, when label is identical, sort on VID
		sort(vertices[i].neigh.begin(), vertices[i].neigh.end());
		vLabel[i] = vertices[i].label;
    }

}

// void Graph::transform(const char* _file) {
//     FILE* fp = fopen(_file, "w+");
//     if(fp == NULL)
//         cout<<"unable to open file "<<_file<<endl;

// 	int vnum = vertices.size();
//     fprintf(fp, "%d\n", vnum);
//     for(int i = 0; i < vnum; ++i)
//         fprintf(fp, "%d %d\n", i, vertices[i].label);

//     for(int i = 0; i < vnum; ++i) {
//         int eenum = vertices[i].in.size();
//         fprintf(fp, "%d\n", eenum);
//         for(int j = 0; j < eenum; ++j)
//             fprintf(fp, "%d %d\n", i, vertices[i].in[j].vid);
//     }

//     fclose(fp);
// }


