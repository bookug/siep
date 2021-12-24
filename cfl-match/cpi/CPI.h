/*=============================================================================
# Filename: CPI.h
# Author: Jiang Yan (Frozenlight)
# Mail: 527507046@qq.com 
=============================================================================*/

#ifndef _CPI_CPI_H
#define _CPI_CPI_H

#include "../util/Util.h"
#include "../graph/Graph.h"

#include "../../control.h"

using namespace std;

class AdjHead{	// Key of each Adjacency List
public:					// 3 paras needed to discern a unique adj-list
	int up, u, v;
	AdjHead(int _up, int _u, int _v): up(_up), u(_u), v(_v) {}
};

class Adjhash{
public:
	size_t operator()(const AdjHead &h)const {
		return h.up * h.u * h.v;	// Naive hash func
	}
};

bool operator==(AdjHead &ahead1,  AdjHead &ahead2);


class CPI{
public:
	Graph *query_graph, *data_graph;
	FILE *ofp;
	/* Spanning Tree's data */
	int rootid, ncalls = 0;
	unsigned maxlevel;				// root must in Core
	int *levup, *pid;

	vector<vector<int> > nodeoflev;		// nodeoflev[i] - all nodes on level i
	vector<int> **pchilds;		// levup[i] - level of ui, pid[i] - ui's parent

	// Candidate set and Adjacency List, essense of CPI!
	vector<int> *C;
	
	// Revised CPI! Remember that this is a multi-mapping from E(Q) to E(G)
	// map<...> instance will be created(new) only when visited!!!
	unordered_map<int, vector<int> > ***pN;
	
	// Matching order sele		ction
	vector<bool> selected;			// wheather a path has been put in *_seq
	vector<vector<int> > path;		// contain all pathes in qt in *_seq
	int selectednum = 0;			// #vertices in *_seq
	unordered_map<AdjHead, long long, Adjhash> CCnt;
	//unordered_map<int, int> ***pCCnt; 

	bool *inseq;

	vector<int> Core_seq, Forest_seq;
	bool Forest_seq_set = false;
	
	// Core and Forest Match!
	bool *visited, *isLeaf, *matched;
	/*unordered_*/map<int, int> M;
	unsigned numofleaves = 0;

	vector<int> Leaves, NECs, *NECcontain;
	int *NEClb;
	bool comparer(int l1, int l2);
	unordered_map<int, vector<int> > Leaf_M;

	CPI(Graph *_q, Graph *_d, FILE *ofp_): query_graph(_q), data_graph(_d), ofp(ofp_){
		double vsize, rss;
		Util::process_mem_usage(vsize, rss);
		printf("CPI::begin %8.0lf\n", vsize);
		
		levup = new int[_q->n];
		pid = new int[_q->n];
		pchilds = new vector<int>*[_q->n];
		memset(levup, 0, _q->n*sizeof(int));
		memset(pid, 0, _q->n*sizeof(int));
		memset(pchilds, 0, _q->n*sizeof(int*));
		
		Util::process_mem_usage(vsize, rss);
		printf("CPI::half %8.0lf\n", vsize);
		
		visited = new bool[_d->n];
		matched = new bool[_q->n];
		isLeaf = new bool[_q->n];
		inseq = new bool[_q->n];
		memset(visited, 0, _d->n*sizeof(bool));
		memset(matched, 0, _q->n*sizeof(bool));
		memset(isLeaf, 0, _q->n*sizeof(bool));
		memset(inseq, 0, _q->n*sizeof(bool));
		
		Util::process_mem_usage(vsize, rss);
		printf("CPI::final %8.0lf\n", vsize);
		
		C = new vector<int>[_q->n];
		
		pN = new unordered_map<int, vector<int> >**[_q->n];
		for(unsigned i = 0; i < _q->n; i++){
			pN[i] = new unordered_map<int, vector<int> >*[_q->n];
			memset(pN[i], 0, _q->n*sizeof(int*));
		}
		Util::process_mem_usage(vsize, rss);
		printf("CPI::end %8.0lf\n", vsize);
		
		NEClb = NULL; NECcontain = NULL;
	}
	void FindRoot();
	void BuildSpanningTree();
	bool CandVerify(int v, int u);
	bool CPI_Construct();

	// class Wrap{
	// public:
	// 	CPI *pc;
	// 	int u;
	// 	Wrap(int u_, CPI *pc_):pc(pc_), u(u_){};
	// 	bool operator<(const Wrap &w)const{
	// 		return pc->C[u].size() < pc->C[w.u].size();
	// 	}
	// };
	void Set_path(int rt, bool *range, bool inForest);
	long long CC(int pid, int u, int v);
	long long CC(int pid);
	void AddVertex(int pid, vector<int> &seq);
	int Find_u(int pid);
	int Find_next(bool *range);
	//void Set_Core_seq_new();
	void Set_Core_seq();
	
	bool ValidateNT(int v, int u);
	void Core_match();
	void Core_match(int x);
	
	void Set_Forest_seq();
	void Forest_match();
	void Forest_match(int x);

	void Leaf_match();
	void Leaf_match(int x);
	void Leaf_match_o(int x);

	void genComb(vector<int> &src, vector<vector<int> > &ret, vector<int> &cur, int x, int tot, int lastid);
	
	void Output();
	~CPI(){
		delete[] pid; delete[] levup; delete[] visited; delete[] matched;
		delete[] inseq; delete[] isLeaf; delete[] C;
		for(unsigned i=0;i<query_graph->n; i++){
			if(pchilds[i] != NULL) delete pchilds[i];
		}
		delete[] pchilds;
		for(unsigned i=0;i<query_graph->n; i++){
			for(unsigned j=0;j<query_graph->n;j++){
				if(pN[i][j]!=NULL) delete pN[i][j];
			} delete[] pN[i];
		} delete[] pN;
		//if(NECcontain) delete[] NECcontain;
		//if(NEClb) delete[] NEClb;
		//printf("CPI deleted\n");
	}
};

#endif
