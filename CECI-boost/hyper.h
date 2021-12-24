
#ifndef _HYPER_H
#define _HYPER_H

#include "util/Util.h"
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

class QDN
{
public:
    //int key;
    int num;  //number of QDC parent
    vector<int> qdcc;   //QDC children
    vector<int> qdel;   //QDE list
    void clear()
    {
        //key = -1;
        qdcc.clear();
        qdel.clear();
    }
};

class HyperVertex
{
public:
    int label;
    int type; //0 single vertex   1 a clique   2 not a clique
    vector<int> vertices;   //real vertices corresponding to this hypervertex
    //se edges: undirected
    vector<labelVlist> see;  //SE edges
    //sc edges: only exist between same-label vertices
    vector<int> scc;   //SC children
    vector<int> scf;   //SC father
    HyperVertex()
    {
    }
    HyperVertex(vector<int>& vals)
    {
        label = vals[1];
        type = vals[2];
        for(int i = 3; i < vals.size(); ++i)
        {
            vertices.push_back(vals[i]);
        }
    }
};

class HyperGraph							//gsh
{
public:
	int LabelNum;					
	int numVertex;					
    int numEdge;
	int *vList;			//vertex label
    int *vType;         //vertex type
    vector<int>* vertices;   //real vertices corresponding to this hyper vertex
	vector <int> *labelList;		//inverse label list
	vector <labelVlist> *graList;	//SE edges
    vector<int> *scc;   //SC children
    vector<int> *scf;   //SC father
    vector<QDN>* drt;   //DRT for QDN, vector<QDN> (an array of all hyper vertices in GSH) for each query node

	HyperGraph()
	{
        LabelNum = numVertex = numEdge = 0;
        vList = vType = NULL;
		labelList = NULL;
		graList = NULL;
        vertices = NULL;
        scc = scf = NULL;
        drt = NULL;
	}
	~HyperGraph()
	{
		if(vList != NULL)
			delete []vList;
        if(vType != NULL)
            delete []vType;
		if(labelList != NULL)
			delete []labelList;
		if(graList != NULL)
			delete []graList;
        if(vertices != NULL)
            delete []vertices;
        if(scc != NULL)
            delete []scc;
        if(scf != NULL)
            delete []scf;
	}

