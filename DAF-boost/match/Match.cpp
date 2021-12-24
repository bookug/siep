/*=============================================================================
# Filename: Match.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-12-15 01:38
# Description: 
=============================================================================*/

#include "Match.h"

using namespace std;

extern double max_mem;
extern long ncalls, nembeddings, bound;

//#define DEBUG 1
#define HYPER_HOMOMORPHISM
//#define PIVOT_DYNAMICCL  //only use dynamicCL for the first pivots of candidate regions
//#define DYNAMIC_CL
//#define PIVOT_QDN
//#define USE_QDN

Match::Match(Graph* _query, Graph* _data, HyperGraph* _gsh)
{
	numofembeddings = 0;
    num_recursive_call = 0;
	this->query = _query;
	this->data = _data;
	this->qsize = _query->vSize();
	this->dsize = _data->vSize();
    this->gsh = _gsh;
    this->hdsize = _gsh->numVertex;
    //WARN: below will cause memory error, because these two objects are released after this function
    //this->qd = Graph(true);
    //this->reverse_qd = Graph(true);
    this->qd.setDirected(true);
    this->reverse_qd.setDirected(true);
    this->data_v_visited = vector<bool>(_data->vertex_num, false);
}

Match::~Match()
{
}

int contain(int e, vector <int> *a)
{
    vector<int>::iterator it = find(a->begin(), a->end(), e);
    if(it == a->end())
    {
        return -1;
    }
    return it-a->begin();
	//int Size = a->size();
	//for(int i = 0; i < Size; i ++)
	//{
		//if((*a)[i] == e)
		//{
			//return i;
		//}
	//}
	//return -1;
}

int contain(int e, vector <labelVlist> *a)
{
    labelVlist tmp;
    tmp.label = e;
    vector<labelVlist>::iterator it = find(a->begin(), a->end(), tmp);
    if(it == a->end())
    {
        return -1;
    }
    return it - a->begin();
	//int Size = temp->size();
	//for(int i = 0; i < Size; i ++)
	//{
		//if((*temp)[i].label == e)
			//return i;
	//}
	//return -1;
}

void computeQDN(Graph* g, Graph* q, int u, int v, vector<int>& adjs)
{
    vector<Neighbor>& uadj = q->vertices[u].in;
    for(int i = 0; i < uadj.size(); ++i)
    {
        int label = uadj[i].elb;
        for(int j = 0; j < g->vertices[v].in.size(); ++j)
        {
            if(g->vertices[v].in[j].elb == label)
            {
                adjs.push_back(g->vertices[v].in[j].vid);
            }
        }
    }
}

bool  isQDC(Graph* g, Graph* q, int u, int v, int v2)
{
    if(g->vertices[v].label != g->vertices[v2].label)
    {
        return false;
    }
    vector<int> adj1, adj2;
    computeQDN(g, q, u, v, adj1);
    computeQDN(g, q, u, v2, adj2);
    for(int i = 0; i < adj1.size(); ++i)
    {
        if(adj1[i] == v2)
        {
            adj1[i] = -1;
            break;
        }
    }
    for(int i = 0; i < adj2.size(); ++i)
    {
        if(adj2[i] == v)
        {
            continue;
        }
        bool flag = false;
        for(int j = 0; j < adj1.size(); ++j)
        {
            if(adj1[j] == adj2[i])
            {
                flag = true;
                break;
            }
        }
        if(!flag)
        {
            return false;
        }
    }
    return true;
}

bool isQDE(Graph* g, Graph* q, int u, int v, int v2)
{
    return isQDC(g, q, u, v, v2) && isQDC(g, q, u, v2, v);
}

