//
//  vf3_sub_state.hpp
//  VF2Plus
//
//  Created by Vincenzo Carletti on 12/11/14.
//  Copyright (c) 2014 Vincenzo Carletti. All rights reserved.
//
#define UNUSED_VARIABLE(x) ((void)(x))
#ifndef VF3_SUB_STATE_HPP
#define VF3_SUB_STATE_HPP

#include <cstring>
#include <iostream>
#include <vector>
#include "../graph/Graph.h"
#include "../hyper.h"

//typedef unsigned char node_dir_t;
#define NODE_DIR_NONE 0
#define NODE_DIR_IN	1
#define NODE_DIR_OUT 2
#define NODE_DIR_BOTH 3

using namespace std;
typedef unsigned int node_id; /**<Type for the id of the nodes in the graph */
const node_id NULL_NODE = 0xffffffff;
extern double max_mem;
extern long numembeddings;
extern long bound;

/*----------------------------------------------------------
 * class VF3SubState
 * A representation of the SSR current state
 * See vf2_state.cc for more details.
 ---------------------------------------------------------*/
class VF3SubState {
public:
	HyperGraph *gsh;
	Graph *data, *query;
	vector<node_id> *pboostC0;
	FILE *ofp;
	bool *used;
	int *pnumofembeddings, *pnum_recursive_call;
		
	int n1, n2;		// Size of each graph
	node_id *order;	// Order to traverse node on the first graph
	
	// CORE SET SIZES
	int core_len;		// Current lenght of the core set
	int orig_core_len;	// Core set lenght of the previous state
	
	int added_node1;	// Last added node	
	node_id *predecessors;  // spanning tree 中的父节点
	
	//TERMINAL SET SIZE
	//BE AWARE: Core nodes are also counted by these GLOBAL SIZE
	int t2both_len;		// Real len of term set for data graph
	int *t1both_len;	// Real len of term set (M与M的邻居) for query graph for each level
	
	//SIZE FOR EACH CLASS
	int *t2both_len_c;	 // Real len of term set for data graph for each class
	int **t1both_len_c;  // Real len of term set (M与M的邻居) for each class and level
	// t1in_len_c[core_len][c]是搜索到core_len层c标签邻居个数
	
	node_id *core_1, *core_2;	// 匹配本身
	node_id *term_2;
	int *usedTimes, *ump;
	
	//Vector of sets used for searching the successors
	//Each class has its set
	int last_candidate_index;	// Index into 当前节点的邻居集
	
	/* Structures for classes */
	int *class_1, *class_2;	   //Classes for nodes 
	int classes_count;  //Number of classes
	long *share_count;  //Count the number of instances sharing the common sets
	
	// PRIVATE METHODS
	void BackTrack();
	void ComputeFirstGraphTraversing();
	void print_terminal(int c);
	
	static long long instance_count;
	VF3SubState(HyperGraph *_gsh, Graph *_data, Graph *_query, int *_class_1, 
			int *_class_2, int _nclass, node_id *_order, FILE *_ofp);
	VF3SubState(const VF3SubState &state);
	~VF3SubState();
	bool NextPair(node_id *pn1, node_id *pn2, 
			node_id prev_n1, node_id prev_n2);
	bool IsFeasiblePair(node_id n1, node_id n2);
	void AddPair(node_id n1, node_id n2);
	bool IsDead();
	void GetCoreSet(node_id c1[], node_id c2[]);
	
	void BuildBoostC0() {
		int u0 = order[0];
		for (int i = 0; i < n2; i++) {
			if (gsh->vList[i] != query->vertices[u0].label) 
				continue;
			if (gsh->real_deg[i] < query->vertices[u0].neigh.size())
				continue;
			if (gsh->scf[i].size() == 0)
				pboostC0->push_back(i);
		}
	}

	void DynamicCL() {
		int node2 = core_1[order[0]];
		for (int i=0; i<gsh->scc[node2].size(); i++) {
			int hi = gsh->scc[node2][i];
			if (ump[hi] == -1)
				ump[hi] = gsh->scf[hi].size();
			ump[hi]--;
			if (ump[hi] == 0)
				pboostC0->push_back(hi);
		}
	}

