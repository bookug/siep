/*=============================================================================
# Filename: CPI.cpp
# Author: Jiang Yan (Frozenlight)
# Mail: 527507046@qq.com 
=============================================================================*/
// Peak memory consumption
// Num of recursive calls
// Match time cost
// Timer thread (10 min)

#include "CPI.h"

using namespace std;

extern bool DBmode;

extern long bound, nmatchings;
extern double max_mem;

bool operator==(const AdjHead &ahead1, const AdjHead &ahead2){
	return (ahead1.up == ahead2.up) && (ahead1.u == ahead2.u) 
		&& (ahead1.v == ahead2.v);
}

/* Find the vertex from query_graph->Core with min index */
void CPI::FindRoot(){
	int tmpid = -1;
	double tmpmin = 1e100;
	int s0 = query_graph->Core.size();
	/* At least one v in Core guaranteed */
	if(DBmode) assert(s0 > 0);
	for(int i = 0; i < s0; i++) {
		int tid = query_graph->Core[i];	// index of this core vertex
		int label = query_graph->vlb[tid];
		if (data_graph->lb_cnt[label] == 0) 
			continue;
		double index = ((double)data_graph->lb_cnt[label]) / 
			query_graph->deg[tid]; //vertices[tid].deg;
		if (index < tmpmin){
			tmpmin = index;
			tmpid = tid;
		}
	}
	rootid = tmpid;
	/* It's possible that (rootid==-1) at this point, when the data graph
	 *  doesn't contain any lable of query's Core. We treat this case in 
	 *  upper-layer function
	 */
}


/* Find root of this spanning tree*/
void CPI::BuildSpanningTree(){
	queue<int> qw;
	bool *inqt = new bool[query_graph->n];
	memset(inqt, 0, query_graph->n * sizeof(bool));
	/* root of spanning tree */
	qw.push(rootid);
	inqt[rootid] = true;
	levup[rootid] = 0;
	maxlevel = 0;
	pid[rootid] = rootid;
	/* BFS to build spanning tree */
	while(!qw.empty()){
		int curid = qw.front(); qw.pop();
		/* curid shall have a levup entry */
		
		unsigned curlev = levup[curid];
		maxlevel = max(maxlevel, curlev);
		if(nodeoflev.size() <= curlev)
			nodeoflev.push_back(vector<int>());
		nodeoflev[curlev].push_back(curid);
		
		/* Iteratively look up all neighbors tryid of curid */
		for(auto i = query_graph->nbs[curid].begin(); i != query_graph->nbs[curid].end(); ++i){
			int tryid = *i;		//vertices[curid].nbs[i];
			/* already in qt (in upper or same level) ==> cont */
			if (inqt[tryid]) 
				continue;
			inqt[tryid] = true;
			pid[tryid] = curid;
			levup[tryid] = curlev+1;
			/* Add to pThisnode's childs */
			if(pchilds[curid]==NULL){
				pchilds[curid] = new vector<int>();
			}
			pchilds[curid]->push_back(tryid);
			qw.push(tryid);
		}
	}/*
	printf("-------------TREE-------------\n");
	for(int i=0; i<nodeoflev.size(); i++){
		printf("Layer %d: ", i);
		for(int j=0; j<nodeoflev[i].size(); j++){
			printf("%d,", nodeoflev[i][j]);
		}printf("\n");
	}*/
	delete []inqt;
}


/* Inspect max_neighbor_deg[u] and nblcnt[u][i] */
bool CPI::CandVerify(int v, int u){
	if (data_graph->mnd[v] < query_graph->mnd[u]) 
		return false;
	/* Iterate on neighbors' label set (map: lb->cnt) of u */
	auto itq = query_graph->nlbcnt[u].begin();
	for(; itq != query_graph->nlbcnt[u].end(); ++itq){
		int usnlb = itq->first;

		auto itd = data_graph->nlbcnt[v].find(usnlb);
		if (itd == data_graph->nlbcnt[v].end())
			return false;	// label of v's nb do not contain usnlb
		if (itd->second < itq->second)
			return false;	// d(v,l) < d(u,l)
	}
	return true;
}


