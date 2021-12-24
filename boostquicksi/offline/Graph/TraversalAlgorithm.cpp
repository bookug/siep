#include"TraversalAlgorithm.h"


vector<TreeNode> TraversalAlgorithm::BFS_VertexSequence(const AdjacenceListsGRAPH* graph, int startVertexId){
	vector<TreeNode> BFS_sequence;
	bool* flags = new bool[graph->getNumberOfVertexes()]();
	int parentId = -1;
	queue<int> Q;

	Q.push(startVertexId);
	BFS_sequence.push_back(TreeNode(startVertexId,-1));
	flags[startVertexId] = true;
	

	while(!Q.empty()){

		AdjacenceListsGRAPH::adjIterator vertexIterator(graph, Q.front());
		parentId = Q.front();
		Q.pop();

		for(AdjacenceListsGRAPH::link t = vertexIterator.begin(); !vertexIterator.end(); t = vertexIterator.next()){
			if(flags[t->v] == true)
				continue;
			else{
				Q.push(t->v);
				BFS_sequence.push_back(TreeNode(t->v,parentId));
				flags[t->v] = true;
			}
		}
	}
	return  BFS_sequence;
}

vector<TreeNode> TraversalAlgorithm::DFS_VertexSequence(const AdjacenceListsGRAPH* graph, int startVertexId){

	vector<TreeNode> DFS_sequence;
	bool* flags = new bool[graph->getNumberOfVertexes()]();
	stack<std::pair<int,int> > S; // id, parentId
	int id, parentId;

	S.push(std::pair<int,int>(startVertexId,-1));

	while(!S.empty()){

		id = S.top().first;
		parentId = S.top().second;
		S.pop();
		if(!flags[id]){
			flags[id] = true;
			DFS_sequence.push_back(TreeNode(id, parentId));
			AdjacenceListsGRAPH::adjIterator vertexIterator(graph, id);

			for(AdjacenceListsGRAPH::link t = vertexIterator.begin(); !vertexIterator.end(); t = vertexIterator.next()){
				if(!flags[t->v]) 
					S.push(std::pair<int,int>(t->v,id));
			}
		}

	}
	return DFS_sequence;
}

