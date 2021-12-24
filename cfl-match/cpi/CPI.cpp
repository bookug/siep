/*=============================================================================
# Filename: CPI.cpp
# Author: Jiang Yan (Frozenlight)
# Mail: 527507046@qq.com 
=============================================================================*/

#include "CPI.h"

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
extern bool DBmode;
extern double max_mem;
extern long numembeddings, bound;
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
			query_graph->nbs[tid].size(); //vertices[tid].deg;
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
		//int s0 = query_graph->nbs[curid].size();	 //vertices[curid].nbs.size();
		for(unsigned i = 0; i < query_graph->nbs[curid].size(); i++){
			int tryid = query_graph->nbs[curid][i];		//vertices[curid].nbs[i];
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
	}
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
		//printf("Check lb%d\n", usnlb);

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
	for (int i=0; i < data_graph->n; i++){
		if (data_graph->vlb[i]  != query_graph->vlb[rootid] || data_graph->nbs[i].size() < query_graph->nbs[rootid].size())	// label and deg check!
			continue;
		if (CandVerify(i, rootid)) C[rootid].push_back(i);
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
			for (int i=0; i<query_graph->nbs[u].size(); i++) {
				int u_ = query_graph->nbs[u][i];
				if (vis[u_] == false && levup[u_] == lev)
					UN[u].push_back(u_);
				else if (vis[u_]){
					/* Each v_ in u_.C */
					for(int j=0; j < C[u_].size(); j++){
						int v_ = C[u_][j];
						/* Each v ajd to v_ */
						for(int h=0; h < data_graph->nbs[v_].size(); h++){
							int v = data_graph->nbs[v_][h];
							if (data_graph->vlb[v] != query_graph->vlb[u])
								continue;
							if (data_graph->nbs[v].size() < query_graph->nbs[u].size())
								continue;
							touched.insert(v);
							if (cnt[v] == Cnt) 
								cnt[v]++;
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
		int s4 = nodeoflev[lev].size();
		for(int k = s4 - 1; k>=0; k--){
			int u = nodeoflev[lev][k];
			Cnt=0;
			vector<int> touched;
			/* For each u_ in u.UN */
			int s5 = UN[u].size();
			for (int i=0; i<s5; i++){
				int u_ = UN[u][i];
				/* Each v_ in u_.C */
				int s6 = C[u_].size();
				for(int j=0; j<s6; j++){
					int v_ = C[u_][j];
					/* Each v ajd to v_ */
					for(unsigned h=0; h<data_graph->nbs[v_].size(); h++){
						int v = data_graph->nbs[v_][h];
						if (data_graph->vlb[v] != query_graph->vlb[u])
							continue;
						if (data_graph->nbs[v].size() < query_graph->nbs[u].size())
							continue;
						touched.push_back(v);
						if (cnt[v] == Cnt) 
							cnt[v]++;
					}
				}
				Cnt++;
			}
			
			/* each v in u.C */
			s5 = C[u].size();
			for(unsigned i=0; i < C[u].size(); i++){
				int v = C[u][i];
				if (cnt[v] != Cnt){
					C[u].erase(C[u].begin()+i);
					i--;
				}
			}
			if(C[u].size() == 0) 
				return false;
			
			/* Reset v.cnt=0 */
			vector<int>::iterator it = touched.begin();
			for(; it != touched.end(); ++it)
				cnt[*it] = 0;
		}
		
		/* Build Adjacency List: Each vertex u of this level at inc order*/
		s4 = nodeoflev[lev].size();
		for(int k=0; k < s4; k++){
			cout.flush();
			int u = nodeoflev[lev][k];
			int up = pid[u];
			/* Each vp in up.C */
			int s5 = C[up].size();
			for (int i=0; i<s5; i++){
				cout.flush();
				int vp = C[up][i];
				/* Each v adj to vp in data graph */
				int s6 = data_graph->nbs[vp].size();
				int prevv=-1;
				for(int j=0; j<s6; j++){
					int v = data_graph->nbs[vp][j];	//vertices[vp].nbs[j];
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
	for(int lev=maxlevel; lev>=0; lev--){
		/* Each u at this lev */
		int s1 = nodeoflev[lev].size();
		for(int k=0; k < s1; k++){
			int u = nodeoflev[lev][k];
			vector<int> touched;
			int Cnt = 0;
			/* u's each (lower-lev) neighbor u_ */
			for (int i=0; i<query_graph->nbs[u].size(); i++){
				int u_ = query_graph->nbs[u][i];
				if (levup[u_] < levup[u])	// Only look for lower-lev nb of u
					continue;
				
				/* Each v_ in u_.C */
				int s3 = C[u_].size();
				for(int j=0; j<s3; j++){
					int v_ = C[u_][j];
					/* Each v ajd to v_ */
					for(int h=0; h<data_graph->nbs[v_].size(); h++){
						int v = data_graph->nbs[v_][h];
						if (data_graph->vlb[v] != query_graph->vlb[u])
							continue;
						if (data_graph->nbs[v].size() < query_graph->nbs[u].size())
							continue;
						touched.push_back(v);
						if (cnt[v] == Cnt) 
							cnt[v]++;
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
					/* All entries h(u,*,v) should be removed from N 
					 * But since unused entry in N do no harm, we skip this step
					 */
				}
			}
			if(C[u].size() == 0) 
				return false;
			
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
				int s3 = pchilds[u]->size();
				for(int j=0; j<s3; j++){
					int u_ = (*pchilds[u])[j];
					if(DBmode) assert(pN[u][u_] != NULL);	// Since edge(u, u_) exist
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
			if(inForest){	// This is a leaf
				isLeaf[curv] = true;
				numofleaves++;
				Leaves.push_back(curv);
			}
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
		//printf("Cache hit: CCnt(%d,%d,%d)=%d\n", pid, u, v, CCnt[ch]);
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
		//printf("Mark\n");
		return 1;	// you don't the second choice
	}
	/* j is set to connect vertex of p and seq */
	do{ 
		j++;
		if(DBmode) assert(j < p.size());
		if(DBmode) assert(p[j] < query_graph->n);
	} while(inseq[p[j]]); j--;
	int u = p[j];
	
	//printf("\nC(%d), sizeof(%d.C):%d", pid, u, C[u].size());
	/* for each v in u.C */
	for (unsigned i=0; i<C[u].size(); i++) {
		int v = C[u][i];
		long long inc = CC(pid, u, v);
		ans += inc;
		//printf("inc(%d,%d):%d,", u, v, inc);
	}
	//printf("\n");
	return ans;
}


/* Add all vertices of path[pid](seq excluded) into seq */
void CPI::AddVertex(int pid, vector<int> &seq) {
	int i = 0;
	vector<int> &p = path[pid];
	/* i is set to first non-seq vertex on p */
	while (inseq[p[i]]) i++;
	if(DBmode) assert(i <= p.size());
	for (; i<p.size(); i++) {
		if(isLeaf[p[i]]) continue;
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


// void CPI::Set_Core_seq_new(){
// 	//Core_seq[0];
// 	query_graph->Core;
// 	bool *setOK = new bool[query_graph->Core.size()];
// 	memset(setOK, 0, sizeof(bool)*query_graph->Core.size());
// 	int setNum = 0;
// 	vector<Wrap> v;

// 	for(int i=0;i<query_graph->Core.size(); i++)
// 		v.push_back(Wrap(query_graph->Core[i], this));
// 	sort(v.begin(), v.end());
// 	for(int i=0; i<query_graph->Core.size(); i++){
// 		Core_seq.push_back(v[i].u);
// 	}
// }

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
bool CPI::ValidateNT(int v, int u) {
	// Each neighbor u_ of u in query_graph
	for(unsigned i=0; i<query_graph->nbs[u].size(); i++) {
		int u_ = query_graph->nbs[u][i];	//vertices[u].nbs[i];
		if (pid[u] == u_) 	// No need to check parent
			continue;
		if (M.find(u_) == M.end()) 	// skip the unmatched neighbors of u
			continue;
		vector<int> &Mu_nbs = data_graph->nbs[M[u_]];	//vertices[M[u_]].nbs;
		if (find(Mu_nbs.begin(), Mu_nbs.end(), v) == Mu_nbs.end())
			return false;	// v is not a neighbor of M[nb(u)]
	}
	return true;
}


void CPI::Core_match() {
	Set_Core_seq();
	Core_match(0);
}

void CPI::Output(){
	auto it = M.begin();
#ifdef PRINT_RESULT
	for(; it != M.end(); ++it)
		fprintf(ofp, "(%d, %d) ", it->first, it->second);
	fprintf(ofp, "\n");
#endif
	numembeddings++;
	double vsize, rss;
	Util::process_mem_usage(vsize, rss);
	if (vsize > max_mem) max_mem = vsize;
}


// Find next query vertex to match
int CPI::Find_next(bool *range){
	vector<int> &r = query_graph->Core;	
	int minid = -1, tmpmin = 1<<29;

	// Find the vertex with mininum potential ebds (u_, u)
	for(int i=0; i<query_graph->Core.size(); i++){
		int u_ = r[i];
		if(!matched[u_]) continue;
		if(pchilds[u_]==NULL) continue;	// This core vertex has no childs
		for(int j=0; j<pchilds[u_]->size(); j++){
			int u = (*pchilds[u_])[j];
			if(matched[u] || !range[u]) continue;
			int nebds = (*pN[u_][u])[M[u_]].size();	// u_ is a matched one
			if(nebds < tmpmin){
				nebds = tmpmin;
				minid = u;
			}
		}
	}
	assert(minid != -1);
	return minid;
}

void CPI::Core_match (int x) {	// x vertices has been matched!
	if(numembeddings > bound) return;
	ncalls++;
	int s0 = query_graph->Core.size();
	if (x == s0) {		// Find another core match!
		if(query_graph->Forest.size() > 0)
			Forest_match();
		else 
			Output();
		return;
	}
	if (x == 0){	// First vertex to match
		// Each v in u.C
		int u = rootid;
		int s1 = C[u].size();
		for (int k=0; k<s1; k++){
			int v = C[u][k];

			auto res = M.insert(pair<int, int>(u, v));
			//M[u] = v;
			visited[v] = true;
			matched[u] = true;
			Core_match(x+1);
			matched[u] = false;
			visited[v] = false;
			M.erase(res.first);
		} return;
	}
	
	//int u = Find_next(query_graph->inCore);
	int u = Core_seq[x];
	int up = pid[u];
	if(DBmode) assert(pN[up][u] != NULL);
	if(DBmode) assert(M.find(up) != M.end());	// up must be a matched one!
	if(DBmode) assert(pN[up][u]->find(M[up]) != pN[up][u]->end());
	vector<int> &proj = (*pN[up][u])[M[up]];
	
	// Each v in N[up, u, M(up)]
	for(unsigned i=0; i<proj.size(); i++){
		int v = proj[i];
		//printf("(u=%d,i=%d)", u, i);
		if (!visited[v] && ValidateNT(v, u)){

			M[u]=v;
			visited[v] = true;
			matched[u] = true;
			Core_match(x+1);	// Find match recursively
			matched[u] = false;
			visited[v] = false;
			M.erase(M.find(u));
		}
	}
}


void CPI::Set_Forest_seq(){
	if(Forest_seq_set) return;
	/* Each tree t in forest */
	/* Build a map: int -> pathes of Forest */
	for(int i=0; i<query_graph->trees.size(); i++){
		//printf("i=%d, %d trees in total, inTree:", i, query_graph->trees.size());
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
			// Check dp correctness
			/*PrintVector(path[i]); cout << ":";
			printf("u=%d, C(p)=%d, |u.C|=%d\n", u, CC(i), C[u].size()); */
			if (index < minindex) {
				index = minindex;
				minid = i;
			}
		}
		AddVertex(minid, Forest_seq);
	}

	Forest_seq_set = true;
}

void CPI::Forest_match(){
	selected.clear();	
	path.clear();
	selectednum = 0;
	CCnt.clear();
	memset(inseq, 0, query_graph->n*sizeof(bool));
	
	/* Each path i */
	Set_Forest_seq();
	Forest_match(0);
}


void CPI::Forest_match (int x) {	// x vertices has been matched!
	if(numembeddings > bound) return;
	ncalls++;
	unsigned s0 = query_graph->Forest.size();
	//printf("Forest_size = %d, match order = %d\n", s0, x);
	if (x == s0 - numofleaves) {	// Find another forest match!		
		Leaf_match(); 
		return;
	}
	int u = Forest_seq[x];
	if (M.find(u) != M.end()){	// u is a tree root
		Forest_match(x+1);
		return;
	}
	
	int up = pid[u];
	if(DBmode) assert(pN[up][u] != NULL);
	if(DBmode) assert(M.find(up) != M.end());	// up must be a matched one!
	vector<int> &proj = (*pN[up][u])[M[up]];
	// Each v in N(<up,u>, <M[up],?>)
	for(unsigned i=0; i<proj.size(); i++){
		int v = proj[i];
		if (!visited[v]){		// v in data_graph should not be a visited one
		
			M[u] = v;
			visited[v] = true;
			matched[u] = true;
			Forest_match(x+1);	// Find match recursively
			visited[v] = false;
			matched[u] = false;
			M.erase(M.find(u));
		}
	}
}

void CPI::Leaf_match(){
	auto bound_cmp = bind(&CPI::comparer, this, _1, _2);
	sort(Leaves.begin(), Leaves.end(), bound_cmp);
	if(DBmode) assert(Leaves.size()==numofleaves);

	NECcontain = new vector<int>[Leaves.size()];
	NEClb = new int[Leaves.size()];
	int cnt = -1, lbcache = -1, pcache = -1, it = 0;
	int s0 = Leaves.size();
	while(it < s0){
		int u = Leaves[it];
		if(query_graph->vlb[u] == lbcache && pid[Leaves[it]] == pcache){
			NECcontain[cnt].push_back(u);
			it++;
		}else{
			cnt++;	// new NEC!
			NECs.push_back(cnt);
			NEClb[cnt] = query_graph->vlb[u];
			pcache = pid[Leaves[it]];
			lbcache = query_graph->vlb[u];
		}
	}
	Leaf_match_o(0);
	delete[] NEClb;
	delete[] NECcontain;
	NECs.clear();
}


// Leaf sorter
bool CPI::comparer(int l1, int l2){
	if(DBmode) assert(isLeaf[l1]==true);
	if(DBmode) assert(isLeaf[l2]==true);

	// Rank1 order: cluster with identical label
	if(query_graph->vlb[l1] != query_graph->vlb[l2]) return query_graph->vlb[l1] < query_graph->vlb[l2];
	
	int p1 = pid[l1], p2 = pid[l2];
	if(DBmode) assert(pN[p1][l1] != NULL);
	if(DBmode) assert(pN[p2][l2] != NULL);
	vector<int> &proj1 = (*pN[p1][l1])[M[p1]], &proj2 = (*pN[p2][l2])[M[p2]];
	
	// Rank2 order: small size first
	if(proj1.size() != proj2.size()) return proj1.size() < proj2.size();
	
	// Rank3 order: cluster with identical parent
	if(pid[l1] != pid[l2]) return pid[l1] < pid[l2];

	// Otherwise: No matter
	return false;
}


void next_Comb(vector<int> &src, vector<int> &ret, int num){
	for(int i = num-1; i >= 0; i--){
		if(ret[i]);
	}
}


void CPI::genComb(vector<int> &src, vector<vector<int> > &combs, vector<int> &cur, int x, int tot, int lastid){
	if(x == tot){
		combs.push_back(cur);
		return;
	}
	
	for(unsigned i=lastid+1; i<src.size(); i++){
		if(visited[src[i]]) continue;
		cur.push_back(src[i]);
		genComb(src, combs, cur, x+1, tot, i);
		cur.pop_back();
	}
}


void CPI::Leaf_match(int x){
	if(numembeddings > bound) return;
	//printf("%d/%d\n", x, NECs.size());
	if (x == NECs.size()) {	// Find another leaf_match
		printf("Leaf_match OK\n");
		Output();
		return;
	}
	int u_ = NECs[x], u0 = NECcontain[x][0];	// first vertex in this NEClass
	int up = pid[u0];
	if(DBmode) assert(M.find(up) != M.end());	// up must be a matched one!
	if(DBmode) assert(pN[up][u0] != NULL);
	vector<int> &proj = (*pN[up][u0])[M[up]];
	vector<int> cur;	// Index record
	vector<vector<int> > combs;
	genComb(proj, combs, cur, 0, NECcontain[x].size(), -1);

	auto it = combs.begin();
	for(; it != combs.end(); ++it){
		for(int i=0; i < it->size(); i++){
			int v = (*it)[i];
			visited[v] = true;
		}
		auto res = Leaf_M.insert(pair<int, vector<int> >(u_, *it));
		assert(res.second == true);
		auto it_u_ = res.first;

		Leaf_match_o(x+1);

		Leaf_M.erase(it_u_);
		for(int i=0; i < it->size(); i++){
			int v = (*it)[i];
			visited[v] = false;
		}
	}
}


void CPI::Leaf_match_o(int x){
	if(numembeddings > bound) return;
	ncalls++;
	if (x == numofleaves) {	// Find another fo
		Output();
		return;
	}
	int u = Leaves[x];
	int up = pid[u];
	assert(M.find(up) != M.end());	// up must be a matched one!

	assert(pN[up][u] != NULL);
	vector<int> &proj = (*pN[up][u])[M[up]];
	for (unsigned i=0; i<proj.size(); i++){
		int v = proj[i];
		if (!visited[v]){		// v in data_graph should not be a visited one
			M[u] = v;
			visited[v] = true;
			matched[u] = true;
			Leaf_match_o(x+1);	// Find match recursively
			visited[v] = false;
			matched[u] = false;
			M.erase(M.find(u));
		}else{
			//printf("(%d,%d)->(%d,%d)failed!\n", up, u, M[up], v);
		}
	}
}