bool CPI::CPI_Construct(){
	FindRoot();
	
	if(rootid < 0) return false;
	//rootid = 0;	// WARNING!!! Remove this line if not using Figure6 as test graph 
	BuildSpanningTree();

	bool *vis = new bool[query_graph->n];
	memset(vis, 0, query_graph->n*sizeof(bool));
	vector<int> *UN = new vector<int>[query_graph->n];

	/* Find root match */
	for (int i = 0; i < data_graph->n; i++){
		if (data_graph->vlb[i] != query_graph->vlb[rootid] || data_graph->deg[i] < query_graph->deg[rootid])	// label and deg check!
			continue;
		if (CandVerify(i, rootid)) 
			C[rootid].push_back(i);
	}vis[rootid] = true;
	int *cnt = new int[data_graph->n];
	memset(cnt, 0, data_graph->n*sizeof(int));


	/* Top-Down Construction */
	for(unsigned lev = 1; lev <= maxlevel; lev++){
		int Cnt = 0;

		/* Each vertex u of this level at inc order*/
		for(unsigned k = 0; k < nodeoflev[lev].size(); k++){
			int u = nodeoflev[lev][k];
			Cnt = 0;
			set<int> touched;
			/* Each vised neighbor u_ of u */
			for (auto i=query_graph->nbs[u].begin(); i!=query_graph->nbs[u].end(); ++i) {
				int u_ = *i;
				//cout<<"u"<<u<<"'s neighbor u"<<u_<<endl;
				if (vis[u_] == false && levup[u_] == lev)
					UN[u].push_back(u_);
				else if (vis[u_]){
					/* Each v_ in u_.C */
					//cout<<"u"<<u<<"'s vised neighbor u_"<<u_<<endl;
					for(int j=0; j < C[u_].size(); j++){
						int v_ = C[u_][j];
						/* Each v ajd to v_ */
						for(auto h=data_graph->nbs[v_].begin(); h != data_graph->nbs[v_].end(); ++h){
							int v = *h;
							if (data_graph->vlb[v] != query_graph->vlb[u])
								continue;
							if (data_graph->deg[v] < query_graph->deg[u])
								continue;
							// if (data_graph->isClique[v] != 1 && data_graph->nbs[v].size() < query_graph->nbs[u].size() || 
							// 	data_graph->isClique[v] == 1 && data_graph->nbs[v].size() + data_graph->ctains[v].size()-1 < query_graph->nbs[u].size())
							// 	continue;
							touched.insert(v);
							if (cnt[v] == Cnt) 
								cnt[v]++;
						}

						// v_ may exists in C[u]
						if (data_graph->isClique[v_] == 1 && data_graph->vlb[v_] == query_graph->vlb[u] && 
							data_graph->deg[v_] >= query_graph->nbs[u].size()){
							touched.insert(v_);
							if (cnt[v_] == Cnt) 
								cnt[v_]++;	
						}
					}
					Cnt++;
				}
			}
			/* each v in touched set */			
			auto it = touched.begin();
			for(; it != touched.end(); ++it){
				int v = *it;
				if (cnt[v] == Cnt && CandVerify(v,u))
					C[u].push_back(v);
				cnt[v] = 0;
			}
			vis[u] = true;	
		}

		/* Each vertex u of this level in reverse order*/
		for(int k = nodeoflev[lev].size() - 1; k>=0; k--){
			int u = nodeoflev[lev][k];
			Cnt = 0;
			vector<int> touched;
			/* For each u_ in u.UN */
			for (unsigned i=0; i<UN[u].size(); i++){
				int u_ = UN[u][i];
				/* Each v_ in u_.C */
				for(unsigned j=0; j<C[u_].size(); j++){
					int v_ = C[u_][j];
					/* Each v ajd to v_ */
					for(auto h = data_graph->nbs[v_].begin(); h != data_graph->nbs[v_].end(); ++h){
						int v = *h;
						if (data_graph->vlb[v] != query_graph->vlb[u])
							continue;
						if (data_graph->deg[v] < query_graph->deg[u])
							continue;
						// if (data_graph->isClique[v] != 1 && data_graph->nbs[v].size() < query_graph->nbs[u].size() || 
						// 	data_graph->isClique[v] == 1 && data_graph->nbs[v].size() + data_graph->ctains[v].size()-1 < query_graph->nbs[u].size())
						// 	continue;
						touched.push_back(v);
						if (cnt[v] == Cnt) 
							cnt[v]++;
					}

					// In case that v_ is a clique node and v is in it
					if (data_graph->isClique[v_] == 1 && data_graph->vlb[v_] == query_graph->vlb[u] && 
						data_graph->deg[v_] >= query_graph->nbs[u].size()) {
						touched.push_back(v_);
						if (cnt[v_] == Cnt) 
							cnt[v_]++;
					}
				}
				Cnt++;
			}
			
			/* each v in u.C */
			for(unsigned i=0; i < C[u].size(); i++){
				int v = C[u][i];
				if (cnt[v] != Cnt){
					C[u].erase(C[u].begin()+i);
					i--;
				}
			}
			if(C[u].size() == 0) {
                printf("Fail in CPI construction top-down phase\n");
				return false;
			}
			
			/* Reset v.cnt=0 */
			vector<int>::iterator it = touched.begin();
			for(; it != touched.end(); ++it)
				cnt[*it] = 0;
		}
		
		/* Build Adjacency List: Each vertex u of this level at inc order*/
		for(unsigned k=0; k < nodeoflev[lev].size(); k++){
			int u = nodeoflev[lev][k];
			int up = pid[u];
			/* Each vp in up.C */
			for (unsigned i=0; i < C[up].size(); i++){
				int vp = C[up][i];
				/* Each v adj to vp in data graph */
				int prevv=-1;
				for(auto j=data_graph->nbs[vp].begin(); j != data_graph->nbs[vp].end(); ++j){
					int v = *j;	//vertices[vp].nbs[j];
					if (data_graph->vlb[v] != query_graph->vlb[u]) 
						continue;
					if (find(C[u].begin(), C[u].end(), v) == C[u].end()) 
						continue;	// Not found v in u.C
					if(pN[up][u]==NULL)
						pN[up][u] = new unordered_map<int, vector<int> >();
					prevv=v;
					(*pN[up][u])[vp].push_back(v);
				}
			}
		}
	}

	/* Bottom-Up Refinement */
	for(int lev = maxlevel; lev >= 0; lev--){
		/* Each u at this lev */
		for(unsigned k=0; k < nodeoflev[lev].size(); k++){
			int u = nodeoflev[lev][k];
			vector<int> touched;
			int Cnt = 0;
			/* u's each (lower-lev) neighbor u_ */
			for (auto i=query_graph->nbs[u].begin(); i != query_graph->nbs[u].end(); ++i){
				int u_ = *i;
				if (levup[u_] < levup[u])	// Only look for lower-lev nb of u
					continue;
				
				/* Each v_ in u_.C */
				for(unsigned j=0; j < C[u_].size(); j++){
					int v_ = C[u_][j];
					/* Each v ajd to v_ */
					for(auto h=data_graph->nbs[v_].begin(); h != data_graph->nbs[v_].end(); ++h){
						int v = *h;
						if (data_graph->vlb[v] != query_graph->vlb[u])
							continue;
						if (data_graph->deg[v] < query_graph->deg[u])
							continue;
						// if (data_graph->isClique[v] != 1 && data_graph->nbs[v].size() < query_graph->nbs[u].size() || 
						// 	data_graph->isClique[v] == 1 && data_graph->nbs[v].size() + data_graph->ctains[v].size()-1 < query_graph->nbs[u].size())
						// 	continue;
						touched.push_back(v);
						if (cnt[v] == Cnt) 
							cnt[v]++;
					}
					// In case that v_ is a clique node and v is in it
					if (data_graph->isClique[v_] == 1 && data_graph->vlb[v_] == query_graph->vlb[u] && 
						data_graph->deg[v_] >= query_graph->nbs[u].size()) {
						touched.push_back(v_);
						if (cnt[v_] == Cnt)
							cnt[v_]++;
					}
				}
				Cnt++;
			}
			
			/* Remove according to v.cnt != Cnt */
			/* each v in u.C */
			for(unsigned i=0; i<C[u].size(); i++){
				int v = C[u][i];
				if (cnt[v] != Cnt){
					/* Remove v from u.C */
					C[u].erase(C[u].begin()+i);
					i--;
					/* All entries h(u,*,v) should be removed from pN
					 * But since unused entry in N do no harm, we skip this step
					 */
				}
			}
			if(C[u].size() == 0) {
				//printf("Fail in CPI-construct bottom-up phase\n");
				return false;
			}
			
			/* Reset v.cnt=0 */
			vector<int>::iterator it = touched.begin();
			for(; it != touched.end(); ++it)
				cnt[*it] = 0;
			
			/* Remove from adj-list */
			/* Each v in u.C */
			for(unsigned i=0; i<C[u].size(); i++){
				int v = C[u][i];
				/* Each u's child u_ */
				if(!pchilds[u]) continue;	// u has no childs!

				for(unsigned j=0; j<pchilds[u]->size(); j++){
					int u_ = (*pchilds[u])[j];
					if(DBmode)
						assert(pN[u][u_] != NULL);	// Since edge(u, u_) exist
					if(pN[u][u_]->find(v) == pN[u][u_]->end()) continue;
					vector<int> &proj = (*pN[u][u_])[v];
					for(unsigned l=0; l<proj.size(); l++){
						int v_ = proj[l];
						if(find(C[u_].begin(), C[u_].end(), v_) != C[u_].end())	// (u,u_)->(v,v_) OK! 
							continue;
						proj.erase(proj.begin()+l);		// Need to remove v_ from vector
						l--;
					}
				}
			}
		}
	}
	if(pN[rootid][rootid]==NULL)
		pN[rootid][rootid] = new unordered_map<int, vector<int> >();
	(*pN[rootid][rootid])[0] = C[rootid];

	delete []vis; delete []UN; delete []cnt;
	return true;
}