//build relation table for each query node,   QDN (query dependent neighbor): QDC and QDE
//NOTICE: the candidates C(u) should be filtered by NLF filter, and only contain vertices whose SC graph in-degree is 0.
//QUERY: in TurboISO, we should use NEC tree/graph instead?   (I suppose it is the same.)
//Note that we do not index hypernodes which are listed in the QDE-List of another indexed hypernode.   
void BuildDRT(vector<int>& C, int u, HyperGraph* gsh, Graph* g, Graph* q)
{
    //NOTICE: this u is for real query vertex.   
    //(When calling this function, just select one real node of the NEC vertex)
    vector<QDN>& drt = gsh->drt[u];
    drt = vector<QDN>(gsh->numVertex);
    for(int i = 0; i < C.size(); ++i)
    {
        int h = C[i];
        if(h == -1) continue;
        if(gsh->scf[h].size() > 0)   continue;
        for(int j = i+1; j < C.size(); ++j)
        {
            int h2 = C[j];
            if(h2 == -1 || h2 == h) continue;
            if(gsh->scf[h2].size() > 0)   continue;
            if(isQDE(g, q, u, gsh->vertices[h][0], gsh->vertices[h2][0]))
            {
                drt[h].qdel.push_back(h2);
                //remove h2 from DRT and C(u)
                drt[h2].clear();
                C[j] = -1;
            }
        }
    }
    int k = 0;
    //NOTICE: C should be compacted
    for(int i = 0; i < C.size(); ++i)
    {
        if(C[i] != -1)
        {
            C[k++] = C[i];
        }
    }
    C.resize(k);
    for(int i = 0; i < C.size(); ++i)
    {
        int h = C[i];
        if(gsh->scf[h].size() > 0)   continue;
        for(int j = i+1; j < C.size(); ++j)
        {
            int h2 = C[j];
            if(h2 == h) continue;
            if(gsh->scf[h2].size() > 0)   continue;
            if(isQDC(g, q, u, gsh->vertices[h][0], gsh->vertices[h2][0]))
            {
                drt[h].qdcc.push_back(h2);
                drt[h2].num++;
            }
            else if(isQDC(g, q, u, gsh->vertices[h][0], gsh->vertices[h2][0]))
            {
                drt[h2].qdcc.push_back(h);
                drt[h].num++;
            }
        }
    }
}

void PureInsert(vector<int>& vecs, int id)
{
    for(int i = 0; i < vecs.size(); ++i)
    {
        if(vecs[i] == id)
        {
            return;
        }
    }
    vecs.push_back(id);
}

void UpdateState(int *M, int *F, int u, int v)
{
    M[u] = v;
    F[v]--;
}

void RestoreState(int *M, int *F, int u, int v)
{
    M[u] = -1;
    F[v]++;
}

//update C(u) according to SC relations
//We use a hash array for ump structure.
void DynamicCL(vector<int>& C, int h, HyperGraph* gsh, vector<int>& ump, int ureal, vector<bool>& hasht)
{
    //WARN:in TurboISO, if we want to use DynamicCL, we must re-explore the  subregion from this added node, otherwise CR parent and CR array corresponding to this parent will be wrong!
    vector<int>& scch = gsh->scc[h];
#ifdef PIVOT_QDN
    QDN& qdn = gsh->drt[ureal][h];
    vector<int>& qdch = qdn.qdcc;
    scch.insert(scch.end(), qdch.begin(), qdch.end());
    for(int i = 0; i < qdn.qdel.size(); ++i)
    {
        int h2 = qdn.qdel[i];
        vector<int>& qdec = gsh->scc[h2];
        scch.insert(scch.end(), qdec.begin(), qdec.end());
    }
#endif
    for(int i = 0; i < scch.size(); ++i)
    {
        int id = scch[i];
        if(ump[id] == -1)  //not exists
        {
            ump[id] = gsh->scf[id].size();
#ifdef PIVOT_QDN
            ump[id] += qdn.num;
#endif
        }
        ump[id] --;
        if(ump[id] == 0 && hasht[id])
        {
            C.push_back(id);
        }
    }
}

