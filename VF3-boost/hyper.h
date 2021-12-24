
#ifndef _HYPER_H
#define _HYPER_H

#include "util/Util.h"
#include "graph/Graph.h"

using namespace std;

struct labelVlist {
	int label;
	vector<int> vlist;
    bool operator<(const labelVlist& lv) const {
        return this->label < lv.label;
    }
    bool operator==(const labelVlist& lv) const {
        return this->label == lv.label;
    }
};


class HyperGraph {
public:
	int LabelNum, numVertex, numEdge;
	int *vList;			// vertex label
    int *vType;         // vertex type (0, 1, 2) -> (single, clique, independent)
    int *real_deg;
    int *usedTimes;
    vector<int> *vertices;  // real vertices, vertices[i] is the set of hypernode i
	vector<int> *labelList; // inverse label list, labelList[i] is the set of hyper with label i
	vector<labelVlist> *graList;	// graList[u][x] is a labelVlist struct containing all neighbors of u with graList[u][x].label
    vector<int> *neigh;
    vector<int> *scc;   //SC children
    vector<int> *scf;   //SC father

	HyperGraph() {
        LabelNum = numVertex = numEdge = 0;
        vList = vType = real_deg = usedTimes = NULL;
		labelList = NULL;
		graList = NULL;
        neigh = NULL;
        vertices = NULL;
        scc = scf = NULL;
	}
	~HyperGraph() {
		if(vList != NULL) delete []vList;
        if(vType != NULL) delete []vType;
        if(real_deg != NULL) delete []real_deg;
		if(labelList != NULL) delete []labelList;
        if(neigh)  delete []neigh;
		if(graList != NULL) delete []graList;
        if(usedTimes != NULL) delete []usedTimes;
        if(vertices != NULL) delete []vertices;
        if(scc != NULL) delete []scc;
        if(scf != NULL) delete []scf;
	}

	void addVertex(vector<int>& vals) {
        int id = vals[0];       // hypernode id
        vList[id] = vals[1];    // label
        vType[id] = vals[2];    // 0, 1, 2
        for(int i = 3; i < vals.size(); ++i)
            vertices[id].push_back(vals[i]);
		labelList[vList[id]].push_back(id);
	}

	int contain(int e, vector<labelVlist> *temp) {
		for (int i=0; i<temp->size(); i++)
			if ((*temp)[i].label == e)
				return i;
		return -1;
	}

    void addEdge(int from, int to) {
		int label = vList[to];
		int pos = contain(label, &(graList[from]));
		if(pos != -1)
			graList[from][pos].vlist.push_back(to);
		else {
			labelVlist l;
			l.label = label;
			l.vlist.push_back(to);
			graList[from].push_back(l);
		}
		
		label = vList[from];
		pos = contain(label, &(graList[to]));
		if(pos != -1)
			graList[to][pos].vlist.push_back(from);
		else {
			labelVlist l;
			l.label = label;
			l.vlist.push_back(from);
			graList[to].push_back(l);
		}
        neigh[from].push_back(to);
        neigh[to].push_back(from);
    }

    // Sort 
    void transform() {
        for(int i = 0; i < LabelNum; ++i)
            sort(labelList[i].begin(), labelList[i].end());
        for(int i = 0; i < numVertex; ++i) {
            sort(graList[i].begin(), graList[i].end());
            for(int j = 0; j < graList[i].size(); ++j) {
                vector<int>& vl = graList[i][j].vlist;
                sort(vl.begin(), vl.end());
            }
        }
        for(int i = 0; i < numVertex; ++i) {
            for (int j = 0; j < neigh[i].size(); j++) {
                int v_ = neigh[i][j];
                real_deg[i] += vertices[v_].size();
            }
            if (vType[i] == 1)
                real_deg[i] += vertices[i].size() - 1;
            sort(scc[i].begin(), scc[i].end());
            sort(scf[i].begin(), scf[i].end());
        }
    }

    void loadSE(string seFile) {
        FILE* fp = fopen(seFile.c_str(), "r");
        char* line = NULL;
        size_t len = 0;
        vector<int> integerValues;
        //WARN: C++ getline is very different from C getline function
        getline (&line, &len, fp);
        while ( len != 0 && line[0] == 't') {
            if(strcmp(line, "t # -1\n") == 0)
                break;
            //read the second line: vertexNum, edgeNum, vlabelMax
            getline (&line, &len, fp);
            // insert all the nodes
            while(getline (&line, &len, fp) && line[0] == 'v') {
                Util::readIntegersFromString(line,integerValues);
                addVertex(integerValues);
            }
            if(line[0] == 'e') {
            // insert all the edges
                do {
                    if (len != 0) {
                        Util::readIntegersFromString(line, integerValues);
                        addEdge(integerValues[0], integerValues[1]);
                    }
                } while(getline(&line, &len, fp) && line[0] == 'e');
            }
            if(strcmp(line, "t # -1\n") == 0)
                break;
        }
        if(line)
            xfree(line);
        fclose(fp);
    }

    void loadSC(string scFile) {
        FILE* fp = fopen(scFile.c_str(), "r");
        char* line = NULL;
        size_t len = 0;
        vector<int> integerValues;
        getline (&line, &len, fp);
        while ( len != 0 && line[0] == 't') {
            if(strcmp(line, "t # -1\n") == 0)
                break;
            getline (&line, &len, fp);
            while(getline(&line, &len, fp) && line[0] == 'e') {
                if (len != 0) {
                    Util::readIntegersFromString(line, integerValues);
                    int src = integerValues[0], dst = integerValues[1];
                    scc[src].push_back(dst);
                    scf[dst].push_back(src);
                }
            }
            if(strcmp(line, "t # -1\n") == 0)
                break;
        }
        if(line) xfree(line);
        fclose(fp);
    }

    void build(string seFile, string scFile) {
        FILE* fp = fopen(seFile.c_str(), "r");
        char* line = NULL;
        size_t len = 0;
        vector<int> integerValues;
        getline (&line, &len, fp);
        while ( len != 0 && line[0] == 't') {
            if(strcmp(line, "t # -1\n") == 0)
                break;
            getline (&line, &len, fp);
            while(getline (&line, &len, fp) && line[0] == 'v') {
                Util::readIntegersFromString(line,integerValues);
                LabelNum = max(LabelNum, integerValues[1]);
                numVertex++;
            }
            do{
                if (len != 0)
                    numEdge++;
            }while(getline(&line, &len, fp) && line[0] == 'e');
            if(strcmp(line, "t # -1\n") == 0)
                break;
        }
        if(line)
            xfree(line);
        fclose(fp);
        //initialize
        this->vList = new int[numVertex];
        this->vType = new int[numVertex];
        this->usedTimes = new int[numVertex];
        memset(usedTimes, 0, numVertex*sizeof(int));
        this->real_deg = new int[numVertex];
        memset(real_deg, 0, numVertex*sizeof(int));
        this->labelList = new vector<int>[LabelNum+1];
        this->neigh = new vector<int>[numVertex];
        this->graList = new vector<labelVlist>[numVertex];
        this->vertices = new vector<int>[numVertex];
        this->scc = new vector<int>[numVertex];
        this->scf = new vector<int>[numVertex];
        loadSE("se.txt");
        loadSC("sc.txt");
        transform();
    }
};


#endif

