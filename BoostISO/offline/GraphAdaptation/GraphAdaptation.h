#ifndef GRAPH_ADAPTATION
#define GRAPH_ADAPTATION

#include "../Graph/AdjacenceListsGRAPH_BOOST.h"
#include "../Graph/AdjacenceListsGraph.h"
#include "../CommonUtility/StringUtility.h"
#include "../Graph/AdjacenceListsGRAPH_IO.h"

using namespace std;

class GraphAdaptation{

private:
	char * inputGraphName;
	char * outputHyperGraphName;
	char * outputContainmentGraphName;

	//std::ifstream inputGraphFile;
	//std::ofstream outputHypergraphFile;
	//std::ifstream inputHypergraphFile;
	//std::ofstream outputContainmentGraphFile;

	AdjacenceListsGRAPH * dataGraph;
	vector<AdjacenceListsGRAPH> dataGraphVector;
	AdjacenceListsGRAPH * hyperGraph;
	vector<AdjacenceListsGRAPH> hyperGraphVector;
	AdjacenceListsGRAPH * containmentGraph;
	vector<AdjacenceListsGRAPH> containmentGraphVector;

public:
	//std::ofstream resultFile;
	GraphAdaptation(bool digraph, char* inputFilename, char * outputHyperFilename, char * outputContainmentGraphName);
	~GraphAdaptation();
	/**
	* @para inputFilename the graph filename
	* @para outputFilename the filename which will store the hyper graph data and a file containing the containment relations
	*/

	void computeHyperGraphs();
	void loadHyperGraphs();
	void computeContainmentGraphs();
	

	void buildHyperGraphAlgorithm(FILE* _fp);
	void buildContainmentGraphAlgorithm(FILE* _fp);

	bool isSyntacticEquivalent(int v, int u);

	bool isSyntacticContainment(int v, int u);

	void ouputStatistics();

private:
};



#endif