	void combs(node_id node1, vector<int> &res) {
if (numembeddings > bound) return;
		node_id node1_bak = node1;
		if (node1 == n1) {
double vsize, rss;
Util::process_mem_usage(vsize, rss);
if (vsize > max_mem) max_mem = vsize;
#ifdef PRINT_RESULT
			for (node_id i = 0; i < node1; i++)
				fprintf(ofp, "(%d, %d) ", i, res[i]);
			fprintf(ofp, "\n");
#endif
			numembeddings++;
			return;
		}
		node_id node2 = core_1[node1];

		// Enumerate all data vertices contained in this hypernode
		for (int i=0; i<gsh->vertices[node2].size(); i++) {
			int v = gsh->vertices[node2][i];
			if (used[v]) 			// h is remapped
				continue;
			used[v] = true;
			res.push_back(v);
			combs(node1 + 1, res);
			res.pop_back();
			used[v] = false;
if (numembeddings > bound) return;
		}
	}
	
	void Output() {
		vector<int> res;
		combs(0, res);
		//double vsize, rss;
		//Util::process_mem_usage(vsize, rss);
		//if (vsize > max_mem) max_mem = vsize;
	}
};


/*----------------------------------------------------------
 * VF3SubState::VF3SubState(g1, g2)
 * Constructor. Makes an empty state.
 ---------------------------------------------------------*/
VF3SubState::VF3SubState(HyperGraph *_gsh, Graph *_data, Graph *_query, int *_class_1, 
		int *_class_2, int _nclass, node_id *_order, FILE *_ofp) {
	//assert(class_1!=NULL && class_2!=NULL);
	data = _data;	query = _query;		gsh = _gsh;
	n1 = query->vertices.size();;
	n2 = gsh->numVertex;
	last_candidate_index = 0;
	ofp = _ofp;
	pnumofembeddings = new int;		*pnumofembeddings = 0;
	pnum_recursive_call = new int;	*pnum_recursive_call = 0;
	pboostC0 = new vector<node_id>;

	order = _order;
	class_1 = _class_1;
	class_2 = _class_2;
	classes_count = _nclass;
	core_len = orig_core_len = 0;
	t2both_len = 0;
	
	//Creazione degli insiemi
	t1both_len = new int[n1+1];
	t1both_len_c = new int*[n1+1]; //(int**)malloc((n1+1)*sizeof(int*));
	
	t2both_len_c = new int[classes_count+1]; //(int*) calloc(classes_count+1, sizeof(int));
	memset(t2both_len_c, 0, (classes_count+1)*sizeof(int));
	added_node1 = NULL_NODE;
	
	core_1 = new node_id[n1];	core_2 = new node_id[n2];
	term_2 = new node_id[n2];
	usedTimes = new int[n2];
	ump = new int[n2];
	used = new bool[data->vertex_num];
	memset(used, 0, data->vertex_num*sizeof(bool));
	memset(ump, -1, n2*sizeof(int));
	predecessors = new node_id[n1];
	share_count = new long;
	
	for (int i=0; i<=n1; i++) {
		if (i<n1)
			core_1[i] = NULL_NODE;
		t1both_len_c[i] = new int[classes_count+1];//(int*) calloc(classes_count, sizeof(int));
		memset(t1both_len_c[i], 0, (classes_count+1)*sizeof(int));
	}
	
	for(int i=0; i<n2; i++) {
		core_2[i] = NULL_NODE;
		term_2[i] = 0;
		usedTimes[i] = 0;
	}
	BuildBoostC0();
	ComputeFirstGraphTraversing();
	*share_count = 1;
}


/*----------------------------------------------------------
 * VF3SubState::VF3SubState(state)
 * Copy constructor.
 ---------------------------------------------------------*/
VF3SubState::VF3SubState(const VF3SubState &state) { 
	data=state.data;
	query=state.query;
	gsh=state.gsh;
	n1=state.n1;
	n2=state.n2;
	pnumofembeddings = state.pnumofembeddings;
	pnum_recursive_call = state.pnum_recursive_call;
	ofp = state.ofp;
	pboostC0 = state.pboostC0;
	
	order=state.order;
	class_1 = state.class_1;
	class_2 = state.class_2;
	classes_count = state.classes_count;
	
	last_candidate_index = state.last_candidate_index;
	core_len = orig_core_len = state.core_len;
	
	t1both_len = state.t1both_len;
	t2both_len = state.t2both_len;
	
	t1both_len_c = state.t1both_len_c;
	t2both_len_c = state.t2both_len_c;
	
	added_node1=NULL_NODE;
	
	core_1=state.core_1;
	core_2=state.core_2;
	term_2 = state.term_2;
	usedTimes = state.usedTimes;
	ump = state.ump;
	used = state.used;

	predecessors = state.predecessors;
	share_count=state.share_count;
	++ *share_count;	
}


