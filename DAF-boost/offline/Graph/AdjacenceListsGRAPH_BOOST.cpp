#include"AdjacenceListsGRAPH_BOOST.h"
#include<algorithm>
#include<fstream>
#include<set>
#include<vector>

using namespace std;

// Global Variables, Only should be used as temparary container
std::map<int,AdjacenceListsGRAPH_BOOST::DTableUnit *>::iterator d_table_iterator;

int isEquivalent(vector<int> & firstVertexList, vector<int> & secondVertexList, AdjacenceListsGRAPH::Vertex * vVertex, AdjacenceListsGRAPH::Vertex * uVertex);

int isContainment(vector<int> & bigVertexList, vector<int> & smallVertexList, AdjacenceListsGRAPH::Vertex * v, AdjacenceListsGRAPH::Vertex * u);


/**
* this function is based on that the adjacentce vertices have already been ordered
* this function is used after the DLF filter has been applied over the v and w vertices
* @return
*	0. is not a equivalent 
*   1. is query dependent equivalent and a clique
*   2. is query dependent equivalent but not clique
*/
int AdjacenceListsGRAPH_BOOST::isEquivalentRelation(AdjacenceListsGRAPH* dataGraph, AdjacenceListsGRAPH* queryGraph, int v, int w, int queryVertexId) { 

	AdjacenceListsGRAPH::Vertex* vVertex = dataGraph->getVertexAddressByVertexId(v);
	AdjacenceListsGRAPH::Vertex* wVertex = dataGraph->getVertexAddressByVertexId(w);
	int relationFlag = 0;

	if(vVertex->isClique + wVertex->isClique == 3){
		// they cannot be equivalent
		return 0;
	}
	if(dataGraph->edge(v, w)){
		relationFlag = 1;

		if(vVertex->isClique + wVertex->isClique == 2){
			return 0;
		}
	}else{
		if(vVertex->isClique + wVertex->isClique == 1){
			return 0;
		}
	}

	map<int, vector<int> >::iterator vVertexNeighbourIterator;
	map<int, vector<int> >::iterator wVertexNeighbourIterator;
	
	std::set<int>* labelSet = &(queryGraph->getVertexAddressByVertexId(queryVertexId)->labelSet);	

	for(std::set<int>::iterator queryLabelIterator = labelSet->begin(); queryLabelIterator!= labelSet->end(); queryLabelIterator++) {
		
		vVertexNeighbourIterator = vVertex->labelVertexList.find(*queryLabelIterator);
		wVertexNeighbourIterator = wVertex->labelVertexList.find(*queryLabelIterator);

		if((vVertexNeighbourIterator == vVertex->labelVertexList.end()) && (wVertexNeighbourIterator == wVertex->labelVertexList.end())){
			continue;
		}else if((vVertexNeighbourIterator == vVertex->labelVertexList.end()) ^ (wVertexNeighbourIterator == wVertex->labelVertexList.end())){
			return 0;
		}

		if(isEquivalent(vVertexNeighbourIterator->second, wVertexNeighbourIterator->second, vVertex, wVertex) == 0){
			return 0;
		}
	}
	
	if(relationFlag == 1){
		return 1;
	}else{
		return 2;
	}

}

