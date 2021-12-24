#include"AdjacenceListsGraph.h"
#include<iostream>
#include<set>
#include <algorithm>    // std::sort
#include"Utility.h"



AdjacenceListsGRAPH::AdjacenceListsGRAPH(int numberOfVertex, bool digraph):numberOfVertex(numberOfVertex),digraph(digraph){
	numberOfEdges = 0;
	numberOfVertex = 0;
}

AdjacenceListsGRAPH::AdjacenceListsGRAPH(bool isdigraph):digraph(isdigraph){
	numberOfEdges = 0;
	numberOfVertex = 0;
}

AdjacenceListsGRAPH::AdjacenceListsGRAPH() {
	digraph = false;
	numberOfEdges = 0;
	numberOfVertex = 0;
	isHyperGraph = false;
}


void AdjacenceListsGRAPH::releaseMemory() {
	// TODO
}

std::vector<AdjacenceListsGRAPH::Edge> *  AdjacenceListsGRAPH::getEdgeList(){
	return & edgeList;
}

std::vector<AdjacenceListsGRAPH::Vertex> * AdjacenceListsGRAPH::getVertexList(){
	return & vertexList;
}

/**
* Each label following the vertices whose label is this label
*/
void AdjacenceListsGRAPH::buildLabelVertexList(){
	for(std::vector<AdjacenceListsGRAPH::Vertex>::iterator vertexIterator = vertexList.begin(); vertexIterator != vertexList.end(); vertexIterator++){
		std::map<int,std::vector<int> >::iterator labelVertexListIterator = labelVertexList.find(vertexIterator->label);
		if(labelVertexListIterator != labelVertexList.end()){
			// label already exists
			(labelVertexListIterator -> second).push_back(vertexIterator -> id);
		}
		else{
			// a new label
			labelVertexList.insert(std::pair<int, std::vector<int> >(vertexIterator->label,std::vector<int>()));
			labelVertexList.find(vertexIterator->label)->second.push_back(vertexIterator -> id);
		}
	}
}

/**
* For each vertex, each label of its neighbour, following the neighbour vertices whose label is this label
*/
void AdjacenceListsGRAPH::buildVertexLabelVertexList(){

	std::map<int,std::vector<int> >::iterator vertexLabelVertexIterator;

	for(std::vector<AdjacenceListsGRAPH::Vertex>::iterator vertexIterator = vertexList.begin(); vertexIterator != vertexList.end(); vertexIterator++){
		
		AdjacenceListsGRAPH::node* adjVertex = vertexIterator->adj;
		while(adjVertex != 0){
			vertexLabelVertexIterator = vertexIterator->labelVertexList.find(vertexList[adjVertex->v].label);
			if(vertexLabelVertexIterator != vertexIterator->labelVertexList.end()){
				vertexLabelVertexIterator->second.push_back(adjVertex->v);
			}else{
				// a new label
				std::vector<int> newVertexLabelList;
				newVertexLabelList.push_back(adjVertex->v);
				vertexIterator->labelVertexList.insert(std::pair<int,std::vector<int> >(vertexList[adjVertex->v].label,newVertexLabelList));
				//insert into labelset
				vertexIterator->labelSet.insert(vertexList[adjVertex->v].label);
			}
			adjVertex = adjVertex->next;
		}
	
	}

}

std::map<int,std::vector<int> > * AdjacenceListsGRAPH::getLabelVertexList(){
	return & labelVertexList;
}

bool AdjacenceListsGRAPH::directed() const{
	return digraph;
}

void AdjacenceListsGRAPH::insert(Vertex v)
{
	vertexList.push_back(v);
	labelSet.insert(v.label); // insert the label into the labelset
	numberOfVertex ++;
}

int AdjacenceListsGRAPH::degree(int v) 
{
	if(!digraph){
		return vertexList[v].inDegree; // indegree is the same as outdegree
	}else{
		return vertexList[v].inDegree + vertexList[v].outDegree;
	}
}