/*---------------------------------------------------------------
 * VF3SubState::~VF3SubState()
 * Destructor.
 --------------------------------------------------------------*/
VF3SubState::~VF3SubState() {
	//cerr<<"Fucking Destructor"<<endl;
	if(-- *share_count > 0)
		BackTrack();
	
	if (*share_count == 0){ 
		delete [] core_1;	core_1=NULL;
		delete [] core_2;	core_2=NULL;
		delete [] term_2;	term_2=NULL;
		delete [] predecessors;	predecessors=NULL;
		delete [] t1both_len;	t1both_len=NULL;
		delete []usedTimes;	usedTimes = NULL;
		delete []used;	used = NULL;
		delete []ump;	ump = NULL;
		delete pnum_recursive_call;  pnum_recursive_call = NULL;
		delete pboostC0;	pboostC0 = NULL;
		delete pnumofembeddings;	pnumofembeddings = NULL;
		
		for(int i = 0; i <= n1; i++)
			delete [] t1both_len_c[i];	

		delete [] t1both_len_c;	t1both_len_c=NULL;
		delete [] t2both_len_c;	t2both_len_c=NULL;
		
		delete share_count;	share_count=NULL;
	}
}


//Provare ad avere in1 ed ou1 predeterminati, senza doverlo calcolare ad ogni iterazione
//La loro dimensione ad ogni livello dell'albero di ricerca e' predeterminato
//In questo modo mi basta conoscere solo l'ordine di scelta e la dimensione di in1 ed out1
// 尝试预先确定 terminal set, 而不必在每次迭代时都进行计算，在搜索树的每个级别上它们的大小都是预先确定的
// 这样，我只需要知道选择的顺序以及 in1 和 out1 的大小
void VF3SubState::ComputeFirstGraphTraversing(){
	//The algorithm start with the node with the maximum degree
	node_id node = 0;	//Current Node
	int node_c = 0; 			//Class of the current node
	bool *inserted = new bool[n1];
	bool *term = new bool[n1];	//Internal Terminal Set used for updating the size of

	//Init vectors and variables
	t1both_len[0] = 0;
	
	for(int i = 0; i < n1; i++) {
		term[i] = false;
		inserted[i] = false;
		predecessors[i] = NULL_NODE;
	}
	
	/* Following the imposed node order */
	for(node_id depth = 0; depth < n1; depth++) {
		// depth=0 时更新 t1both_len[1], 即匹配完 order[0] 后的 terminal set
		node = order[depth];
		node_c = query->vertices[node].label;
		inserted[node] = true;
		
		// Copy 给下一层的 Terminal set
		t1both_len[depth +1] = t1both_len[depth];
		for (int j = 0; j < classes_count; j++) 
			t1both_len_c[depth +1][j] = t1both_len_c[depth][j];

		// 把node自己加入到 Terminal set 中
		//Terminal set sizes depends on the depth
		// < depth non sono nell'insieme
		// >= depth sono nell'insieme
		if (!term[node]) {
			term[node] = true;			// 在下层入坑
			t1both_len[depth+1]++;	// 下层入坑人数++
			t1both_len_c[depth+1][node_c]++;
		}
		
		// Updating terminal sets
		int other, other_c;
		other_c = -1;
		query->vertices[node];
		for(int i=0; i<query->vertices[node].neigh.size(); i++) {	// Iterate through all neighbors of node
			other = query->vertices[node].neigh[i].vid;
			if (!term[other])	{	// node 入坑时间在周期末尾, 其坑外邻居在下层入坑
				other_c = query->vertices[other].label;
				term[other] = true;
				t1both_len[depth+1]++;
				t1both_len_c[depth+1][other_c]++;
				if(!inserted[other])
					if(predecessors[other] == NULL_NODE)
						predecessors[other] = node;
			}
		}
	} delete [] inserted; delete []term;
}