/**
* this function is based on that the adjacentce vertices have already been ordered
*
* @return
*	0. is not a relation 
*	1. v query dependent contains w
*   2. w query dependent contains v
*/
int AdjacenceListsGRAPH_BOOST::isContainmentRelation(AdjacenceListsGRAPH* dataGraph, AdjacenceListsGRAPH* queryGraph, int v, int w, int queryVertexId){ 

	AdjacenceListsGRAPH::Vertex* vVertex = dataGraph->getVertexAddressByVertexId(v);
	AdjacenceListsGRAPH::Vertex* wVertex = dataGraph->getVertexAddressByVertexId(w);

	map<int, vector<int> >::iterator vVertexNeighbourIterator;
	map<int, vector<int> >::iterator wVertexNeighbourIterator;
	
	std::set<int>* labelSet = &(queryGraph->getVertexAddressByVertexId(queryVertexId)->labelSet);	
	int relationFlag = 0;

	for(std::set<int>::iterator queryLabelIterator = labelSet->begin(); queryLabelIterator!= labelSet->end(); queryLabelIterator++) {
		
		vVertexNeighbourIterator = vVertex->labelVertexList.find(*queryLabelIterator);
		wVertexNeighbourIterator = wVertex->labelVertexList.find(*queryLabelIterator);

		if((vVertexNeighbourIterator == vVertex->labelVertexList.end()) && (wVertexNeighbourIterator == wVertex->labelVertexList.end())){
			continue;
		}else if((vVertexNeighbourIterator == vVertex->labelVertexList.end()) ^ (wVertexNeighbourIterator == wVertex->labelVertexList.end())){
			return 0;
		}

		if(vVertexNeighbourIterator->second.size() > wVertexNeighbourIterator->second.size()){
			if(relationFlag == 2){
				return 0;
			}
			if(isContainment(vVertexNeighbourIterator->second, wVertexNeighbourIterator->second, vVertex, wVertex) == 0){
				return 0;
			}else{
				relationFlag = 1;
			}
		}else if(vVertexNeighbourIterator->second.size() < wVertexNeighbourIterator->second.size()){
			if(relationFlag == 1){
				return 0;
			}
			if(isContainment(wVertexNeighbourIterator->second, vVertexNeighbourIterator->second, wVertex, vVertex) == 0){
				return 0;
			}else{
				relationFlag = 2;
			}
		}else{
			if(isContainment(wVertexNeighbourIterator->second, vVertexNeighbourIterator->second, wVertex, vVertex) == 0){
				return 0;
			}
		}
	}

	return relationFlag;

}

/**
* 0 means they are not equivalent 
* 1 means they are equivalent 
*/
int isEquivalent(vector<int> & firstVertexList, vector<int> & secondVertexList, AdjacenceListsGRAPH::Vertex * vVertex, AdjacenceListsGRAPH::Vertex * uVertex){
	for(int i=0,j=0;i < firstVertexList.size() && j < secondVertexList.size(); ) {
		if(firstVertexList[i] == uVertex->id){
			i++;
		}
		else if(secondVertexList[j] == vVertex->id) {
			j++;
		}
		else if(firstVertexList[i] != uVertex->id && secondVertexList[j] != vVertex->id){
			if(firstVertexList[i] != secondVertexList[j]){
				return 0;
			}
			i++;
			j++;
		}
	}
	return 1;
}


/**
* 0 means first does not contains the second
* 1 means first does contains the second
*/
int isContainment(vector<int> & bigVertexList, vector<int> & smallVertexList, AdjacenceListsGRAPH::Vertex * vVertex, AdjacenceListsGRAPH::Vertex * uVertex){
	vector<int>::iterator vVertexNeighbourIterator = bigVertexList.begin();
	for(vector<int>::iterator uVertexNeighbourIterator = smallVertexList.begin(); uVertexNeighbourIterator != smallVertexList.end(); uVertexNeighbourIterator++){
		if(*uVertexNeighbourIterator == vVertex->id){
				continue;
		}
		while(*vVertexNeighbourIterator != *uVertexNeighbourIterator){
			vVertexNeighbourIterator ++;
			if(vVertexNeighbourIterator == bigVertexList.end()){
				return 0;
			}
		}
	}
	return 1;
}


