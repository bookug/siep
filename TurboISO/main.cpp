#include "util.h"

#include "../control.h"

using namespace std;

long num_recursive_call = 0;
long numofembeddings = 0, bound = 100000;
double max_mem = 0.0;
long TIME_LIMIT_SECONDS = 600; //10 min

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
    //if(a->size() == 0)
    //{
        //return -1;
    //}
    labelVlist tmp;
    tmp.label = e;
    //assert(a->size()>0);
    //assert((*a)[0].label>0);
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

int ChooseStartQVertex(Query *q, Graph *g)
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
            //assert(Vj < q->numVertex);
            //if(q->graList[Vj].size()==0)
            //{
                //cout<<"error: "<<Vj<<endl;
            //}
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
	q_prime->NEC.push_back(root);
	q_prime->parent.push_back(-1);
	Child child;
	child.s = -1;
	child.e = -1;
	q_prime->child.push_back(child);
	flag[us] = true;

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

void ClearCR(int uc_Dprime, int v_prime, CRTree *CR)
{
	int pos = contain(v_prime, &(CR->parent[uc_Dprime]));
	vector <int>::iterator Iter1 = CR->parent[uc_Dprime].begin() + pos;
	CR->parent[uc_Dprime].erase(Iter1);
	vector <vector <int> >::iterator Iter2 = CR->CR[uc_Dprime].begin() + pos;
	CR->CR[uc_Dprime].erase(Iter2);
}

