

#ifndef _MATCH_MATCH_H
#define _MATCH_MATCH_H

#include "../util/Util.h"
#include "../graph/Graph.h"
#include "../io/IO.h"

#include "../../control.h"

using namespace std;


class Match
{
public:
    vector<int> tmp_matched_graph;
    vector<int> matched_data_v_list;
    vector<bool> data_v_visited;

	IO* io_ptr;
	int query_v_num;
	int data_v_num;
	int data_hv_num;

	QueryGraph* query_graph;
	Hyper_Data_Graph* hyper_data_graph;

	int* qv_degrees;

	std::vector< set<int> > qv_profile_vec;
	map<int, set<int> > data_hv_profile_map;

	//int* query_v_match_seq;
	//int qv_match_seq_pos;
	
	bool* qv_visited_list;
	vector<bool> hv_is_cand_list;
	vector<int> hv_ump_list;
	vector<int> hv_remaining_v_num_list;
	vector<bool> root_hv_searched_list;

	std::map<int,int> query_v_to_data_hv;

	std::map< int, std::set<int> > mark_qv_dhv;
	



	Match(QueryGraph* _query, Hyper_Data_Graph* _hyper_data_graph_ptr);

	~Match();

	bool qv_dhv_initial_cand_test(int _query_vid,int _data_hvid);
	void add_candidate(int _q_vid,int _d_vid);
	void init_candidates(int _q_vid);

	void add_qv_nei_profile(std::set<int>* _qv_this_hop,int _nei_radius,std::set<int>* _qv_profile,
		bool* _qv_visited);
	void build_qv_nei_profile(int _qv_id,int _nei_radius);
	void build_qv_nei_profile_list(int _nei_radius);
	void add_data_hv_nei_profile(set<int>* _data_hv_this_hop,int _nei_radius,set<int>* _data_hv_profile,
		bool* _dv_visited_list);
	void build_data_hv_nei_profile(int _data_hv_id, int _nei_radius);
	bool data_hv_prof_include_query_v_prof(set<int>* _data_hv_profile, set<int>* _query_v_profile);

	void qv_local_filter(int _qv,int _nei_radius);
	void local_filter(int _nei_radius);

	bool exist_equal(set<int>* _dhv_set, set<int>* _cand_set);
	bool bipart_semi_perf(int _qv,int _dv);
	bool is_candidate(int _qv, int _dhv);
	void mark_pair(int _qv,int _dhv);
	void mark_potential_pairs(int _rt_qv,int _rt_dv);
	void global_refine_search(int _global_refine_level);
	void global_refine(int _global_refine_level);

	void build_data_hv_is_cand_list(int _query_v_id);
	void build_data_hv_ump_list();
	//void construct_qv_join_seq();
	//int construct_next_join_qv(bool* qv_visited);
	int get_first_qv();
	int get_next_qv();

	void build_remaining_v_num_list();
	bool visit_data_hv(int _data_hv_id);
	void init_root_hv_searched_list();
	int dynamic_cand_load_next_root_hv();
	void matched_root_data_hv_change_ump(int _root_data_hv_id);
	//void root_dcl_search(int _start_qv);
	void output_matched_subgraph_step(int _matched_query_v_cnt);
	void output_subgraph_matched();
	void get_subgraph_matched();

	bool exist_query_edge(int _query_v_a,int _query_v_b);
	bool exist_data_edge(int _data_hv_a, int _data_hv_b);
	bool check_isjoinable(int _qv,int _dv);
	void non_dcl_search(int _start_qv,int _matched_q_num);

	
	void match(IO* io,int _nei_radius,int _global_refine_level);

	
	


	// bool tell_duplicate(int* _matched_graph);
	// void output_subgraph(int* _matched_subgraph);




	// bool is_candidate(int _qv, int _dv);
	// bool exist_equal(std::set<int>* _dv_set, std::set<int>* _cand_set);
	// bool bipart_semi_perf(int _qv,int _dv);
	// void mark_pair(int _qv,int _dv);
	// void mark_potential_pairs(int _rt_qv,int _rt_dv);
	// void global_refine_search(int _global_refine_level);
	// void global_refine(int _global_refine_level);



private:

};

#endif

