/*=============================================================================
# Filename: util.h
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2018-03-11 16:31
# Description: 
The original code only deals with undirected graphs, each vertex has many labels.
Here we want to deal with directed graphs with edge labels, a simple strategy is 
to treat graphs as undirected graphs without edge labels first, do subgraph isomorphism
with TurboISO, and finally verify each answer according to the real graph restrictions.
=============================================================================*/

#ifndef _UTIL_H
#define _UTIL_H

#include <iostream>
#include <vector>
#include <time.h>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <locale.h>
#include <assert.h>
#include <libgen.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

//NOTICE:below are restricted to C++, C files should not include(maybe nested) this header!
#include <bitset>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <map>
#include <set>
#include <stack>
#include <queue>
#include <deque>
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <functional>
#include <utility>
#include <new>

//NOTICE:below are libraries need to link
//#include <thread>    //only for c++11 or greater versions
//#include <atomic> 
//#include <mutex> 
//#include <condition_variable> 
//#include <future> 
//#include <memory> 
//#include <stdexcept> 
#include <pthread.h> 
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>

//Below are for boost
//Added for the json-example
//#define BOOST_SPIRIT_THREADSAFE
//#include <boost/spirit.hpp>
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/json_parser.hpp>

//Added for the default_resource example
//#include <boost/filesystem.hpp>
//#include <boost/regex.hpp>
//#include <boost/thread/thread.hpp>
//#include <boost/bind.hpp>
//#include <boost/asio.hpp>
//#include <boost/utility/string_ref.hpp>
//#include <boost/algorithm/string/predicate.hpp>
//#include <boost/functional/hash.hpp>
//#include <unordered_map>
//#include <random>
//#include <type_traits>

#include "Graph.h"

using namespace std;

int sizebool = sizeof(bool);
int sizeint = sizeof(int);


struct Child
{
	int s;
	int e;
};

struct labelVlist
{
	int label;
	vector <int> vlist;
    bool operator<(const labelVlist& lv) const
    {
        return this->label < lv.label;
    }
    bool operator==(const labelVlist& lv) const
    {
        return this->label == lv.label;
    }
};

//a label set for each vertex
class Graph							//G'
{
public:
	int LabelNum;					
	int numVertex;					

	vector <int> *vList;			//a label list of each veretx

	vector <int> *labelList;		//inverse label list

	vector <labelVlist> *graList;	//edge info

	DGraph* real_graph;   //directed graph with edge labels

	Graph()
	{
		vList = NULL;
		labelList = NULL;
		graList = NULL;
		real_graph = NULL;
	}
	
	~Graph()
	{
		if(vList != NULL)
			delete []vList;
		if(labelList != NULL)
			delete []labelList;
		if(graList != NULL)
			delete []graList;
		if(real_graph != NULL)
		{
			delete real_graph;
		}
	}

    void transform()
    {
        //BETTER: sort vertices in labelList
        for(int i = 0; i < numVertex; ++i)
        {
            sort(graList[i].begin(), graList[i].end());
            for(int j = 0; j < graList[i].size(); ++j)
            {
                vector<int>& vl = graList[i][j].vlist;
                sort(vl.begin(), vl.end());
            }
        }
    }

	void init(int _numVertex, int _LabelNum)
	{
		LabelNum = _LabelNum;
		numVertex = _numVertex;
		
		vList = new vector <int> [numVertex];

		labelList = new vector <int> [LabelNum];

		graList = new vector <labelVlist> [numVertex];
	}

	void addVertex(int v, int l)
	{
		vList[v].push_back(l);
		labelList[l].push_back(v);
	}

	int contain(int e, vector <labelVlist> *temp)
	{
		int Size = temp->size();
		for(int i = 0; i < Size; i ++)
		{
			if((*temp)[i].label == e)
				return i;
		}
		return -1;
	}

