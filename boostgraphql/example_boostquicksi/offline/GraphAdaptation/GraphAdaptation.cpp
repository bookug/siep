#include"GraphAdaptation.h"

using namespace std;



GraphAdaptation::GraphAdaptation(bool digraph, char* inputFilename, char * outputHyperFilename, char * outputContainmentFilename) 
{
	inputGraphName = inputFilename;
	outputHyperGraphName = outputHyperFilename;
	outputContainmentGraphName = outputContainmentFilename;

	//std::ifstream inputGraphFile(inputGraphName);
	//std::ofstream outputHypergraphFile(outputHyperGraphName);
	//std::ifstream inputHypergraphFile(outputHyperGraphName);
	//std::ofstream outputContainmentGraphFile(outputContainmentFilename);
	//std::ofstream resultFile("statistic.txt", std::ios_base::app);

	//if(!inputGraphFile.is_open()) {
		//cout<<"data graph file doesn't exist"<<endl;
		//exit(1);
	//}
	
	// load the data graph
	AdjacenceListsGRAPH_IO::loadGraphFromFile(inputGraphName, dataGraphVector);
    cout<<"load input graph from file"<<endl;

	// build data graph label vertex label vertex index
	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		dataGraphVector[dataGraphIndex].buildVertexLabelVertexList();
	}
	// build data graph label vertex index
	/*for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		dataGraphVector[dataGraphIndex].buildLabelVertexList();
	}*/
}

GraphAdaptation::~GraphAdaptation() {
	//inputGraphFile.close();
	//outputContainmentGraphFile.close();
	//inputHypergraphFile.close();
}

void  GraphAdaptation::computeHyperGraphs()
{
    FILE* fp = fopen(outputHyperGraphName, "w+");
	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) 
    {
		dataGraph = &dataGraphVector[dataGraphIndex];
		fprintf(fp, "t # %d\n", dataGraphIndex);
        fprintf(fp, "0 0 0\n");  //the second line of data
		buildHyperGraphAlgorithm(fp);
	}
    fprintf(fp, "t # -1\n");
    fclose(fp);
	// release the memory of the original data graph
	// dataGraphVector.clear();
}

void  GraphAdaptation::loadHyperGraphs()
{
	// load the hyper graph, only load the id and label
	AdjacenceListsGRAPH_IO::loadGraphFromFile(outputHyperGraphName, hyperGraphVector);

	// build hyper graph label vertex label vertex index
	for(int hyperGraphIndex = 0; hyperGraphIndex < hyperGraphVector.size(); hyperGraphIndex ++) {
		hyperGraphVector[hyperGraphIndex].buildVertexLabelVertexList();
	}

	// build hyper graph label vertex index
	/*for(int hyperGraphIndex = 0; hyperGraphIndex < hyperGraphVector.size(); hyperGraphIndex ++) {
		hyperGraphVector[hyperGraphIndex].buildLabelVertexList();
	}*/

}

void GraphAdaptation::computeContainmentGraphs()
{
    FILE* fp = fopen(outputContainmentGraphName, "w+");
	for(int hyperGraphIndex = 0; hyperGraphIndex < hyperGraphVector.size(); hyperGraphIndex ++) {
		
		hyperGraph = &hyperGraphVector[hyperGraphIndex];
        fprintf(fp, "t # %d\n", hyperGraphIndex);
        fprintf(fp,"0 0 0\n");  //the second line of data
		buildContainmentGraphAlgorithm(fp);
	}
    fprintf(fp, "t # -1\n");
    fclose(fp);
}


