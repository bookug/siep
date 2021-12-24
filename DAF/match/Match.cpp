/*=============================================================================
# Filename: Match.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-12-15 01:38
# Description: 
=============================================================================*/

#include "Match.h"

using namespace std;
//#define DEBUG 1

extern long bound, numofembeddings, ncalls;
extern double max_mem;

Match::Match(Graph* _query, Graph* _data)
{
	this->query = _query;
	this->data = _data;
	this->qsize = _query->vSize();
	this->dsize = _data->vSize();
    //WARN: below will cause memory error, because these two objects are released after this function
    //this->qd = Graph(true);
    //this->reverse_qd = Graph(true);
    this->qd.setDirected(true);
    this->reverse_qd.setDirected(true);
}

Match::~Match()
{
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
    this->dparr = new bool[qsize*dsize];
    memset(dparr, 0, sizeof(bool)*qsize*dsize);
    vector<int> count(qsize, 0);
    //collect initial candidates
    for(int i = 0; i < qsize; ++i)
    {
        int lb = this->query->vertices[i].label;
        int deg = this->query->vertices[i].in.size();
        for(int j = 0; j < dsize; ++j)
        {
            if(this->data->vertices[j].label != lb)
            {
                continue;
            }
            if(this->data->vertices[j].in.size() < deg)
            {
                continue;
            }
            dparr[i*dsize+j] = true;
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
        for(int v = 0; v < dsize; ++v)
        {
            if(this->dparr[u*dsize+v] == false)
            {
                continue;
            }
            bool flag = true;
            for(int j = 0; j < qdt.vertices[u].out.size(); ++j)
            {
                int uc = qdt.vertices[u].out[j].vid;
                bool found = false;
                for(int k = 0; k < (this->data->vertices[v].out.size()); ++k)
                {
                    int vc = this->data->vertices[v].out[k].vid;
                    if(this->dparr[uc*dsize+vc] == true)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    flag = false;
                    break;
                }
            }
            if(!flag)
            {
                this->dparr[u*dsize+v] = false;
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
        for(int j = 0; j < dsize; ++j)
        {
            if(this->dparr[u*dsize+j])
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
                    for(int p = 0; p < this->data->vertices[v2].out.size(); ++p)
                    {
                        int vc = this->data->vertices[v2].out[p].vid;
                        if(this->dparr[u*dsize+vc])
                        {
                            tmpv.push_back(vc);
                        }
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
        wuv.push_back(vector<int>(dsize, 1));
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

void 
Match::match(IO& io)
{
	if(qsize > dsize)
	{
		return;
	}
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
    int* F = new int[dsize];
    memset(M, -1, sizeof(int)*qsize);
    memset(F, -1, sizeof(int)*dsize);
    int source = this->topological_order[0];
    for(int i = 0; i < vcs[source].size(); ++i)
    {
        int v = vcs[source][i];
        M[source] = v; F[v] = source;
        BitArray bv = backtrack(1, M, F, -1, 0, io);
        M[source] = -1; F[v] = -1;
        bool flag = bv.prune(source);
        bv.release();
        if(flag)
        {
            break;
        }
    }
    delete[] M;
    delete[] F;
    release();
    //Util::noTimeLimit();
}


BitArray Match::backtrack(int num, int* M, int* F, int ur, int pos, IO& io) {
    ncalls++;
if (numofembeddings > bound) return BitArray();
	if(num == this->qsize) {
double vsize, rss;
Util::process_mem_usage(vsize, rss);
if (vsize > max_mem) max_mem = vsize;
#ifdef PRINT_RESULT
		io.output(M, this->qsize);
#endif
		numofembeddings++;
		return BitArray();
	}

    //select an extendable query vertex u
    int u = -1;
    vector<int> cmu;
    //u = this->topological_order[num];
    //use adaptive path-size order
    if(ur == -1) {
        vector<int> degs(qsize, 0);
        for(int i = 0; i < qsize; ++i) {
            if(M[i] == -1) continue;
            for(int j = 0; j < this->qd.vertices[i].out.size(); ++j) {
                int u2 = this->qd.vertices[i].out[j].vid;
                degs[u2]++;
            }
        }
        int minv = INT_MAX;
        bool exist_internal = false;
        for(int i = 0; i < qsize; ++i) {
            if(M[i] != -1 || degs[i] < this->qd.vertices[i].in.size()) continue;
            if(!this->qd.isLeaf(i)) {
                exist_internal = true;
                break;
            }
        }
        for(int i = 0; i < qsize; ++i) {
            if(M[i] != -1 || degs[i] < this->qd.vertices[i].in.size()) continue;
            if(exist_internal && this->qd.isLeaf(i)) continue;
            vector<int> tmp;
            computeCMU(M, i, tmp);
            int sum = 0;
            for(int j = 0; j < tmp.size(); ++j)
                sum += wuv[i][tmp[j]];
            if(sum < minv) {
                minv = sum;
                ur = i;
            }
        }
        pos = 0;
    }
    u = this->mtlp[ur][pos++];
    if(pos == this->mtlp[ur].size())
    {
        ur = -1;  
    }
    computeCMU(M, u, cmu);
    if(cmu.empty())
    {
        return BitArray(ancs[u]);
    }

    //pruning by failing sets
    BitArray fset(qsize);
    bool alrset = false;
    for(int i = 0; i < cmu.size(); ++i)
    {
        int v = cmu[i];
        BitArray bv;
        if(F[v] != -1)
        {
            //bv = BitArray(ancs[u], ancs[F[v]]);
            if(!alrset)
            {
                //fset.myunion(bv);
                fset.myunion(ancs[u]);
                fset.myunion(ancs[F[v]]);
            }
            //bv.release();
            continue;
        }
        M[u] = v; F[v] = u;
        bv = backtrack(num+1, M, F, ur, pos, io);
        M[u] = -1; F[v] = -1;
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
if (numofembeddings > bound) return BitArray();
    }
    return fset;
}

void
Match::computeCMU(int* M, int u, vector<int>& cmu)
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
        }
        else
        {
            Util::intersect(cmu, cans);
        }
    }
}

void
Match::release()
{
    for(int i = 0; i < ancs.size(); ++i)
    {
        ancs[i].release();
    }
}





