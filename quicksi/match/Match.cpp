#include "Match.h"

using namespace std;

const int INF=2e8;
extern long bound, nembeddings, ncalls;
extern double max_mem;

//#define DEBUG_PRECISE 1

Match::Match(Graph* _query_graph_ptr, Graph* _data_graph_ptr,IO* _io_ptr) {
	this->query_graph_ptr=_query_graph_ptr;
    this->data_graph_ptr=_data_graph_ptr;
    this->io_ptr=_io_ptr;
    this->query_v_num=_query_graph_ptr->vertices.size();
    this->data_v_num=_data_graph_ptr->vertices.size();
}

Match::~Match() {}


bool Match::gen_seq() {
    Weight_Graph weight_query_graph;

    map<int,int> data_v_label_cnt;
    // scan the data graph and count the number of each label
    vector<Vertex>* data_vertices_vec_ptr=&(this->data_graph_ptr->vertices);
    for(vector<Vertex>::iterator iter=data_vertices_vec_ptr->begin();
    iter!=data_vertices_vec_ptr->end();++iter) {
        int v_label=(*iter).vlabel;
        if(data_v_label_cnt.find(v_label)==data_v_label_cnt.end())
            data_v_label_cnt.insert(pair<int,int>(v_label,1));
        else
            ++data_v_label_cnt[v_label];
    }
    vector<Vertex>* query_vertices_vec_ptr=&(this->query_graph_ptr->vertices);
    for(vector<Vertex>::iterator iter=query_vertices_vec_ptr->begin();
    iter!=query_vertices_vec_ptr->end();++iter) {
        int v_label=(*iter).vlabel;
        int v_weight=data_v_label_cnt[v_label];
        weight_query_graph.add_vertex(v_label,v_weight);
    }

    // calc weight of edge
    map<int, map<int,int> > data_e_pattern_cnt;
    vector<Edge>* data_edges_vec_ptr=&(this->data_graph_ptr->edges);
    for(vector<Edge>::iterator iter=data_edges_vec_ptr->begin();
    iter!=data_edges_vec_ptr->end();++iter){
        int left_v_id=(*iter).left_vid;
        int right_v_id=(*iter).right_vid;
        int left_v_label=this->data_graph_ptr->vertices[left_v_id].vlabel;
        int right_v_label=this->data_graph_ptr->vertices[right_v_id].vlabel;
        if(left_v_label>right_v_label) {
            //swap
            int tmp=left_v_label;
            left_v_label=right_v_label;
            right_v_label=tmp;
        }
        if(data_e_pattern_cnt.find(left_v_label)==data_e_pattern_cnt.end()) {
            map<int,int> map_inner;
            map_inner.insert(pair<int,int>(right_v_label,1));
            data_e_pattern_cnt.insert(pair<int, map<int,int> >(left_v_label,map_inner));
        }
        else {
            map<int,int>* inner_map_ptr=&(data_e_pattern_cnt[left_v_label]);
            if(inner_map_ptr->find(right_v_label)==inner_map_ptr->end())
                inner_map_ptr->insert(pair<int,int>(right_v_label,1));
            else
                ++(*inner_map_ptr)[right_v_label];
        }
    }
    vector<Edge>* query_edges_vec_ptr=&(this->query_graph_ptr->edges);
    for(vector<Edge>::iterator iter=query_edges_vec_ptr->begin();
    iter!=query_edges_vec_ptr->end();++iter) {
        int left_vid=(*iter).left_vid;
        int right_vid=(*iter).right_vid;
        int left_v_label=this->query_graph_ptr->vertices[left_vid].vlabel;
        int right_v_label=this->query_graph_ptr->vertices[right_vid].vlabel;
        if(left_v_label>right_v_label) {
            //swap
            int tmp=left_v_label;
            left_v_label=right_v_label;
            right_v_label=tmp;
        }
        int edge_weight=data_e_pattern_cnt[left_v_label][right_v_label];
        weight_query_graph.add_edge(left_vid,right_vid,edge_weight,false);
    }

    // initialize degrees of all vertices
    weight_query_graph.initialize_degrees();
#ifdef DEBUG_PRECISE
    printf("weighted query graph created\n");
    weight_query_graph.print_weight_graph();
#endif

    int query_v_num= this->query_graph_ptr->vertices.size();
    
    // find edges with min weight
    int min_edge_weight= INF;
    // vector<Weight_Vertex>* weight_query_vertex_vec_ptr=&(weight_query_graph.weight_vertices);
    vector<Weight_Edge>* weight_query_edge_vec_ptr=&(weight_query_graph.weight_edges);
    for(vector<Weight_Edge>::iterator iter=weight_query_edge_vec_ptr->begin();
    iter!=weight_query_edge_vec_ptr->end();++iter) {
        int edge_weight=(*iter).weight;
        if(edge_weight<min_edge_weight)
            min_edge_weight=edge_weight;
    }
    vector<int> min_weight_edges_id;
    int query_e_num=weight_query_graph.weight_edges.size();
    for(int eid=0;eid<query_e_num;++eid) {
        int edge_weight=weight_query_graph.weight_edges[eid].weight;
        if(edge_weight==min_edge_weight)
            min_weight_edges_id.push_back(eid);
    }


    

    this->seq=SEQ();

    //select first edge from edges with min weight
    int first_eid=this->weight_q_graph_first_eid(min_weight_edges_id,&weight_query_graph);
#ifdef DEBUG_PRECISE
    printf("first eid selected: %d\n",first_eid);
#endif
    // select vertex with less weight as first vertex
    int first_e_left_vid=weight_query_graph.weight_edges[first_eid].left_vid;
    // printf("first_e_left_vid: %d\n",first_e_left_vid);
    int first_e_right_vid=weight_query_graph.weight_edges[first_eid].right_vid;
    int first_e_left_v_weight=weight_query_graph.weight_vertices[first_e_left_vid].weight;
    int first_e_right_v_weight=weight_query_graph.weight_vertices[first_e_right_vid].weight;
    int first_vid;
    int second_vid;
    if(first_e_left_v_weight<first_e_right_v_weight) {
        first_vid=first_e_left_vid;
        second_vid=first_e_right_vid;
    }
    else {
        first_vid=first_e_right_vid;
        second_vid=first_e_left_vid;
    }

    this->seq.add_seq_entry(-1,first_vid,&weight_query_graph);
    this->seq.add_seq_entry(0,second_vid,&weight_query_graph);
    // remove an edge from weighted query graph
    weight_query_graph.remove_edge(first_eid);
#ifdef DEBUG_PRECISE
    printf("first two seq entries added\n");
    this->seq.print_seq();
#endif

    // add remaining edges into SEQ
    while(this->seq.seq_len<query_v_num) {
#ifdef DEBUG_PRECISE
        printf("find a new edge to add into seq tree\n");
#endif
        vector<Candidate_Edge> candidate_edges=this->seq_get_candidate_edges(&weight_query_graph);
        // printf("%d initial candidate edges get\n",(int)(candidate_e_list->size()));
        Candidate_Edge selected_cand_edge=this->select_candidate_edge(candidate_edges,\
        &weight_query_graph);
        Candidate_Edge* selected_cand_e_ptr=&selected_cand_edge;
#ifdef DEBUG_PRECISE
        printf("a candidate edge selected\n");
        this->print_candidate_edge(selected_cand_e_ptr);
#endif
        int selected_eid=selected_cand_e_ptr->eid;
        int in_seq_vid=selected_cand_e_ptr->in_seq_vid;
        int parent_entry_id=this->seq.get_entry_id_by_vid(in_seq_vid);
        int outside_seq_vid=selected_cand_e_ptr->outside_seq_vid;
        this->seq.add_seq_entry(parent_entry_id,outside_seq_vid,&weight_query_graph);
        weight_query_graph.weight_edges[selected_eid].removed=true;
#ifdef DEBUG_PRECISE
        printf("a new seq entry added\n");
#endif
        
        vector<Weight_Edge>* weight_edges_ptr=&(weight_query_graph.weight_edges);
        vector<Weight_Edge*> extra_edges_add_to_seq;
        for(vector<Weight_Edge>::iterator iter=weight_edges_ptr->begin();
        iter!=weight_edges_ptr->end();++iter) {
            Weight_Edge* weight_edge_ptr=&(*iter);
            if(weight_edge_ptr->removed)
                continue;

            int left_vid=weight_edge_ptr->left_vid;
            int right_vid=weight_edge_ptr->right_vid;
            int entry_id_a=this->seq.get_entry_id_by_vid(left_vid);
            int entry_id_b=this->seq.get_entry_id_by_vid(right_vid);
            if(entry_id_a==-1||entry_id_b==-1)
                continue;
            extra_edges_add_to_seq.push_back(weight_edge_ptr);
        }
        sort(extra_edges_add_to_seq.begin(),extra_edges_add_to_seq.end(),weight_edge_ptr_sort);
        for(vector<Weight_Edge*>::iterator iter=extra_edges_add_to_seq.begin();\
        iter!=extra_edges_add_to_seq.end();++iter)
        {
            Weight_Edge* weight_edge_ptr=*iter;
            int weight=weight_edge_ptr->weight;
            int left_vid=weight_edge_ptr->left_vid;
            int right_vid=weight_edge_ptr->right_vid;
            this->seq.add_seq_extra_edge(left_vid,right_vid);
            weight_edge_ptr->removed=true;
        }
        

    }

    //this->seq.print_seq();

    return true;
}