//output a result after verified
int output(vector<bool>& data_v_visited, int* M, int qVNum, FILE* fpR, HyperGraph* gsh, int pos, int us)
{
	if (nembeddings > bound) return 0;
    if(pos == qVNum)
    {
double vsize, rss;
Util::process_mem_usage(vsize, rss);
if(vsize > max_mem)
    max_mem = vsize;
        //NOTICE: we should output the whole matching in the end.
#ifdef PRINT_RESULT
        for(int i = 0; i < qVNum; ++i)
            fprintf(fpR, "(%d, %d) ", i, M[i]);
        fprintf(fpR, "\n");
#endif
        nembeddings++;
        return 1;
    }
    int hv = M[pos];
    vector<int> rv = gsh->vertices[hv];
    //Below is only for qdel of pivots
#ifdef PIVOT_QDN
    if(pos == us)   //pivots
    {
    for(int i = 0; i < gsh->drt[pos][hv].qdel.size(); ++i)
    {
        vector<int>& tmp = gsh->vertices[gsh->drt[pos][hv].qdel[i]];
        rv.insert(rv.end(), tmp.begin(), tmp.end());
    }
    }
#endif
    int sum = 0;
    for(int i = 0; i < rv.size(); ++i)
    {
        //bool flag = true;
        //NOTICE: we need this judgement to remove duplicates
        //(u1, h1) (u2, h1)   h1 includes {v1, v2}, 2 valid enumeration, 2 invalid.
        //This can also help remove the duplicates caused by BoostISO QDE error.
        //However, it is a type of pos-processing, and is inefficient, leading to many unpromising enumerations.
        //for(int j = 0; j < pos; ++j)
        //{
            //if(M[j] == rv[i])
            //{
                //flag = false;
                //break;
            //}
        //}
        //if(!flag)  continue;
        if(data_v_visited[rv[i]]) continue;
        data_v_visited[rv[i]] = true;
        //fprintf(fpR, "(%d, %d) ", pos, rv[i]);
        M[pos] = rv[i];
        sum += output(data_v_visited, M, qVNum, fpR, gsh, pos+1, us);
        data_v_visited[rv[i]] = false;
        if (nembeddings > bound) return sum;
    }
    M[pos] = hv;
	//for(int i = 0; i < qVNum; ++i) {
		//fprintf(fpR, "(%d, %d) ", i, M[i]);
	//}
	//fprintf(fpR, "\n");
    return sum;
}

void compress(vector<int>& ev)
{
    int num = ev.size();
    int i = 0, k =0;
    while(i < num)
    {
        if(ev[i] != -1)
        {
            ev[k++] = ev[i];
        }
        ++i;
    }
    ev.resize(k);
}