bool AdjacenceListsGRAPH_BOOST::degreeFilter(AdjacenceListsGRAPH* queryGraph, AdjacenceListsGRAPH* dataGraph, int u, int v){

	AdjacenceListsGRAPH::Vertex * queryVertexU = queryGraph -> getVertexAddressByVertexId(u);
	AdjacenceListsGRAPH::Vertex * dataVertexV = dataGraph -> getVertexAddressByVertexId(v);
	
	std::set<int>* queryLabelSet = &queryVertexU->labelSet;
	std::set<int>::iterator setIterator = queryLabelSet->begin();

	vector<int>::iterator neighbourIterator;
	int count;

	std::map<int,std::vector<int> >::iterator dataLabelVertexIterator;


	for(; setIterator!= queryLabelSet->end(); setIterator++) {

		dataLabelVertexIterator = dataVertexV->labelVertexList.find(*setIterator);

		if(*setIterator == dataVertexV->label){
			count = 0;
			count += dataVertexV->vertexList.size() - 1;
			if(dataLabelVertexIterator ==  dataVertexV->labelVertexList.end()){
				if(count < queryVertexU->labelVertexList.find(*setIterator)->second.size())
					return false;
			}else {
				for(neighbourIterator = dataLabelVertexIterator->second.begin(); neighbourIterator != dataLabelVertexIterator->second.end(); neighbourIterator++){
					count += dataGraph->getVertexAddressByVertexId(*neighbourIterator)->vertexList.size();
				}
				if(count < queryVertexU->labelVertexList.find(*setIterator)->second.size())
					return false;	
			}
		}else{
			if(dataLabelVertexIterator ==  dataVertexV->labelVertexList.end()){
				return false;
			}else{
				count = 0;
				for(neighbourIterator = dataLabelVertexIterator->second.begin(); neighbourIterator != dataLabelVertexIterator->second.end(); neighbourIterator++){
					count += dataGraph->getVertexAddressByVertexId(*neighbourIterator)->vertexList.size();
				}
				if(count < queryVertexU->labelVertexList.find(*setIterator)->second.size())
					return false;
			}	
		}
	}

	return true;
	
}