// 找一个查询点 curr_n1 可能匹配上的数据点
bool VF3SubState::NextPair(node_id *pn1, node_id *pn2, 
		node_id prev_n1, node_id prev_n2) {
	
	node_id curr_n1;
	node_id pred_pair; //Node mapped with the predecessor
	node_id pred_set_size = 0;
	int c = 0;
	pred_pair = NULL_NODE;
	
	//core_len 搜索深度
	curr_n1 = order[core_len];	// Currently matching query node
	c = query->vertices[curr_n1].label;
	
	if (predecessors[curr_n1] != NULL_NODE) {	// 不是第一个查询节点...
		if (prev_n2 == NULL_NODE)		// 第一次来到这个节点
			last_candidate_index = 0;
		else
			last_candidate_index++; 	//Next Element
		
		pred_pair = core_1[predecessors[curr_n1]];
		pred_set_size = gsh->neigh[pred_pair].size();
		while (last_candidate_index < pred_set_size) {	// 遍历所有邻居
			prev_n2 = gsh->neigh[pred_pair][last_candidate_index];
			if (gsh->vList[prev_n2] != c || usedTimes[prev_n2] >= gsh->vertices[prev_n2].size())
				last_candidate_index++;			// 这个邻居不行,继续吧
			else
				break;		// 这个邻居可以
		}

		if (last_candidate_index == pred_set_size) {
			if (gsh->vType[pred_pair] == 1 && usedTimes[pred_pair] < gsh->vertices[pred_pair].size()) {
				// 生成树上父节点匹配点pred_pair是clique, 且有空位可用
				prev_n2 = pred_pair;
				last_candidate_index++;
			}else {		// 父节点的匹配点pred_pair是single/independent
				return false;
			}
		} else if (last_candidate_index > pred_set_size){
			// All possibilities have been tried...
			return false;
		}
	}
	else {	// 第一个查询点!!!
		//Recupero il nodo dell'esterno
		if(prev_n2 == NULL_NODE)
			prev_n2 = 0;
		else
			prev_n2++;

		while (prev_n2 < n2 && (usedTimes[prev_n2] >= gsh->vertices[prev_n2].size() 
				|| gsh->vList[prev_n2] != c))
			prev_n2++;
	}
	
	if (prev_n2 < n2) {
		*pn1 = curr_n1;
		*pn2 = prev_n2;
		return true;
	}
	return false;
}


/*---------------------------------------------------------------
 * bool VF3SubState::IsFeasiblePair(node1, node2)
 * Returns true if (node1, node2) can be added to the state
 * NOTE:
 *   The attribute compatibility check (methods CompatibleNode
 *   and CompatibleEdge of ARGraph) is always performed
 *   applying the method to g1, and passing the attribute of
 *   g1 as first argument, and the attribute of g2 as second
 *   argument. This may be important if the compatibility
 *   criterion is not symmetric.
 --------------------------------------------------------------*/
bool VF3SubState::IsFeasiblePair(node_id node1, node_id node2) {
//NOTICE: we choose the definition of embedding subgraph isomorphism, i.e. monomorphism

	assert(node1<n1); assert(node2<n2);
	assert(core_1[node1]==NULL_NODE); //assert(core_2[node2]==NULL_NODE);
	
	//1. check the vertex labels uv标签要相同
	if(query->vertices[node1].label != gsh->vList[node2])
		return false;
	
	// 2. check the connections with already matched pairs
	// check the count of edges first (a naive judgement)
	// 2. 检查degree
	if (query->vertices[node1].neigh.size() > gsh->real_deg[node2])
		return false;

	int other1, other2;
	// Check the 'out' edges of node1
	for(int i=0; i<query->vertices[node1].neigh.size(); i++) {
		other1 = query->vertices[node1].neigh[i].vid;	// node1的第i个邻居o1
		if (core_1[other1] != NULL_NODE) {		// o1 matched!
			other2 = core_1[other1];			// Edge (core_1[o1], node2) 存在?
			if (other2 != node2) {				// Normal Validate
				auto noans = gsh->neigh[node2].end();
				if (find(gsh->neigh[node2].begin(), noans, other2) == noans)
					return false;
			} else{
				if (gsh->vType[node2] != 1) return false;	// node2 isn't a clique
				if (usedTimes[node2] >= gsh->vertices[node2].size())
					return false;
			}
		}
	}
	return true;
}



/*--------------------------------------------------------------
 * void VF3SubState::AddPair(node1, node2)
 * Adds a pair to the Core set of the state.
 * Precondition: the pair must be feasible
 -------------------------------------------------------------*/