//BETTER: apply leaf-decomposition in CFL-Match
//1. leaves are matched at last. (already used)
//2. Merge leaves into NECs first.
//CONSIDER: not filter leaves, generate their solutions from adjs directly.
void 
Match::buildDAG()
{
    reverse_qd.vertex_num = qd.vertex_num = qsize;
    qd.initLabelFreq(this->query->vertexLabelNum);
    reverse_qd.initLabelFreq(this->query->vertexLabelNum);
    for(int i = 0; i < qsize; ++i)
    {
        qd.addVertex(this->query->vertices[i].label);
        reverse_qd.addVertex(this->query->vertices[i].label);
    }
    this->dparr = new bool[qsize*hdsize];
    memset(dparr, 0, sizeof(bool)*qsize*hdsize);
    vector<int> count(qsize, 0);
    //collect initial candidates
    //cout<<"hdsize: "<<hdsize<<endl;
    for(int i = 0; i < qsize; ++i)
    {
        int lb = this->query->vertices[i].label;
        int deg = this->query->vertices[i].in.size();
        for(int j = 0; j < hdsize; ++j)
        {
            int rv = gsh->vertices[j][0];
            if(this->data->vertices[rv].label != lb)
            {
                continue;
            }
            if(this->data->vertices[rv].in.size() < deg)
            {
                continue;
            }
            dparr[i*hdsize+j] = true;
            count[i]++;
        }
    }
    //select the source node of DAG
    double maxv = DBL_MAX;
    int maxi = -1;
    for(int i = 0; i < qsize; ++i)
    {
        double ele = (double)(count[i])/(this->query->vertices[i].in.size());
        if(ele < maxv)
        {
            maxv = ele;
            maxi = i;
        }
    }
    //perform BFS to acquire DAG  (directed edges)
    queue<int> qq;  qq.push(maxi);
    vector<int> visited(qsize, -1);
    vector<bool> visited_edges(this->query->edge_num, false);
    visited[maxi] = 0;
    while(!qq.empty())
    {
        int u = qq.front();
        int level = visited[u];
        qq.pop();
        for(int j = 0; j < this->query->vertices[u].in.size(); ++j)
        {
            int u2 = this->query->vertices[u].in[j].vid;
            //below(SNIPPET_EID_CHECK) is to avoid duplicate edges
//BEGIN: SNIPPET_EID_CHECK
            int eid = this->query->vertices[u].in[j].eid;
            if(visited_edges[eid])
            {
                continue;
            }
            else
            {
                visited_edges[eid] = true;
            }
//END: SNIPPET_EID_CHECK
            if(visited[u2] == -1)
            {
                visited[u2] = level+1;
                qq.push(u2);
                qd.addEdge(u, u2, 2);
                reverse_qd.addEdge(u2, u, 2);
            }
            else
            {
                //already visited, already in queue
                //maybe the same level, or the next level
                if(visited[u2] == level)
                {
                    //same level: sort on label and degree
                    int f1 = this->data->getLabelFreq(this->query->vertices[u].label);
                    int f2 = this->data->getLabelFreq(this->query->vertices[u2].label);
                    if(f1 < f2)
                    {
                        qd.addEdge(u, u2, 2);
                        reverse_qd.addEdge(u2, u, 2);
                    }
                    else if(f1 > f2)
                    {
                        qd.addEdge(u2, u, 2);
                        reverse_qd.addEdge(u, u2, 2);
                    }
                    else   //f1 == f2
                    {
                        if(this->query->vertices[u].in.size() < this->query->vertices[u2].in.size())
                        {
                            qd.addEdge(u2, u, 2);
                            reverse_qd.addEdge(u, u2, 2);
                        }
                        else
                        {
                            qd.addEdge(u, u2, 2);
                            reverse_qd.addEdge(u2, u, 2);
                        }
                    }
                }
                else if(visited[u2] == level+1)
                {
                    //up level to low level (only gap 1 level)
                    qd.addEdge(u, u2, 2);
                    reverse_qd.addEdge(u2, u, 2);
                }
                //NOTICE: another case is visited[u2]==level-1, because an edge is stored twice in undirected graph.
                //WARN: the duplicates may also occur between same levels! We can not solve this problem by defining an order of IDs, e.g., only store the edges from small ID to large ID. 
                //The best way is to assign eid for each edge, and judge if an edge is already visited.
            }
        }
    }

    //pre-compute maximal tree-like paths rooted at u, in topological order
    for(int i = 0; i < qsize; ++i)
    {
        vector<int> tmpv;
        queue<int> qq; qq.push(i);
        while(!qq.empty())
        {
            int u = qq.front();  qq.pop();
            tmpv.push_back(u);
            for(int j = 0; j < qd.vertices[u].out.size(); ++j)
            {
                int u2 = qd.vertices[u].out[j].vid;
                if(qd.vertices[u2].in.size() != 1)
                    continue;
                qq.push(u2);
            }
        }
        mtlp.push_back(tmpv);
    }
}