void CPI::Set_path(int rt, bool *range, bool inForest = false){
	
	vector<int> p;
	stack<int> s;
	s.push(rt);
	while(!s.empty()){
		int curv = s.top(); s.pop();
		int childinrange = 0;
		/* meet BEG in stack */
		if(curv == -1) {
			p.pop_back(); 
			continue;
		}
		p.push_back(curv);
		s.push(-1);
		/* No child ==> end of path */
		if(pchilds[curv]) {
			for(unsigned i=0; i<pchilds[curv]->size(); i++) {
				if (!range[(*pchilds[curv])[i]]) 
					continue;
				s.push((*pchilds[curv])[i]);
				childinrange++;
			} 
		}
		/* End of another path */
		
		/* End of this path */
		if(childinrange == 0) {
			path.push_back(p);
			selected.push_back(false);
			continue;
		}
	}
}


/* Calc #embeddings of u->v in dataGraph on path[pid] */
long long CPI::CC(int pid, int u, int v){
	vector<int> &p = path[pid];
	if (p.back() == u) 
		return 1;	// Leaf vertex
	long long ans = 0;
	int u_ = *(find(p.begin(), p.end(), u) + 1);	// (u)->(u_)
	AdjHead ch(pid, u, v); 
	if (CCnt[ch] != 0) {
		return CCnt[ch];
	}
	/* Each v_ in N[u, u_, v] */
	if(DBmode) assert(pN[u][u_] != NULL);
	vector<int> &proj = (*pN[u][u_])[v];
	for(unsigned k=0; k<proj.size(); k++){
		int v_ = proj[k];
		ans += CC(pid, u_, v_);
	}
	CCnt[ch] = ans;
	return ans;
}