/**
* compute the dynamic containment table and hypervertexCandidates
*/
void AdjacenceListsGRAPH_BOOST::computeDTable(AdjacenceListsGRAPH* queryGraph, AdjacenceListsGRAPH* dataGraph, std::map<int, vector<int>* >& candidates, std::map<int,std::vector<DTableUnit*> *> & hyperCandidateSetsList, std::map<int,DTableUnit*>& d_table ) {

	std::map<int,std::vector<DTableUnit*> *> tempHyperCandidateSetsList;
	DTableUnit* newDTableUnit;
	vector<DTableUnit*>* hyperCandidateVertexList;
	int relationFlag; //only used in the temporary value of isQdeEuqal()
	vector<int>::iterator candidateVertexVectorLooper; // used in computeHyperCandidateSets
	vector<DTableUnit*>::iterator candidateSuperVertexVectorLooper; // used in computeHyperCandidateSets

	for(std::map<int, vector<int>* >::iterator candidateSetIterator = candidates.begin(); candidateSetIterator != candidates.end(); candidateSetIterator ++) {

		int * visitedFlags = new int[candidateSetIterator->second->size()];
		for(int i=0;i<candidateSetIterator->second->size();i++) {
			visitedFlags[i] = 0;
		}

		// for each candidate set , create a <queryVertexId,hyperVertexlist>
		tempHyperCandidateSetsList.insert(std::pair<int,vector<DTableUnit*>*>(candidateSetIterator->first, new vector<DTableUnit*>()));
		hyperCandidateVertexList = tempHyperCandidateSetsList.find(candidateSetIterator->first)->second;
	
		//**Used Global Variables 
		for(vector<int>::iterator candidateIterator = candidateSetIterator->second->begin(); candidateIterator != candidateSetIterator->second->end(); candidateIterator++) {
			
			if(visitedFlags[candidateIterator - candidateSetIterator->second->begin()] == -1) {
				continue;
			}else{
				visitedFlags[candidateIterator - candidateSetIterator->second->begin()] = -1; // mark as visited
				//TODO? currently we incorporate the degree filter at here
				if(!AdjacenceListsGRAPH_BOOST::degreeFilter(queryGraph, dataGraph, candidateSetIterator->first, *candidateIterator)){
					continue;
				}
			}

			hyperCandidateVertexList->push_back(new DTableUnit(dataGraph->getVertexAddressByVertexId(*candidateIterator)));
			newDTableUnit = *(hyperCandidateVertexList->rbegin());
			newDTableUnit->totalNumberOfEquivalentVertices += newDTableUnit->flagVertex->vertexList.size();
			//refresh the d-table
			d_table.insert(std::pair<int,DTableUnit*>(*candidateIterator, newDTableUnit));

			candidateVertexVectorLooper = candidateIterator + 1;
			while(candidateVertexVectorLooper != candidateSetIterator->second->end() 
				  && visitedFlags[candidateVertexVectorLooper - candidateSetIterator->second->begin()] != -1) {

					if(!AdjacenceListsGRAPH_BOOST::degreeFilter(queryGraph, dataGraph, candidateSetIterator->first, *candidateVertexVectorLooper)){
						visitedFlags[candidateVertexVectorLooper - candidateSetIterator->second->begin()] = -1; // mark as visited

					}else{
						relationFlag = AdjacenceListsGRAPH_BOOST::isEquivalentRelation(dataGraph,queryGraph,*candidateIterator,*candidateVertexVectorLooper, candidateSetIterator->first);
						if(relationFlag != 0){
							visitedFlags[candidateVertexVectorLooper - candidateSetIterator->second->begin()] = -1;
							newDTableUnit->flagClique = relationFlag;
							newDTableUnit->equivalentVerticeList->push_back(dataGraph->getVertexAddressByVertexId(*candidateVertexVectorLooper));
							newDTableUnit->totalNumberOfEquivalentVertices += dataGraph->getVertexAddressByVertexId((*candidateVertexVectorLooper))->vertexList.size();
						}
					}

					candidateVertexVectorLooper ++;
			}
			
		}
		delete [] visitedFlags; // will this slow down speed ?
	}

	for(std::map<int,std::vector<DTableUnit*> *>::iterator hyperCandidateSetIterator = tempHyperCandidateSetsList.begin(); hyperCandidateSetIterator != tempHyperCandidateSetsList.end(); hyperCandidateSetIterator++){
		
		int * visitedFlags  = new int[hyperCandidateSetIterator->second->size()];
		for(int i=0;i<hyperCandidateSetIterator->second->size();i++){
			visitedFlags[i] = 0;
		}
		//**Used Global Variables 
		for(vector<DTableUnit*>::iterator candidateIterator = hyperCandidateSetIterator->second->begin(); candidateIterator != hyperCandidateSetIterator->second->end(); candidateIterator++) {
			if(visitedFlags[candidateIterator - hyperCandidateSetIterator->second->begin()] == -1) {
				continue;
			}else{
				visitedFlags[candidateIterator - hyperCandidateSetIterator->second->begin()] = -1; // mark as visited
				candidateSuperVertexVectorLooper = candidateIterator + 1;
				while(candidateSuperVertexVectorLooper != hyperCandidateSetIterator->second->end() 
					  && visitedFlags[candidateSuperVertexVectorLooper - hyperCandidateSetIterator->second->begin()] != -1) {

						  relationFlag = AdjacenceListsGRAPH_BOOST::isContainmentRelation(dataGraph,queryGraph,(*candidateIterator)->flagVertex->id,(*candidateSuperVertexVectorLooper)->flagVertex->id, hyperCandidateSetIterator->first);
							if(relationFlag == 1){

								d_table.find((*candidateIterator)->flagVertex->id)->second->containementChildList->push_back(*candidateSuperVertexVectorLooper);
								d_table_iterator = d_table.find((*candidateSuperVertexVectorLooper)->flagVertex->id);
								d_table_iterator->second->inDegree ++;
								d_table_iterator->second->spongeInDegree ++;

							}else if(relationFlag == 2){
								d_table.find((*candidateSuperVertexVectorLooper)->flagVertex->id)->second->containementChildList->push_back(*candidateIterator);
								d_table_iterator = d_table.find((*candidateIterator)->flagVertex->id);
								d_table_iterator->second->inDegree ++;
								d_table_iterator->second->spongeInDegree ++;
							}
						
							candidateSuperVertexVectorLooper ++;
					}
				}
			}


		// for each candidate set , create a <queryVertexId,hyperVertexlist>
		hyperCandidateSetsList.insert(std::pair<int,vector<DTableUnit*>*>(hyperCandidateSetIterator->first, new vector<DTableUnit*>()));
		hyperCandidateVertexList = hyperCandidateSetsList.find(hyperCandidateSetIterator->first)->second;
		
		for(vector<DTableUnit*>::iterator candidateIterator = hyperCandidateSetIterator->second->begin(); candidateIterator != hyperCandidateSetIterator->second->end(); candidateIterator++) {		
			if(d_table.find((*candidateIterator)->flagVertex->id)->second->inDegree == 0){
				hyperCandidateVertexList->push_back((*candidateIterator));
			}	
		}

		delete [] visitedFlags; // will this slow down speed ?
	}

}

