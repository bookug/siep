/*=============================================================================
# Filename: main.cpp
# Author: bookug 
# Mail: bookug@qq.com
# Last Modified: 2019-11-12 19:30
# Description: Implementation of boosted CECI algorithm
=============================================================================*/

#include "hyper.h"

#include "../control.h"

using namespace std;

//#define CECI_PROFILE     //detailed profiling information
#define HYPER_HOMOMORPHISM
//#define DYNAMIC_CL
//#define PIVOT_DYNAMICCL  //only use dynamicCL for the first pivots of candidate regions
//#define PIVOT_QDN

long num_recursive_call = 0;
long numofembeddings = 0;
long bound = 100000; // jiangyan add:
long TIME_LIMIT_SECONDS = 600;    //10 min
double max_mem_usage = 0.0;

class Elem
{
public:
	int v;
	double value;
};

struct cmpQ
{
	bool operator()(const Elem &a, const Elem &b)
	{
		if(b.value - a.value > 1e-6)
			return true;
		else if(a.value - b.value > 1e-6)
			return false;
		else if(a.v < b.v)
			return true;
		else if(a.v > b.v)
			return false;
		else return false;
	}
};

int cmpE(const void *a, const void *b)
{
	if(fabs(((Elem *)a)->value - ((Elem *)b)->value) > 1e-6)
	{
		if(((Elem *)a)->value - ((Elem *)b)->value > 1e-6)
		{
			return 1;
		}
		else return -1;
	}
	else return ((Elem *)a)->v - ((Elem *)b)->v;
}

class Neighbor
{
public:
	int uc_prime;
	int pos;
	int NeighborN;
};

int cmp(const void *a, const void *b)
{
	if(((Neighbor *)b)->NeighborN != ((Neighbor *)a)->NeighborN)
	{
		return ((Neighbor *)b)->NeighborN - ((Neighbor *)a)->NeighborN;
	}
	else return ((Neighbor *)a)->uc_prime - ((Neighbor *)b)->uc_prime;
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
    return it-a->begin();
	//int Size = temp->size();
	//for(int i = 0; i < Size; i ++)
	//{
		//if((*temp)[i].label == e)
			//return i;
	//}
	//return -1;
}

//we ensure that V1 has a neighbor V2, V2 has a neighbor V1, and they have the same label.
bool exclusive_equals(Query* q, int V1, int V2)
{
    vector<labelVlist>* v1 = &(q->graList[V1]);
    vector<labelVlist>* v2 = &(q->graList[V2]);
	int Size1 = v1->size();
	int Size2 = v2->size();
	if(Size1 != Size2)
	{
        return false;
    }
    for(int i = 0; i < Size1; i ++)
    {
        int label = (*v1)[i].label;
        int j = contain(label, v2);
        if(j == -1)
            return false;
        
        vector <int> *p1 = &((*v1)[i].vlist);
        vector <int> *p2 = &((*v2)[j].vlist);
        int Size3 = p1->size();
        int Size4 = p2->size();
        if(Size3 != Size4)
        {
            return false;
        }
        for(int k = 0; k < Size3; k ++)
        {
            if((*p1)[k] == V2)
            {
                continue;
            }
            if(contain((*p1)[k], p2) == -1)
                return false;
        }
    }
    return true;
}

bool equals(vector <labelVlist> *v1, vector <labelVlist> *v2)
{
	int Size1 = v1->size();
	int Size2 = v2->size();
	if(Size1 == Size2)
	{
		for(int i = 0; i < Size1; i ++)
		{
			int label = (*v1)[i].label;
			int j = contain(label, v2);
			if(j == -1)
				return false;
			
			vector <int> *p1 = &((*v1)[i].vlist);
			vector <int> *p2 = &((*v2)[j].vlist);
			int Size3 = p1->size();
			int Size4 = p2->size();
			if(Size3 == Size4)
			{
				for(int k = 0; k < Size3; k ++)
				{
					if(contain((*p1)[k], p2) == -1)
						return false;
				}
			}
			else return false;
		}
		return true;
	}
	else return false;
}

void computeQDN(Graph* g, Query* q, int u, int v, vector<int>& adjs)
{
    vector<labelVlist>& uadj = q->graList[u];
    for(int i = 0; i < uadj.size(); ++i)
    {
        int label = uadj[i].label;
        int pos = contain(label, &(g->graList[v]));
        if(pos == -1)
        {
            continue;
        }
        for(int j = 0; j < g->graList[v][pos].vlist.size(); ++j)
        {
            adjs.push_back(g->graList[v][pos].vlist[j]);
        }
    }
}