void 
Match::refineCS(Graph& qdt, vector<int>& _order)
{
    for(int i = 0; i < _order.size(); ++i)
    {
        int u = _order[i];
        for(int v = 0; v < hdsize; ++v)
        {
            if(this->dparr[u*hdsize+v] == false)
            {
                continue;
            }
            bool flag = true;
            for(int j = 0; j < qdt.vertices[u].out.size(); ++j)
            {
                int uc = qdt.vertices[u].out[j].vid;
                bool found = false;
                //ADD: for hyper graphs
            //A hyper vertex can exist both in parent and child
            if(gsh->vertices[v].size()>1 && this->dparr[uc*hdsize+v] && gsh->vType[v] == 1)
            {
                found = true;
                goto CHECK_FOUND;
            }
                
                for(int k = 0; k < (this->gsh->graList[v].size()); ++k)
                {
                    labelVlist& lv = this->gsh->graList[v][k];
                    for(int p = 0; p < lv.vlist.size(); ++p)
                    {
                        int vc = lv.vlist[p];
                        if(this->dparr[uc*hdsize+vc] == true)
                        {
                            found = true;
                            //break;
                            goto CHECK_FOUND;
                        }
                    }
                }
CHECK_FOUND:
                if(!found)
                {
                    flag = false;
                    break;
                }
            }
            if(!flag)
            {
                this->dparr[u*hdsize+v] = false;
            }
        }
    }
}

