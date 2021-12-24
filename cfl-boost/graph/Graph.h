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
	}
};


class Graph {
public:
	// Vertex (not node) nlbcnt and mnd
	int *mnd;
	vector<unordered_map<int, int> > nlbcnt;

	// Node attributes
	int *lb_cnt, *isClique;
	int *vlb, *deg, n, lbn;
	set<int> *nbs;
	vector<int> *ctains;
	set<int> *nblbs;
	
	// Syntactic Containment Graph only (Exist only if *this is a data graph)
	vector<int> *scout, *scin;
	int *scindeg, *scoutdeg;

	// Exist only if *this is a query graph
	vector<VID> Core, Forest;
	bool *inCore;
	vector<Tree*> trees;
	
	Graph(int numNode, int lbn_):n(numNode), lbn(lbn_) {
		vlb = new int[n];
		deg = new int[n];
		scindeg = new int[n];
		scoutdeg = new int[n];
		memset(scindeg, 0, n*sizeof(int));
		memset(scoutdeg, 0, n*sizeof(int));
		nbs = new set<int>[n];
		nblbs = new set<int>[n];
		mnd = new int[n];
		ctains = new vector<int>[n];
		lb_cnt = new int[lbn+1];
		scout = new vector<int>[n];
		scin = new vector<int>[n];
		isClique = new int[n];
		memset(lb_cnt, 0, (lbn+1)*sizeof(int));
		inCore = NULL; 
	}

	~Graph() {
		delete[] vlb; delete[] deg; 
		delete[] nbs; delete[] nblbs;
		delete[] scindeg; delete[] scoutdeg;
		delete[] scout; delete[] scin;
		delete[] mnd; delete[] isClique; 
		delete[] lb_cnt;
		if(inCore) delete[] inCore; 
		for(int i=0; i<trees.size(); i++){
			delete trees[i];
		}
		delete[] ctains;
		//printf("The fucking Graph destructor\n");
	}
	
	void addNode(int nodeid, int _nodelb, int clique);
	void addEdge(unsigned _from, unsigned _to);
	void addVertex(int nodeid, int vid);
	void addSC(int from, int to);
	void decompose();
	void Precond();
    void outputGraph(string tmpFile) {
        FILE* fp = fopen(tmpFile.c_str(), "w+");
        if(fp == NULL)
            cout<<"Error in outputGraph()"<<endl;
        fprintf(fp, "t # 0\n");
        fprintf(fp, "%d %d %d\n", n, 0, lbn);
        for(int i = 0; i < n; ++i)
            fprintf(fp, "v %d %d\n", i, vlb[i]);

        for(int i = 0; i < n; ++i) {
            //NOTICE: one undirected edge should be output once.
			for(auto j = nbs[i].begin(); j != nbs[i].end(); ++j){
                if(i < *j)
                    fprintf(fp, "e %d %d\n", i, *j);
            }
        }
        fprintf(fp, "t # -1\n");
        fclose(fp);
    }

};

#endif

