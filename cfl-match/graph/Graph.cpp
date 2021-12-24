/*=============================================================================
# Filename: Graph.cpp
# Author: Jiang Yan (Frozenlight)
# Mail: 527507046@qq.com 
=============================================================================*/

#include "Graph.h"
using namespace std;

void Graph::addVertex(int vid, int _vlb){
	vlb[vid] = _vlb;
	//vertices.push_back(Vertex(_vlb));
	lb_cnt[_vlb]++;
	nlbcnt.push_back(unordered_map<int,int>());
}

void Graph::addEdge(VID _from, VID _to){
    //treated as undirected edge
	nbs[_from].push_back(_to);
	nbs[_to].push_back(_from);
	deg[_from]++;
	deg[_to]++;

	/*
	vertices[_from].nbs.push_back(_to);
	vertices[_to].nbs.push_back(_from);
	vertices[_from].deg++;
	vertices[_to].deg++;
	*/
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
		for(int j=0; j<nbs[i].size(); j++){
			G[i].push_back(nbs[i][j]);	
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
		tmp_max = 0;				// max neighbor degree
		for(unsigned i = 0; i < nbs[u].size(); i++){	// Process each nb of u
			int nbid = nbs[u][i];
			if (nbs[nbid].size() > tmp_max)
				tmp_max = nbs[nbid].size();
			//printf("u=%d, vlb[nbid]=%d, \n ", u, vlb[nbid]);
			//printf("nlbcnt[u][0]=%d\n", nlbcnt[u][0]);
			nlbcnt[u][0];
			fflush(stdout);
			nlbcnt[u][vlb[nbid]]++;					// update label cnt of u
		}
		// Since u in graph is processed in sequence, we do not need mnd[u]
		//mnd.push_back(tmp_max);
		mnd[u] = tmp_max;
	}
	
}