int Match::weight_q_graph_first_eid(vector<int>& _min_weight_edges_id,
Weight_Graph* _weight_graph_ptr)
{
    int min_weight_edges_num=_min_weight_edges_id.size();
    if(min_weight_edges_num==0)
    {
        // wrong
        return -1;
    }
    else if(min_weight_edges_num==1)
    {
        return _min_weight_edges_id[0];
    }
    else
    {
        // select edges with min sum degree of 2 endpoints
        int min_endpoints_d_sum=INF;
        vector<int> min_d_sum_eid_list;
        // all edges are not removed because this is to select first edge
        for(vector<int>::iterator iter=_min_weight_edges_id.begin();
        iter!=_min_weight_edges_id.end();++iter)
        {
            int eid=*iter;
            int left_vid=_weight_graph_ptr->weight_edges[eid].left_vid;
            int right_vid=_weight_graph_ptr->weight_edges[eid].right_vid;
            int left_degree=_weight_graph_ptr->weight_vertices[left_vid].degree;
            int right_degree=_weight_graph_ptr->weight_vertices[right_vid].degree;
            int endpoints_d_sum=left_degree+right_degree;
            if(endpoints_d_sum==min_endpoints_d_sum)
            {
                min_d_sum_eid_list.push_back(eid);
            }
            else if(endpoints_d_sum<min_endpoints_d_sum)
            {
                min_endpoints_d_sum=endpoints_d_sum;
                // clear the vector and release the space
                this->clear_int_vec(&min_d_sum_eid_list);
                // vector<int>().swap(min_d_sum_eid_list);
                // min_d_sum_eid_list.clear();
                min_d_sum_eid_list.push_back(eid);
            }
        }
        return random_select_int(&min_d_sum_eid_list);
    }
    
}