bool ExploreCR(int u_prime, vector <int> *VM, CRTree *CR, int v, NECTree *q_prime, Graph *g, Query *q, bool *visited)
{
	int VMSize = VM->size();
	for(int i = 0; i < VMSize; i ++)
	{
		if(visited[(*VM)[i]])
			continue;
		int u_primeNEC1 = q_prime->NEC[u_prime][0];
		int v_prime = (*VM)[i];

        //find a region: check degrees
		int Size1 = q->graList[u_primeNEC1].size();
		int Size2 = g->graList[v_prime].size();
		if(Size1 > Size2)
			continue;

		bool f = true;
        //check label-grouped neighbor num
		for(int j = 0; j < Size1; j ++)
		{
			int label = q->graList[u_primeNEC1][j].label;
			int pos = contain(label, &(g->graList[v_prime]));
			if(pos == -1)
			{
				f = false;
				break;
			}
			if(q->graList[u_primeNEC1][j].vlist.size() > g->graList[v_prime][pos].vlist.size())
			{
				f = false;
				break;
			}
		}
		if(!f)
			continue;

		visited[(*VM)[i]] = true;
		bool matched = true;

		int S = q_prime->child[u_prime].s;
		if(S != -1)  //has children
		{
			int E = q_prime->child[u_prime].e;
			int Len = E - S + 1;

			Neighbor *neighbor = new Neighbor[Len];
			for(int j = S; j <= E; j ++)
			{
				int label = q_prime->vList[j];
				int pos = contain(label, &(g->graList[v_prime]));
                //NOTICE: this must be checked!
                if(pos == -1)
                {
                    matched = false;
                    goto ExploreCR_L1;
                }
				neighbor[j - S].uc_prime = j;
				neighbor[j - S].pos = pos;
                //assert(pos>=0);
				//int check = g->graList[v_prime][pos].vlist.size();
                //assert(check > 0);
				neighbor[j - S].NeighborN = g->graList[v_prime][pos].vlist.size();
			}
			qsort(neighbor, Len, sizeof(Neighbor), cmp);
			for(int j = 0; j < Len; j ++)
			{
				bool success = ExploreCR(neighbor[j].uc_prime, &(g->graList[v_prime][neighbor[j].pos].vlist), CR, v_prime, q_prime, g, q, visited);
				if(!success)
				{
					for(int k = 0; k < j; k ++)
					{
						ClearCR(neighbor[k].uc_prime, v_prime, CR);
					}
					matched = false;
					break;
				}
			}
ExploreCR_L1:
			delete []neighbor;
		}
		visited[(*VM)[i]] = false;
		if(!matched)
			continue;

		int pos = contain(v, &(CR->parent[u_prime]));
		if(pos == -1)
		{
            //if no parent(the root), then -1.
			CR->parent[u_prime].push_back(v);
			vector <int> tempV;
			tempV.push_back(v_prime);
			CR->CR[u_prime].push_back(tempV);
		}
		else 
        {
            PureInsert(CR->CR[u_prime][pos], v_prime);
            //CR->CR[u_prime][pos].push_back(v_prime);
        }
	}

    //if v is -1, and u_prime is the root, then its parent is -1
	int pos = contain(v, &(CR->parent[u_prime]));
	if(pos == -1)
	{
		return false;
	}
	if(CR->CR[u_prime][pos].size() < q_prime->NEC[u_prime].size())
	{
		ClearCR(u_prime, v, CR);
		return false;
	}
	return true;
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

double DetermineMatchingOrder(NECTree *q_prime, CRTree *CRTree, Elem *order, int v, int product, Query *q)
{
	int ENum = 0;
	int nSize = q_prime->NEC[v].size();
	for(int ii = 0; ii < nSize; ii ++)
	{
		int vv = q_prime->NEC[v][ii];
		int graS = q->graList[vv].size();
		for(int i = 0; i < graS; i ++)
			ENum += q->graList[vv][i].vlist.size();
	}

	if(nSize > 1)
	{
		int Label = q_prime->vList[v];
		vector <labelVlist> *p = &(q->graList[q_prime->NEC[v][0]]);
		int pos = contain(Label, p);
		if(pos != -1)
		{			
			if(contain(q_prime->NEC[v][1], &((*p)[pos].vlist)) != -1)
			{
				ENum -= nSize * (nSize - 1) / 2;
			}
		}
	}

	int S = q_prime->child[v].s;
	if(S != -1)
	{
		int E = q_prime->child[v].e;
		int Product;
		if(v == 0)
		{
			int eNum = 0;
			for(int jj = S; jj <= E; jj ++)
			{
				eNum += nSize * q_prime->NEC[jj].size();
			}
			Product = product * (ENum - eNum + 1);
		}
		else
		{
			int eNum = nSize * q_prime->NEC[q_prime->parent[v]].size();
			for(int jj = S; jj <= E; jj ++)
			{
				eNum += nSize * q_prime->NEC[jj].size();
			}
			Product = product * (ENum - eNum + 1);
		}
		double Min = DBL_MAX;
		for(int i = S; i <= E; i ++)
		{
			double tempD = DetermineMatchingOrder(q_prime, CRTree, order, i, Product, q);
			if(Min - tempD > 1e-6)
				Min = tempD;
		}
		order[v].v = v;
		order[v].value = Min;
		return order[v].value;
	}
	else
	{
		int eNum = nSize * q_prime->NEC[q_prime->parent[v]].size();
		int Product = product * (ENum - eNum + 1);
		int Size = q_prime->NEC[v].size();
		if(Size == 1)
		{
			int Num = 0;
			int Size2 = CRTree->CR[v].size();
			for(int i = 0; i < Size2; i ++)
			{
				Num += CRTree->CR[v][i].size();
			}
			order[v].v = v;
			order[v].value = (double)Num / Product;
		}
		else
		{
			double Num = 0.0;
			int Size2 = CRTree->CR[v].size();
			for(int i = 0; i < Size2; i ++)
			{
				Num += C(CRTree->CR[v][i].size(), Size);
			}
			order[v].v = v;
			order[v].value = Num / Product;
		}
		return order[v].value;
	}
}

void UpdateState(int *M, bool *F, vector <int> *qV, vector <int> *gV)
{
	int Size = qV->size();
	for(int i = 0; i < Size; i ++)
	{
		F[(*gV)[i]] = true;
		M[(*qV)[i]] = (*gV)[i];
	}
}

void RestoreState(int *M, bool *F, vector <int> *qV, vector <int> *gV)
{
	int Size = qV->size();
	for(int i = 0; i < Size; i ++)
	{
		F[(*gV)[i]] = false;
		M[(*qV)[i]] = -1;
	}
}

void NextComb(vector <int> *C, int Size, vector <int> *rank, vector <int> *value)
{
	if((*rank)[0] == -1)
	{
		(*rank)[0] = 0;
		(*value).push_back((*C)[0]);
		for(int i = 1; i < Size; i ++)
		{
			(*rank).push_back(i);
			(*value).push_back((*C)[i]);
		}
	}
	else
	{
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

			(*value)[Pos] = (*C)[(*rank)[Pos]];
			for(int pos = Pos + 1; pos < Size; pos ++)
			{
				(*rank)[pos] = (*rank)[pos - 1] + 1;
				(*value)[pos] = (*C)[(*rank)[pos]];
			}
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

		temp = M[(*qV)[rank[ii]]];
		M[(*qV)[rank[ii]]] = M[(*qV)[rank[iii]]];
		M[(*qV)[rank[iii]]] = temp;

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

		return false;
	}
}

bool verify(int* M, Query* q, Graph* g)
{
    //cout<<"find an asnwer!"<<endl;
    return true;
    //NOTICE: below checks the restrictions of directed edges and edge labels
	int qVNum = q->numVertex;
	//enumerate each edge in the query graph to check if this is a valid mapping
	vector<Vertex>& qvlist = q->real_graph->vertices;
	for(int i = 0; i < qvlist.size(); ++i)
	{
		//NOTICE: we only need to check all incoming edges to avoid duplicates
		vector<DNeighbor>& in = qvlist[i].in;
		for(int j = 0; j < in.size(); ++j)
		{
			int to = M[i];
			int from = M[in[j].vid];
			int label = in[j].elb;
			bool flag = g->real_graph->isEdgeContained(from, to, label);
			if(!flag)
			{
				return false;
			}
		}
	}
	return true;
}

//output a result after verified
void output(int* M, int qVNum, FILE* fpR) {
	if (numofembeddings > bound)
		return;
#ifdef PRINT_RESULT
	for(int i = 0; i < qVNum; ++i)
		fprintf(fpR, "(%d, %d) ", i, M[i]);
	fprintf(fpR, "\n");
#endif
	double vsize, rss;
	process_mem_usage(vsize, rss);
	if (vsize > max_mem)
		max_mem = vsize;
	numofembeddings++;
}

void GenPerm(int *M, NECTree *q_prime, int i, FILE *fpR, Query* q, Graph* g) {
	if (numofembeddings > bound)
		return;
	int qVNum = q->numVertex;
	if(i == q_prime->numVertex)
	{
		if(verify(M, q, g))
		{
			//cout<<"found a valid answer"<<endl;
			output(M, qVNum, fpR);
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
	if(Size == 1)
	{
		GenPerm(M, q_prime, i + 1, fpR, q, g);
	}
	else
	{
		int *rank = new int[Size];
		rank[0] = -1;
		while(!NextPerm(M, &(q_prime->NEC[i]), rank))
		{
			if (numofembeddings > bound) return;	
			GenPerm(M, q_prime, i + 1, fpR, q, g);
		}
	}
}

void SubgraphSearch(Query *q, NECTree *q_prime, Graph *g, Elem *order, int dc, int *M, bool *F, CRTree *CR, FILE *fpR) {
	if (numofembeddings > bound )return; // jiangyan add
    num_recursive_call++;
	int u_prime = order[dc].v;
	int p_u_prime = q_prime->parent[u_prime];

	vector <int> C, C1;
    int csize = 0;
	int Size = q_prime->NEC[p_u_prime].size();
	for(int i = 0; i < Size; i ++) {
		int v = M[q_prime->NEC[p_u_prime][i]];
		int pos = contain(v, &(CR->parent[u_prime]));
        if(pos == -1) {
            csize = 0;
            break;
        }
		vector <int> *t = &(CR->CR[u_prime][pos]);
		if(i == 0) {
			//cout<<"check stack: "<<t->size()<<endl;
			C1 = *t;
            csize = C1.size();
		}
		else {
			for(int j = 0; j < C1.size(); ++j) {
                if(C1[j] == -1)
                    continue;
				if(contain(C1[j], t) == -1) {
                    csize--;
                    C1[j] = -1;
                    continue;
				}
			}
		}
	}

	//NOTICE: this judgement is needed, otherwise bug may exist in some cases
    if(csize < q_prime->NEC[u_prime].size())
		return;
    for(int i = 0; i < C1.size(); ++i) {
        if(C1[i] == -1)
            continue;
        C.push_back(C1[i]);
    }

	//int *Srank = new int[Size];
	vector <int> Srank;
	Srank.push_back(-1);
	vector <int> value;						//C'

	while(true) {
		Size = q_prime->NEC[u_prime].size();	
		NextComb(&C, Size, &Srank, &value);
		if(Srank[0] == -1)
			break;
		bool Continue = false;
		for(int i = 0; i < Size; i ++) {
			if(F[value[i]]) {
				Continue = true;
				break;
			}
		}
		if(Continue)
			continue;

		if(Size > 1) {
			int Label = q_prime->vList[u_prime];
			vector <labelVlist> *p = &(q->graList[q_prime->NEC[u_prime][0]]);
			int pos = contain(Label, p);
			if(pos != -1) {
				if(contain(q_prime->NEC[u_prime][1], &((*p)[pos].vlist)) != -1) {
					for(int i = 0; i < Size; i ++) {
						vector <labelVlist> *p = &(g->graList[value[i]]);
						int Size2 = p->size();
						int pos = contain(Label, p);
						if(pos == -1) {
							Continue = true;
							break;
						}
						for(int j = i + 1; j < Size; j ++) {
							if(contain(value[j], &((*p)[pos].vlist)) == -1) {
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
		bool matched = true;
		for(int i = 0; i < Size; i ++) {
			if(!IsJoinable(q, g, M, q_prime->NEC[u_prime][i], value[i])) {
				matched = false;
				break;
			}
		}

		if(!matched) continue;
		UpdateState(M, F, &(q_prime->NEC[u_prime]), &value);
		if(q_prime->numVertex == dc + 1)
			GenPerm(M, q_prime, 0, fpR, q, g);
		else 
            SubgraphSearch(q, q_prime, g, order, dc + 1, M, F, CR, fpR);
		RestoreState(M, F, &(q_prime->NEC[u_prime]), &value);
	}
}

void TurboISO(Query *q, Graph *g, FILE *fpR) {
	long rewrite_t = 0, explore_t = 0, order_t = 0, join_t = 0, begin, end;
	begin = get_cur_time();
	NECTree q_prime;
	q_prime.init();
	int us = ChooseStartQVertex(q, g);

	//if(g->labelList != NULL)
	//	delete [](g->labelList);

	//merge similar query nodes, generate a BFS tree
	RewriteToNECTree(q, us, &q_prime);
	//cout<<"rewrite to nec tree"<<endl;
	end = get_cur_time();
	rewrite_t += (end-begin);

	for(int i = 0; i < g->numVertex; i ++) {
		//cout<<"this is the "<<i<<"th data vertex"<<endl;
		if(contain(q->vList[us], &(g->vList[i])) != -1) { //find a region: check labels
			//cout<<"to init CR"<<endl;
			CRTree CR;
			CR.init(q_prime.numVertex);
			vector <int> VM;  //for each candidate region
			VM.push_back(i);
			bool *visited = new bool[g->numVertex];
			memset(visited, false, sizebool * g->numVertex);
			long begin = get_cur_time();
			bool explore = ExploreCR(0, &VM, &CR, -1, &q_prime, g, q, visited);
			long end = get_cur_time();
			explore_t += (end-begin);
			if(visited != NULL)
				delete []visited;

			/*for(int j = 0; j < q_prime.numVertex; j ++)
			{
				for(int k = 0; k < CR.parent[j].size(); k ++)
					printf("%d ", CR.parent[j][k]);
				printf("\n");
			}
			printf("\n");
			for(int j = 0; j < q_prime.numVertex; j ++) {
				for(int k = 0; k < CR.CR[j].size(); k ++) {
					for(int l = 0; l < CR.CR[j][k].size(); l ++) {
						printf("%d ", CR.CR[j][k][l]);
					} printf("\n");
				} printf("\n");
			}*/

			if(explore) {
				begin = get_cur_time();
				Elem *order = new Elem[q_prime.numVertex];
				//NOTICE: a new matching order for each candidate region
				//cout<<"to choose matching order"<<endl;
				DetermineMatchingOrder(&q_prime, &CR, order, 0, 1, q);
				qsort(order, q_prime.numVertex, sizeof(Elem), cmpE);
				end = get_cur_time();
				order_t += (end-begin);

				/*for(int j = 0; j < q_prime.numVertex; j ++) {
					printf("%d %lf\n", order[j].v, order[j].value);
				}*/

				begin =	get_cur_time();
				//Subgraph search process for each candidate region (CR)
				int *M = new int[q->numVertex];
				for(int j = 0; j < q->numVertex; j ++)
					M[j] = -1;
				bool *F = new bool[g->numVertex];
				memset(F, false, sizebool * g->numVertex);
				vector <int> qV;
				qV.push_back(us);
				vector <int> gV;
				gV.push_back(i);
				UpdateState(M, F, &qV, &gV);
				//cout<<"to do subgraph search"<<endl;
				SubgraphSearch(q, &q_prime, g, order, 1, M, F, &CR, fpR);
				end = get_cur_time();
				join_t += (end-begin);

				//cout<<"after subgraph search"<<endl;
				if(order != NULL) delete []order;
				//RestoreState(M, F, &qV, &gV);
				if(M != NULL) delete []M;
				if(F != NULL) delete []F;
			}
		}
	}/*
	cout<<"recursive call: "<<num_recursive_call<<endl;
	cout<<"rewrite time: "<<rewrite_t<<endl;
	cout<<"explore time: "<<explore_t<<endl;
	cout<<"order time: "<<order_t<<endl;
	cout<<"join time: "<<join_t<<endl;
*/
}

bool CheckQ(Query *q, Graph *g) {
	if (q->LabelNum > g->LabelNum) return false;
	return true;
}

//argv[1]: graph file	 argv[2]: query file	  optional: argv[3], result file
int main(int argc, char** argv) {
	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL) {
		printf("cannot open\n");
		exit(0);
	}
	FILE *fpQ = fopen(argv[2], "r");
	if(fpQ == NULL) {
		printf("cannot open\n");
		exit(0);
	}
	string query_path = argv[2];
    int pos1 = query_path.rfind("/");
    int pos2 = query_path.rfind(".");
    string qid = query_path.substr(pos1+1, pos2-pos1-1);
	vector<Query*> qlist;
	while(1) {
		Query* q = new Query;
		q->createGraph(fpQ);
		if(q->real_graph == NULL) {
			delete q; break;
		}
		qlist.push_back(q);
	}
	fclose(fpQ);

	string result = "ans.txt";
	if(argc > 3) result = argv[3];
	FILE *fpR = fopen(result.c_str(), "w");
	if(fpR == NULL) {
		printf("cannot open\n"); exit(0);
	}

	long sumt = 0;
	int dgcnt = -1;
	cerr << "TurboIso input OK" << endl;
	while(true) {
		Graph* g = new Graph;g->createGraph(fp);
		if(g->real_graph == NULL) {
			delete g; break;
		}
		dgcnt++;

		for(int i = 0; i < qlist.size(); ++i) {
            //num_recursive_call = numofembeddings = 0;
			long begin = get_cur_time();
			numofembeddings = 0;
			Query* q = qlist[i];
#ifdef _PRINT_ANS
			fprintf(fpR, "query graph:%d	data graph:%d\n", i, dgcnt);
			fprintf(fpR, "============================================================\n");
#endif
            timeLimit(TIME_LIMIT_SECONDS);
			if(CheckQ(q, g))  //check the maximum label num
				TurboISO(q, g, fpR);
            noTimeLimit();
#ifdef _PRINT_ANS	
			fprintf(fpR, "\n\n\n");
			fflush(fpR);
#endif
			long end = get_cur_time();
			sumt += (end-begin);
		}
		delete g;
	}

	fclose(fp);
	printf("%-15s nembeddings:%6ld;   ncalls:%8ld;   time:%8ld ms;   mem:%8.0lf kB    %s\n", 
		"turbo: ", numofembeddings, num_recursive_call, sumt, max_mem, qid.c_str());

	/*
	cout << "TurboISO: " << endl;
	cout << "  nembeddings: " << numofembeddings << endl;
	cout << "  ncalls: " << num_recursive_call << endl;
	cout << "  time: " << sumt << "ms" << endl;
	cout << "  max_mem: " << max_mem << "kB" << endl;
	*/

	fclose(fpR);
	cout.flush();
	for(int i = 0; i < qlist.size(); ++i) delete qlist[i];
	return 0;
}

