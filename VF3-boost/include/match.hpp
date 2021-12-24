/**
 * @file match.hpp
 * @author P. Foggia (pfoggia\@unisa.it)
 * @author V.Carletti (vcarletti\@unisa.it)
 * @date   December, 2014
 * @brief  Declaration of the match function.
 */

#ifndef MATCH_H
#define MATCH_H

#include <stack>
//#include "argraph.hpp"
#include "vf3_sub_state.hpp"

using namespace std;

typedef int data_t;
typedef unsigned int node_id; /**<Type for the id of the nodes in the graph */

extern long numembeddings, ncalls, bound;
extern double max_mem;

/**
 * @brief Definition of the match_visitor type.\n
 * A match visitor is a function that is invoked for
 * each match that has been found.
 * If the function returns FALSE, then the next match is
 * searched; else the seach process terminates.
 * @param [in] n Number of nodes.
 * @param [in] c1 Core Set of the first graph.
 * @param [in]c2 Core Set of the second graph.
 * @param [in/out] usr_data User defined parameter.
 * @return TRUE If the matching process must be stopped.
 * @return FALSE If the matching process must continue.
*/



/**
 * @brief Visits all the matchings between two graphs,  starting
 * from state s.
 * @note  c1 and c2 will contain the ids of the corresponding nodes
 * in the two graphs.
 * @param [in] s Initial State.
 * @param [out] c1 Core Set of the first graph.
 * @param [out] c2 Core Set of the second graph.
 * @param [out] pn Number of matched nodes.
 * @param [in] vis Matching visitor.
 * @param [in/out] usr_data User defined parameter for the visitor.
 * @return TRUE If if the caller must stop the visit.
 * @return FALSE If if the caller must continue the visit.
*/
bool match(VF3SubState &s) {
    if (numembeddings > bound) return false;
	ncalls++;
	if (s.core_len == s.n1) {
    	s.Output();
		return true;
	}
	node_id n1 = NULL_NODE, n2 = NULL_NODE;
	bool flag = false;
	if (s.IsDead()) return false;
	while (s.NextPair(&n1, &n2, n1, n2)){
		if (numembeddings > bound) return false;
	   	if (s.IsFeasiblePair(n1, n2)) {
			VF3SubState s1(s);	// New state
			s1.AddPair(n1, n2);
    		if (match(s1))
				flag = true;
		}
	}
	return flag;
}


#endif

