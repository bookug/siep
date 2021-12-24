#include "AdjacenceListsGRAPH_IO.h"


using namespace std;


void AdjacenceListsGRAPH_IO::show(const AdjacenceListsGRAPH* G){
	for(int s=0; s < G->getNumberOfVertexes(); s++){
		std::cout.width(2);
		std::cout<< s << ":";
		AdjacenceListsGRAPH::adjIterator A(G,s);
		for(AdjacenceListsGRAPH::link t=A.begin();  !A.end(); t = A.next()){
			std::cout.width(2);
			std::cout<< t->v << " ";
		}
		std::cout << std::endl;
	}
};




void AdjacenceListsGRAPH_IO::loadGraphFromFile(string graphFile, std::vector<AdjacenceListsGRAPH> & graphList) 
{
    FILE* fp = fopen(graphFile.c_str(), "r");
	// graph data loaded into memory
    char* line = NULL;
    size_t len = 0;
	vector<int> integerValues;

    //WARN: C++ getline is very different from C getline function
    //cout<<"to read the first line"<<endl;
	getline (&line, &len, fp);
	while ( len != 0 && line[0] == 't'){
        if(strcmp(line, "t # -1\n") == 0)
        {
            //cout<<"find the end"<<endl;
            break;
        }
        //read the second line: vnum, enum, vlabelMax
	getline (&line, &len, fp);
		// A new graph
		graphList.push_back(AdjacenceListsGRAPH(false));
		AdjacenceListsGRAPH & graph = *(graphList.begin() + graphList.size() - 1);
		// insert all the nodes
		while(getline (&line, &len, fp) && line[0] == 'v')
        {
			String_Utility::readIntegersFromString(line,integerValues);
			// only consider one label per vertex
			AdjacenceListsGRAPH::Vertex vertex =  AdjacenceListsGRAPH::Vertex(integerValues[0], integerValues[1]);

			/****************************************/
			/* add the vertexList if a hyper vertex */
			/****************************************/
			if(integerValues.begin()+2 != integerValues.end())
            {
				graph.isHyperGraph = true;
				// hyper vertex starts from the second element of this vector, the first element is a flag to mark it as clique
				vertex.isClique = *(integerValues.begin()+2);
				for(vector<int>::iterator vertexListIterator = integerValues.begin() + 3; vertexListIterator != integerValues.end(); vertexListIterator++){
					vertex.vertexList.push_back(*vertexListIterator);	
				}
			}
			graph.insert(vertex);
            //cout<<"insert a vertex"<<endl;
		}
		// insert all the edges
		do{
			if (len != 0)
			{
				String_Utility::readIntegersFromString(line, integerValues);
				if (*integerValues.begin() != *(integerValues.begin() + 1)) {
					//AdjacenceListsGRAPH::Edge edge = AdjacenceListsGRAPH::Edge(integerValues[0], integerValues[1], integerValues[2]);
                    AdjacenceListsGRAPH::Edge edge = AdjacenceListsGRAPH::Edge(integerValues[0], integerValues[1], 1);
					graph.insert(edge);
                    //cout<<"insert an edge"<<endl;
				}
			}
		}while(getline(&line, &len, fp) && line[0] == 'e');
        if(strcmp(line, "t # -1\n") == 0)
        {
            //cout<<"find the end"<<endl;
            break;
        }
	}
    if(line)
    {
        xfree(line);
    }
    fclose(fp);
}