int Match::random_select_int(vector<int>* _vec)
{
    int vec_len=(*_vec).size();
    srand((unsigned)time(NULL)); 
    int rand_id=rand()%vec_len;
    return (*_vec)[rand_id];
}

vector<Candidate_Edge> Match::seq_get_candidate_edges(Weight_Graph* _weight_query_graph_ptr)
{
    vector<Candidate_Edge> candidate_edge_list;
    vector<Weight_Edge>* _weight_query_edges_ptr=&(_weight_query_graph_ptr->weight_edges);
    // some edges might be deleted
    int prev_e_num=_weight_query_edges_ptr->size();
#ifdef DEBUG_PRECISE
    printf("prev_e_num: %d\n",prev_e_num);
#endif
    for(int eid=0; eid<prev_e_num;++eid)
    {
        Weight_Edge* weight_edge_ptr=&((*_weight_query_edges_ptr)[eid]);
        if(weight_edge_ptr->removed)
        {
            continue;
        }
        int left_vid=weight_edge_ptr->left_vid;
        int right_vid=weight_edge_ptr->right_vid;
        bool left_v_in_seq=this->find_vid_in_seq(left_vid);
        bool right_v_in_seq=this->find_vid_in_seq(right_vid);
        if(left_v_in_seq&&!right_v_in_seq)
        {
            candidate_edge_list.push_back(Candidate_Edge(eid,left_vid,right_vid));
        }
        else if(right_v_in_seq&&!left_v_in_seq)
        {
            candidate_edge_list.push_back(Candidate_Edge(eid,right_vid,left_vid));
        }
        // if(left_v_in_seq&&!right_v_in_seq)
        // {
        //     this->seq.add_seq_entry(left_vid,right_vid,_weight_query_graph_ptr);
        // }
        // else if(right_v_in_seq&&!left_v_in_seq)
        // {
        //     this->seq.add_seq_entry(right_vid,left_vid,_weight_query_graph_ptr);
        // }
    }
    // printf("length of candidate edge list: %d\n",(int)(candidate_edge_list.size()));
#ifdef DEBUG_PRECISE
    this->print_cand_edges(&candidate_edge_list);
#endif
    return candidate_edge_list;

}