/* Calc #embeddings of p(seq exclueded) in data graph */
long long CPI::CC(int pid){
	long long ans = 0;
	int j = 0;
	if(DBmode) assert(pid < path.size());
	vector<int> &p = path[pid];
	if(p.size() == 1){	// Only one vertex in Core...
		return 1;	// you don't the second choice
	}
	/* j is set to connect vertex of p and seq */
	do{ 
		j++;
		if(DBmode) assert(j < p.size());
		if(DBmode) assert(p[j] < query_graph->n);
	} while(inseq[p[j]]); j--;
	int u = p[j];
	
	/* for each v in u.C */
	for (unsigned i=0; i<C[u].size(); i++) {
		int v = C[u][i];
		long long inc = CC(pid, u, v);
		ans += inc;
	}
	return ans;
}


/* Add all vertices of path[pid](seq excluded) into seq */
void CPI::AddVertex(int pid, vector<int> &seq) {
	int i = 0;
	vector<int> &p = path[pid];
	/* i is set to first non-seq vertex on p */
	while (inseq[p[i]]) i++;
	/*printf("root=%d, path[%d]:", rootid, pid);
	for(int i=0; i<p.size(); i++)
		printf("%d ", p[i]);
	printf("\n");
	printf("i=%d, p.size()=%d\n", i, p.size());*/
	if(DBmode) assert(i <= p.size());
	for (; i<p.size(); i++) {
		//if(isLeaf[p[i]]) continue;
		seq.push_back(p[i]);
		inseq[p[i]] = true;
	}
	selected[pid] = true;	// path[pid] has been selected
	selectednum++;
}

