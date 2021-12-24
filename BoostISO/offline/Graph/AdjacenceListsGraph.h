#ifndef ADJACENCE_LISTS_GRAPH
#define ADJACENCE_LISTS_GRAPH

#include "../../util/Util.h"
class AdjacenceListsGRAPH{

public:

	struct node{
		int v;
		node * next;
		node(int x, node *t){v = x; next = t;};
	};

	struct Vertex {
		int id;
		int label;
		node * adj;
		int inDegree;   // if the graph is indirected graph, inDegree = outdegree
		int outDegree;  // if the graph is indirected graph, both outdegree and indegree are are the degree of the vertex
		int  spongeIndegree; // is the same as the inDegree but is used as a changable variable. 
		Vertex(int id = -1,int label = -1):id(id),label(label){ adj = 0;inDegree=0;outDegree=0; isClique = 0;spongeIndegree=0;};
		// 0. Only one vertex 1. IsClique 2. is not a clique
		int isClique;
		std::vector<int> vertexList; // if the vertex is an hyper-vertex. this is the vertex list belonging to the hypervertex 
		std::map< int,std::vector<int> > labelVertexList; // save a list of vertices for each neighbour labels
		std::set<int> labelSet;
	};
	struct Edge{
		int source,destination;
		int label;
		Edge(int sour=-1,int dest=-1, int label=-1):source(sour),destination(dest),label(label){};
	};

	typedef node* link;

	
	bool isHyperGraph;

private:
	int numberOfVertex, numberOfEdges;
	bool digraph;
	std::vector<Vertex> vertexList;
	// invert list of label -> vertexes
	std::map<int,std::vector<int> > labelVertexList;   
	std::vector<Edge> edgeList;
	// store all the distinct labels 
	std::set<int> labelSet; 
public:

	AdjacenceListsGRAPH(int numberOfVertex, bool digraph);
	AdjacenceListsGRAPH(bool digraph);
	AdjacenceListsGRAPH();

	void releaseMemory();

	bool directed() const;

	void insert(Vertex v);
	void insert_order_helper(AdjacenceListsGRAPH::Vertex* vertex, int w);

	void insert(Edge e);


	std::vector<Vertex> * getVertexList();
	std::vector<Edge>* getEdgeList();
	// build the inverted list of label->vertexes
	void buildLabelVertexList();

	// for each vertex build its labelset and for each vertex build vertex list according to the each label
	void buildVertexLabelVertexList();


	std::map<int,std::vector<int> > * getLabelVertexList();
	std::set<int>* getLabelSet();

	int getNumberOfVertexes() const;
	int getNumberOfEdges() const;

	Vertex getVertexByVertexId(int vertexId);
	Vertex* getVertexAddressByVertexId(int vertexId);

	int maxDegree();
	int degree(int);

	void remove(Edge);
	bool edge(int,int);

	class adjIterator{
		protected:
			const AdjacenceListsGRAPH *G;
			int v;
			AdjacenceListsGRAPH::link t;
		public:
			adjIterator(const AdjacenceListsGRAPH* G, int v);
			AdjacenceListsGRAPH::link begin();
			AdjacenceListsGRAPH::link next();
			bool end();
	};

	void clear();

};


#endif
