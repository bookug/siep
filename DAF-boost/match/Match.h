/*=============================================================================
# Filename:		Match.h
# Author: bookug
# Mail: bookug@qq.com
# Last Modified:	2019-09-24 10:45
# Description: find all subgraph-graph mappings between query graph and data graph
=============================================================================*/

#ifndef _MATCH_MATCH_H
#define _MATCH_MATCH_H

#include "../util/Util.h"
#include "../graph/Graph.h"
#include "../io/IO.h"
#include "../hyper.h"

#include "../../control.h"

using namespace std;


#define MAX_QSIZE 1000

typedef vector< vector<int> > DVEC;   //double vector
typedef unordered_map<int, vector<int> > ECPT; //edge candidate pair type

class BitArray
{
public:
    unsigned* ptr;
    int num;   //num of 1s
    int capacity; //num of unsigneds
    BitArray()
    {
        ptr = NULL;
        num = 0;
        capacity = 0;
    }
    BitArray(int _size)
    {
        ptr = NULL;
        num = 0;
        capacity = 0;
        if(_size > 0)
        {
            capacity = (_size+31)/32;
            ptr = new unsigned[capacity];
            memset(ptr, 0, capacity*sizeof(unsigned));
        }
    }
    BitArray(const BitArray& bv)
    {
        this->num = bv.num;
        this->capacity = bv.capacity;
        this->ptr = NULL;
        if(this->capacity > 0)
        {
            ptr = new unsigned[capacity];
            memcpy(ptr, bv.ptr, capacity*sizeof(unsigned));
        }
    }
    BitArray(const BitArray& bv1, const BitArray& bv2)
    {
        this->num = bv1.num;
        this->capacity = bv1.capacity;
        this->ptr = NULL;
        if(this->capacity > 0)
        {
            ptr = new unsigned[capacity];
            memcpy(ptr, bv1.ptr, capacity*sizeof(unsigned));
            this->myunion(bv2);
        }
    }
    ~BitArray()
    {
        //NOTICE: to avoid memory error when copying objects.
        //delete[] ptr;
    }
    void release()
    {
        delete[] ptr;
        ptr = NULL;
        num = 0;
    }
    void clear()
    {
        if(capacity > 0)
        {
            memset(ptr, 0, capacity*sizeof(unsigned));
        }
        this->num = 0;
    }
    bool get(int pos)
    {
        unsigned ele = ptr[(pos>>5)];
        pos = (pos & 0x1f);
        return (ele&(1<<pos));
    }
    void set(int pos)
    {
        unsigned ele = ptr[(pos>>5)];
        ele |= (1<<(pos&0x1f));
        ptr[(pos>>5)] = ele;
        num++;
    }
    bool prune(int pos)
    {
        if(num == 0) return false;
        return !get(pos);
    }
    void myunion(const BitArray& bv)
    {
        //NOTICE: we require that two BitArrays be the same capacity.
        for(int i = 0; i < capacity; ++i)
        {
            this->ptr[i] |= bv.ptr[i];
        }
    }
};

class Match
{
public:
    std::vector<bool> data_v_visited;

	Match(Graph* _query, Graph* _data, HyperGraph* gsh);
	void match(FILE* ofp);
	//void match(IO& io);
	~Match();

private:
	int numofembeddings;
	int num_recursive_call;
	static const int MATCH_NUM_BOUND = 5000;
    //static const int TIME_LIMIT_SECONDS = 10*60;   //10min
	//static const int TIME_LIMIT_SECONDS = 10;   //just for test
	int qsize, dsize, hdsize;
	Graph* query;
	Graph* data;
    HyperGraph* gsh;

    bool* dparr;
    Graph qd; //directed 
    std::vector<int> topological_order;
    Graph reverse_qd; //directed
    std::vector<int> reverse_order;
    std::vector<BitArray> ancs;
    void buildDAG();
    void buildCS();
    void refineCS(Graph& qdt, std::vector<int>& _order);
    vector< vector<int> > vcs;   //vertex candidates
    vector< vector<int> > wuv;   //pre-computed weights of Wu(v)
    vector< vector<int> > mtlp;   //maximal tree-like paths rooted at u, in topological order
	vector < vector < unordered_map<int, vector<int> > > > CS;    //candidate space including edges
	BitArray backtrack(int num, int* M, int* F, int ur, int pos, FILE* ofp);
    void computeCMU(int* M, int* F, int u, vector<int>& cmu);
    bool IsJoinable(int u, int v, int* F);
    void release();
};

#endif