bool  isQDC(Graph* g, Query* q, int u, int v, int v2)
{
    if(g->vList[v] != g->vList[v2])
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

bool isQDE(Graph* g, Query* q, int u, int v, int v2)
{
    return isQDC(g, q, u, v, v2) && isQDC(g, q, u, v2, v);
}

//build relation table for each query node,   QDN (query dependent neighbor): QDC and QDE
//NOTICE: the candidates C(u) should be filtered by NLF filter, and only contain vertices whose SC graph in-degree is 0.
//QUERY: in TurboISO, we should use NEC tree/graph instead?   (I suppose it is the same.)
//Note that we do not index hypernodes which are listed in the QDE-List of another indexed hypernode.   
void BuildDRT(vector<int>& C, int u, HyperGraph* gsh, Graph* g, Query* q)
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


int ChooseStartQVertex(Query *q, HyperGraph *g)
{
	int K = 3;
	int *u = new int[K];
	int *candN = new int[K];
	memset(candN, 0, K * sizeint);

	priority_queue <Elem, vector <Elem>, cmpQ> pq;
	for(int i = 0; i < q->numVertex; i ++)
	{
		Elem temp;
		temp.v = i;
		temp.value = 0.0;
		int Size = q->graList[i].size();
		for(int j = 0; j < Size; j ++)
			temp.value += q->graList[i][j].vlist.size();
		temp.value = g->labelList[q->vList[i]].size() / temp.value;
		pq.push(temp);
		if(pq.size() > K)
			pq.pop();
	}

	int I = K;
	while(I --)
	{
		Elem temp = pq.top();
		pq.pop();
		u[I] = temp.v;
	}

	for(int i = 0; i < K; i ++)
	{
		int label = q->vList[u[i]];
		int Size = g->labelList[label].size();
		for(int j = 0; j < Size; j ++)
		{
			int v = g->labelList[label][j];
			int Size2 = q->graList[u[i]].size();
			bool Flag = true;
			for(int k = 0; k < Size2; k ++) //对u的邻居的每个label
			{
				int Label = q->graList[u[i]][k].label;
				int curN = q->graList[u[i]][k].vlist.size();
				int Size3 = g->graList[v].size();
				bool flag = false;
				for(int l = 0; l < Size3; l ++)
				{
					if(g->graList[v][l].label == Label)
					{
						if(curN <= g->graList[v][l].vlist.size())
							flag = true;
						break;
					}
				}
				if(!flag)
				{
					Flag = false;
					break;
				}
			}
			if(Flag)
				candN[i] ++;
		}
	}

	int Min = g->numVertex + 1;
	int curU = -1;
	for(int i = 0; i < K; i ++)
	{
		if(candN[i] < Min)
		{
			Min = candN[i];
			curU = i;
		}
	}
	int res = u[curU];

	if(u != NULL)
		delete []u;
	if(candN != NULL)
		delete []candN;
	return res;
}

void FindNEC(vector <vector <int> > *NECV, vector <int> *vertexlist, Query *q)
{
	int Size = vertexlist->size();
	if(Size == 1)
	{
		(*NECV).push_back(*vertexlist);
        return;
	}

    //NOTICE: the first class, there does not exist edges between equivalent query nodes.
    int *p = new int[Size];
    p[0] = 0;
    //union set
    for(int i = 1; i < Size; i ++)
    {
        int flag = i;
        for(int j = 0; j < i; j ++)
        {
            if(equals(&(q->graList[(*vertexlist)[i]]), &(q->graList[(*vertexlist)[j]])))
            {
                flag = j;
                break;
            }
        }
        //form a linked circle
        p[i] = p[flag];
        p[flag] = i;
    }
    for(int i = 0; i < Size; i ++)
    {
        if(p[i] > i)
        {
            vector <int> vlist;
            vlist.push_back((*vertexlist)[i]);
            int Pos = p[i];
            while(Pos != i)
            {
                vlist.push_back((*vertexlist)[Pos]);
                Pos = p[Pos];
            }
            (*NECV).push_back(vlist);
        }
    }
    for(int i = 0, j = 0; i < Size; i ++)
    {
        //already in a NEC node
        if(p[i] != i)
        {
            vector <int>::iterator Iter = vertexlist->begin() + j;
            //this is slow for vector, but this function is only called once.
            (*vertexlist).erase(Iter);
        }
        else
        {
            j ++;
        }
    }
    if(p != NULL)
        delete []p;

    Size = vertexlist->size();
    if(Size == 1)
    {
        (*NECV).push_back(*vertexlist);
        return;
    }

    //NOTICE: the second class, there are edges between equivalent query nodes.
    //These NECs can not be found in the above process, e.g., v1-v2, they have different neighbors.
    int label = q->vList[(*vertexlist)[0]];
    //all vertices in this group has the same label
    for(int i = 0; i < vertexlist->size(); ++i)
    {
        if((*vertexlist)[i] == -1)
        {
            continue;
        }
        vector <int> a;
        vector <int> b;
        int Vi = (*vertexlist)[i];
        a.push_back(Vi);
        b.push_back(i);
        int pos2 = contain(label, &(q->graList[Vi]));
        if(pos2 == -1) //Vi has no neighbor in this group
        {
            (*NECV).push_back(a);
            (*vertexlist)[i] = -1;
            //vector <int>::iterator Iter = vertexlist->begin() + i;
            //(*vertexlist).erase(Iter);
            continue;
        }
        for(int j = i + 1; j < vertexlist->size(); j ++)
        {
            int Vj = (*vertexlist)[j];
            if(Vj == -1)
            {
                continue;
            }
            int pos3 = contain(label, &(q->graList[Vj]));
            if(pos3 == -1)
                continue;
            int sizea = a.size();
            bool fmm = true;
            for(int mm = 0; mm < sizea; mm ++)
            {
                int posmm = contain(a[mm], &(q->graList[Vj][pos3].vlist));
                if(posmm == -1)
                {
                    fmm = false;
                    break;
                }
            }
            if(fmm)
            {
                //NOTICE: we just need to check one pair when adding a new one, because we already ensure that these nodes form a clique.
                bool success = exclusive_equals(q, Vi, Vj);
                if(success)
                {
                    a.push_back(Vj);
                    b.push_back(j);
                }
            }
        }
        //find the maximal one
        (*NECV).push_back(a);
        int sizea = a.size();
        for(int nn = sizea - 1; nn >= 0; nn --)
        {
            (*vertexlist)[b[nn]] = -1;
            //vector <int>::iterator Iter = vertexlist->begin() + b[nn];
            //(*vertexlist).erase(Iter);
        }
    }
}

void RewriteToNECTree(Query *q, int us, NECTree *q_prime)
{
	bool *flag = new bool[q->numVertex];
	memset(flag, false, sizebool * q->numVertex);

	//create a root NEC vertex u'_s
	q_prime->numVertex ++;
	q_prime->vList.push_back(q->vList[us]);
	vector <int> root;
	root.push_back(us);
    //the root NEC always contains a single vertex
	q_prime->NEC.push_back(root);
	q_prime->parent.push_back(-1);
	Child child;
	child.s = -1;
	child.e = -1;
	q_prime->child.push_back(child);
	flag[us] = true;

    //breadth-first search (BFS)
	int currentS = -1;
	int currentE = -1;
	int nextS = 0;
	int nextE = 0;
	int childS = 0;
	int childE = 0;
	while(nextE >= nextS)
	{
		currentS = nextS;
		currentE = nextE;
		nextS = currentE + 1;
		nextE = currentE;
		childS = currentE + 1;
		childE = currentE;
        //i is the NEC node ID
		for(int i = currentS; i <= currentE; i ++)
		{
			vector <labelVlist> C;
			int Size2 = q_prime->NEC[i].size();
			for(int j = 0; j < Size2; j ++)
			{
				int v = q_prime->NEC[i][j];
				int Size3 = q->graList[v].size();
				for(int k = 0; k < Size3; k ++)
				{
					labelVlist *temp = (&q->graList[v][k]);
					int Size4 = temp->vlist.size();
					int ll;
					for(ll = 0; ll < Size4; ll ++)
					{
						if(!flag[temp->vlist[ll]])
							break;
					}
					if(ll >= Size4)
						continue;
					int pos = contain(temp->label, &C);
					if(pos == -1)
					{
						labelVlist vlist;
						vlist.label = temp->label;
						vlist.vlist.push_back(temp->vlist[ll]);
						flag[temp->vlist[ll]] = true;
						ll ++;
						for(; ll < Size4; ll ++)
						{
							if(!flag[temp->vlist[ll]])
							{
								flag[temp->vlist[ll]] = true;
								vlist.vlist.push_back(temp->vlist[ll]);
							}
						}
						C.push_back(vlist);
					}
					else
					{
						int pos2 = contain(temp->vlist[ll], &(C[pos].vlist));
						if(pos2 == -1)
						{
							flag[temp->vlist[ll]] = true;
							C[pos].vlist.push_back(temp->vlist[ll]);
						}
						ll ++;
						for(; ll < Size4; ll ++)
						{
							if(!flag[temp->vlist[ll]])
							{
								int pos3 = contain(temp->vlist[ll], &(C[pos].vlist));
								if(pos3 == -1)
								{
									flag[temp->vlist[ll]] = true;
									C[pos].vlist.push_back(temp->vlist[ll]);
								}
							}
						}
					}
				}
			}
			if(!C.empty())
			{
				int Size3 = C.size();
				for(int j = 0; j < Size3; j ++)
				{
					vector < vector <int> > NECV;
					FindNEC(&NECV, &(C[j].vlist), q);

					int Size4 = NECV.size();
					nextE += Size4;
					childE += Size4;
					q_prime->numVertex += Size4;
					if(q_prime->child[i].s == -1)
					{
						q_prime->child[i].s = childS;
					}
					q_prime->child[i].e = childE;
					for(int k = 0; k < Size4; k ++)
					{
						q_prime->vList.push_back(C[j].label);
						q_prime->NEC.push_back(NECV[k]);
						q_prime->parent.push_back(i);
						q_prime->child.push_back(child);
					}
				}
				childS = childE + 1;
			}
		}
	}
	if(flag != NULL)
		delete []flag;
}


double C(int n, int m)
{
	if(n < m)
		return 0.0;
	else if(n == m)
		return 1.0;
	else
	{
		double res = 1.0;
		for(int i = 0; i < m; i ++)
		{
			res *= (double)(n - i) / (m - i);
		}
		return res;
	}
}

void UpdateState(int *M, int *F, vector <int> *qV, vector <int> *gV)
{
	for(int i = 0; i < qV->size(); i ++)
	{
		M[(*qV)[i]] = (*gV)[i];
		F[(*gV)[i]] --;
	}
}

void RestoreState(int *M, int *F, vector <int> *qV, vector <int> *gV)
{
	for(int i = 0; i < qV->size(); i ++)
	{
		M[(*qV)[i]] = -1;
		F[(*gV)[i]] ++;
	}
}

//pre-aassign Size spaces for these vectors, then enough and safe.
//NOTICE: the length of rank may vary due to different num of h.
//Below can be simply implemented by a recursive function, or use a stack to implement it.
//Here we use a special way, to avoid too deep recursion.
void NextComb(vector <int> *C, int Size, vector <int> *rank, vector <int> *value)
{
	if((*rank)[0] == -1)
	{
		(*rank)[0] = 0;
		(*value)[0] = (*C)[0];
		for(int i = 1; i < Size; i ++)
		{
			(*rank)[i] = i;
			(*value)[i] = (*C)[i];
		}
        return;
	}
    int obj = C->size() - 1;
    int Pos = -1;
    for(int pos = Size - 1; pos >= 0; pos --, obj --)
    {
        if((*rank)[pos] != obj)
        {
            Pos = pos;
            break;
        }
    }
    if(Pos == -1)
    {
        (*rank)[0] = -1;
        return;
    }
    else
    {
        (*rank)[Pos] ++;

        //cout<<"check C: "<<C->size()<<" "<<(*rank)[Pos]<<endl;
        //WARN+DEBUG: here some deadly errors may happen because (*rank)[Pos] is out of the bound of (*C)
        //The judgement below is used to avoid the segment fault here, but it can not make the answers totally correct
        if((*rank)[Pos] >= C->size())
        {
            (*rank)[0] = -1;
            return; 
        }

        //ADD: to remove duplicates when processing hyper graphs
        int oldval = (*value)[Pos], newval = (*C)[(*rank)[Pos]];
        (*value)[Pos] = (*C)[(*rank)[Pos]];
        for(int pos = Pos + 1; pos < Size; pos ++)
        {
            (*rank)[pos] = (*rank)[pos - 1] + 1;
            (*value)[pos] = (*C)[(*rank)[pos]];
        }
        if(oldval == newval)
        {
            NextComb(C, Size, rank, value);
            return;
        }
    }
}

bool IsJoinable(Query *q, Graph *g, int *M, int qV, int gV)
{
	int Size = q->graList[qV].size();
	for(int i = 0; i < Size; i ++)
	{
		int label = q->graList[qV][i].label;
		int Size2 = q->graList[qV][i].vlist.size();
		for(int j = 0; j < Size2; j ++)
		{
			int u = q->graList[qV][i].vlist[j];
			int v = M[u];
			if(v != -1)
			{
				int pos = contain(label, &(g->graList[gV]));
				if(pos == -1)
					return false;
				if(contain(v, &(g->graList[gV][pos].vlist)) == -1)
					return false;
			}
		}
	}
	return true;
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

bool NextPerm(int *M, vector <int> *qV, int *rank)
{
	int Size = qV->size();
	if(rank[0] == -1)
	{
		for(int i = 0; i < Size; i ++)
			rank[i] = i;
		return false;
	}
	else
	{
		if(Size == 1)
			return true;
		int ii = -1;
		for(int i = Size - 2; i >= 0; i --)
		{
			if(rank[i] < rank[i + 1])
			{
				ii = i;
				break;
			}
		}
		if(ii == -1)
			return true;

		int iii;
		for(int i = Size - 1; i > ii; i --)
		{
			if(rank[i] > rank[ii])
			{
				iii = i;
				break;
			}
		}

		int temp = rank[ii];
		rank[ii] = rank[iii];
		rank[iii] = temp;

        //ADD: to remove duplicates when processing hyper graphs, we need a judge here.
        int oldval = M[(*qV)[rank[ii]]], newval = M[(*qV)[rank[iii]]];
		M[(*qV)[rank[ii]]] = newval;
		M[(*qV)[rank[iii]]] = oldval;

		int E = (ii + Size - 1) / 2;
		for(int i = ii + 1; i <= E; i ++)
		{
			int j = ii + Size - i;

			temp = rank[i];
			rank[i] = rank[j];
			rank[j] = temp;

			temp = M[(*qV)[rank[i]]];
			M[(*qV)[rank[i]]] = M[(*qV)[rank[j]]];
			M[(*qV)[rank[j]]] = temp;
		}

        if(oldval == newval)
        {
            return NextPerm(M, qV, rank);
        }

		return false;
	}
}

bool verify(int* M, Query* q, HyperGraph* gsh)
{
    //cout<<"find an asnwer!"<<endl;
    return true;
}

//output a result after verified
void output(vector<bool>& data_v_visited, int* M, int qVNum, FILE* fpR, HyperGraph* gsh, int pos, int us)
{
if (numofembeddings > bound) return;	
    if(pos == qVNum)
    {
double vsize, rss;
Util::process_mem_usage(vsize, rss);
if(vsize > max_mem_usage) max_mem_usage = vsize;
        //NOTICE: we should output the whole matching in the end.
#ifdef PRINT_RESULT
		for(int i = 0; i < qVNum; ++i)
            fprintf(fpR, "(%d, %d) ", i, M[i]);
        fprintf(fpR, "\n");
#endif
        numofembeddings++; 	// jiangyan add
		//double vsize, rss;
		//process_mem_usage(vsize, rss);
		//if (vsize > max_mem)
		//	max_mem = vsize;
        return;
    }
    if(pos >= qVNum)
    {
        cout<<"ERROR: "<<pos<<" "<<qVNum<<endl;
    }
    int hv = M[pos];
    vector<int> rv = gsh->vertices[hv];
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
        output(data_v_visited, M, qVNum, fpR, gsh, pos+1, us);
        data_v_visited[rv[i]] = false;
if (numofembeddings > bound) return;	
    }
    M[pos] = hv;
	//for(int i = 0; i < qVNum; ++i) {
		//fprintf(fpR, "(%d, %d) ", i, M[i]);
	//}
	//fprintf(fpR, "\n");
}

void GenPerm(vector<bool>& data_v_visited, int *M, NECTree *q_prime, int i, FILE *fpR, Query* q, HyperGraph* gsh, int us) 
{
	int qVNum = q->numVertex;
	if (numofembeddings > bound) return;	
	if(i == q_prime->numVertex) {
		if(verify(M, q, gsh)){
			//cout<<"found a valid answer"<<endl;
			output(data_v_visited, M, qVNum, fpR, gsh, 0, us);
		}
		//char buffer[1000];
		//buffer[0] = '\0';
		//char num[100];
		//for(int i = 0; i < qVNum; i ++)
		//{
			//itoa(M[i], num, 10);
			//strcat(buffer, num);
			//strcat(buffer, " ");
		//}
		//int len = strlen(buffer);
		//buffer[len - 1] = '\n';
		////printf("%s", buffer);
		//fputs(buffer, fpR);
		return;
	}
	int Size = q_prime->NEC[i].size();
	if(Size == 1) {
		GenPerm(data_v_visited, M, q_prime, i + 1, fpR, q, gsh, us);
	}
	else {
		int *rank = new int[Size];
		rank[0] = -1;
		while(!NextPerm(M, &(q_prime->NEC[i]), rank)){
			if (numofembeddings > bound) return;	
			GenPerm(data_v_visited, M, q_prime, i + 1, fpR, q, gsh, us);
		}
	}
}

//qv is a real query vertex
bool filter(Query* q, HyperGraph* gsh, Graph* g, int qv, int hv)
{
    int gv = gsh->vertices[hv][0];
    if(contain(q->vList[qv], &(g->vList[gv])) == -1) //check labels
    {
        return false;
    }
    //check degrees
    int Size1 = q->graList[qv].size();
    int Size2 = g->graList[gv].size();
    if(Size1 > Size2)
    {
        return false;
    }
    //check label-grouped neighbor num
    for(int j = 0; j < Size1; j ++)
    {
        int label = q->graList[qv][j].label;
        int pos = contain(label, &(g->graList[gv]));
        if(pos == -1)
        {
            return false;
        }
        if(q->graList[qv][j].vlist.size() > g->graList[gv][pos].vlist.size())
        {
            return false;
        }
    }
    return true;
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

void removeFirst(ECPT& um, int v)
{
    um.erase(v);
}

void removeSecond(ECPT& um, int v)
{
    unordered_map<int, vector<int> >::iterator it;
    for(it = um.begin(); it != um.end(); )
    {
        vector<int>& cand = it->second;
        int count = cand.size();
        for(int j = 0; j < cand.size(); ++j)
        {
            if(cand[j] == v)
            {
                cand[j] = -1;
                //count--;

                //there is no multiple edges
                //for(int k = j+1; k < cand.size(); ++k)
                //{
                    //cand[k-1] = cand[k];
                //}
                //cand.resize(count);
                break;
            }
        }
        //if(count == 0)
        //{
            //it = um.erase(it);
        //}
        //else
        //{
            //++it;
        //}
        ++it;
    }
}

bool clear(vector<ECPT>& tec)
{
    ECPT::iterator mit;
    for(int i = 0; i < tec.size(); ++i)
    {
        for(mit = tec[i].begin(); mit != tec[i].end(); )
        {
            compress(mit->second);
            if(mit->second.empty())
            {
                mit = tec[i].erase(mit);
                //BETTER: to remove it->second from the parent node (removeSecond)
            }
            else
            {
                ++mit;
            }
        }
        if(tec[i].empty())
        {
            return false;
        }
    }
    return true;
}

bool clear2(CECItree& ctr)
{
    bool success = clear(ctr.TEC);
    if(!success)
    {
        return false;
    }
    for(int i = 0; i < ctr.NTEC.size(); ++i)
    {
        success = clear(ctr.NTEC[i]);
        if(!success)
        {
            return false;
        }
    }
    return true;
}

//Find complete candidate sets for both tree edges and non-tree edges in Q
bool BFSFilter(Query* q, HyperGraph* gsh, Graph* g, NECTree& q_prime, int* qv2nec, DVEC& ntes, DVEC& ntes_child, CECItree& ctr)
{
    ECPT::iterator mit;
    //tree edge candidates
    for(int i = 1; i < q_prime.numVertex; ++i)  // not the root node
    {
        int qv = q_prime.NEC[i][0];
        int pid = q_prime.parent[i];
        ctr.TEC.push_back(ECPT());
        vector<int> frontier;
        vector<bool> dupr(gsh->numVertex, false);
        //union all candidates to generate frontier
        for(mit = ctr.TEC[pid].begin(); mit != ctr.TEC[pid].end(); ++mit)
        {
            vector<int>& cand = mit->second;
            for(int j = 0; j < cand.size(); ++j)
            {
                if(cand[j] != -1)
                    frontier.push_back(cand[j]);
            }
        }
        for(int j = 0; j < frontier.size(); ++j)
        {
            int vf = frontier[j];
            if(dupr[vf])
            {
                continue;
            }
            dupr[vf] = true;
            ctr.TEC[i][vf] = vector<int>();
            int qlabel = q_prime.vList[i];
            //ADD: a hyper vertex can exist both in parent and child
            if(gsh->vertices[vf].size()>1 && qlabel == gsh->vList[vf] && gsh->vType[vf] == 1)
            {
                ctr.TEC[i][vf].push_back(vf);
            }
            int l = contain(qlabel, &(gsh->graList[vf]));
            if(l != -1)
            {
                for(int k = 0; k < gsh->graList[vf][l].vlist.size(); ++k)
                {
                    int v = gsh->graList[vf][l].vlist[k];
                    if(!filter(q, gsh, g, qv, v))
                    {
                        continue;
                    }
                    ctr.TEC[i][vf].push_back(v);
                }
            }
            //cout<<"check: "<<ctr.TEC[i][vf][0]<<endl;
            //assert(!ctr.TEC[i][vf].empty());
            //BETTER?: the sum of real vertex num should >= NEC size
            if(ctr.TEC[i][vf].empty())
            {
                //remove vf from TEC[pid]
                removeSecond(ctr.TEC[pid], vf);
                for(int z = q_prime.child[pid].s; z != -1 && z <= q_prime.child[pid].e && z <= i; ++z)
                {
                    removeFirst(ctr.TEC[z], vf);
                }
            }
        }
        bool success = clear(ctr.TEC);
        if(!success)
        {
            return false;
        }
    }

    //non-tree edge candidates
    ctr.NTEC.push_back(vector< ECPT >());
    ctr.NTEC_parent.push_back(vector<int>());
    for(int i = 1; i < q_prime.numVertex; ++i) // not the root node
    {
        ctr.NTEC.push_back(vector< ECPT >());
        ctr.NTEC_parent.push_back(vector<int>());
        int qv = q_prime.NEC[i][0];
        for(int j = 0; j < ntes[i].size(); ++j)
        {
            int up = ntes[i][j];
            ctr.NTEC_parent[i].push_back(up);
            ctr.NTEC[i].push_back(ECPT());
            vector<int> frontier;
            vector<bool> dupr(gsh->numVertex, false);
            //union all candidates to generate frontier
            for(mit = ctr.TEC[up].begin(); mit != ctr.TEC[up].end(); ++mit)
            {
                vector<int>& cand = mit->second;
                for(int k = 0; k < cand.size(); ++k)
                {
                    if(cand[k] != -1)
                        frontier.push_back(cand[k]);
                }
            }
            //BETTER: use intersection rather than union for these frontiers?
            //for(int k = 0; k < ctr.NTEC[up].size(); ++k)
            //{
                //for(mit = ctr.NTEC[up][k].begin(); mit != ctr.NTEC[up][k].end(); ++mit)
                //{
                    //vector<int>& cand = mit->second;
                    //for(int j = 0; j < cand.size(); ++j)
                    //{
                        //if(cand[j] != -1)
                            //frontier.push_back(cand[j]);
                    //}
                //}
            //}
            for(int e = 0; e < frontier.size(); ++e)
            {
                int vf = frontier[e];
                if(dupr[vf])
                {
                    continue;
                }
                dupr[vf] = true;
                ctr.NTEC[i][j][vf] = vector<int>();
                int qlabel = q_prime.vList[i];
            //ADD: a hyper vertex can exist both in parent and child
                if(gsh->vertices[vf].size()>1 && qlabel == gsh->vList[vf] && gsh->vType[vf] == 1)
                {
                    ctr.NTEC[i][j][vf].push_back(vf);
                }
                int l = contain(qlabel, &(gsh->graList[vf]));
                if(l != -1)
                {
                    for(int k = 0; k < gsh->graList[vf][l].vlist.size(); ++k)
                    {
                        int v = gsh->graList[vf][l].vlist[k];
                        if(!filter(q, gsh, g, qv, v))
                        {
                            continue;
                        }
                        ctr.NTEC[i][j][vf].push_back(v);
                    }
                }
                if(ctr.NTEC[i][j][vf].empty())
                {
                    removeFirst(ctr.NTEC[i][j], vf);
                    removeSecond(ctr.TEC[up], vf);
                    for(int k = 0; k < ctr.NTEC[up].size(); ++k)
                    {
                        removeSecond(ctr.NTEC[up][k], vf);
                    }
                    //BETTER: remove in TEC children and NTEC children
                    //for(int z = 0; z < ntes_child.size(); ++z)
                    //{
                        //if(ntes_child[up][z] > i)
                        //{
                            //continue;
                        //}
                        //removeFirst(ctr.TEC[z], vf);
                    //}
                }
            }
        }
    bool success = clear2(ctr);
    if(!success)
    {
        return false;
    }
    }
    return true;
}

bool FindInNTEC(vector<ECPT>& nec, int dv)
{
    ECPT::iterator mit;
    for(int i = 0; i < nec.size(); ++i)
    {
        ECPT& tmp = nec[i];
        //we must find dv in each nec[i]
        bool flag = false;
        for(mit = tmp.begin(); mit != tmp.end(); ++mit)
        {
            vector<int>& cand = mit->second;
            vector<int>::iterator vit = find(cand.begin(), cand.end(), dv);
            if(vit != cand.end())
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

//refine both tree edge candidates and non-tree edge candidates
//In fact, in this function, gsh and g are not used.
bool ReverseBFSRefine(Query* q, HyperGraph* gsh, Graph* g, NECTree& q_prime, int* qv2nec, DVEC& ntes, DVEC& ntes_child, CECItree& ctr)
{
    ECPT::iterator mit;
    //the root node has no NTEC (the vector is empty)
    for(int i = q_prime.numVertex-1; i > 0; --i)
    {
        int score = 0;
        for(mit = ctr.TEC[i].begin(); mit != ctr.TEC[i].end(); ++mit)
        {
            vector<int>& cand = mit->second;
            for(int j = 0; j < cand.size(); ++j)
            {
                int dv = cand[j];
                bool flag = FindInNTEC(ctr.NTEC[i], dv);
                if(flag)
                {
                    score++;
                    continue;
                }
                cand[j] = -1;
                //tree-edge children
                for(int k = q_prime.child[i].s; k != -1 && k <= q_prime.child[i].e; ++k)
                {
                    removeFirst(ctr.TEC[k], dv);
                }
                //non-tree edge children
                for(int k = 0; k < ntes_child[i].size(); ++k)
                {
                    int uc = ntes_child[i][k];
                    for(int z = 0; z < ctr.NTEC[uc].size(); ++z)
                    {
                        if(ctr.NTEC_parent[uc][z] == i)
                        {
                            removeFirst(ctr.NTEC[uc][z], dv);
                            break;
                        }
                    }
                }
            }
        }
        if(score == 0)  // this node has no candidates
        {
            return false;
        }
    }
    bool success = clear(ctr.TEC);
    return success;
}

//for QDE we must take care of the num of F[]
//in output()
//in BuildDRT(), the qde vertices should also be removed from C(u), how about the links in I?  (better to initialize C(u) first, refine by BuildDRT, finally build I)
//
//NOTICE: pivot alway has a single real vertex for NEC.
//We add a final check for the num of qde vertices when outputing results.
bool SubgraphSearch(vector<bool>& data_v_visited, Query* q, HyperGraph* gsh, NECTree& q_prime, CECItree& ctr, int uprime, int* M, int* F, FILE* fpR, DVEC& ntes, int us)
{
    if (numofembeddings > bound) return false;	// jiangyan add
    num_recursive_call++;
    //below is invalid, C++ reference is different from pointer. 
    //It must be initialized, and can not be changed. (the value that this reference links can be changed, but not the reference itself)
    //int& xxx;
    //xxx = uprime;
    int puprime = q_prime.parent[uprime];
	vector <int> C, C1;
    int csize;
    vector<int> links = ntes[uprime];
    links.push_back(puprime);
    for(int k = 0; k < links.size(); ++k)
    {
        int u = links[k];
        int Size = q_prime.NEC[u].size();
    //NOTICE: candidates may also contain the hyper vertices of parent, if size>1, is clique and label is the same.
        ECPT* tmpm = &(ctr.TEC[uprime]);
        if(k < links.size()-1)
        {
            tmpm = &(ctr.NTEC[uprime][k]);
        }
        for(int i = 0; i < Size; i ++)
        {
            int fv = M[q_prime.NEC[u][i]];
            ECPT::iterator mit = tmpm->find(fv);
            if(mit == tmpm->end())
            {
                return false;
            }
            vector<int>* t = &((*tmpm)[fv]);
            if(k == 0 && i == 0)
            {
                csize = 0;
                //NOTICE: we ensure that zero h is not included here.
                for(int j = 0; j < t->size(); ++j)
                {
                    int id = (*t)[j];
                    if(F[id] > 0)
                    {
                        csize++;
                        C1.push_back(id);
                    }
                }
                //C1 = *t;
                //csize = C1.size();
            }
            else
            {
                for(int j = 0; j < C1.size(); ++j)
                {
                    if(C1[j] == -1)
                    {
                        continue;
                    }
                    if(contain(C1[j], t) == -1)
                    {
                        csize--;
                        C1[j] = -1;
                        continue;
                    }
                }
            }
            if(csize == 0)
            //NOTICE: below is hyper vertex
            //if(csize < q_prime.NEC[uprime].size())    
            {
                return false;
            }
        }
    }
    int hsize = 0;
	//NOTICE: this judgement is needed, otherwise bug may exist in some cases
    for(int i = 0; i < C1.size(); ++i)
    {
        if(C1[i] == -1)
        {
            continue;
        }
        hsize += F[C1[i]];
        for(int j = 0; j < F[C1[i]]; ++j)
        {
            C.push_back(C1[i]);
        }
    }
    if(hsize < q_prime.NEC[uprime].size())    
    {
        return false;
    }

    int Size = q_prime.NEC[uprime].size();
	vector <int> Srank(Size, -1);
	vector <int> value(Size, -1);						//C'
	//vector <int> valcnt(Size, 0);       //used num of each h
    vector <int> valcnt(Size, 1);       //used num of each h
    bool flag = false;
	while(true)
	{
        //NOTICE: we can not extend all h by its num directly in value[], otherwise many duplicate combinations will be produced.
        //ADD: we can extend first, then remove duplicates in comb.
		NextComb(&C, Size, &Srank, &value);
		if(Srank[0] == -1)
		{
			break;
		}
		bool Continue = false;

//NOTICE: we allow homomorphism on hyper graph
#ifndef HYPER_HOMOMORPHISM
		for(int i = 0; i < Size; i ++)
		{
            //we must ensure that no vertex in combination has been visited before.
			if(F[value[i]])
			{
				Continue = true;
				break;
			}
		}
		if(Continue)
			continue;
#endif

		if(Size > 1)
		{
            //NOTICE: especially handle the case that a clique compressed into a NEC node.
            //gsh must be reused here! Data vertices must also form a clique.
			int Label = q_prime.vList[uprime];
			vector <labelVlist> *p = &(q->graList[q_prime.NEC[uprime][0]]);
			int pos = contain(Label, p);
			if(pos != -1)
			{
				if(contain(q_prime.NEC[uprime][1], &((*p)[pos].vlist)) != -1)
				{
					for(int i = 0; i < Size; i ++)
					{
						vector <labelVlist> *p = &(gsh->graList[value[i]]);
						int Size2 = p->size();
						int pos = contain(Label, p);
						//if(pos == -1)
						//{
							//Continue = true;
							//break;
						//}
                        bool clique = (gsh->vType[value[i]] == 1);
						for(int j = i + 1; j < Size; j ++)
						{
                            if(value[i] == value[j])   //the same hyper vertex
                            {
                                if(clique)
                                {
                                    continue;
                                }
                                else
                                {
                                    Continue = true;
                                    break;
                                }
                            }
							if(pos == -1 || contain(value[j], &((*p)[pos].vlist)) == -1)
							{
								Continue = true;
								break;
							}
						}
						if(Continue)
							break;
					}
					if(Continue)
						continue;
				}
			}
		}

        //NOTICE: because CECI already prepares candidates of non-tree edges in structures, thus it does not need to perform IsJoinable check here.
		//bool matched = true;
		//for(int i = 0; i < Size; i ++)
		//{
			//if(!IsJoinable(q, g, M, q_prime.NEC[uprime][i], value[i]))
			//{
				//matched = false;
				//break;
			//}
		//}
		//if(!matched)
			//continue;

		UpdateState(M, F, &(q_prime.NEC[uprime]), &value);
        bool ret = false;
		if(q_prime.numVertex == uprime + 1)
        {
			GenPerm(data_v_visited, M, &q_prime, 0, fpR, q, gsh, us);
#ifdef DYNAMIC_CL
            for(int i = 0; i < q_prime.NEC[uprime].size(); ++i) {
                //ADD: for hyper graphs
                //if we want to use DynamicCL, BFSFilter must be modified, only add SC-indegree 0 vertices.
                //BETTER: the first pivots can also be dynamically loaded?
                //Question 1: NEC vertices may map to same h
                //Question 2: Besides, C is variable, but we need to enumerate a combination each time.
                //Question 3: What is more, the ctr structure does not include new added candidates. (CECI is not like TurboISO, only need to re-explore all 1-hop edges, instead of the entire sub-regions.)
                DynamicCL(C, M[q_prime.NEC[uprime][i]], gsh);
            }
#endif
            ret = true;
        }
		else 
        {
            ret = SubgraphSearch(data_v_visited, q, gsh, q_prime, ctr, uprime + 1, M, F, fpR, ntes, us);
#ifdef DYNAMIC_CL
    if(ret && uprime > 0)  //no need to add for the first pivot
    {
        for(int i = 0; i < q_prime.NEC[uprime].size(); ++i)
        {
            DynamicCL(C, M[q_prime.NEC[uprime][i]], gsh);
        }
    }
#endif
        }
        if(ret)
        {
            flag = true;
        }
		RestoreState(M, F, &(q_prime.NEC[uprime]), &value);
	}

    return flag;
}

void CECI(vector<bool>& data_v_visited, Query *q, HyperGraph* gsh, Graph *g, FILE *fpR)
{
#ifdef CECI_PROFILE
    long rewrite_t = 0, cluster_t = 0, link_t = 0, filter_t = 0, refine_t = 0, join_t = 0, begin, end;
#endif

#ifdef CECI_PROFILE
            begin = get_cur_time();
#endif
	NECTree q_prime;
	q_prime.init();
	int us = ChooseStartQVertex(q, gsh);
    //cout<<"us: "<<us<<endl;
    //us = 0;   //just for debug
    //merge similar query nodes, generate a BFS tree
	RewriteToNECTree(q, us, &q_prime);
#ifdef CECI_PROFILE
	//cout<<"rewrite to nec tree"<<endl;
    //cout<<"Done: rewrite to NECTree"<<endl;
            end = get_cur_time();
            rewrite_t += (end-begin);
#endif

#ifdef CECI_PROFILE
            begin = get_cur_time();
#endif
    //NOTICE: the vertices are all sorted
    CECItree ctr;
    ctr.TEC.push_back(ECPT());
    ctr.TEC[0][0] = vector<int>();
    //BETTER: may also use inverse label list
	for(int i = 0; i < gsh->numVertex; i ++)
	{
		//cout<<"this is the "<<i<<"th data vertex"<<endl;
        if(!filter(q, gsh, g, us, i))
        {
            continue;
        }
//#ifdef PIVOT_DYNAMICCL
        //if(gsh->scf[i].size() > 0)
        //{
            //continue;
        //}
//#endif
        ctr.TEC[0][0].push_back(i);
	}
    //cout<<"Done: add cluster pivots"<<endl;
#ifdef CECI_PROFILE
            end = get_cur_time();
            cluster_t += (end-begin);
#endif

#ifdef PIVOT_QDN
    //NOTICE: for simplicity and performance, we only use QDN for pivots.
    //ADD: buildDRT for hyper graphs
    gsh->drt = new vector<QDN>[q->numVertex];
    //we need to build DRT for pivots first, then we can remove candidates QD-contained by others.
    BuildDRT(ctr.TEC[0][0], us, gsh, g, q);
    vector<int> new_cand;
    for(int i = 0; i < ctr.TEC[0][0].size(); ++i)
    {
        int id = ctr.TEC[0][0][i];
        if(gsh->drt[us][id].num == 0)
            new_cand.push_back(id);
    }
    ctr.TEC[0][0] = new_cand;
#endif

#ifdef CECI_PROFILE
            begin = get_cur_time();
#endif
    //NOTICE: here we adopt the default BFS order of NEC Tree q_prime.
    //
    //find non-tree edges in NEC Tree
    int* qv2nec = new int[q->numVertex];
    for(int i = 0; i < q_prime.numVertex; ++i)
    {
        for(int j = 0; j < q_prime.NEC[i].size(); ++j)
        {
            qv2nec[q_prime.NEC[i][j]] = i;
        }
    }
    vector< vector<int> > ntes(q_prime.numVertex);  //non-tree edge set, only record big->small (child->parent)
    vector< vector<int> > ntes_child(q_prime.numVertex);  //non-tree edge set, only record small->big (parent->child)
    for(int i = 0; i < q_prime.numVertex; ++i)
    {
        vector<bool> flag(q_prime.numVertex, false);
        if(i > 0)
            flag[q_prime.parent[i]] = true;
        //WARN: when a node has no child, s = e = -1.
        for(int j = q_prime.child[i].s; j != -1 && j <= q_prime.child[i].e; ++j)
        {
            flag[j] = true;
        }
        //NOTICE: we consider non-tree edge only once
        for(int j = 0; j <= i; ++j)
        {
            flag[j] = true;
        }
        int vid = q_prime.NEC[i][0];
        vector<labelVlist>& tv = q->graList[vid];
        for(int j = 0; j < tv.size(); ++j)
        {
            int lb = tv[j].label;
            for(int k = 0; k < tv[j].vlist.size(); ++k)
            {
                int neighbor = tv[j].vlist[k];
                int tmp = qv2nec[neighbor];
                if(!flag[tmp])
                {
                    //found a non-tree edge
                    ntes[tmp].push_back(i);
                    ntes_child[i].push_back(tmp);
                }
            }
        }
    }
    //cout<<"Done: find linking edges"<<endl;
#ifdef CECI_PROFILE
            end = get_cur_time();
            link_t += (end-begin);
#endif

    //build Compact Embedding Cluster Index (CECI) for NEC Tree
    //
    //Naive methods: include in all edge candidates.
    //add tree edge candidates
    //for(int i = 0; i < q_prime.numVertex; ++i)
    //{
        //unordered_map<int, vector<int> > tmpm;
        //for(int j = q_prime.child[i].s; j <= q_prime.child[i].e; ++j)
        //{
            ////add candidates of edge (i,j) in q_prime
            //int vid1 = q_prime.NEC[i][0], vid2 = q_prime.NEC[j][0];
            //int lb1 = q->vList[vid1], lb2 = q->vList[vid2];
            //for(int k = 0; k < g->numVertex; ++k)
            //{
                //if(contain(lb1, &(g->vList[k])) == -1)
                //{
                    //continue;
                //}
                //int idx = contain(lb2, &(g->graList[k]));
                //if(idx == -1)
                //{
                    //continue;
                //}
                //tmpm[k] = g->graList[k][idx].vlist;
            //}
            //ctr.TEC.push_back(tmpm);
        //}
    //}
    //add non-tree edge candidates
    //for(int i = 0; i < q_prime.numVertex; ++i)
    //{
        //vector< unordered_map<int, vector<int> > > Vtmpm;
        //for(int j = 0; j < ntes[i].size(); ++j)
        //{
            //unordered_map<int, vector<int> > tmpm;
            //int vid1 = q_prime.NEC[ntes[i][j]][0], vid2 = q_prime.NEC[i][0];
            //int lb1 = q->vList[vid1], lb2 = q->vList[vid2];
            //for(int k = 0; k < g->numVertex; ++k)
            //{
                //if(contain(lb1, &(g->vList[k])) == -1)
                //{
                    //continue;
                //}
                //int idx = contain(lb2, &(g->graList[k]));
                //if(idx == -1)
                //{
                    //continue;
                //}
                //tmpm[k] = g->graList[k][idx].vlist;
            //}
            //Vtmpm.push_back(tmpm);
        //}
        //ctr.NTEC.push_back(Vtmpm);
    //}

    //BFS based Construction and Filtering
    bool success = BFSFilter(q, gsh, g, q_prime, qv2nec, ntes, ntes_child, ctr);
    if(!success)
        goto CECI_EXIT;
    //cout<<"Done: BFSFilter"<<endl;

    //Reverse BFS based Refinement
    success = ReverseBFSRefine(q, gsh, g, q_prime, qv2nec, ntes, ntes_child, ctr);
    if(!success)
        goto CECI_EXIT;
    //cout<<"Done: ReverseBFSRefine"<<endl;

{
//NOTICE: we build full indices above.
#ifdef PIVOT_DYNAMICCL
    vector<int> cans;
    vector<int> ump(gsh->numVertex, -1);
    vector<bool> hasht(gsh->numVertex, false);
    for(int i = 0; i < ctr.TEC[0][0].size(); ++i)
    {
        int id = ctr.TEC[0][0][i];
        hasht[id] = true;
        if(gsh->scf[id].size() == 0)
        {
            cans.push_back(id);
        }
    }
#else
    vector<int> cans = ctr.TEC[0][0];
#endif

    //ADD: buildDRT for other query vertices
    //for(int i = 1; i < q_prime.numVertex; ++i)
    //{
        //vector<bool> dupr(gsh->numVertex, false);
        //vector<int> nodes;
        //for(ECPT::iterator mit = ctr.TEC[i].begin(); mit != ctr.TEC[i].end(); ++mit)
        //{
            //vector<int>& cand = mit->second;
            //for(int j = 0; j < cand.size(); ++j)
            //{
                //int id = cand[j];
                //if(!dupr[id])
                //{
                    //nodes.push_back(id);
                    //dupr[id] = true;
                //}
            //}
        //}
        //BuildDRT(nodes, q_prime.NEC[i][0], gsh, g, q);
    //}
    //NOTICE: used times of h and its qde h2?  this may cause error. (Bug of BoostISO paper)
    //Thus, we need to solve this problem bu utilizing F[]

#ifdef CECI_PROFILE
            begin = get_cur_time();
#endif
    //Subgraph search process for each embedding cluster
    //NOTICE: no need to store g now, use TEC and NTEC intersection instead.
    //delete g; g = NULL;
    //backtracking on CECI index
    //BETTER?: use new order for each cluster embedding
    int *M = new int[q->numVertex];
    for(int j = 0; j < q->numVertex; j ++)
        M[j] = -1;
    int *F = new int[gsh->numVertex];
    for(int j = 0; j < gsh->numVertex; j ++)
        F[j] = gsh->vertices[j].size();

    //double vsize, rss;
    //Util::process_mem_usage(vsize, rss);
    //if(vsize > max_mem_usage)
        //max_mem_usage = vsize;

    for(int i = 0; i < cans.size(); ++i)
    {
        int dv = cans[i];
        //memset(F, false, sizebool * gsh->numVertex);
        vector <int> qV;
        qV.push_back(us);
        vector <int> gV;
        gV.push_back(dv);
        UpdateState(M, F, &qV, &gV);
        //BETTER: given a specific order, pre-compute the linking edges once (like VF3)
        bool ret = SubgraphSearch(data_v_visited, q, gsh, q_prime, ctr, 1, M, F, fpR, ntes, us);
        if(ret)
        {
            //cout<<"find a complete match"<<endl;
#ifdef PIVOT_DYNAMICCL
            //ADD for hyper graphs
            //dynamic loading candidates for the pivot nodes. (no NEC)
            DynamicCL(cans, dv, gsh, ump, us, hasht);   //we use real u here
#endif
        }
        RestoreState(M, F, &qV, &gV);
    }
    delete []M;
    delete []F;
}
#ifdef CECI_PROFILE
    end = get_cur_time();
    join_t += (end-begin);
#endif

    //cout<<"Done: CECI process"<<endl;
CECI_EXIT: 
    delete[] qv2nec;
    //NOTICE: drt is built for each query, thus we must release it each time.
#ifdef PIVOT_QDN
    delete[]  gsh->drt;
#endif
    //cout<<"recursive call: "<<num_recursive_call<<endl;
    //cout<<"maximum memory usage: "<<max_mem_usage <<" KB"<<endl;
}

bool CheckQ(Query *q, HyperGraph *gsh)
{
	if (q->LabelNum > gsh->LabelNum) return false;
	return true;
}

//argv[1]: graph file     argv[2]: query file      optional: argv[3], result file
int main(int argc, char** argv) {
    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL) {
        printf("cannot open\n");
        exit(0);
    }

	//Graph g;
	//g.createGraph(fp);
	//fclose(fp);
	FILE *fpQ = fopen(argv[2], "r");
	if(fpQ == NULL)	{
		printf("cannot open\n");
		exit(0);
	}
	string query_path = argv[2];
    int pos1 = query_path.rfind("/");
    int pos2 = query_path.rfind(".");
    string qid = query_path.substr(pos1+1, pos2-pos1-1);
//	Query q;
//	q.createGraph(fpQ);
    vector<Query*> qlist;
    //cout<<"now to collect query graphs"<<endl;
    while(1){
        Query* q = new Query;
        q->createGraph(fpQ);
        if(q->real_graph == NULL){
            delete q; break;
        }
        qlist.push_back(q);
    }
	fclose(fpQ);
    //cout<<"close query file"<<endl;
    //cout<<qlist.size()<<endl;

	string result = "ans.txt";
	if(argc > 3) result = argv[3];
	FILE *fpR = fopen(result.c_str(), "w");
	if(fpR == NULL) {
		printf("cannot open\n");
		exit(0);
	}

    long sumt = 0;
    int dgcnt = -1;
	cerr << "CECI-boost input OK" << endl;
	while(true){
		Graph* g = new Graph;
		g->createGraph(fp);
		if(g->real_graph == NULL){
			delete g;	break;
		}
		dgcnt++;
        	//cerr<<"data"<<dgcnt<<endl;

		cerr.flush();

            ////offline computing SE and SC for BoostISO
			//string tmpFile = "temp.g";
            //g->outputGraph(tmpFile);
			//string cmd = "./gshBoostISO.exe " + tmpFile + " se.txt sc.txt";
            //system(cmd.c_str());

            //NOTICE: in se.txt sc.txt , the second line is always 0.
	        HyperGraph* gsh = new HyperGraph;
        	gsh->build("se.txt", "sc.txt");
            vector<bool> data_v_visited(g->numVertex, false);

			long begin = get_cur_time();
		for(int i = 0; i < qlist.size(); ++i) {
            // num_recursive_call =  numofembeddings = 0;   //reset for each matching iteration
    		Util::timeLimit(TIME_LIMIT_SECONDS);
			numofembeddings = 0;
			Query* q = qlist[i];
#ifdef PRINT_RESULT
			fprintf(fpR, "query graph:%d    data graph:%d\n", i, dgcnt);
			fprintf(fpR, "============================================================\n");    
#endif
			if(CheckQ(q, gsh))  //check the maximum label num
				CECI(data_v_visited, q, gsh, g, fpR);
#ifdef PRINT_RESULT
			fprintf(fpR, "\n\n\n");
			fflush(fpR);
#endif
			Util::noTimeLimit();
		}
			long end = get_cur_time();
			sumt += (end-begin);
		delete g;
        delete gsh;
	}

	fclose(fp);
	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%8.0lf kB    %s\n", 
		"ceciBoosted: ", numofembeddings, num_recursive_call, sumt, max_mem_usage, qid.c_str());

	/*
	cout << "CECI-boost: " << endl;
	cout << "  nembeddings: " << numofembeddings << endl;
	cout << "  ncalls: " << num_recursive_call << endl;
	cout << "  time: " << sumt << "ms" << endl;
	cout << "  max_mem: " << max_mem_usage << "kB" << endl;
	*/

	fclose(fpR);
	for(int i = 0; i < qlist.size(); ++i)
		delete qlist[i];

	//system("pause");
	return 0;
}