void
Match::buildCS()
{
    //collect anc(u), which is stored as bitset.
    for(int i = 0; i < qsize; ++i)
    {
        BitArray bv(qsize);
        queue<int> qq; qq.push(i); bv.set(i);
        while(!qq.empty())
        {
            int u = qq.front();
            qq.pop();
            for(int j = 0; j < this->qd.vertices[u].in.size(); ++j)
            {
                int u2 = this->qd.vertices[u].in[j].vid;
                if(!bv.get(u2))
                {
                    qq.push(u2);
                    bv.set(u2);
                }
            }
        }
        ancs.push_back(bv);
    }
    //collect dec(u), which is stored as bitset.
    //for(int i = 0; i < qsize; ++i)
    //{
        //vector<bool> bv(qsize, false);
        //queue<int> qq; qq.push(i); bv[i] = true;
        //while(!qq.empty())
        //{
            //int u = qq.front();
            //qq.pop();
            //for(int j = 0; j < this->qd.vertices[u].out.size(); ++j)
            //{
                //int u2 = this->qd.vertices[u].out[j].vid;
                //if(!bv[u2])
                //{
                    //qq.push(u2);
                    //bv[u2] = true;
                //}
            //}
        //}
        //decs.push_back(bv);
    //}
    //acquire topological order (and the reverse scan is reverse topological order)
    vector<int> degs(qsize);
    queue<int> qq;
    for(int i = 0; i < qsize; ++i)
    {
        degs[i] = this->qd.vertices[i].in.size();
        if(degs[i] == 0)
        {
            qq.push(i);
        }
    }
    while(!qq.empty())
    {
        int u = qq.front();
        qq.pop();
        this->topological_order.push_back(u);
        for(int j = 0; j < this->qd.vertices[u].out.size(); ++j)
        {
            int u2 = this->qd.vertices[u].out[j].vid;
            degs[u2]--;
            if(degs[u2] == 0)
            {
                qq.push(u2);
            }
        }
    }
    //assert(topological_order.size()==qsize);

    //refine CS with qd or reverse qd (qd-1)
    //Empirically, 3 steps are enough: qd-1, qd, qd-1
    for(int i = qsize-1; i >= 0; --i)
    {
        this->reverse_order.push_back(topological_order[i]);
    }
    refineCS(reverse_qd, reverse_order);
    refineCS(qd, topological_order);
    refineCS(reverse_qd, reverse_order);

    //materialize all edges associated with the candidate sets
    for(int i = 0; i < qsize; ++i)
    {
        this->CS.push_back(vector< ECPT >(qsize));
        this->vcs.push_back(vector<int>());
    }
    for(int i = 0; i < qsize; ++i)
    {
        int u = this->topological_order[i];
        int qlabel = this->query->vertices[u].label;
        for(int j = 0; j < hdsize; ++j)
        {
            if(this->dparr[u*hdsize+j])
            {
                this->vcs[u].push_back(j);
            }
        }
        if(i > 0)  //not the source
        {
            //find all parents in qd
            for(int j = 0; j < this->qd.vertices[u].in.size(); ++j)
            {
                int u2 = this->qd.vertices[u].in[j].vid;
                for(int k = 0; k < this->vcs[u2].size(); ++k)
                {
                    int v2 = this->vcs[u2][k];
                    vector<int> tmpv;
                    //NOTICE: we must ensure the candidate set is ordered!
                    int toAdd = INT_MAX;
            if(gsh->vertices[v2].size()>1 && this->dparr[u*hdsize+v2] && gsh->vType[v2] == 1)
            {
                //tmpv.push_back(v2);
                toAdd = v2;
            }
            //NOTICE: we do not directly enumerate all data vertices (then check dparr state) for every father v2, because it may be time-consuming.  (There are many v2)
                    for(int p = 0; p < this->gsh->graList[v2].size(); ++p)
                    {
                        labelVlist& lv = gsh->graList[v2][p];
                        if(lv.label != qlabel) continue;
                        for(int q = 0; q < lv.vlist.size(); ++q)
                        {
                            int vc = lv.vlist[q];
                            if(this->dparr[u*hdsize+vc])
                            {
                                if(toAdd < vc)
                                {
                                    tmpv.push_back(toAdd);
                                    toAdd = INT_MAX;
                                }
                                tmpv.push_back(vc);
                            }
                        }
                    }
                    if(toAdd < INT_MAX)
                    {
                        tmpv.push_back(toAdd);
                    }
                    if(tmpv.size() > 0)
                        this->CS[u][u2][v2] = tmpv;
                }
            }
        }
    }

    //pre-compute wuv weights
    for(int i = 0; i < qsize; ++i)
    {
        wuv.push_back(vector<int>(hdsize, 1));
    }
    for(int i = 0; i < qsize; ++i)
    {
        int u = this->reverse_order[i];
        vector<int> tmpv;
        for(int j = 0; j < qd.vertices[u].out.size(); ++j)
        {
            int u2 = qd.vertices[u].out[j].vid;
            if(qd.vertices[u2].in.size() != 1)
                continue;
            tmpv.push_back(u2);
        }
        if(tmpv.empty())
        {
            //wuv[u] = vector<int>(vcs[u].size(), 1);
            continue;
        }
        for(int j = 0; j < vcs[u].size(); ++j)
        {
            int minv = INT_MAX;
            int v = vcs[u][j];
            for(int k = 0; k < tmpv.size(); ++k)
            {
                int u2 = tmpv[k];
                vector<int>& cans = this->CS[u2][u][v];
                int sum = 0;
                for(int p = 0; p < cans.size(); ++p)
                {
                    int v2 = cans[p];
                    //int idx = lower_bound(vcs[u2].begin(), vcs[u2].end(), v2) - vcs[u2].begin();
                    //sum += wuv[u2][idx];
                    sum += wuv[u2][v2];
                }
                if(sum < minv)
                {
                    minv = sum;
                }
            }
            //wuv[u].push_back(minv);
            wuv[u][v] = minv;
        }
    }

    delete[] this->dparr;
    return;
}