/**
* The containment graph shares the same set of data vertices with the hyper graph and is a transitive reduction of a DAG
*/
void GraphAdaptation::buildContainmentGraphAlgorithm(FILE* _fp)
{
	set<std::pair<int, int> > containmentEdges;
	/**
	* compute containment graph
	*/
	for(int hyperVertexIndex = 0; hyperVertexIndex < hyperGraph->getNumberOfVertexes(); hyperVertexIndex++){

		AdjacenceListsGRAPH::Vertex* hyperVertex = hyperGraph -> getVertexAddressByVertexId(hyperVertexIndex);
		
		// iterate 1_step reacheability neighbours
		map<int, vector<int> >::iterator sameLabelNeighbourIterator = hyperVertex->labelVertexList.find(hyperVertex -> label);
		if(sameLabelNeighbourIterator != hyperVertex->labelVertexList.end()){
			for(vector<int>::iterator samelabelNeighbourIndex = sameLabelNeighbourIterator->second.begin(); samelabelNeighbourIndex != sameLabelNeighbourIterator->second.end(); samelabelNeighbourIndex++) {
				if(isSyntacticContainment(hyperVertex->id, *samelabelNeighbourIndex)){
					containmentEdges.insert(std::pair<int, int>(hyperVertex->id, *samelabelNeighbourIndex));
				}
			}
		}

		// iterate 2_step reacheability neighbours
		AdjacenceListsGRAPH::adjIterator adjIterator(hyperGraph, hyperVertex->id);
		for(AdjacenceListsGRAPH::link t = adjIterator.begin();  !adjIterator.end(); t=adjIterator.next()) {

			map<int, vector<int> >::iterator _stepVertexIterator = hyperGraph->getVertexAddressByVertexId(t->v)->labelVertexList.find(hyperVertex->label);
			if(_stepVertexIterator != hyperGraph->getVertexAddressByVertexId(t->v)->labelVertexList.end()) {
				for(vector<int>::iterator samelabelNeighbourIndex = _stepVertexIterator->second.begin(); samelabelNeighbourIndex != _stepVertexIterator->second.end(); samelabelNeighbourIndex++){
					if (hyperGraph->edge(hyperVertex->id, *samelabelNeighbourIndex)) {
						continue;
					}
					if (hyperVertex->id == *samelabelNeighbourIndex) {
						continue;
					}
					if(isSyntacticContainment(hyperVertex->id, *samelabelNeighbourIndex)){
						containmentEdges.insert(std::pair<int, int>(hyperVertex->id, *samelabelNeighbourIndex));
					}
				}
			}
		}
	}

	// print the containment edges
	for (set<std::pair<int, int> >::iterator scEdgeIter = containmentEdges.begin(); 
		scEdgeIter != containmentEdges.end(); scEdgeIter++)
	{
		fprintf(_fp, "e %d %d\n", scEdgeIter->first, scEdgeIter->second);
	}
}