bool Match::find_vid_in_seq(int _vid)
{
    map<int,int>* vid_to_seq_entry_id_ptr= &(this->seq.vid_to_entry_id);
    if(vid_to_seq_entry_id_ptr->find(_vid)==vid_to_seq_entry_id_ptr->end())
    {
        return false;
    }
    else
    {
        return true;
    }
    
}

int Match::get_entry_id_from_vid(int _vid)
{
    map<int,int>* vid_to_seq_entry_id_ptr= &(this->seq.vid_to_entry_id);
    if(vid_to_seq_entry_id_ptr->find(_vid)==vid_to_seq_entry_id_ptr->end())
    {
        return -1;
    }
    else
    {
        return (*vid_to_seq_entry_id_ptr)[_vid];
    } 
}

Candidate_Edge Match::select_candidate_edge(vector<Candidate_Edge> _candidate_edges,
Weight_Graph* _weight_graph_ptr)
{
#ifdef DEBUG_PRECISE
    printf("_candidate_edges: ");
    this->print_cand_edges(&_candidate_edges);
#endif
    if(_candidate_edges.size()==1)
    {
        return _candidate_edges[0];
    }
    // select edges with min weight in remaining weighted edge graph
    vector<Candidate_Edge> min_weight_cand_edges;
    int min_weight=INF;
    for(vector<Candidate_Edge>::iterator iter=_candidate_edges.begin();\
    iter!=_candidate_edges.end();++iter)
    {
        int eid=(*iter).eid;
        int e_weight=_weight_graph_ptr->weight_edges[eid].weight;
        if(e_weight<min_weight)
        {
            min_weight=e_weight;
        }
    }

    for(vector<Candidate_Edge>::iterator iter=_candidate_edges.begin();\
    iter!=_candidate_edges.end();++iter)
    {
        int eid=(*iter).eid;
        int e_weight=_weight_graph_ptr->weight_edges[eid].weight;
        if(e_weight==min_weight)
        {
            min_weight_cand_edges.push_back(*iter);
        }
    }
#ifdef DEBUG_PRECISE
    printf("min_weight_cand_edges: ");
    this->print_cand_edges(&min_weight_cand_edges);
#endif

    if(min_weight_cand_edges.size()==1)
    {
        return min_weight_cand_edges[0];
    }

    // filter min_weight_cand_edges
    // select edges by induced graph
    // count induced graph edges
    vector<int> candidates_ind_sizes;
    for(vector<Candidate_Edge>::iterator iter=min_weight_cand_edges.begin();
    iter!=min_weight_cand_edges.end(); ++iter)
    {
        Candidate_Edge* candidate_edge_ptr= &(*iter);
        // int candidate_edge_eid=candidate_edge_ptr->eid;
        // int in_seq_vid=candidate_edge_ptr->in_seq_vid;
        int out_seq_vid=candidate_edge_ptr->outside_seq_vid;
        int ind_size=this->count_induced_graph_edges_num(out_seq_vid,_weight_graph_ptr);
        candidates_ind_sizes.push_back(ind_size);
    }
    int max_ind_g_edge_num=-1;
    for(vector<int>::iterator ind_size_iter=candidates_ind_sizes.begin();
    ind_size_iter!=candidates_ind_sizes.end();++ind_size_iter)
    {
        int ind_size=*ind_size_iter;
        if(ind_size>max_ind_g_edge_num)
        {
            max_ind_g_edge_num=ind_size;
        }
    }
    vector<Candidate_Edge> max_induce_cand_edges;
    int min_weight_cand_num=min_weight_cand_edges.size();
    for(int pos=0;pos<min_weight_cand_num;++pos)
    {
        int ind_g_edge_num=candidates_ind_sizes[pos];
        if(ind_g_edge_num==max_ind_g_edge_num)
        {
            max_induce_cand_edges.push_back(min_weight_cand_edges[pos]);
        }
    }

    if(max_induce_cand_edges.size()==1)
    {
        return max_induce_cand_edges[0];
    }

    // filter max_induce_cand_edges
    int min_out_v_degree=INF;
    for (vector<Candidate_Edge>::iterator iter=max_induce_cand_edges.begin();
    iter!=max_induce_cand_edges.end();++iter)
    {
        int outside_seq_vid=(*iter).outside_seq_vid;
        int out_v_degree=_weight_graph_ptr->weight_vertices[outside_seq_vid].degree;
        if(out_v_degree<min_out_v_degree)
        {
            min_out_v_degree=out_v_degree;
        }
    }
    vector<Candidate_Edge> final_candidate_edges;
    for(vector<Candidate_Edge>::iterator iter=max_induce_cand_edges.begin();
    iter!=max_induce_cand_edges.end();++iter)
    {
        int outside_seq_vid=(*iter).outside_seq_vid;
        int out_v_degree=_weight_graph_ptr->weight_vertices[outside_seq_vid].degree;
        if(out_v_degree==min_out_v_degree)
        {
            final_candidate_edges.push_back(*iter);
        }
    }

    int final_candidate_num=final_candidate_edges.size();
    if(final_candidate_num==1)
    {
        return final_candidate_edges[0];
    }
    vector<int> pos_list;
    for(int pos=0;pos<final_candidate_num;++pos)
    {
        pos_list.push_back(pos);
    }
    int pos_selected=this->random_select_int(&pos_list);
    return final_candidate_edges[pos_selected];



}