void Match::match(FILE* ofp) {
    //NOTICE: here should be dsize, not hdsize
	if(qsize > dsize)
		return;
	if (nembeddings > bound) return;
    //Util::timeLimit(TIME_LIMIT_SECONDS);
    //below is just for timer test
    //while(1)
    //{
        //sleep(2);
        //cout<<"I am matching!"<<endl;
    //}

    //build a DAG qd from q
    buildDAG();

    //build CS structure via DAG-graph DP
    buildCS();

    //backtracking on CS directly
    int* M = new int[qsize];
    int* F = new int[hdsize];
    memset(M, -1, sizeof(int)*qsize);
    memset(F, -1, sizeof(int)*hdsize);
    //NOTICE: we use F[] for count of hyper vertices here, different from the original DAF Implementation.
    //The reason is that in BoostISO, multiple queries can be mapped to one hyper vertex, thus the BitArray of Failing set may be ancs(u1) | ancs(u2) | ...
    for(int j = 0; j < hdsize; j ++)
        F[j] = gsh->vertices[j].size();

    //ADD: DynamicCL
    //We choose to maintain the entire CS space first, regardless of using DynamicCL strategy.
    //This is a trade-off: maintain all may bring more computation if many candidates can be pruned by DynamicCL; Otherwise, if DynamicCL does not works well, only maintain partial indices, expand and re-explore will bring much duplicate computation.
    //Then, DynamicCL is used on each CM(u), maintain a ump[]
    int source = this->topological_order[0];
#ifdef PIVOT_DYNAMICCL
    vector<int> cans;
    vector<int> ump(gsh->numVertex, -1);
    vector<bool> hasht(gsh->numVertex, false);
    for(int i = 0; i < vcs[source].size(); ++i)
    {
        int id = vcs[source][i];
        hasht[id] = true;
        if(gsh->scf[id].size() == 0)
        {
            cans.push_back(id);
        }
    }
#endif

#ifdef PIVOT_QDN
    //NOTICE: for simplicity and performance, we only use QDN for pivots.
    //ADD: buildDRT for hyper graphs
    gsh->drt = new vector<QDN>[qsize];
    //we need to build DRT for pivots first, then we can remove candidates QD-contained by others.
    BuildDRT(cans, source, gsh, data, query);
    vector<int> new_cand;
    for(int i = 0; i < cans.size(); ++i)
    {
        int id = cans[i];
        if(gsh->drt[source][id].num == 0)
            new_cand.push_back(id);
    }
    cans = new_cand;   new_cand.clear();
#endif

#ifdef PIVOT_DYNAMICCL
    for(int i = 0; i < cans.size(); ++i)
#else
    for(int i = 0; i < vcs[source].size(); ++i)
#endif
    {
#ifdef PIVOT_DYNAMICCL
        int v = cans[i];
#else
        int v = vcs[source][i];
#endif
        //M[source] = v; F[v] = source;
        UpdateState(M, F, source, v);
        //NOTICE: judge true or false, can be done by checking if bv is empty.
        BitArray bv = backtrack(1, M, F, -1, 0, ofp);
        //M[source] = -1; F[v] = -1;
        RestoreState(M, F, source, v);
        bool flag = bv.prune(source);
        bool success = (bv.num == 0);
        bv.release();
        if(flag)
        {
            break;
        }
#ifdef PIVOT_DYNAMICCL
        if(success)
        {
            DynamicCL(cans, v, gsh, ump, source, hasht);   
        }
#endif
//if (nembeddings > bound) return;
    }
    delete[] M;
    delete[] F;
#ifdef PIVOT_QDN
    delete[] gsh->drt;
#endif
    //cout<<"ncalls: "<<num_recursive_call<<endl;
    //cout<<"max_mem: "<<max_mem_usage <<" kB"<<endl;
    release();
}