/* Find vid of first non-seq vertex on path[pid] */
int CPI::Find_u(int pid) {
	int i = 0;
	vector<int> &p = path[pid];
	while (inseq[p[i]]) i++; 
	i--;
	if (i < 0) i=0;
	return p[i];
}

void CPI::Set_Core_seq(){
	Set_path(rootid, query_graph->inCore);
	int minindex = 1<<30, minid = -1;
	for(unsigned i = 0; i < path.size(); i++){
		int index = CC(i);
		if(index < minindex){
			minindex = index;
			minid = i;
		}
	}
	if(DBmode) assert(minid!=-1);
	AddVertex(minid, Core_seq);
	/* Greedy to find next path to match */
	while (selectednum != path.size()){
		double minindex = 1e100;
		int minid = -1;
		/* Find path with mininum index --> minid */
		int s1 = path.size();
		for (int i=0; i<s1; i++){
			if (selected[i]) 
				continue;
			int u = Find_u(i);
			double index = ((double)CC(i)) / C[u].size();
			// Check dp correctness
			if (index < minindex) {
				index = minindex;
				minid = i;
			}
		}
		AddVertex(minid, Core_seq);
	}
}

// Potential Optimization
bool CPI::ValidateNT(int h, int u) {
	set<int> &unbs = query_graph->nbs[u];
	set<int> &hnbs = data_graph->nbs[h];
	
	if (data_graph->vlb[h] != query_graph->vlb[u]) return false;
	for (int ui = 0; ui < query_graph->n; ui++){
		if (!matched[ui]) 
			continue;
		if (M[ui] != h) {		// Normal Validate
			auto it = unbs.find(ui);
			auto it2 = hnbs.find(M[ui]);
			if (it != unbs.end() && it2 == hnbs.end())
				return false;
		}else{					// u is match to the same node as ui
			auto it = unbs.find(ui);
			if (it != unbs.end() && data_graph->isClique[h] != 1)	// This is not a clique!
				return false;

			if (usedTimes[h] >= data_graph->ctains[h].size())
				return false;
		}
	}
	return true;
}


void CPI::Core_match() {
	Set_Core_seq();
	Core_match(0);
}


void CPI::Output(){
	vector<int> res;
	combs(0, res);
	//double vsize, rss;
	//Util::process_mem_usage(vsize, rss);
	//if (vsize > max_mem) max_mem = vsize;
}


void CPI::ResetCL() {
	// Do nothing...
}

