/*=============================================================================
# Filename: Graph.h
# Author: Jiang Yan (Frozenlight)
# Mail: 527507046@qq.com 
=============================================================================*/

#ifndef _GRAPH_GRAPH_H
#define _GRAPH_GRAPH_H

#include "../util/Util.h"
using namespace std;

class Tree{
public:
	int root, n;
	//unordered_map<int, bool> inTree;
	bool *inTree;
	//vector<int> treenodes;
	Tree(int _rt, int query_size): root(_rt), n(query_size){	
		inTree = new bool[n];
		memset(inTree, 0, n*sizeof(bool));
	}
	~Tree(){	
		delete[] inTree;
		//printf("The fucking Tree destructor\n");
	}
};

class Graph {
public:
	//unordered_map<int, int> lb_cnt;
	int *lb_cnt;
	//vector<Vertex> vertices;
	int *vlb, *deg, n, m, lbn;
	vector<int> *nbs;
	
	// Will be init for ease of CPI construction
	int *mnd;
	vector<unordered_map<int, int> > nlbcnt;	// nlbcnt[i] is neighbor label set of vi
	
	// Exist only if *this is a query graph
	vector<VID> Core, Forest;
	bool *inCore;

	vector<Tree*> trees;
	
	Graph(int numVertex, int numEdge, int lbn_):n(numVertex), m(numEdge), lbn(lbn_) {
		vlb = new int[n];
		deg = new int[n];
		nbs = new vector<int>[n];
		mnd = new int[n];
		lb_cnt = new int[lbn+1];
		memset(lb_cnt, 0, (lbn+1)*sizeof(int));
		inCore = NULL; 
	}
	~Graph() {
		delete[] vlb; delete[] deg; 
		//delete[] nbs; 
		delete[] mnd; 
		delete[] lb_cnt;
		if(inCore) delete[] inCore; 
		for(int i=0; i<trees.size(); i++){
			delete trees[i];
		}
		//printf("The fucking Graph destructor\n");
	}
	void addVertex(int vid, int _vlb);
	void addEdge(VID _from, VID _to);
	void decompose();
	void Precond();
};

#endif