/**
* v id label isCliqueFlag (vertex list) 
* isCliqueFlag
	0 means single vertex, 1 means a clique, 2 means not a clique 
*/
void GraphAdaptation::buildHyperGraphAlgorithm(FILE* _fp) 
{
	// only save one representative node into the hyoernode in memory
	std::map<int, int> hyperNode;
	int numberOfHyperVertex = 0;
	string compressedFileLine;

	bool * visitedFlag = new bool[dataGraph->getNumberOfVertexes()];
	int isCliqueFlag = 0;

	for(int i=0; i<dataGraph->getNumberOfVertexes(); i++)
    {
		visitedFlag[i] = false;
	}

	for(int dataVertexIndex = 0; dataVertexIndex < dataGraph->getNumberOfVertexes(); dataVertexIndex++) 
    {
		isCliqueFlag = false;
		if(visitedFlag[dataVertexIndex] == true){
			continue;
		}
		visitedFlag[dataVertexIndex] = true;

		AdjacenceListsGRAPH::Vertex* dataVertex = dataGraph -> getVertexAddressByVertexId(dataVertexIndex);

		compressedFileLine.clear();
		// create a hypervertex and output it 
		compressedFileLine.append("v ").append(std::to_string(static_cast<long long> (numberOfHyperVertex))).append(" ").append(std::to_string(static_cast<long long> (dataVertex->label))).append(" ");

		// iterate 1_step reacheability neighbours
		map<int, vector<int> >::iterator sameLabelNeighbourIterator = dataVertex->labelVertexList.find(dataVertex -> label);
		if(sameLabelNeighbourIterator != dataVertex->labelVertexList.end()){
			for(vector<int>::iterator samelabelNeighbourIndex = sameLabelNeighbourIterator->second.begin(); samelabelNeighbourIndex != sameLabelNeighbourIterator->second.end(); samelabelNeighbourIndex++){
				if(isSyntacticEquivalent(dataVertex->id, *samelabelNeighbourIndex)){
				 
					visitedFlag[*samelabelNeighbourIndex] = true;

					if(isCliqueFlag == 0) {
						isCliqueFlag = 1;
						// 1 represents a clique
						compressedFileLine.append("1 ");
						compressedFileLine.append(std::to_string(static_cast<long long> (dataVertexIndex))).append(" ");
					}

					compressedFileLine.append(std::to_string(static_cast<long long> (*samelabelNeighbourIndex))).append(" ");
				}
			}	
		}

		// if no nodes is found in the 1_step()
		if(isCliqueFlag == 0) {
		
			AdjacenceListsGRAPH::adjIterator adjIterator(dataGraph, dataVertex->id);
			for(AdjacenceListsGRAPH::link t = adjIterator.begin();  !adjIterator.end(); t=adjIterator.next()) {
				map<int, vector<int> >::iterator _stepVertexIterator = dataGraph->getVertexAddressByVertexId(t->v)->labelVertexList.find(dataVertex->label);
				if(_stepVertexIterator != dataGraph->getVertexAddressByVertexId(t->v)->labelVertexList.end()) {
					for(vector<int>::iterator samelabelNeighbourIndex = _stepVertexIterator->second.begin(); samelabelNeighbourIndex != _stepVertexIterator->second.end(); samelabelNeighbourIndex++){
					
						if(visitedFlag[*samelabelNeighbourIndex] == false && isSyntacticEquivalent(dataVertex->id, *samelabelNeighbourIndex)){
							visitedFlag[*samelabelNeighbourIndex] = true;

							if(isCliqueFlag == 0) {
								isCliqueFlag = 2;
								// 2 represents not a clique		
								compressedFileLine.append("2 ");
								compressedFileLine.append(std::to_string(static_cast<long long> (dataVertexIndex))).append(" ");
							}

							compressedFileLine.append(std::to_string(static_cast<long long> (*samelabelNeighbourIndex))).append(" ");
						}
					}
				}
			}
		}
		if(isCliqueFlag == 0){
			// 0 represents a single vertex		
			compressedFileLine.append("0 ");
			compressedFileLine.append(std::to_string(static_cast<long long> (dataVertexIndex))).append(" ");
		}
		// save a representative node into memory, which will be used to add edges
		hyperNode.insert(std::pair<int,int>(numberOfHyperVertex, dataVertexIndex));
		// ready to output
		compressedFileLine.append("\n");
		fprintf(_fp, "%s", compressedFileLine.c_str());
		numberOfHyperVertex++;

		// release memory
		//dataVertex->labelSet.clear();
		//dataVertex->labelVertexList.clear();
	}

	// add edges into compressed group
	for(std::map<int, int>::iterator hyperNodeIterator = hyperNode.begin(); hyperNodeIterator != hyperNode.end(); hyperNodeIterator++){
		string compressedFileLine;

		std::map<int,int>::iterator loopIterator = hyperNodeIterator;
		int hyperNodeRepre =  (*hyperNodeIterator).second;
		loopIterator ++;
		while(loopIterator !=  hyperNode.end()){
			int a = 0;
			if(dataGraph->edge(hyperNodeRepre, (*loopIterator).second)){
				compressedFileLine.clear();
				// add the hypernode
				compressedFileLine.append("e ")
								  .append(std::to_string(static_cast<long long> (hyperNodeIterator->first)))
								  .append(" ")
								  .append(std::to_string(static_cast<long long> (loopIterator->first)))
								  .append(" 0").append("\n");
				fprintf(_fp, "%s", compressedFileLine.c_str());
			}
			loopIterator++;
		}
	}

	delete [] visitedFlag;

}

bool GraphAdaptation::isSyntacticContainment(int v, int u){
	AdjacenceListsGRAPH::Vertex* vVertex = hyperGraph->getVertexAddressByVertexId(v);
	AdjacenceListsGRAPH::Vertex* uVertex = hyperGraph->getVertexAddressByVertexId(u);

	if(vVertex->inDegree <= uVertex->inDegree){
		return false;
	}

	if(vVertex->labelSet.size() < uVertex->labelSet.size()){
		return false;
	}

	for(map<int, vector<int> >::iterator ulabelIndex = uVertex->labelVertexList.begin(); ulabelIndex != uVertex->labelVertexList.end(); ulabelIndex++){
		map<int, vector<int> >::iterator vLabelIndex = vVertex->labelVertexList.find(ulabelIndex->first);
		if(vLabelIndex ==  vVertex->labelVertexList.end()){
			return false;
		}else if(vLabelIndex->second.size() < ulabelIndex->second.size()){
			return false;
		}
		else {
			vector<int>::iterator vVertexNeighbourIterator = vLabelIndex->second.begin();
			for(vector<int>::iterator uVertexNeighbourIterator = ulabelIndex->second.begin(); uVertexNeighbourIterator != ulabelIndex->second.end(); uVertexNeighbourIterator++){
				if(*uVertexNeighbourIterator == vVertex->id){
					continue;
				}
				while(*vVertexNeighbourIterator != *uVertexNeighbourIterator){
					vVertexNeighbourIterator ++;
					if(vVertexNeighbourIterator == vLabelIndex->second.end()){
						return false;
					}
				}
			}
		}
	}
	return true;
}