/*
// random order
void AdjacenceListsGRAPH::insert(Edge e){
	int v = e.source, w = e.destination, l = e.label;
	vertexList[v].adj = new node(w, vertexList[v].adj);
	vertexList[v].outDegree++;
	vertexList[w].inDegree++;
	if(!digraph) {
		vertexList[w].adj = new node(v, vertexList[w].adj);
		vertexList[w].outDegree++;
		vertexList[v].inDegree++;
	}
	numberOfEdges ++;

	edgeList.push_back(e);
}*/

//For the performance, we put the adjacence list in an ascending order
void AdjacenceListsGRAPH::insert(Edge e) {
	int v = e.source, w = e.destination, l = e.label;
	
	if(vertexList[v].adj == 0){
		vertexList[v].adj = new AdjacenceListsGRAPH::node(w, 0);
	}
	else {
		insert_order_helper(&vertexList[v], w);
	}

	vertexList[v].outDegree++;
	vertexList[w].inDegree++;
	vertexList[w].spongeIndegree++;
    //NOTICE: each undirected edge is treated as two dirceted edges
	if(!digraph) {

		if(vertexList[w].adj == 0) {
			vertexList[w].adj = new AdjacenceListsGRAPH::node(v, 0);
		}
		else {
			insert_order_helper(&vertexList[w], v);
		}
		vertexList[w].outDegree++;
		vertexList[v].inDegree++;
		vertexList[v].spongeIndegree++;
	}
	numberOfEdges ++;

	edgeList.push_back(e);
}

void AdjacenceListsGRAPH::insert_order_helper(AdjacenceListsGRAPH::Vertex* vertex, int  w){

	AdjacenceListsGRAPH::link looper = vertex->adj;
	AdjacenceListsGRAPH::link lastScannedNode = vertex->adj;
	
	if(looper->v > w){
		vertex->adj = new AdjacenceListsGRAPH::node(w, vertex->adj);
		return ;
	}else{
		looper = looper->next;
	}

	while(looper != 0 && looper->v < w){
		looper = looper->next;
		lastScannedNode = lastScannedNode -> next;
	}
	
	lastScannedNode->next = new AdjacenceListsGRAPH::node(w, lastScannedNode->next);

}




void AdjacenceListsGRAPH::remove(Edge){
	// stuff
}

int AdjacenceListsGRAPH::getNumberOfVertexes() const {
	return numberOfVertex;
}
int AdjacenceListsGRAPH::getNumberOfEdges() const {
	return numberOfEdges;
}

void AdjacenceListsGRAPH::clear(){
	numberOfVertex = 0;
	numberOfEdges = 0;
	vertexList.clear();
}

bool AdjacenceListsGRAPH::edge(int v, int w){
	link adjacenceNode = vertexList[v].adj;
	while(adjacenceNode != 0){
		if((*adjacenceNode).v == w)
			return true;
		adjacenceNode = adjacenceNode->next;
	}
	return false;
}


AdjacenceListsGRAPH::adjIterator::adjIterator(const AdjacenceListsGRAPH* GRef, int v):G(GRef),v(v){
	t=0;
}
AdjacenceListsGRAPH::link AdjacenceListsGRAPH::adjIterator::begin(){
	t = G->vertexList[v].adj;
	return t?t:0;
}
AdjacenceListsGRAPH::link AdjacenceListsGRAPH::adjIterator::next(){
	if(t) t = t->next; return t?t:0;
}
bool AdjacenceListsGRAPH::adjIterator::end() {
	if(t==0) return true;
	else return false;
}

AdjacenceListsGRAPH::Vertex AdjacenceListsGRAPH::getVertexByVertexId(int vertexId){
	return vertexList[vertexId];
}


AdjacenceListsGRAPH::Vertex* AdjacenceListsGRAPH::getVertexAddressByVertexId(int vertexId){
	return &vertexList[vertexId];
}

std::set<int>* AdjacenceListsGRAPH::getLabelSet(){
	return &labelSet;
}