	void addVertex(vector<int>& vals)
	{
        int id = vals[0];
        vList[id] = vals[1];
        vType[id] = vals[2];
        for(int i = 3; i < vals.size(); ++i)
        {
            vertices[id].push_back(vals[i]);
        }
		labelList[vList[id]].push_back(id);
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

    void transform()
    {
        for(int i = 0; i < LabelNum; ++i)
        {
            sort(labelList[i].begin(), labelList[i].end());
        }
        for(int i = 0; i < numVertex; ++i)
        {
            sort(graList[i].begin(), graList[i].end());
            for(int j = 0; j < graList[i].size(); ++j)
            {
                vector<int>& vl = graList[i][j].vlist;
                sort(vl.begin(), vl.end());
            }
        }
        for(int i = 0; i < numVertex; ++i)
        {
            sort(scc[i].begin(), scc[i].end());
            sort(scf[i].begin(), scf[i].end());
        }
    }
    void loadSE(string seFile)
    {
        FILE* fp = fopen(seFile.c_str(), "r");
        char* line = NULL;
        size_t len = 0;
        vector<int> integerValues;
        //WARN: C++ getline is very different from C getline function
        getline (&line, &len, fp);
        while ( len != 0 && line[0] == 't')
        {
            if(strcmp(line, "t # -1\n") == 0)
            {
                break;
            }
            //read the second line: vertexNum, edgeNum, vlabelMax
            getline (&line, &len, fp);
            // insert all the nodes
            while(getline (&line, &len, fp) && line[0] == 'v')
            {
                Util::readIntegersFromString(line,integerValues);
                addVertex(integerValues);
            }
            // insert all the edges
            if(line[0] == 'e')
            {
            do {
                if (len != 0)
                {
                    Util::readIntegersFromString(line, integerValues);
                    //cout<<"check: "<<integerValues[0]<<" "<<integerValues[1]<<endl;
                    addEdge(integerValues[0], integerValues[1]);
                }
            } while(getline(&line, &len, fp) && line[0] == 'e');
            }
            if(strcmp(line, "t # -1\n") == 0)
            {
                break;
            }
        }
        if(line)
        {
            xfree(line);
        }
        fclose(fp);
    }
    void loadSC(string scFile)
    {
        FILE* fp = fopen(scFile.c_str(), "r");
        char* line = NULL;
        size_t len = 0;
        vector<int> integerValues;
        getline (&line, &len, fp);
        while ( len != 0 && line[0] == 't')
        {
            if(strcmp(line, "t # -1\n") == 0)
            {
                break;
            }
            getline (&line, &len, fp);
            while(getline(&line, &len, fp) && line[0] == 'e')
            {
                if (len != 0)
                {
                    Util::readIntegersFromString(line, integerValues);
                    int src = integerValues[0], dst = integerValues[1];
                    scc[src].push_back(dst);
                    scf[dst].push_back(src);
                }
            }
            if(strcmp(line, "t # -1\n") == 0)
            {
                break;
            }
        }
        if(line)
        {
            xfree(line);
        }
        fclose(fp);
    }
    void build(string seFile, string scFile)
    {
        FILE* fp = fopen(seFile.c_str(), "r");
        char* line = NULL;
        size_t len = 0;
        vector<int> integerValues;
        getline (&line, &len, fp);
        while ( len != 0 && line[0] == 't')
        {
            if(strcmp(line, "t # -1\n") == 0)
            {
                break;
            }
            getline (&line, &len, fp);
            while(getline (&line, &len, fp) && line[0] == 'v')
            {
                Util::readIntegersFromString(line,integerValues);
                LabelNum = max(LabelNum, integerValues[1]);
                numVertex++;
            }
            do{
                if (len != 0)
                {
                    numEdge++;
                }
            }while(getline(&line, &len, fp) && line[0] == 'e');
            if(strcmp(line, "t # -1\n") == 0)
            {
                break;
            }
        }
        if(line)
        {
            xfree(line);
        }
        fclose(fp);
        //initialize
        //cout<<"numVertex: "<<numVertex<<endl;
        this->vList = new int[numVertex];
        this->vType = new int[numVertex];
        this->labelList = new vector<int>[LabelNum+1];
        this->graList = new vector<labelVlist>[numVertex];
        this->vertices = new vector<int>[numVertex];
        this->scc = new vector<int>[numVertex];
        this->scf = new vector<int>[numVertex];
        loadSE("se.txt");
        loadSC("sc.txt");
        transform();
    }
};

//a label set for each vertex
class Graph							//G'
{
public:
	int LabelNum;					
	int numVertex;					
    int numEdge;
	vector <int> *vList;			//a label list of each veretx
	vector <int> *labelList;		//inverse label list
	vector <labelVlist> *graList;	//edge info
	DGraph* real_graph;   //directed graph with edge labels
    //vector<HyperVertex> gsh; //hyper graph of BoostISO

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
                this->numEdge = numEdge;
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
    void outputGraph(string tmpFile)
    {
        FILE* fp = fopen(tmpFile.c_str(), "w+");
        if(fp == NULL)
        {
            cout<<"Error in outputGraph()"<<endl;
        }
        fprintf(fp, "t # 0\n");
        fprintf(fp, "%d %d %d\n", this->numVertex, this->numEdge, this->LabelNum);
        DGraph* dg = this->real_graph;
        for(int i = 0; i < dg->vertices.size(); ++i)
        {
            fprintf(fp, "v %d %d\n", i, dg->vertices[i].label);
        }
        for(int i = 0; i < dg->vertices.size(); ++i)
        {
            for(int j = 0; j < dg->vertices[i].out.size(); ++j)
            {
                fprintf(fp, "e %d %d\n", i, dg->vertices[i].out[j].vid);
            }
        }
        fprintf(fp, "t # -1\n");
        fclose(fp);
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

typedef vector< vector<int> > DVEC;   //double vector
typedef unordered_map<int, vector<int> > ECPT; //edge candidate pair type
class CECItree
{
public:
    //the first query node (source of BFS)
	//vector <int> cps;				//cluster pivot set
    //other query nodes, ID x corresponding to TEC[x] and NTEC[x]
	vector < unordered_map<int, vector<int> > > TEC;    //tree edge candidates
	vector < vector < unordered_map<int, vector<int> > > > NTEC;    //non-tree edge candidates
    DVEC NTEC_parent;//the parent node in q_prime for each uprime (non-tree edges parent)
    CECItree()
    {
        //unordered_map<int, vector<int> > tmp1;
        //TEC.push_back(tmp1);
        //vector< unordered_map<int, vector<int> > > tmp2;
        //NTEC.push_back(tmp2);
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

#endif


