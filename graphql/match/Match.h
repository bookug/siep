

#ifndef _MATCH_MATCH_H
#define _MATCH_MATCH_H

#include "../util/Util.h"
#include "../graph/Graph.h"
#include "../io/IO.h"

#include "../../control.h"

class Match
{
public:
    std::vector<int> tmp_matched_graph;

	// int numofembeddings, bound;
	IO* io_ptr;
	int query_v_num;
	int data_v_num;
	QueryGraph* query_graph;
	DataGraph* data_graph;
	std::map< int, std::set<int> > mark_qv_dv;

	// std::vector<QueryVertex> q_vertices;
	
	bool* qv_visited_list;	
	bool* dv_visited;
	int* qv_degrees;
	//int* qv_select_seq;
	// int* dv_matched_seq;
	//int qv_select_seq_pos;

	std::map<int,int> qv2dv;
	// all matched subgraphs now
	std::vector<int*> matched_subgraphs;



	Match(QueryGraph* _query, DataGraph* _data, int n_, int b_);

	~Match();

	bool dv_qv_pre_match(DataVertex* _dv_ptr,QueryVertex* _qv_ptr);
	bool dv_qv_label_match(DataVertex* _dv,QueryVertex* _qv);
	void add_candidate(int _q_vid,int _d_vid);

	void init_candidates(int _q_vid);

	
	//void prepare_for_search();
	// void init_dv_matched_seq();
	//int construct_next_qv(bool* qv_visited);
	int get_first_qv();
	int get_next_qv();

	bool exist_query_edge(int _qv_a,int _qv_b);
	bool exist_data_edge(int _dv_a, int _dv_b);

	void search(int _start_qv,int _matched_q_num);
	bool check(int _qv,int _dv);

	bool tell_duplicate(int* _matched_graph);
	void output_subgraph();
	void get_subgraph_matched();

	void match(IO* io,int _nei_radius,int _global_refine_level);

	void add_qv_nei_profile(std::set<int>* _qv_this_hop,int _nei_radius,std::set<int>* _qv_profile,
bool* _qv_visited);
	void add_dv_nei_profile(std::set<int>* _dv_this_hop,int _nei_radius,std::set<int>* _dv_profile,
bool* _dv_visited);
	bool dv_prof_include_qv_prof(std::set<int>* _dv_profile, std::set<int>* _qv_profile);
	void qv_local_filter(int _qv,int _nei_radius);
	void local_filter(int _nei_radius);

	bool is_candidate(int _qv, int _dv);
	bool exist_equal(std::set<int>* _dv_set, std::set<int>* _cand_set);
	bool bipart_semi_perf(int _qv,int _dv);
	void mark_pair(int _qv,int _dv);
	void mark_potential(int _rt_qv,int _rt_dv);
	void global_refine_search(int _global_refine_level);
	void global_refine(int _global_refine_level);

private:

};

#endif