bool CPI::Core_match (int x) {	// x vertices has been matched!
	if(nmatchings > bound)  return false;
	ncalls++;
	bool flag = false;
	if (x == query_graph->Core.size()) {	// Find another core match!
		if(query_graph->Forest.size() > 0) {
			flag = Forest_match();
		}
		else {
			Output();
			flag = true;
		}
		return flag;
	}
	int u = Core_seq[x];
	int up = pid[u];
	vector<int> &proj = (*pN[up][u])[M[up]];
	
	int v = M[up];
	if (x>0 && data_graph->isClique[v] && ValidateNT(v, u)) {
		M[u] = v;
		matched[u] = true;
		usedTimes[v]++;
		if (Core_match(x+1))
			flag = true;
		usedTimes[v]--;
		matched[u] = false;
		auto res = M.find(u);
		if (res == M.end()) printf("Core_match: remove a nonexistant pair\n");
		M.erase(res);
	}


	// Each v in N[up, u, M(up)]
	for(unsigned i=0; i<proj.size(); i++){
		if (nmatchings > bound) return flag;
		int v = proj[i];
		if (ValidateNT(v, u)) {
			M[u] = v;
			matched[u] = true;
			usedTimes[v]++;
			if (Core_match(x+1))
				flag = true;
			usedTimes[v]--;
			matched[u] = false;
			auto res = M.find(u);
			if (res == M.end()) printf("Core_match: remove a nonexistant pair\n");
			M.erase(res);
		}
	}
	return flag;
}


void CPI::Set_Forest_seq() {
	if(Forest_seq_set) return;
	/* Each tree t in forest */
	/* Build a map: int -> pathes of Forest */
	for(int i=0; i<query_graph->trees.size(); i++){
		Tree *pt = query_graph->trees[i];
		Set_path(pt->root, pt->inTree, true);
	}
	
	long long minindex = 1<<30;
	int minid = -1;
	for(int i=0; i<path.size(); i++){
		long long index = CC(i);
		if(index < minindex){
			minindex = index;
			minid = i;
		}
	}
	if (path.size()==1) minid=0;
	if(DBmode) assert(minid!=-1);
	AddVertex(minid, Forest_seq);
	
	/* Greedy to find next path to match in the whole forest */
	while (selectednum != path.size()){
		double minindex = 1e100;
		int minid = -1;
		/* Find path with min index --> minid */
		for (int i=0; i<path.size(); i++){
			if(DBmode) assert((unsigned)i < selected.size());
			if (selected[i]) 
				continue;
			int u = Find_u(i);
			double index = ((double)CC(i)) / C[u].size();
			if (index < minindex) {
				index = minindex;
				minid = i;
			}
		}
		AddVertex(minid, Forest_seq);
	}
	Forest_seq_set = true;
	
	// Remove Core nodes from Forest_seq
	for(int i = 0; i < Forest_seq.size(); i++) {
		int u = Forest_seq[i];
		if (query_graph->inCore[u]) {
			Forest_seq.erase(Forest_seq.begin() + i);
			i--;
		}
	}
}


bool CPI::Forest_match() {
	selected.clear();	
	path.clear();
	selectednum = 0;
	CCnt.clear();
	memset(inseq, 0, query_graph->n*sizeof(bool));
	
	/* Each path i */
	Set_Forest_seq();
	return Forest_match(0);
}


// Omit Leaf_match stage here!
bool CPI::Forest_match (int x) {	// x vertices has been matched!
	if (nmatchings > bound) return false;
	ncalls++;
	if (x == Forest_seq.size()) {	// Find another forest match!
		Output();
		return true;
	}
	bool flag = false;
	int u = Forest_seq[x];
	
	int up = pid[u];
	if(DBmode) assert(pN[up][u] != NULL);
	if(DBmode) assert(M.find(up) != M.end());	// up must be a matched one!

	vector<int> &proj = (*pN[up][u])[M[up]];
	int v = M[up];
	if (data_graph->isClique[v] && ValidateNT(v, u)) {
		M[u] = v;
		matched[u] = true;
		usedTimes[v]++;
		if (Forest_match(x+1))
			flag = true;
		usedTimes[v]--;
		matched[u] = false;
		auto res = M.find(u);
		if (res == M.end()) printf("Core_match: remove a nonexistant pair\n");
		M.erase(res);
	}

	// Each v in N(<up,u>, <M[up],?>)
	for(unsigned i = 0; i < proj.size(); i++){
		if (nmatchings > bound) return flag;
		int v = proj[i];
		if (ValidateNT(v, u)){
			M[u]=v;
			matched[u] = true;
			usedTimes[v]++;
			if (Forest_match(x+1))
				flag = true;
			usedTimes[v]--;
			matched[u] = false;
			M.erase(M.find(u));
		}
	}
	return flag;
}
