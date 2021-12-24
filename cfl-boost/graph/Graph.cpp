/*=============================================================================
# Filename: Graph.cpp
# Author: Jiang Yan (Frozenlight)
# Mail: 527507046@qq.com 
=============================================================================*/

#include "Graph.h"
using namespace std;

void Graph::addNode(int nodeid, int _nodelb, int clique){
	vlb[nodeid] = _nodelb;
	lb_cnt[_nodelb]++;
	nlbcnt.push_back(unordered_map<int,int>());
	isClique[nodeid] = clique;
}

void Graph::addVertex(int nodeid, int vid){
	ctains[nodeid].push_back(vid);
}

void Graph::addEdge(unsigned _from, unsigned _to){
    //treated as undirected edge
	nbs[_from].insert(_to);
	nbs[_to].insert(_from);
	nblbs[_from].insert(vlb[_to]);
	nblbs[_to].insert(vlb[_from]);
}

void Graph::addSC(int from, int to){
	scout[from].push_back(to);
	scin[to].push_back(from);
	scindeg[to]++;
	scoutdeg[from]++;
}

/*
 * Decompose this query graph into core and forest
 * Note: this method is excuted only if *this is a query graph
 */
void Graph::decompose(){
	inCore = new bool[n];
	//memset(inCore, 0, n*sizeof(bool));
	vector<vector<int> > G;
	G.resize(n);
	for(int i=0; i<n; i++){
		for(auto j=nbs[i].begin(); j!=nbs[i].end(); ++j){
			G[i].push_back(*j);	
		}
	}
	queue<int> degone;
	bool *suspend = new bool[n];
	memset(suspend, 0, n*sizeof(bool));
	for(int i=0;i<n;i++)
		inCore[i] = true;
	
	for(int i=0; i<n;i++)
		if(G[i].size()==1)
			degone.push(i);
	
	while(!degone.empty()){
		int tid = degone.front();
		inCore[tid] = false;
		suspend[tid] = true;
		degone.pop();

		int nb = G[tid][0];
		G[nb].erase(find(G[nb].begin(), G[nb].end(), tid));
		//G[tid][0] = -1;		// Mark as removed
		if(G[nb].size() == 1) degone.push(nb);
	}

	for (int i = 0; i < n; i++)
		if (inCore[i]) 
			Core.push_back(i);
	
	/* At least one vertex shall in Core */
	if (Core.size() == 0) {
		Core.push_back(0);
		inCore[0] = true;
	}
	Tree *pt = NULL;
	/* Build trees */
	for (int i = 0; i < n; i++){
		if (!inCore[i]) {
			Forest.push_back(i);
			continue;
		}
		bool isrt = false;

		auto it = nbs[i].begin();			// go through all neighbors of ui
		for(; it != nbs[i].end(); ++it){	// If a notCore is around, new root!
			if(!inCore[*it]){
				isrt = true;
				break;
			}
		}

		if(isrt){						// Node i lead a new tree!
			Forest.push_back(i);
			//cout << "u"<<i << " is root."<<endl;
			//Tree t(i, n);
			pt = new Tree(i, n);
			queue<int> q; 
			q.push(i);	// q is vertices waiting to push into treenodes
			while(q.size() > 0){		// Find all nodes of this tree
				int id = q.front(); q.pop();
				//t.treenodes.push_back(id);
				pt->inTree[id] = true;
				//assert(id < n);
				suspend[id] = false;	// Not suspend == already assigned into treenodes
				auto it = nbs[id].begin();
				for(; it != nbs[id].end(); ++it){
					//assert(*it < n);
					if(suspend[*it])
						q.push(*it);
				}
			}
			trees.push_back(pt);
		}

	}
	delete[] suspend;
/*
	printf("Decompose finished, #tree=%ld, #Core=%ld, #Forest=%ld\n", trees.size(), Core.size(), Forest.size());
	printf("Core:");
	
	for(int i=0;i<Core.size();i++){
		printf("%d,", Core[i]);
	}printf("\n");*/
}

/*
 * Build mnd and nlbcnt for *this query graph
 * both data and query graph need this method
 */
void Graph::Precond(){
	int tmp_max = 0;

	// Process each u in graph iteratively
	for (int u = 0; u < n; u++){
		switch (isClique[u]) {
		case 0: deg[u] = 0; break;
		case 2: deg[u] = 0; break;
		case 1: deg[u] = ctains[u].size()-1;break;
		default:printf("ERROR `isClique` identifier:%d\n", isClique[u]);exit(1);
		}
		// Calculate real degree of each query vertex
		for(auto i = nbs[u].begin(); i != nbs[u].end(); ++i){
			int u_ = *i;
			if (isClique[u_] == 0)
				deg[u]++;
			else
				deg[u] += ctains[u_].size();
		}
	}

	// Calculate real max-neighbor-degree of each hypernode!
	for (int u = 0; u < n; u++) {
		nlbcnt[u][0];
		if (isClique[u] == 1)	// Add vertex u itself if it's a clique node
			nlbcnt[u][vlb[u]] += (ctains[u].size() - 1);
		tmp_max = 0;
		for(auto i = nbs[u].begin(); i != nbs[u].end(); ++i){	// Process each nb u_ of u
			int u_ = *i;
			tmp_max = deg[u_] > tmp_max ? deg[u_] : tmp_max;
			nlbcnt[u][vlb[u_]] += ctains[u_].size();			// update label cnt of u
		}
		mnd[u] = tmp_max;
	}
	
}