int Match::count_induced_graph_edges_num(int _another_ind_vid, Weight_Graph* _weight_graph_ptr)
{
    // some edges might be deleted
    vector<Weight_Edge>* weight_edges_ptr= &(_weight_graph_ptr->weight_edges);
    int ind_g_edge_num=0;
    for(vector<Weight_Edge>::iterator iter=weight_edges_ptr->begin();
    iter!=weight_edges_ptr->end();++iter)
    {
        Weight_Edge* weight_edge_ptr= &(*iter);
        if(weight_edge_ptr->removed)
        {
            continue;
        }
        int left_vid=weight_edge_ptr->left_vid;
        int right_vid=weight_edge_ptr->right_vid;
        bool left_v_is_ind_g_vertex=this->is_ind_g_vertex(left_vid,_another_ind_vid);
        if(left_v_is_ind_g_vertex==false)
        {
            continue;
        }
        bool right_v_is_ind_g_vertex=this->is_ind_g_vertex(right_vid,_another_ind_vid);
        if(right_v_is_ind_g_vertex==false)
        {
            continue;
        }
        ++ind_g_edge_num;
    }
    return true;
}

bool Match::is_ind_g_vertex(int _vid, int _another_ind_vid)
{
    if(_vid==_another_ind_vid)
    {
        return true;
    }
    else if(this->seq.vid_to_entry_id.find(_vid)!=this->seq.vid_to_entry_id.end())
    {
        return true;
    }
    else
    {
        return false;
    }
    
}