	void addEdge(int from, int to)
	{
		int Size = vList[to].size();
		for(int i = 0; i < Size; i ++)   //enumerate each label of to
		{
			int label = vList[to][i];
			int pos = contain(label, &(graList[from]));
			if(pos != -1)
			{
				graList[from][pos].vlist.push_back(to);
			}
			else
			{
				labelVlist l;
				l.label = label;
				l.vlist.push_back(to);
				graList[from].push_back(l);
			}
		}

		Size = vList[from].size();
		for(int i = 0; i < Size; i ++)
		{
			int label = vList[from][i];
			int pos = contain(label, &(graList[to]));
			if(pos != -1)
			{
				graList[to][pos].vlist.push_back(from);
			}
			else
			{
				labelVlist l;
				l.label = label;
				l.vlist.push_back(from);
				graList[to].push_back(l);
			}
		}
	}

	void createGraph(FILE *fp)
	{
		int numVertex, numEdge, vertexLabelNum, edgeLabelNum;
		char c1, c2;
		int id0, id1, id2, lb;
		bool flag = false;
		while(true)
		{
			fscanf(fp, "%c", &c1);
			if(c1 == 't')
			{
				if(flag)
				{
					fseek(fp, -1, SEEK_CUR);
					break;
				}
				flag = true;
				fscanf(fp, " %c %d\n", &c2, &id0);
				if(id0 == -1){
					this->real_graph = NULL;
					return;
					break;
				}
				else
				{
					this->real_graph = new DGraph;
				}
				//read vertex num, edge num, vertex label num, edge label num
				fscanf(fp, " %d %d %d\n", &numVertex, &numEdge, &vertexLabelNum);
				this->numVertex = numVertex;
				this->LabelNum = vertexLabelNum;
				//cout<<numVertex<<endl;
				vList = new vector<int> [numVertex];
				//cout<<vertexLabelNum<<endl;
				//NOTICE: the label ID begin from 1
				labelList = new vector <int> [vertexLabelNum+1];
				graList = new vector <labelVlist> [numVertex];
			}
			else if(c1 == 'v')
			{
				fscanf(fp, " %d %d\n", &id1, &lb);
				addVertex(id1, lb); 
				this->real_graph->addVertex(lb);
			}
			else if(c1 == 'e')
			{
				fscanf(fp, " %d %d\n", &id1, &id2);
				addEdge(id1, id2);
				this->real_graph->addEdge(id1, id2, 1);
			}
			else 
			{
				cerr<<"ERROR in input() -- invalid char"<<endl;
				return;
			}
		}
		//char buffer[1024];
		//if(fgets(buffer, 1024, fp) != NULL)
		//{
			//int _numVertex, _LabelNum;
			//sscanf(buffer, "%d %d\n", &_numVertex, &_LabelNum);
			//init(_numVertex, _LabelNum);
		//}
		//for(int i = 0; i < numVertex; i ++)
		//{
			//if(fgets(buffer, 1024, fp) != NULL)
			//{
				//int len = strlen(buffer);
				//int pos = 0;
				//while(true)
				//{
					//if(pos >= len || buffer[pos] == '\n')
						//break;
					//else if(buffer[pos] == ' ')
						//pos ++;
					//else
					//{
						//int value = atoi(&(buffer[pos]));
						//vList[i].push_back(value);
						//while(true)
						//{
							//if(pos >= len || buffer[pos] == ' ' || buffer[pos] == '\n')
								//break;
							//else pos ++;
						//}
					//}
				//}
			//}
		//}

		//while(!feof(fp))
		//{
			//if(fgets(buffer, 1000, fp) != NULL)
			//{
				//int from, to;
				//sscanf(buffer, "%d %d\n", &from, &to);
				//addEdge(from, to);
			//}
		//}
        this->transform();
	}
};

//一个结点只有一个label
class Query
{
public:
	int LabelNum;					
	int numVertex;					

	int *vList;						

	//vector <int> *labelList;		

	vector <labelVlist> *graList;	

