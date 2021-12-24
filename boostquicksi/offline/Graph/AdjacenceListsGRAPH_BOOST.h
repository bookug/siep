#ifndef ADJACENCELISTS_GRAPH_BOOST
#define ADJACENCELISTS_GRAPH_BOOST

#include "../../util/Util.h"
#include "AdjacenceListsGraph.h"
#include "../CommonUtility/StringUtility.h"

using namespace std;

class AdjacenceListsGRAPH_BOOST {

public:
//@unique for qdcBoost
	typedef struct DTableUnit {
	public:
		AdjacenceListsGRAPH::Vertex* flagVertex;
		int flagClique; // 1.it's a clique. 2 it's not a clique 
		
		int inDegree;
		int spongeInDegree;
		int totalNumberOfEquivalentVertices; // the number of original data vertices 

		std::vector<AdjacenceListsGRAPH::Vertex* > * equivalentVerticeList;
		std::vector<DTableUnit *> * containementChildList;

		DTableUnit(AdjacenceListsGRAPH::Vertex* flagVertex) {
			DTableUnit::flagVertex = flagVertex;
			flagClique = 2;
			spongeInDegree = 0;
			totalNumberOfEquivalentVertices = 0;
			inDegree = 0;
			equivalentVerticeList = new std::vector<AdjacenceListsGRAPH::Vertex*>(); // the vertice list doesn't include the flagvertex itself
			containementChildList = new std::vector<DTableUnit *>(); // the vertice list doesn't include the flagvertex itself
		}
	} DTableUnit;

public:
	static int isContainmentRelation(AdjacenceListsGRAPH* dataGraph, AdjacenceListsGRAPH* queryGraph, int v, int w, int queryVertexId);
	static int isEquivalentRelation(AdjacenceListsGRAPH* dataGraph, AdjacenceListsGRAPH* queryGraph, int v, int w, int queryVertexId);
	static bool degreeFilter(AdjacenceListsGRAPH* queryGraph, AdjacenceListsGRAPH* dataGraph, int u, int v);
	static void computeDTable(AdjacenceListsGRAPH* queryGraph, AdjacenceListsGRAPH* dataGraph, std::map<int, vector<int>* >& candidates, std::map<int,std::vector<DTableUnit*> *>& hyperCandidateSetsList, std::map<int,DTableUnit*>& d_table);


	static void buildLabelRootMap(vector<AdjacenceListsGRAPH> & containmentGraphVector);
	static void loadContainmentGraph(vector<AdjacenceListsGRAPH> & hyperGraphVector, vector<AdjacenceListsGRAPH> & containmentGraphVector, std::ifstream & containmentGraphFile);

};

#endif