vector<int> Match::arr_to_ascend_vec(int* _arr)
{
    vector<int> ascend_vec;
    int arr_len=this->query_v_num;
    for(int i=0;i<arr_len;++i)
    {
        ascend_vec.push_back(_arr[i]);
    }
    sort(ascend_vec.begin(),ascend_vec.end());
    return ascend_vec;
}

bool Match::vector_duplicate(vector<int>* _vec_a_ptr,vector<int>* _vec_b_ptr)
{
    int vec_len=_vec_a_ptr->size();
    for(int i=0;i<vec_len;++i)
    {
        int val_a=(*_vec_a_ptr)[i];
        int val_b=(*_vec_b_ptr)[i];
        if(val_a!=val_b)
        {
            return false;
        }
    }
    return true;
}

bool Match::find_duplicate_res()
{
    
    vector<int> tmp_ascend_vec=this->arr_to_ascend_vec(this->matched_data_vid_arr);
    for(vector< vector<int> >::iterator vec_iter=this->matched_graphs.begin();
    vec_iter!=this->matched_graphs.end();++vec_iter)
    {
        vector<int>* matched_graph_vec_ptr=&(*vec_iter);
        bool tmp_duplicate=this->vector_duplicate(matched_graph_vec_ptr,&tmp_ascend_vec);
        if(tmp_duplicate)
        {
            return true;
        }
    }
    // add to matched graph
    this->matched_graphs.push_back(tmp_ascend_vec);
    this->ascend_vid_vec=tmp_ascend_vec;
    return false;
}

void Match::try_res_output()
{
    //bool res_already_exist=this->find_duplicate_res();
    //if(res_already_exist)
    //{
        //return;
    //}
    int* res_arr=new int[this->query_v_num];
    for(int i = 0; i < this->query_v_num; ++i)
    {
        //res_arr[i]=this->ascend_vid_vec[i];
        int qid = this->seq.entries[i].vid;
        res_arr[qid] = this->matched_data_vid_arr[i];
    }
    // std::copy(this->ascend_vid_vec.begin(),this->ascend_vid_vec.end(),res_arr);
#ifdef PRINT_RESULT
	this->io_ptr->output(res_arr,this->query_v_num);
#endif
    delete []res_arr;
}

void Match::quicksi(int _depth) {
    if (nembeddings > bound) return;
	if(_depth==this->query_v_num) {
        // found a match
        this->try_res_output();
		nembeddings++;
		double vsize, rss;
		Util::process_mem_usage(vsize, rss);
		if (vsize > max_mem) max_mem = vsize;
        return;
    }
	ncalls++;
    // candidates of this depth
    vector<int> candidates_this_depth;
    SEQ_Entry* entry_ptr=&(this->seq.entries[_depth]);
    int seq_entry_label=entry_ptr->vlabel;
    if(_depth==0) {
        for(int data_vid=0;data_vid<this->data_v_num;++data_vid) {
            Vertex* data_vertex_ptr=&(this->data_graph_ptr->vertices[data_vid]);
            int data_v_label=data_vertex_ptr->vlabel;
            if(data_v_label!=seq_entry_label)
                continue;
            if(this->data_v_visited_arr[data_vid])
                continue;
            candidates_this_depth.push_back(data_vid);
        }
    }
    else if(_depth!=0) {
        // parent
        int parent_entry_id=entry_ptr->parent_entry_id;
        int parent_vid=this->matched_data_vid_arr[parent_entry_id];
        // extra edge endpoint
        vector<int> extra_endpoint_vid_vec;
        vector<int>* extra_endpoint_entries_ptr= &(entry_ptr->extra_edges_end_entries);
        for(vector<int>::iterator iter=extra_endpoint_entries_ptr->begin();\
        iter!=extra_endpoint_entries_ptr->end();++iter) {
            int endpoint_entry_id=*iter;
            int endpoint_vid=this->matched_data_vid_arr[endpoint_entry_id];
            extra_endpoint_vid_vec.push_back(endpoint_vid);
        }

        vector<int>& neighbors = this->data_graph_ptr->vertices[parent_vid].neighbors;
        for(int i = 0; i < neighbors.size(); ++i) {
            int data_vid = neighbors[i];
            if(this->data_v_visited_arr[data_vid])
                continue;
            Vertex* data_vertex_ptr=&(this->data_graph_ptr->vertices[data_vid]);
            int data_v_label=data_vertex_ptr->vlabel;
            if(data_v_label!=seq_entry_label)
                continue;
            // extra edge restriction
            bool meet_extra_edge_restr=extra_edge_restriction(data_vid,&extra_endpoint_vid_vec);
            if(!meet_extra_edge_restr)
                continue;
            candidates_this_depth.push_back(data_vid);
        }
    }
    // one step further
    for(vector<int>::iterator iter=candidates_this_depth.begin();iter!=candidates_this_depth.end();
    ++iter) {
        int cand_vid=*iter;
        this->matched_data_vid_arr[_depth]=cand_vid;
        this->data_v_visited_arr[cand_vid]=true;
        quicksi(_depth+1);
        this->data_v_visited_arr[cand_vid]=false;
if (nembeddings > bound) return;
    }

}

