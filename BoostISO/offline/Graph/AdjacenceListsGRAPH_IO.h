#ifndef ADJACENCE_LISTS_GRAPH_IO
#define ADJACENCE_LISTS_GRAPH_IO

#include "../../util/Util.h"
#include "AdjacenceListsGraph.h"
#include "../CommonUtility/StringUtility.h"


class AdjacenceListsGRAPH_IO{
public:
	static void show(const AdjacenceListsGRAPH*);
	static void scanEZ(AdjacenceListsGRAPH*);
	static void scan(AdjacenceListsGRAPH*);
	static void loadGraphFromFile(std::string graphFile, std::vector<AdjacenceListsGRAPH> & graphList);
};

#endif
