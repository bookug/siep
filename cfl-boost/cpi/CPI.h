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

extern long nmatchings;
extern long bound;
extern double max_mem;
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
	int *levup, *pid;//, *ump;

	vector<vector<int> > nodeoflev;		// nodeoflev[i] - all nodes on level i
	vector<int> **pchilds;		// levup[i] - level of ui, pid[i] - ui's parent

	// Candidate set and Adjacency List, essense of CPI!
	vector<int> *C;
	
	// Revised CPI! Remember that this is a multi-mapping from E(Q) to E(G)
	// map<...> instance will be created(new) only when visited!!!
	unordered_map<int, vector<int> > ***pN;
	
	// Matching order selection
	vector<bool> selected;			// wheather a path has been put in *_seq
	vector<vector<int> > path;		// contain all pathes in qt in *_seq
	int selectednum = 0;			// #vertices in *_seq
	unordered_map<AdjHead, long long, Adjhash> CCnt;
	bool *inseq;
	vector<int> Core_seq, Forest_seq;
	bool Forest_seq_set = false;
	
	// Core and Forest Match!
	bool *matched;
    //map<int, bool> used;
    vector<bool> used;
    int real_vertex_num;

	unsigned *usedTimes;
	/*unordered_*/map<int, int> M;

	CPI(Graph *_q, Graph *_d, FILE *_ofp, int _real_vertex_num): query_graph(_q), data_graph(_d), ofp(_ofp) {
        this->real_vertex_num = _real_vertex_num;

		levup = new int[_q->n];
		pid = new int[_q->n];
		pchilds = new vector<int>*[_q->n];
		memset(levup, 0, _q->n*sizeof(int));
		memset(pid, 0, _q->n*sizeof(int));
		memset(pchilds, 0, _q->n*sizeof(int*));

		matched = new bool[_q->n];
		inseq = new bool[_q->n];
		memset(matched, 0, _q->n*sizeof(bool));
		memset(inseq, 0, _q->n*sizeof(bool));

		C = new vector<int>[_q->n];
		usedTimes = new unsigned[_d->n];
		memset(usedTimes, 0, _d->n*sizeof(unsigned));
		
		pN = new unordered_map<int, vector<int> >**[_q->n];
		for(unsigned i = 0; i < _q->n; i++){
			pN[i] = new unordered_map<int, vector<int> >*[_q->n];
			memset(pN[i], 0, _q->n*sizeof(int*));
		}
		//printf("CPI constructed\n");
        this->used = vector<bool>(_real_vertex_num, false);
	}
	void FindRoot();
	void BuildSpanningTree();
	bool CandVerify(int v, int u);
	bool CPI_Construct();

	void Set_path(int rt, bool *range, bool inForest);
	long long CC(int pid, int u, int v);
	long long CC(int pid);
	void AddVertex(int pid, vector<int> &seq);
	int Find_u(int pid);
	void Set_Core_seq_new();
	void Set_Core_seq();

	void ResetCL();
	
	bool ValidateNT(int v, int u);
	void Core_match();
	bool Core_match(int x);
	
	void Set_Forest_seq();
	bool Forest_match();
	bool Forest_match(int x);
	void Output();

	void combs(int u, vector<int> &res) {
		if (nmatchings > bound) return;
		if (u == query_graph->n) {
double vsize, rss;
Util::process_mem_usage(vsize, rss);
if(vsize > max_mem)
    max_mem = vsize;
#ifdef PRINT_RESULT
			for (int i = 0; i < u; i++)
				fprintf(ofp, "(%d, %d) ", i, res[i]);
			fprintf(ofp, "\n");
#endif
			nmatchings++;
			return;
		}
		int h = M[u];

		// Enumerate all data vertices contained in this hypernode
		for (int i = 0; i < data_graph->ctains[h].size() && nmatchings <= bound; i++) {
			int v = data_graph->ctains[h][i];
			if (used[v]) 			// h is remapped
				continue;
			used[v] = true;
			res.push_back(v);
			combs(u + 1, res);
			res.pop_back();
			used[v] = false;
		if (nmatchings > bound) return;
		}
	}


	void getCrossSet(vector<int> &v1, vector<int> &v2, vector<int> &res){
		for(int i=0; i<v1.size(); i++){
			int h = v1[i];
			if (find(v2.begin(), v2.end(), h) != v2.end())
				res.push_back(h);
		}
	}

	int Contains(set<int> &s1, set<int> &s2){
		auto it1 = s1.begin(), it2 = s2.begin();
		if (s1.size() == s2.size()){
			for(; it1!=s1.end(); ++it1, ++it2)
				if (*it1 != *it2) 
					return -1;
			return 0;
		} else if (s1.size() > s2.size()){
			for(; it2 != s2.end(); ++it1, ++it2){
				while(*it1 != *it2) {
					++it1;
					if (*it1 > *it2 || it1 == s1.end())
						return -1;
				}
			} return 1;
		} else{
			for(; it1 != s1.end(); ++it1, ++it2){
				while(*it1 != *it2) {
					++it2;
					if (*it2 > *it1 || it2 == s2.end())
						return -1;
				}
			} return 2;
		}
	}
	
	~CPI(){
		delete[] pid; delete[] levup; //delete[] used; 
		delete[] matched; delete[] inseq; delete[] C;
		for(unsigned i=0;i<query_graph->n; i++){
			if(pchilds[i] != NULL) delete pchilds[i];
		}
		delete[] pchilds; delete[] usedTimes;
		for(unsigned i=0;i<query_graph->n; i++){
			for(unsigned j=0;j<query_graph->n;j++){
				if(pN[i][j]!=NULL) delete pN[i][j];
			} delete[] pN[i];
		} delete[] pN;
		//delete data_graph; delete query_graph;
		//if(NECcontain) delete[] NECcontain;
		//if(NEClb) delete[] NEClb;
		//printf("CPI deleted\n");
	}
};

#endif