bool Match::data_edge_exist(int vid_0,int vid_1)
{
    Vertex* vertex_0_ptr= &(this->data_graph_ptr->vertices[vid_0]);
    vector<int>* v0_neighbors_ptr= &(vertex_0_ptr->neighbors);
    for(vector<int>::iterator iter=v0_neighbors_ptr->begin();iter!=v0_neighbors_ptr->end();\
    ++iter)
    {
        int nei_vid=*iter;
        if(nei_vid==vid_1)
        {
            return true;
        }
    }
    return false;
}

bool Match::extra_edge_restriction(int _data_vid,vector<int>* _end_vid_vec_ptr)
{
    for(vector<int>::iterator iter=_end_vid_vec_ptr->begin();iter!=_end_vid_vec_ptr->end();
    ++iter)
    {
        int endpoint_vid=*iter;
        bool edge_exist=this->data_edge_exist(_data_vid,endpoint_vid);
        if(!edge_exist)
        {
            return false;
        }
    }
    return true;
}

void Match::match()
{
    this->gen_seq();
#ifdef DEBUG_PRECISE
    printf("seq generated\n");
#endif
    this->matched_data_vid_arr=new int[this->query_v_num];
    this->data_v_visited_arr=new bool[this->data_v_num];
    for(int vid=0;vid<this->data_v_num;++vid)
    {
        this->data_v_visited_arr[vid]=false;
    }
    this->quicksi(0);

    delete []this->matched_data_vid_arr;
    delete []this->data_v_visited_arr;
}

void Match::clear_int_vec(vector<int>* _int_vec_ptr)
{
    vector<int>().swap(*_int_vec_ptr);
    (*_int_vec_ptr).clear();
}

void Match::print_cand_edges(vector<Candidate_Edge>* _cand_edge_vec_ptr)
{
    printf("%d candidate edges: [eid,in_seq_vid,outside_seq_vid]\n",\
    (int)(_cand_edge_vec_ptr->size()));
    for(vector<Candidate_Edge>::iterator iter=_cand_edge_vec_ptr->begin();
    iter!=_cand_edge_vec_ptr->end();++iter)
    {
        Candidate_Edge* cand_edge_ptr=&(*iter);
        int eid=cand_edge_ptr->eid;
        int in_seq_vid=cand_edge_ptr->in_seq_vid;
        int outside_seq_vid=cand_edge_ptr->outside_seq_vid;
        printf("[%d,%d,%d] ",eid,in_seq_vid,outside_seq_vid);
    }
    printf("\n");
}

void Match::print_candidate_edge(Candidate_Edge* _cand_edge_ptr)
{
    printf("candidate_edge: [eid,in_seq_vid,outside_seq_vid]\n");
    int eid=_cand_edge_ptr->eid;
    int in_seq_vid=_cand_edge_ptr->in_seq_vid;
    int outside_seq_vid=_cand_edge_ptr->outside_seq_vid;
    printf("[%d,%d,%d] ",eid,in_seq_vid,outside_seq_vid);
    printf("\n");
}