/**
* build the 0 degree label vertex list, it will be saved into the LabelVertexList of the containment graph
*/
void AdjacenceListsGRAPH_BOOST::buildLabelRootMap(vector<AdjacenceListsGRAPH> & containmentGraphVector){
	// build the labelRootVertexMap
	for(int containmentGraphIndex = 0; containmentGraphIndex < containmentGraphVector.size(); containmentGraphIndex++){
		vector<AdjacenceListsGRAPH::Vertex>* vertexList = containmentGraphVector[containmentGraphIndex].getVertexList();
		std::map<int,std::vector<int> > * labelRootList = containmentGraphVector[containmentGraphIndex].getLabelVertexList();

		labelRootList->clear();
		for(std::vector<AdjacenceListsGRAPH::Vertex>::iterator vertexIterator = vertexList->begin(); vertexIterator != vertexList->end(); vertexIterator++){

			if(vertexIterator->inDegree != 0){
				continue;
			}

			std::map<int,std::vector<int> >::iterator labelVertexListIterator = labelRootList->find(vertexIterator->label);

			if(labelVertexListIterator != labelRootList->end()){
				// label already exists
				(labelVertexListIterator -> second).push_back(vertexIterator -> id);
			}
			else{
				// a new label
				labelRootList->insert(std::pair<int, std::vector<int> >(vertexIterator->label,std::vector<int>()));
				labelRootList->find(vertexIterator->label)->second.push_back(vertexIterator -> id);
			}
		}
	}	
}

/**
* load the containment graph, the containment graph edges are saved in the file, and it shares the same set of vertices with that of hyperGraph 
*/
void AdjacenceListsGRAPH_BOOST::loadContainmentGraph(vector<AdjacenceListsGRAPH> & hyperGraphVector, vector<AdjacenceListsGRAPH> & containmentGraphVector, std::ifstream & containmentGraphFile){

	for(int hyperGraphIndex = 0; hyperGraphIndex < hyperGraphVector.size(); hyperGraphIndex ++) {

		// containment graph is a directed graph
		containmentGraphVector.push_back(AdjacenceListsGRAPH(true));
		AdjacenceListsGRAPH & containmentGraph = *(containmentGraphVector.begin() + containmentGraphVector.size() - 1);

		// containment graph shares the same set of data vertices with the hyper graph
		vector<AdjacenceListsGRAPH::Vertex>* dataVertexList = hyperGraphVector[hyperGraphIndex].getVertexList();
		for(int dataVertexIndex = 0; dataVertexIndex < dataVertexList->size(); dataVertexIndex++){
			AdjacenceListsGRAPH::Vertex vertex =  AdjacenceListsGRAPH::Vertex(dataVertexIndex, (*dataVertexList)[dataVertexIndex].label);
			containmentGraph.insert(vertex);
		}
	}

	// graph data loaded into memory
	string line;
	vector<int> integerValues;
	int containmengGraphIndex = 0;
	getline (containmentGraphFile,line);
	while ( line.size() != 0 && (*line.begin()) == 't') {
		// A new graph
		AdjacenceListsGRAPH & graph = containmentGraphVector[containmengGraphIndex];
		// insert all the edges
		while(getline (containmentGraphFile,line) && (*line.begin() == 'e')) {
			String_Utility::readIntegersFromString(line, integerValues);
			AdjacenceListsGRAPH::Edge edge =  AdjacenceListsGRAPH::Edge(*integerValues.begin(),*(integerValues.begin()+1));
			graph.insert(edge);
		}
	}

}
