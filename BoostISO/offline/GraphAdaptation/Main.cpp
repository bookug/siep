#include "GraphAdaptation.h"
#include "../Graph/AdjacenceListsGraph.h"
#include "../CommonUtility/StringUtility.h"
#include "../CommonUtility/FileUtility.h"
#include "../CommonUtility/TimeUtility.h"

using namespace std;


/**
* take one parameter which is the graph file name
*/
int main(int argc,char * argv[]) 
{
	if(argc != 4) 
    {
		std::cout<<"File name specified error."<<endl<<"Example: GraphAdaptation sourceFilename destFilename, containmentFileName" << endl;
		return 0;
	}

    //the first parameter denotes that the input is undirected graph
	GraphAdaptation * adapter = new GraphAdaptation(false, argv[1],  argv[2], argv[3]);
	//cout<<"** 1. Done loading the data graph **"<<endl;
	
    long t1 = Util::get_cur_time();
	adapter->computeHyperGraphs(); 
    long t2 = Util::get_cur_time();
	//cout<<"** 2. Done computing the hypergraph" <<(t2-t1)<<"ms"<<endl;

	adapter->loadHyperGraphs();
    long t3 = Util::get_cur_time();
	//cout<<"** 3. Done loading the hypergraph "<<(t3-t2)<<"ms"<<endl;

	adapter->computeContainmentGraphs();
    long t4 = Util::get_cur_time();
	//cout<<"** 4. Done computing the containment graph: "<<(t4-t3)<<"ms"<<endl;

    long sumt = t4-t3 + t2-t1;
    //cout<<"match used: "<<sumt<<"ms"<<endl;
	//adapter->resultFile<<"Time cost to build "<<argv[1]<<" adaptaed graph is "<<totalTimeCost / (1000 * 60)<<" minutes!"<<endl;
	
	adapter->ouputStatistics();

	return 0;
}