void VF3SubState::AddPair(node_id node1, node_id node2) {
	assert(node1<n1); assert(node2<n2);
	assert(core_len<n1); assert(core_len<n2);
	
	// Updating the core lenght
	core_len++;
	(*pnum_recursive_call)++;
	added_node1 = node1;
	int node_c = query->vertices[node1].label;

	// 把node2自己加入 terminal set
	if (!term_2[node2]) {
		term_2[node2] = core_len;	// 记录下 node2 加入到 term 的时间
		// 对 vType[node2] 分类讨论:
		//  - if (vType[node2] == 0) 即单点, ++
		//  - else if (vType[node2] == 1) 即团, 整个团都加入 term
		//  - else if (vType[node2] == 2) 即独立集, 此处也全部加入 term
		//		但事实上独立集中可能有很多节点并不能算作是在独立集中, 细粒度控制太麻烦
		if (gsh->vType[node2] == 1 || gsh->vType[node2] == 2) {
			t2both_len += gsh->vertices[node2].size();
			t2both_len_c[node_c] += gsh->vertices[node2].size();
		} else {
			t2both_len++;
			t2both_len_c[node_c]++;
		}
	}
	
	//Inserting nodes into the core set
	core_1[node1] = node2;
	//core_2[node2] = node1;
	usedTimes[node2]++;
	
	int other, other_c;
	other_c = -1;
	
	// 把node2的邻居全加入 terminal set, term_2相当于bitmap
	for(int i=0; i<gsh->neigh[node2].size(); i++){
		other = gsh->neigh[node2][i];
		if (!term_2[other]) {
			other_c = gsh->vList[other];
			term_2[other] = core_len;
			if (gsh->vType[other] == 1 || gsh->vType[other] == 2) {
				t2both_len += gsh->vertices[other].size();
				t2both_len_c[other_c] += gsh->vertices[other].size();
			} else {
				t2both_len++;
				t2both_len_c[other_c]++;
			}
		}
	}
}



/*--------------------------------------------------------------
 * void VF3SubState::GetCoreSet(c1, c2)
 * Reads the core set of the state into the arrays c1 and c2.
 * The i-th pair of the mapping is (c1[i], c2[i])
 --------------------------------------------------------------*/
void VF3SubState::GetCoreSet(node_id c1[], node_id c2[]) {
	for (int i=0, j=0; i<n1; i++)
		if (core_1[i] != NULL_NODE) {
			c1[j] = i;
			c2[j] = core_1[i];
			j++;
		}
}

/*----------------------------------------------------------------
 * Undoes the changes to the shared vectors made by the
 * current state. Assumes that at most one AddPair has been
 * performed.
 ----------------------------------------------------------------*/
void VF3SubState::BackTrack() {
	
	assert(core_len - orig_core_len <= 1);
	assert(added_node1 != NULL_NODE);
	
	int other_c = 0;
	int node_c = query->vertices[added_node1].label;
	
	if (orig_core_len < core_len) {		// 为什么会为否???
		int node2;
		node2 = core_1[added_node1];
		
		if (term_2[node2] == core_len){		// node2 本轮刚刚加入 term
			term_2[node2] = 0;
			if (gsh->vType[node2] == 1 || gsh->vType[node2] == 2)
				t2both_len_c[node_c] -= gsh->vertices[node2].size();
			else
				t2both_len_c[node_c]--;
		}
		
		//Backtraking neightborhood
		for(int i=0; i<gsh->neigh[node2].size(); i++) {
			int other = gsh->neigh[node2][i];
			other_c = gsh->vList[other];
			if (term_2[other] == core_len){
				term_2[other] = 0;
				if (gsh->vType[other] == 1 || gsh->vType[other] == 2)
					t2both_len_c[other_c] -= gsh->vertices[other].size();
				else {
					if (other_c > classes_count) {
						printf("ERROR lable=%d, max label=%d\n", other_c, classes_count);
						exit(0);
					}
					t2both_len_c[other_c]--;
				}
			}
		}

		core_1[added_node1] = NULL_NODE;
		// core_2[node2] = NULL_NODE;
		usedTimes[node2]--;
		assert(usedTimes[node2] >= 0);
		
		core_len = orig_core_len;
		added_node1 = NULL_NODE;
	}
}

// 检查 core_len 的
bool VF3SubState::IsDead() {
	if (n1 > n2) return true;
	if (t1both_len[core_len] > t2both_len)
		return true;
	
	for (int c = 0; c < classes_count; c++)
		if (t1both_len_c[core_len][c] > t2both_len_c[c])
			return true;
	return false;
}

#endif