bool GraphAdaptation::isSyntacticEquivalent(int v, int u)
{
	AdjacenceListsGRAPH::Vertex* vVertex = dataGraph->getVertexAddressByVertexId(v);
	AdjacenceListsGRAPH::Vertex* uVertex = dataGraph->getVertexAddressByVertexId(u);
	
	if(vVertex->inDegree != uVertex->inDegree){
		return false;
	}

	if(vVertex->labelSet.size() != uVertex->labelSet.size()){
		return false;
	}

	for(map<int, vector<int> >::iterator labelIndex = vVertex->labelVertexList.begin(); labelIndex != vVertex->labelVertexList.end(); labelIndex++){
		map<int, vector<int> >::iterator uVertexIterator = uVertex->labelVertexList.find(labelIndex->first);
		if(uVertexIterator ==  uVertex->labelVertexList.end())
			return false;
		if(labelIndex -> first == vVertex->label){
			if(labelIndex->second.size() != uVertexIterator->second.size()){
				return false;
			}else{
				for(int i=0,j=0;i < labelIndex->second.size() && j < uVertexIterator->second.size(); ){
					if(labelIndex->second[i] == uVertex->id){
						i++;
					}
					else if(uVertexIterator->second[j] == vVertex->id){
						j++;
					}
					else if(labelIndex->second[i] != uVertex->id && uVertexIterator->second[j] != vVertex->id){
						if(labelIndex->second[i] != uVertexIterator->second[j]){
							return false;
						}
						i++;
						j++;
					}
				}
			}
		}else{
			if(uVertexIterator->second != labelIndex->second)
				return false;
		}
	}
	
	return true;	
}


void GraphAdaptation::ouputStatistics() 
{
	long numberOfOriginalVertices = 0;
	long numberOfHyperVertices = 0;
	long numberOfOriginalEgdes = 0;
	long numberOfHyperEgdes = 0;
	long numberOfContainmentRoots = 0;
	long numberOfContainmentEdges = 0;

	// build data graph label vertex label vertex index
	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		numberOfOriginalVertices = dataGraphVector[dataGraphIndex].getNumberOfVertexes();
		numberOfOriginalEgdes = dataGraphVector[dataGraphIndex].getNumberOfEdges();
	}

	for(int hyperGraphIndex = 0; hyperGraphIndex < hyperGraphVector.size(); hyperGraphIndex ++) {
		numberOfHyperVertices = hyperGraphVector[hyperGraphIndex].getNumberOfVertexes();
		numberOfHyperEgdes = hyperGraphVector[hyperGraphIndex].getNumberOfEdges();
	}

	std::ifstream inputContainmentGraphFile(outputContainmentGraphName);
	AdjacenceListsGRAPH_BOOST::loadContainmentGraph(hyperGraphVector, containmentGraphVector, inputContainmentGraphFile);
	AdjacenceListsGRAPH_BOOST::buildLabelRootMap(containmentGraphVector);

	for(int containmentGraphindex = 0; containmentGraphindex < containmentGraphVector.size(); containmentGraphindex ++) {
		map<int, vector<int> >::iterator rootListIterator = containmentGraphVector[containmentGraphindex].getLabelVertexList()->begin();
		for(; rootListIterator != containmentGraphVector[containmentGraphindex].getLabelVertexList()->end(); rootListIterator ++){
			numberOfContainmentRoots += rootListIterator->second.size();
		}
		numberOfContainmentEdges += containmentGraphVector[containmentGraphindex].getNumberOfEdges();
	}

	//cout<<"numberOfOriginalVertices: "<<numberOfOriginalVertices<<endl;
    //cout<<"numberOfHyperVertices: "<<numberOfHyperVertices<<endl;
    //cout<<"numberOfOriginalEgdes: "<<numberOfOriginalEgdes<<endl;
    //cout<<"numberOfOriginalVertices: "<<numberOfOriginalVertices<<endl;

	//compute the containment ratio
	//cout<<"Size ratio: "<< (float)(numberOfHyperVertices + numberOfHyperEgdes + numberOfContainmentEdges)/ (numberOfOriginalVertices + numberOfOriginalEgdes)<<endl;
	//cout<<"SC Ratio: "<<100 - (float) numberOfContainmentRoots / numberOfHyperVertices * 100<<endl;
	//cout<<"Vertices ratio: "<<100 - (float) numberOfHyperVertices / numberOfOriginalVertices * 100<<endl;
}