	DGraph* real_graph;   //directed graph with edge labels

	Query()
	{
		vList = NULL;
		graList = NULL;
		real_graph = NULL;
	}

	~Query()
	{
		if(vList != NULL)
			delete []vList;
		//if(labelList != NULL)
		//	delete []labelList;
		if(graList != NULL)
			delete []graList;
		if(real_graph != NULL)
		{
			delete real_graph;
		}
	}

    void transform()
    {
        for(int i = 0; i < numVertex; ++i)
        {
            sort(graList[i].begin(), graList[i].end());
            for(int j = 0; j < graList[i].size(); ++j)
            {
                vector<int>& vl = graList[i][j].vlist;
                sort(vl.begin(), vl.end());
            }
        }
    }

	void init(int _numVertex, int _LabelNum)
	{
		LabelNum = _LabelNum;
		numVertex = _numVertex;
		
		vList = new int [numVertex];

		//labelList = new vector <int> [LabelNum];

		graList = new vector <labelVlist> [numVertex];
	}

	void addVertex(int v, int l)
	{
		vList[v] = l;
		//labelList[l].push_back(v);
	}

	int contain(int e, vector <labelVlist> *temp)
	{
		int Size = temp->size();
		for(int i = 0; i < Size; i ++)
		{
			if((*temp)[i].label == e)
				return i;
		}
		return -1;
	}

	void addEdge(int from, int to)
	{
		int label = vList[to];
		int pos = contain(label, &(graList[from]));
		if(pos != -1)
		{
			graList[from][pos].vlist.push_back(to);
		}
		else
		{
			labelVlist l;
			l.label = label;
			l.vlist.push_back(to);
			graList[from].push_back(l);
		}
		
		label = vList[from];
		pos = contain(label, &(graList[to]));
		if(pos != -1)
		{
			graList[to][pos].vlist.push_back(from);
		}
		else
		{
			labelVlist l;
			l.label = label;
			l.vlist.push_back(from);
			graList[to].push_back(l);
		}
	}

	//NOTICE: maintain two kinds of structures, undirected(for TurboISO) and directed(for verification)
	//There are two createGraph() functions in this file, one for Query and one for Graph
	void createGraph(FILE *fp)
	{
		int numVertex, numEdge, vertexLabelNum, edgeLabelNum;
		char c1, c2;
		int id0, id1, id2, lb;
		bool flag = false;
		while(true)
		{
			fscanf(fp, "%c", &c1);
			if(c1 == 't')
			{
				if(flag)
				{
					fseek(fp, -1, SEEK_CUR);
					break;
				}
				flag = true;
				fscanf(fp, " %c %d\n", &c2, &id0);
				if(id0 == -1){
					this->real_graph = NULL;
					return;
					break;
				}
				else
				{
					this->real_graph = new DGraph;
				}
				//read vertex num, edge num, vertex label num, edge label num
				fscanf(fp, " %d %d %d %d\n", &numVertex, &numEdge, &vertexLabelNum, &edgeLabelNum);
				this->numVertex = numVertex;
				this->LabelNum = vertexLabelNum;
				//cout<<numVertex<<endl;
				vList = new int [numVertex];
				//cout<<vertexLabelNum<<endl;
				//NOTICE: the label ID begin from 1
				//labelList = new vector <int> [vertexLabelNum+1];
				graList = new vector <labelVlist> [numVertex];
			}
			else if(c1 == 'v')
			{
				fscanf(fp, " %d %d\n", &id1, &lb);
				addVertex(id1, lb); 
				this->real_graph->addVertex(lb);
			}
			else if(c1 == 'e')
			{
				fscanf(fp, " %d %d %d\n", &id1, &id2, &lb);
				addEdge(id1, id2);
				this->real_graph->addEdge(id1, id2, lb);
			}
			else 
			{
				cerr<<"ERROR in input() -- invalid char"<<endl;
				return;
			}
		}
		//char buffer[1024];
		//if(fgets(buffer, 1024, fp) != NULL)
		//{
			//int _numVertex, _LabelNum;
			//sscanf(buffer, "%d %d\n", &_numVertex, &_LabelNum);
			//init(_numVertex, _LabelNum);
		//}
		//for(int i = 0; i < numVertex; i ++)
		//{
			//if(fgets(buffer, 1024, fp) != NULL)
			//{
				//vList[i] = atoi(buffer);
			//}
		//}

		//while(!feof(fp))
		//{
			//if(fgets(buffer, 1000, fp) != NULL)
			//{
				//int from, to;
				//sscanf(buffer, "%d %d\n", &from, &to);
				//addEdge(from, to);
			//}
		//}
        this->transform();
	}
};