BitArray Match::backtrack(int num, int* M, int* F, int ur, int pos, FILE* ofp) {
    //num_recursive_call++;
	ncalls++;
	if (nembeddings > bound) return BitArray();
	if(num == this->qsize) {
		//cerr<<"find a mapping here"<<endl;
        int source = this->topological_order[0];
        output(this->data_v_visited, M, this->qsize, ofp, gsh, 0, source);
        //int ret = output(M, this->qsize, ofp, gsh, 0, source);
		//nembeddings += ret;
		return BitArray();
	}

    //select an extendable query vertex u
    int u = -1;
    vector<int> cmu;
    //u = this->topological_order[num];
    //use adaptive path-size order
    if(ur == -1)
    {
        vector<int> degs(qsize, 0);
        for(int i = 0; i < qsize; ++i)
        {
            if(M[i] == -1) continue;
            for(int j = 0; j < this->qd.vertices[i].out.size(); ++j)
            {
                int u2 = this->qd.vertices[i].out[j].vid;
                degs[u2]++;
            }
        }
        int minv = INT_MAX;
        bool exist_internal = false;
        for(int i = 0; i < qsize; ++i)
        {
            if(M[i] != -1 || degs[i] < this->qd.vertices[i].in.size()) continue;
            if(!this->qd.isLeaf(i))
            {
                exist_internal = true;
                break;
            }
        }
        for(int i = 0; i < qsize; ++i)
        {
            if(M[i] != -1 || degs[i] < this->qd.vertices[i].in.size()) continue;
            if(exist_internal && this->qd.isLeaf(i)) continue;
            vector<int> tmp;
            computeCMU(M, F, i, tmp);
            int sum = 0;
            for(int j = 0; j < tmp.size(); ++j)
            {
                sum += wuv[i][tmp[j]];
            }
            if(sum < minv)
            {
                minv = sum;
                ur = i;
            }
        }
        pos = 0;
    }
    u = this->mtlp[ur][pos++];
    if(pos == this->mtlp[ur].size())
        ur = -1;  
    computeCMU(M, F, u, cmu);
    if(cmu.empty())
        return BitArray(ancs[u]);

    //pruning by failing sets
    BitArray fset(qsize);
    bool alrset = false;
    for(int i = 0; i < cmu.size(); ++i)
    {
        int v = cmu[i];
        BitArray bv;
        if(F[v] == 0)   //a conflict case
        //if(!IsJoinable(u, v, F))
        {
            //bv = BitArray(ancs[u], ancs[F[v]]);
            if(!alrset)
            {
                //fset.myunion(bv);
                fset.myunion(ancs[u]);
                for(int j = 0; j < this->qsize; ++j)
                {
                    if(M[j] == v)
                        fset.myunion(ancs[j]);
                }
            }
            //bv.release();
            continue;
        }
        //M[u] = v; F[v] = u;
        UpdateState(M, F, u, v);
        bv = backtrack(num+1, M, F, ur, pos, ofp);
        //M[u] = -1; F[v] = -1;
        RestoreState(M, F, u, v);
        bool flag = bv.prune(u);
        if(flag)
        {
            //alrset = true;
            fset = bv;
            break;
        }
        if(bv.num == 0)
        {
            alrset = true;
            fset.clear();
        }
        else if(!alrset)
        {
            //bv must not be empty here.
            fset.myunion(bv);
        }
        bv.release();
if (nembeddings > bound) return BitArray();
    }
    return fset;
}

void
Match::computeCMU(int* M, int* F, int u, vector<int>& cmu)
{
    //NOTICE: the linking query vertices are exactly recorded in qd.
    for(int i = 0; i < this->qd.vertices[u].in.size(); ++i)
    {
        int u2 = this->qd.vertices[u].in[i].vid;
        int v2 = M[u2];
        vector<int>& cans = this->CS[u][u2][v2];
        if(i == 0)
        {
            cmu = cans;
            //ADD: for hyper graphs
            //for(int j = 0; j < cans.size(); ++j)
            //{
                //int id = cans[j];
                //if(F[id] > 0)
                //{
                    //cmu.push_back(id);
                //}
            //}
        }
        else
        {
            Util::intersect(cmu, cans);
        }
    }
}

bool 
Match::IsJoinable(int u, int v, int* F)
{
    if(F[v] > 0)
        return true;
    vector<int>& tmp = this->gsh->drt[u][v].qdel;
    for(int i = 0; i < tmp.size(); ++i)
    {
        int id = tmp[i];
        if(F[id] > 0)
        {
            //DEBUG: this may bring error!
            //WE must need to enumerate all possible combinations!
            F[v] = F[id];
            F[id] = 0;
            return true;
        }
    }
    return false;
}

void
Match::release()
{
    for(int i = 0; i < ancs.size(); ++i)
    {
        ancs[i].release();
    }
}





