

#ifndef _MATCH_MATCH_H
#define _MATCH_MATCH_H

#include "../util/Util.h"
#include "../graph/Graph.h"
#include "../io/IO.h"
#include "../seq/Seq.h"
#include "../hyper.h"

#include "../../control.h"

using namespace std;

class Match
{
public:
    std::vector<bool> data_v_visited;

    FILE* ofp;
    HyperGraph* gsh;
    Graph* query_graph_ptr;
    Graph* data_graph_ptr;
    SEQ seq;
    IO* io_ptr;
    int query_v_num;
    int data_v_num;  //we use this to represent the num of vertices in hyper graph
    //int hsize;   //num of vertices in hyper graph
    //entry id to data id
    int* matched_data_vid_arr;
    int* data_v_visited_arr;
    // by ascending order
    vector< vector<int> >matched_graphs;
    vector<int> ascend_vid_vec;

	//int numofembeddings, num_recursive_call;
	static const int MATCH_NUM_BOUND = 5000;


    Match(Graph* _query_graph_ptr, Graph* _data_graph_ptr,IO* _io_ptr, HyperGraph* _gsh);
    ~Match();
    bool gen_seq();
    int weight_q_graph_first_eid(vector<int>& _min_weight_edges_id,
    Weight_Graph* _weight_graph_ptr);
    //void initialize_weight_v_degree(Weight_Graph* _weight_graph_ptr);
    int random_select_int(vector<int>* _vec);
    vector<Candidate_Edge> seq_get_candidate_edges(Weight_Graph* _weight_query_graph_ptr);
    bool find_vid_in_seq(int _vid);
    int get_entry_id_from_vid(int _vid);
    Candidate_Edge select_candidate_edge(vector<Candidate_Edge> _candidate_edges, 
    Weight_Graph* _weight_graph_ptr);
    int count_induced_graph_edges_num(int _another_vid, Weight_Graph* _weight_graph_ptr);
    bool is_ind_g_vertex(int _vid, int _another_ind_vid);
    bool quicksi(int _depth);
    bool data_edge_exist(int vid_0,int vid_1);
    bool extra_edge_restriction(int _data_vid,vector<int>* _end_vid_vec_ptr);
    void match(FILE* ofp);
    bool find_duplicate_res();
    bool vector_duplicate(vector<int>* _vec_a_ptr,vector<int>* _vec_b_ptr);
    vector<int> arr_to_ascend_vec(int* _arr);
    void try_res_output();
    void clear_int_vec(vector<int>* _int_vec_ptr);
    void print_cand_edges(vector<Candidate_Edge>* _cand_edge_vec_ptr);
    void print_candidate_edge(Candidate_Edge* _cand_edge_ptr);


private:

};

#endif