class NECTree
{
public:
	int numVertex;					
	vector <int> vList;				//the label of NECTree node
	vector < vector <int> > NEC;		//the real query nertices of a NECTree node
	vector <int> parent;			//parent node
	vector <Child> child;			//child nodes

	void init()
	{
		numVertex = 0;
	}
};

class CRTree
{
public:
	vector <vector <int> > *CR;	 //the real data vertices of CRTree
	vector <int> *parent;			//parent nodes: each query node u has a set of parent nodes v (data nodes)   CR(uprime,v) of vprimes

	void init(int num)
	{
		CR = new vector <vector <int> > [num];
		parent = new vector <int> [num];
	}

	~CRTree()
	{
		if(CR != NULL)
			delete []CR;
		if(parent != NULL)
			delete []parent;
	}
};

string int2string(long n)
{
    string s;
    stringstream ss;
    ss<<n;
    ss>>s;
    return s;
}

//NOTICE: there does not exist itoa() function in Linux, atoi() is included in stdlib.h
//itoa() is not a standard C function, and it is only used in Windows.
//However, there do exist a function called sprintf() in standard library which can replace itoa()
//char str[255];
//sprintf(str, "%x", 100); //change 100 to 16-base string
char* itoa(int num, char* str, int radix) //the last parameter means the number's radix: decimal, or octal formats
{
	//index table
	char index[]="0123456789ABCDEF";
	unsigned unum;
	int i=0,j,k;
	if(radix==10&&num<0)  //negative in decimal
	{
		unum=(unsigned)-num;
		str[i++]='-';
	}
	else unum=(unsigned)num;
	do{
		str[i++]=index[unum%(unsigned)radix];
		unum/=radix;
	}while(unum);
	str[i]='\0';
	//reverse order
	if(str[0]=='-')k=1;
	else k=0;
	char temp;
	for(j=k;j<=(i-1)/2;j++)
	{
		temp = str[j];
		str[j] = str[i-1+k-j];
		str[i-1+k-j] = temp;
	}
	return str;
}

long get_cur_time()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

void process_mem_usage(double& vm_usage, double& resident_set) {
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   unsigned long vsize;  // for virtual memory, the virtual size (B)
   long rss;    //for physical memory, number of real pages
   //In /proc/$pid/status, these corresponding to VmSize and VmRSS

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}

void myTimeout(int signo) {
    switch(signo) {
        case SIGALRM:
            printf("This query runs time out!\n");
            exit(1);
        default:
            break;
    }
}

void timeLimit(int seconds) {
    struct itimerval tick;
    //signal(SIGALRM, exit);
    signal(SIGALRM, myTimeout);
    memset(&tick, 0, sizeof(tick));
    //Timeout to run first time
    tick.it_value.tv_sec = seconds;
    tick.it_value.tv_usec = 0;
    //After first, the Interval time for clock
    tick.it_interval.tv_sec = seconds;
    tick.it_interval.tv_usec = 0;
    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
        printf("Set timer failed!\n");
}

void noTimeLimit() {
    struct itimerval tick;
    memset(&tick, 0, sizeof(tick));
    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
        printf("Withdraw timer failed!\n");
}



#endif


