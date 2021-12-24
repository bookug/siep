#include "Match.h"

using namespace std;

//#define DEBUG 1

extern long bound, numofembeddings, ncalls;
extern double max_mem;

Match::Match(QueryGraph* _query, Hyper_Data_Graph* _hyper_data_graph_ptr)
{
	this->query_graph=_query;
    this->hyper_data_graph=_hyper_data_graph_ptr;
    this->query_v_num=_query->vertices.size();
    this->data_v_num=_hyper_data_graph_ptr->v_num;
    this->data_hv_num=_hyper_data_graph_ptr->hv_num;

    this->qv_degrees=(int*)malloc(this->query_v_num*sizeof(int));
    for(int qi=0;qi<query_v_num;++qi)
    {
        int tmp_qv_degree=query_graph->vertices[qi].neighbors.size();
        this->qv_degrees[qi]=tmp_qv_degree;
    }
    
    // this->qv_select_seq_pos=-1;
    this->tmp_matched_graph = vector<int>(this->query_v_num);
    this->matched_data_v_list = vector<int>(this->query_v_num);
    this->data_v_visited = vector<bool>(this->data_v_num, false);
}

Match::~Match()
{
	if(this->qv_visited_list!=NULL)
	{
		free(this->qv_visited_list);
		this->qv_visited_list=NULL;
	}
}

bool Match::qv_dhv_initial_cand_test(int _query_vid,int _data_hvid)
{
    QueryVertex* query_v_ptr=&(this->query_graph->vertices[_query_vid]);
    int query_v_label=query_v_ptr->vlabel;
    int data_hv_label=this->hyper_data_graph->get_hv_label(_data_hvid);
    if(query_v_label!=data_hv_label)
        return false;
    // degree
    int query_v_degree=this->qv_degrees[_query_vid];
    int data_hv_actual_degree=hyper_data_graph->get_hv_actual_degree(_data_hvid);
    if(query_v_degree>data_hv_actual_degree)
        return false;
    return true;
}


void Match::add_candidate(int _q_vid,int _data_hvid)
{
    // add _d_vid to candidate list of _q_vid
    this->query_graph->vertices[_q_vid].candidates.insert(_data_hvid);
}

void Match::init_candidates(int _q_vid)
{
    // candidates for each query vetex, matching predicates
    // QueryVertex* q_vertex_ptr=&(this->query_graph->vertices[_q_vid]);
    for(int data_hvid=0;data_hvid<this->data_hv_num;++data_hvid)
    {
        bool initial_match = this->qv_dhv_initial_cand_test(_q_vid,data_hvid);
        if(!initial_match)
        {
            continue;
        }
        else
        {
            this->add_candidate(_q_vid,data_hvid);
        }
        
    }
}



void Match::add_qv_nei_profile(set<int>* _qv_this_hop,int _nei_radius,set<int>* _qv_profile,
bool* _qv_visited)
{
    for(set<int>::iterator iter=(*_qv_this_hop).begin();iter!=(*_qv_this_hop).end();++iter)
    {
        int tmp_qv=*iter;
        _qv_visited[tmp_qv]=true;
        int tmp_qv_label=this->query_graph->vertices[tmp_qv].vlabel;
        (*_qv_profile).insert(tmp_qv_label);
    }
    if(_nei_radius==0)
    {
        return;
    }
    // next hop
    set<int> qv_next_hop;
    for(set<int>::iterator iter=(*_qv_this_hop).begin();iter!=(*_qv_this_hop).end();++iter)
    {
        int this_hop_qv=*iter;
        vector<Neighbor>* neighbors_ptr=&this->query_graph->vertices[this_hop_qv].neighbors;
        int nei_num=(*neighbors_ptr).size();
        for(int nei_pos=0;nei_pos<nei_num;++nei_pos)
        {
            int nei_qvid=(*neighbors_ptr)[nei_pos].vid;
            if(!_qv_visited[nei_qvid])
            {
                qv_next_hop.insert(nei_qvid);
            }
        }
    }
    add_qv_nei_profile(&qv_next_hop,_nei_radius-1,_qv_profile,_qv_visited);
}

void Match::build_qv_nei_profile(int _qv_id,int _nei_radius)
{
    set<int> qv_profile;
    bool* qv_visited=(bool*)malloc(this->query_v_num*sizeof(bool));
    for(int i=0;i<this->query_v_num;++i)
    {
        qv_visited[i]=false;
    }
    set<int> qv_this_hop;
    qv_this_hop.insert(_qv_id);
    add_qv_nei_profile(&qv_this_hop,_nei_radius,&qv_profile,qv_visited);
    this->qv_profile_vec.push_back(qv_profile);
    free(qv_visited);
}

void Match::build_qv_nei_profile_list(int _nei_radius)
{
    for(int qv_id=0;qv_id<this->query_v_num;++qv_id)
    {
        this->build_qv_nei_profile(qv_id,_nei_radius);
    }
}

void Match::add_data_hv_nei_profile(set<int>* _data_hv_this_hop,int _nei_radius,set<int>* _data_hv_profile,
bool* _data_hv_visited_list)
{
    for(set<int>::iterator iter=(*_data_hv_this_hop).begin();iter!=(*_data_hv_this_hop).end();++iter)
    {
        int tmp_data_hv=*iter;
        _data_hv_visited_list[tmp_data_hv]=true;
        int tmp_data_hv_label=this->hyper_data_graph->get_hv_label(tmp_data_hv);
        (*_data_hv_profile).insert(tmp_data_hv_label);
    }
    if(_nei_radius==0)
    {
        return;
    }
    // next hop
    set<int> data_hv_next_hop;
    for(set<int>::iterator iter=(*_data_hv_this_hop).begin();iter!=(*_data_hv_this_hop).end();++iter)
    {
        int tmp_data_hv=*iter;
        vector<HV_Neighbour_Group_Specific_Label>* hv_neighbour_group_by_label_ptr=&(this->hyper_data_graph->hv_neighbour_group_by_label_by_hv[tmp_data_hv]);
        for(vector<HV_Neighbour_Group_Specific_Label>::iterator group_iter=hv_neighbour_group_by_label_ptr->begin();
        group_iter!=hv_neighbour_group_by_label_ptr->end();++group_iter)
        {
            vector<int>* group_hv_list_ptr=&((*group_iter).hv_list);
            for(vector<int>::iterator hv_iter=group_hv_list_ptr->begin();hv_iter!=group_hv_list_ptr->end();++hv_iter)
            {
                int data_hv=*hv_iter;
                if(!_data_hv_visited_list[data_hv])
                    data_hv_next_hop.insert(data_hv);
            }
        }
    }
    // cout<<"next hop:"<<endl;
    // for(set<int>::iterator iter=data_hv_next_hop.begin();iter!=data_hv_next_hop.end();++iter)
    // {
    //     cout<<*iter;
    // }
    // cout<<endl;
    add_data_hv_nei_profile(&data_hv_next_hop,_nei_radius-1,_data_hv_profile,_data_hv_visited_list);
}

void Match::build_data_hv_nei_profile(int _data_hv_id, int _nei_radius)
{   
    if(this->data_hv_profile_map.find(_data_hv_id)!=this->data_hv_profile_map.end())
        return;
    set<int> data_hv_profile;
    bool* data_hv_visited_list=(bool*)malloc(this->data_hv_num*sizeof(bool));
    for(int i=0;i<this->data_hv_num;++i)
    {
        data_hv_visited_list[i]=false;
    }
    set<int> data_hv_this_hop;
    data_hv_this_hop.insert(_data_hv_id);
    add_data_hv_nei_profile(&data_hv_this_hop,_nei_radius,&data_hv_profile,data_hv_visited_list);

    free(data_hv_visited_list);

    // cout<<"data_hv_profile of hv "<<_data_hv_id<<":"<<endl;
    // for(set<int>::iterator iter=data_hv_profile.begin();iter!=data_hv_profile.end();++iter)
    // {
    //     cout<<*iter<<" ";
    // }
    // cout<<endl;
    this->data_hv_profile_map.insert(make_pair(_data_hv_id,data_hv_profile));
}

bool Match::data_hv_prof_include_query_v_prof(set<int>* _data_hv_profile, set<int>* _query_v_profile)
{
    set<int>::iterator q_iter=(*_query_v_profile).begin();
    set<int>::iterator d_iter=(*_data_hv_profile).begin();
    set<int>::iterator q_end=(*_query_v_profile).end();
    set<int>::iterator d_end=(*_data_hv_profile).end();
    while(d_iter!=d_end)
    {
        if(q_iter==q_end)
            return true;
        int tmp_qlabel=*q_iter;
        int tmp_dlabel=*d_iter;
        if(tmp_qlabel==tmp_dlabel)
        {
            ++q_iter;
            ++d_iter;
        }
        else if(tmp_dlabel<tmp_qlabel)
        {
            ++d_iter;
        }
        else
        {
            return false;
        } 
    }
    if(q_iter==q_end)
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

void Match::qv_local_filter(int _qv,int _nei_radius)
{
    set<int>* qv_profile_ptr=&(this->qv_profile_vec[_qv]);

    set<int>* candidates_ptr=&this->query_graph->vertices[_qv].candidates;
    // int cand_num=(*candidates_ptr).size();
    // printf("candidate num of qv %d : %d\n",_qv,cand_num);
    // for each candidate of _qv, if its profile does not include _qv profile, remove it
    set<int> new_candidates;
    for(set<int>::iterator iter=(*candidates_ptr).begin();iter!=(*candidates_ptr).end();++iter)
    {
        int cand_data_hv_id=*iter;
        this->build_data_hv_nei_profile(cand_data_hv_id,_nei_radius);
        set<int>* cand_data_hv_profile_ptr=&(this->data_hv_profile_map[cand_data_hv_id]);

        // cout<<"cand_dv "<<cand_dv<<" profile: ";
        // for(set<int>::iterator iter=cand_dv_profile.begin();iter!=cand_dv_profile.end();++iter)
        // {
        //     cout<<*iter<<" ";
        // }
        // cout<<endl;

        // whether dv profile includes qv profile
        bool cand_profile_match= data_hv_prof_include_query_v_prof(cand_data_hv_profile_ptr,qv_profile_ptr);
        // cout<<"candidate semi match: "<<cand_semi_match<<endl;
        if(cand_profile_match)
        {
            new_candidates.insert(cand_data_hv_id);
        }
    }
    this->query_graph->vertices[_qv].candidates=new_candidates;
}



void Match::local_filter(int _nei_radius)
{
    // gen profiles of query vertices
    this->build_qv_nei_profile_list(_nei_radius);
    // for(int q_vid=0;q_vid<this->query_v_num;++q_vid)
    // {   
    //     cout<<"profile of qv "<<q_vid<<": "<<endl;
    //     set<int>* qv_profile_vec_ptr=&(this->qv_profile_vec[q_vid]);
    //     for(set<int>::iterator iter=qv_profile_vec_ptr->begin();iter!=qv_profile_vec_ptr->end();++iter)
    //     {
    //         cout<<*iter<<",";
    //     }
    //     cout<<endl;
    // }

    for(int qv=0;qv<this->query_v_num;++qv)
    {
        qv_local_filter(qv,_nei_radius);
    }
}

bool Match::exist_equal(set<int>* _dhv_set, set<int>* _cand_set)
{
    set<int>::iterator dhv_set_iter=(*_dhv_set).begin();
    set<int>::iterator cand_set_iter=(*_cand_set).begin();
    set<int>::iterator dhv_set_end=(*_dhv_set).end();
    set<int>::iterator cand_set_end=(*_cand_set).end();
    while(dhv_set_iter!=dhv_set_end&&cand_set_iter!=cand_set_end)
    {
        int dhv_set_ele=*dhv_set_iter;
        int cand_set_ele=*cand_set_iter;
        if(dhv_set_ele==cand_set_ele)
            return true;
        else if(dhv_set_ele<cand_set_ele)
        {
            ++dhv_set_iter;
        }
        else if(dhv_set_ele>cand_set_ele)
        {
            ++cand_set_iter;
        }
    }
    return false;
}

bool Match::bipart_semi_perf(int _qv,int _dhv)
{
    vector<Neighbor>* qv_neighbors=&this->query_graph->vertices[_qv].neighbors;
    set<int> qv_nei_ids;
    int qv_nei_num=(*qv_neighbors).size();
    for(int pos=0;pos<qv_nei_num;++pos)
    {
        int tmp_nei_qvid=(*qv_neighbors)[pos].vid;
        qv_nei_ids.insert(tmp_nei_qvid);
    }

    vector<int>* dhv_neighbors=&this->hyper_data_graph->nei_hv_vec_by_hv[_dhv];
    // hyper vector itself can be its neighbor
    set<int> dhv_nei_hv_set;

    for(vector<int>::iterator iter=dhv_neighbors->begin();iter!=dhv_neighbors->end();++iter)
    {
        int nei_dhv=*iter;
        dhv_nei_hv_set.insert(nei_dhv);
    }
    int dhv_type=this->hyper_data_graph->hv_type_list[_dhv];
    if(dhv_type==1)
    {
        // clique
        dhv_nei_hv_set.insert(_dhv);
    }

    // int dhv_nei_num=dhv_nei_hv_set.size();

    
    for(set<int>::iterator iter=qv_nei_ids.begin();iter!=qv_nei_ids.end();++iter)
    {
        int nei_qv=*iter;
        set<int>* nei_qv_candidates=&this->query_graph->vertices[nei_qv].candidates;
        bool dvs_satisfy_nei_qv=this->exist_equal(&dhv_nei_hv_set,nei_qv_candidates);
        if(!dvs_satisfy_nei_qv)
        {
            // _dv is not candidate of _qv
            return false;
        }
    }
    return true;
}

bool Match::is_candidate(int _qv, int _dhv)
{
    set<int>* candidates=&this->query_graph->vertices[_qv].candidates;
    for(set<int>::iterator iter=(*candidates).begin();iter!=(*candidates).end();++iter)
    {
        int tmp_cand=*iter;
        if(_dhv==tmp_cand)
            return true;
    }
    return false;
}

void Match::mark_pair(int _qv,int _dhv)
{
    if(this->mark_qv_dhv.find(_qv)==this->mark_qv_dhv.end())
    {
        set<int> mark_dhv;
        mark_dhv.insert(_dhv);
        mark_qv_dhv.insert(pair<int, set<int> >(_qv,mark_dhv));
    }
    else
    {
        // key _qv exists
        this->mark_qv_dhv[_qv].insert(_dhv);
    }
}

void Match::mark_potential_pairs(int _rt_qv,int _rt_dhv)
{
    vector<Neighbor>* q_neighbors=&this->query_graph->vertices[_rt_qv].neighbors;
    set<int> nei_qvs;
    for(vector<Neighbor>::iterator iter=(*q_neighbors).begin();iter!=(*q_neighbors).end();++iter)
    {
        int nei_qv=(*iter).vid;
        nei_qvs.insert(nei_qv);
    }
    vector<int>* nei_dhv_vec_ptr=&(this->hyper_data_graph->nei_hv_vec_by_hv[_rt_dhv]);
    set<int> nei_dhv_set;   
    for(vector<int>::iterator iter=(*nei_dhv_vec_ptr).begin();iter!=(*nei_dhv_vec_ptr).end();++iter)
    {
        int nei_dhv=*iter;
        nei_dhv_set.insert(nei_dhv);
    }
    for(set<int>::iterator q_iter=nei_qvs.begin();q_iter!=nei_qvs.end();++q_iter)
    {
        int nei_qv=*q_iter;
        if(nei_qv==_rt_qv)
        {
            // cannot link to oneself
            continue;
        }
        for(set<int>::iterator d_iter=nei_dhv_set.begin();d_iter!=nei_dhv_set.end();++d_iter)
        {
            int nei_dhv=*d_iter;
            bool is_cand=this->is_candidate(nei_qv,nei_dhv);
            if(is_cand)
            {
                this->mark_pair(nei_qv,nei_dhv);
            }
        }
    }
}

void Match::global_refine_search(int _global_refine_level)
{
    if(_global_refine_level==0)
        return;
    if(this->mark_qv_dhv.empty())
    {
        // no mark
        return;
    }
    for(int qv=0;qv<this->query_v_num;++qv)
    {
        if(this->mark_qv_dhv.find(qv)==this->mark_qv_dhv.end())
        {
            continue;
        }
        set<int>* marked_cand=&this->mark_qv_dhv.at(qv);
        for(set<int>::iterator iter=(*marked_cand).begin();iter!=(*marked_cand).end();++iter)
        {
            int cand_dhv=*iter;
            bool semi_perf_match=this->bipart_semi_perf(qv,cand_dhv);
            if(semi_perf_match)
            {
                continue;
            }
            else
            {
                // cand_dv is not candidate of qv
                this->query_graph->vertices[qv].candidates.erase(cand_dhv);
                //  potential changes
                this->mark_potential_pairs(qv,cand_dhv);
            }  
            //this->mark_qv_dhv[qv].erase(cand_dhv);
        }
        // this->mark_qv_dhv[qv]=new_marked_cand_set;
        this->mark_qv_dhv[qv]=set<int>();

    }

    this->global_refine_search(_global_refine_level-1);
}

void Match::global_refine(int _global_refine_level)
{
    if(_global_refine_level==0)
        return;
    // initialize Mark set
    for(int qv=0;qv<this->query_v_num;++qv)
    {
        set<int> qv_candidates;
        set<int>* candidates_ptr=&this->query_graph->vertices[qv].candidates;
        for(set<int>::iterator iter=(*candidates_ptr).begin();iter!=(*candidates_ptr).end();++iter)
        {
            qv_candidates.insert(*iter);
        }
        this->mark_qv_dhv.insert(pair<int,set<int> >(qv,qv_candidates));
    }
    this->global_refine_search(_global_refine_level);
}

//void Match::construct_qv_join_seq()
//{
//    bool* qv_visited=(bool*)malloc(query_v_num*sizeof(bool));
//    for(int qi=0;qi<query_v_num;++qi)
//    {
//        qv_visited[qi]=false;
//    }
//
//    this->query_v_match_seq=(int*)malloc(this->query_v_num*sizeof(int));
//    // choose remaining vertex with least degree
//    for(int i=0;i<this->query_v_num;++i)
//    {
//        this->query_v_match_seq[i]=construct_next_join_qv(qv_visited);
//    }
//    this->qv_match_seq_pos=0;
//}
//
//int Match::construct_next_join_qv(bool* qv_visited)
//{
//    // choose vertex with least degree in remaining query vertices
//    int min_degree_v=-1;
//    int min_degree=1e8;
//    for(int qv_i=0;qv_i<this->query_v_num;++qv_i)
//    {
//        if(qv_visited[qv_i])
//        {
//            continue;
//        }
//        int tmp_degree=this->qv_degrees[qv_i];
//        if(tmp_degree<min_degree)
//        {
//            min_degree=tmp_degree;
//            min_degree_v=qv_i;
//        }
//    }
//    if(min_degree_v!=-1)
//    {
//        qv_visited[min_degree_v]=true;
//    }
//    return min_degree_v;
//}
//

int Match::get_first_qv()
{
        int min_cand_size=1e8;
        for(int qv_i=0;qv_i<this->query_v_num;++qv_i)
        {
                int qv_cand_size=this->query_graph->vertices[qv_i].candidates.size();
                if(qv_cand_size<min_cand_size)
                        min_cand_size=qv_cand_size;
        }
        vector<int> min_cand_size_qvid_vec;
        for(int qv_i=0;qv_i<this->query_v_num;++qv_i)
        {
                int qv_cand_size=this->query_graph->vertices[qv_i].candidates.size();
                if(qv_cand_size==min_cand_size)
                        min_cand_size_qvid_vec.push_back(qv_i);
        }
        int selected_qv_num=min_cand_size_qvid_vec.size();
        if(selected_qv_num==1)
                return min_cand_size_qvid_vec[0];
        int max_degree_qv=-1;
        int max_degree=-1;
        for(int pos=0;pos<selected_qv_num;++pos)
        {
                int qv_i=min_cand_size_qvid_vec[pos];
                int tmp_degree=this->qv_degrees[qv_i];
                if(tmp_degree>max_degree)
                {
                        max_degree=tmp_degree;
                        max_degree_qv=qv_i;
                }
        }
        return max_degree_qv;
}

int Match::get_next_qv()
{
        set<int> qv_connect_set;
        for(int qv_i=0;qv_i<this->query_v_num;++qv_i)
        {
                if(this->qv_visited_list[qv_i])
                {
                        vector<Neighbor>* qv_nei_ptr=&(this->query_graph->vertices[qv_i].neighbors);
                        for(vector<Neighbor>::iterator iter=qv_nei_ptr->begin();iter!=qv_nei_ptr->end();++iter)
                        {
                                int nei_qv_id=(*iter).vid;
                                if(!this->qv_visited_list[nei_qv_id])
                                        qv_connect_set.insert(nei_qv_id);
                        }
                }
        }
        int min_cand_size=1e8;
        int min_cand_size_qv_id=-1;
        for(set<int>::iterator iter=qv_connect_set.begin();iter!=qv_connect_set.end();++iter)
        {
                int qv_id=*iter;
                int qv_cand_size=this->query_graph->vertices[qv_id].candidates.size();
                if(qv_cand_size<min_cand_size)
                {
                        min_cand_size=qv_cand_size;
                        min_cand_size_qv_id=qv_id;
                }
        }
        return min_cand_size_qv_id;

}

void Match::build_data_hv_is_cand_list(int _query_v_id)
{
    set<int>* query_v_candidates_ptr=&(this->query_graph->vertices[_query_v_id].candidates);
    for(int i=0;i<this->data_hv_num;++i)
    {
        this->hv_is_cand_list.push_back(false);
    }
    for(set<int>::iterator cand_iter=query_v_candidates_ptr->begin();cand_iter!=query_v_candidates_ptr->end();++cand_iter)
    {
        int cand_data_hv_id=*cand_iter;
        this->hv_is_cand_list[cand_data_hv_id]=true;
    }
}

void Match::build_data_hv_ump_list()
{
    // initialize to 0 and -1
    for(int data_hv_id=0;data_hv_id<this->data_hv_num;++data_hv_id)
    {
        bool hv_is_cand=this->hv_is_cand_list[data_hv_id];
        if(hv_is_cand)
            this->hv_ump_list.push_back(0);
        else
        {
            this->hv_ump_list.push_back(-1);
        }   
    }
    for(int data_hv_id=0;data_hv_id<this->data_hv_num;++data_hv_id)
    {
        bool hv_is_cand=this->hv_is_cand_list[data_hv_id];
        if(!hv_is_cand)
            continue;
        vector<int>* sc_children_vec_ptr=&(this->hyper_data_graph->sc_children_list[data_hv_id]);
        for(vector<int>::iterator child_hv_iter=sc_children_vec_ptr->begin();child_hv_iter!=sc_children_vec_ptr->end();++child_hv_iter)
        {
            int child_hv_id=*child_hv_iter;
            // check if child_hv is in candidate set
            if(this->hv_is_cand_list[child_hv_id])
            {
                ++this->hv_ump_list[child_hv_id];
            }
        }
    }
}





void Match::build_remaining_v_num_list()
{
    vector<int>* v_num_by_hv_ptr=&(this->hyper_data_graph->v_num_by_hv);
    for(vector<int>::iterator iter=v_num_by_hv_ptr->begin();iter!=v_num_by_hv_ptr->end();++iter)
    {
        int hv_v_num=*iter;
        this->hv_remaining_v_num_list.push_back(hv_v_num);
    }
}

bool Match::visit_data_hv(int _data_hv_id)
{
    if(this->hv_remaining_v_num_list[_data_hv_id]==0)
        return false;
    --this->hv_remaining_v_num_list[_data_hv_id];
    return true;
}

void Match::init_root_hv_searched_list()
{
    for(int i=0;i<this->data_hv_num;++i)
        this->root_hv_searched_list.push_back(false);
}

int Match::dynamic_cand_load_next_root_hv()
{
    for(int data_hv_id=0;data_hv_id<this->data_hv_num;++data_hv_id)
    {
        bool root_hv_searched=this->root_hv_searched_list[data_hv_id];
        if(this->hv_ump_list[data_hv_id]==0&&(!root_hv_searched))
        {
            return data_hv_id;
        }
    }
    return -1;
}

void Match::matched_root_data_hv_change_ump(int _root_data_hv_id)
{
    vector<int>* sc_children_ptr=&(this->hyper_data_graph->sc_children_list[_root_data_hv_id]);
    for(vector<int>::iterator child_hv_iter=sc_children_ptr->begin();child_hv_iter!=sc_children_ptr->end();++child_hv_iter)
    {
        int child_hv_id=*child_hv_iter;
        if(!(this->hv_is_cand_list[child_hv_id]))
            continue;
        --this->hv_ump_list[child_hv_id];
        if(this->hv_ump_list[child_hv_id]<0)
            cerr<<"ump wrong"<<endl;
    }
}

//void Match::root_dcl_search(int _start_qv)
//{   
//    cout<<"start query v: "<<_start_qv<<endl;
//    cout<<"begin root_dcl_search"<<endl;
//    while(true)
//    {
//        int root_data_hv_id=this->dynamic_cand_load_next_root_hv();
//        cout<<"root_data_hv_id: "<<root_data_hv_id<<endl;
//        if(root_data_hv_id==-1)
//            return;
//        bool try_visit=this->visit_data_hv(root_data_hv_id);
//        
//        if(!try_visit)
//            continue;
//        // cout<<"try_visit:"<<try_visit<<endl;
//        this->query_v_to_data_hv.insert(pair<int,int>(_start_qv,root_data_hv_id));
//        int next_query_v_id=this->get_next_qv();
//        // search tree from root data can get at least one result
//        this->non_dcl_search(next_query_v_id,1);
//        // cout<<"root_data_hv_can_match: "<<root_data_hv_can_match<<endl;
//        this->root_hv_searched_list[root_data_hv_id]=true;
//        // if(root_data_hv_can_match)
//        // {
//        this->matched_root_data_hv_change_ump(root_data_hv_id);
//        // }
//        // restore visited state
//        --this->qv_match_seq_pos;
//        ++this->hv_remaining_v_num_list[root_data_hv_id];
//        this->query_v_to_data_hv.erase(_start_qv);
//        cout<<"this loop completed"<<endl;
//    }
//    return;
//}
//
void Match::output_matched_subgraph_step(int _matched_query_v_cnt)
{
    if (numofembeddings > bound) return;
    if(_matched_query_v_cnt==this->query_v_num)
    {
double vsize, rss;
Util::process_mem_usage(vsize, rss);
if (vsize > max_mem)
    max_mem = vsize;
        // all query matched
        // print matched_data_v_list
#ifdef PRINT_RESULT
        this->io_ptr->output(matched_data_v_list,this->query_v_num);
#endif
        ++numofembeddings;
        return;
    }
    int data_hv_id=tmp_matched_graph[_matched_query_v_cnt];
    vector<int>* data_v_vec_ptr=&(this->hyper_data_graph->real_vertices_list[data_hv_id]);
    for(vector<int>::iterator iter=data_v_vec_ptr->begin();iter!=data_v_vec_ptr->end();++iter)
    {
        int data_v_id=*iter;
        if(data_v_visited[data_v_id])
            continue;
        data_v_visited[data_v_id]=true;
        matched_data_v_list[_matched_query_v_cnt]=data_v_id;
        this->output_matched_subgraph_step(_matched_query_v_cnt+1);
        data_v_visited[data_v_id]=false;
        if (numofembeddings > bound) return;
    }
}

//void Match::output_subgraph_matched(int* _matched_data_hv_list)
void Match::output_subgraph_matched()
{
    //bool* data_v_visited=(bool*)malloc(this->data_v_num*sizeof(bool));
    //for(int data_v_id=0; data_v_id<this->data_v_num;++data_v_id)
    //{
        //data_v_visited[data_v_id]=false;
    //}
    //int* matched_data_v_list=(int*)malloc(this->query_v_num*sizeof(int));
    //this->output_matched_subgraph_step(0,_matched_data_hv_list, data_v_visited, matched_data_v_list);
    this->output_matched_subgraph_step(0);
    
    // io_ptr->output(_matched_subgraph,this->query_v_num);
}

void Match::get_subgraph_matched()
{
    // get a subgraph match
    //int* tmp_matched_graph=(int*)malloc(this->query_v_num*sizeof(int));
    for(int query_v_id=0;query_v_id<this->query_v_num;++query_v_id)
    {
        tmp_matched_graph[query_v_id]=this->query_v_to_data_hv[query_v_id];
    }
    // cout<<"matched hv graph:"<<endl;
    // for(int query_v_id=0;query_v_id<this->query_v_num;++query_v_id)
    // {
    //     cout<<tmp_matched_graph[query_v_id]<<" ";
    // }
    // cout<<endl;
    //this->output_subgraph_matched(tmp_matched_graph);
    this->output_subgraph_matched();
    // cout<<"this matched subgraph written"<<endl;
}


bool Match::exist_query_edge(int _query_v_a,int _query_v_b)
{
    bool edge_found=false;
    std::vector<Neighbor>* a_nei_vec_ptr =&this->query_graph->vertices[_query_v_a].neighbors;
    int a_nei_num=a_nei_vec_ptr->size();
    for(int a_nei_pos=0;a_nei_pos<a_nei_num;++a_nei_pos)
    {
        if((*a_nei_vec_ptr)[a_nei_pos].vid==_query_v_b)
        {
            edge_found=true;
            break;
        }
    }
    return edge_found;
}

bool Match::exist_data_edge(int _data_hv_a, int _data_hv_b)
{
    if(_data_hv_a==_data_hv_b)
    {
        int hv_type=this->hyper_data_graph->hv_type_list[_data_hv_a];
        if(hv_type==1)
            return true;
        else
        {
            return false;
        }   
    }
    else
    {
        vector<HV_Neighbour_Group_Specific_Label>* a_nei_group_vec_ptr=&(this->hyper_data_graph->hv_neighbour_group_by_label_by_hv[_data_hv_a]);
        for(vector<HV_Neighbour_Group_Specific_Label>::iterator group_iter=a_nei_group_vec_ptr->begin();group_iter!=a_nei_group_vec_ptr->end();
        ++group_iter)
        {
            vector<int>* nei_hv_vec_ptr=&((*group_iter).hv_list);
            for(vector<int>::iterator iter=nei_hv_vec_ptr->begin();iter!=nei_hv_vec_ptr->end();++iter)
            {
                int nei_hv=*iter;
                if(nei_hv==_data_hv_b)
                    return true;
            }
        }
    }
    return false;
    
}

bool Match::check_isjoinable(int _query_v_id,int _data_hv_id)
{
    // qv2dv is not empty
    for(map<int,int>::iterator iter=this->query_v_to_data_hv.begin();iter!=this->query_v_to_data_hv.end();++iter)
    {
        int tmp_query_v=iter->first;
        int tmp_data_hv=iter->second;
        if(exist_query_edge(_query_v_id,tmp_query_v))
        {
            if(!exist_data_edge(_data_hv_id,tmp_data_hv))
            {
                return false;
            }
        }
    }
    return true;
}

void Match::non_dcl_search(int _query_vid,int _matched_q_num)
{
    if (numofembeddings > bound) return;
    ++ncalls;
    // cout<<"non_dcl_search query_vid:"<<_query_vid<<endl;
    if(_matched_q_num==this->query_v_num)
    {
        // get a subgraph match successfully
        // cout<<"one matched graph"<<endl;
        this->get_subgraph_matched();
        //double vsize, rss;
		//Util::process_mem_usage(vsize, rss);
		//if (vsize > max_mem)
			//max_mem = vsize;
        return;
    }
    //bool at_least_one_match=false;
    set<int>* query_v_candidates_ptr= &(this->query_graph->vertices[_query_vid].candidates);
    
    for(set<int>::iterator iter=(*query_v_candidates_ptr).begin();iter!=(*query_v_candidates_ptr).end();++iter)
    {
        int cand_data_hv_id=*iter;
        bool try_visit_hv=this->visit_data_hv(cand_data_hv_id);
        if(!try_visit_hv)
        {
            // all corresponding v are visited
            continue;
        }
        bool cand_isjoinable=check_isjoinable(_query_vid,cand_data_hv_id);
        // if(_matched_q_num==0)
        // {
        //     cout<<"matched_q_num: "<<_matched_q_num<<" query_v_id: "<<_query_vid;
        //     cout<<" cand_data_hv "<<cand_data_hv_id<<" is joinable: "<<cand_isjoinable<<endl;
        // }
        if(!cand_isjoinable)
        {
            // --this->qv_match_seq_pos;
            ++hv_remaining_v_num_list[cand_data_hv_id];
            continue;
        }
        else
        { 
            this->query_v_to_data_hv.insert(pair<int,int>(_query_vid,cand_data_hv_id));
	    
	    if(_matched_q_num==this->query_v_num-1)
	    {
		non_dcl_search(-1,this->query_v_num);
	    }
	    else
	    {
              int next_qv=this->get_next_qv();
	      this->qv_visited_list[next_qv]=true;
              non_dcl_search(next_qv,_matched_q_num+1);
	      this->qv_visited_list[next_qv]=false;
            //if(search_res)
                //at_least_one_match=true;
	    }
            this->query_v_to_data_hv.erase(_query_vid);
            ++this->hv_remaining_v_num_list[cand_data_hv_id];
        }
    }
    return;
}

void Match::match(IO* io,int _nei_radius,int _global_refine_level)
{
    this->io_ptr=io;
    
    if(this->query_v_num>this->data_v_num)
    {
        cerr<<"query vertices num > data vertices num"<<endl;
        return;
    }

    // cout<<"query vertex num: "<<this->query_v_num<<endl;
    // cout<<"data hyper vertex num: "<<this->data_hv_num<<endl;
    // cout<<"data vertex num: "<<this->data_v_num<<endl;

    for(int query_vid=0;query_vid<this->query_v_num;++query_vid)
    {
        this->init_candidates(query_vid);
    }
    // cout<<"candidates initialized"<<endl;

    //print all candidates
    // cout<<"before local filter"<<endl;
    // for(int qv=0;qv<this->query_v_num;++qv)
    // {
    //     cout<<qv<<" candidates: ";
    //     set<int>* candidates=&this->query_graph->vertices[qv].candidates;
    //     for(set<int>::iterator iter=(*candidates).begin();iter!=(*candidates).end();++iter)
    //     {
    //         int cand=*iter;
    //         cout<<cand<<" ";
    //     }
    //     cout<<endl;
    // }
    // cout<<"local filter begin"<<endl;
    this->local_filter(_nei_radius);
    // cout<<"local filter end"<<endl;
    // cout<<"after local filter"<<endl;
    // for(int qv=0;qv<this->query_v_num;++qv)
    // {
    //     cout<<" candidates of qv "<<qv<<" : "<<endl;
    //     set<int>* candidates=&this->query_graph->vertices[qv].candidates;
    //     for(set<int>::iterator iter=(*candidates).begin();iter!=(*candidates).end();++iter)
    //     {
    //         int cand=*iter;
    //         cout<<cand<<" ";
    //     }
    //     cout<<endl;
    //     cout<<endl;
    // }

    this->global_refine(_global_refine_level);

    //cout<<"after global refine"<<endl;
    // for(int qv=0;qv<this->query_v_num;++qv)
    // {
    //     cout<<qv<<" candidates: ";
    //     set<int>* candidates=&this->query_graph->vertices[qv].candidates;
    //     for(set<int>::iterator iter=(*candidates).begin();iter!=(*candidates).end();++iter)
    //     {
    //         int cand=*iter;
    //         cout<<cand<<" ";
    //     }
    //     cout<<endl;
    // }

    // cout<<"begin join"<<endl;
    //this->construct_qv_join_seq();
    // cout<<"qv join seq:"<<endl;
    // for(int q_pos=0;q_pos<this->query_v_num;++q_pos)
    // {
    //     int qv_to_match=this->query_v_match_seq[q_pos];
    //     cout<<qv_to_match<<",";
    // }
    // cout<<endl;
    this->qv_visited_list=(bool*)malloc(this->query_v_num*sizeof(bool));
    for(int qv_i=0;qv_i<this->query_v_num;++qv_i)
    {
        this->qv_visited_list[qv_i]=false;
    }

    int join_start_query_vid=this->get_first_qv();

    this->build_data_hv_is_cand_list(join_start_query_vid);
    // cout<<"data_hv_is_cand_list:"<<endl;
    // for(vector<bool>::iterator iter=hv_is_cand_list.begin();iter!=hv_is_cand_list.end();++iter)
    // {
    //     cout<<*iter<<" ";
    // }
    // cout<<endl;
    this->build_data_hv_ump_list();
    // cout<<"hv_ump_list: "<<endl;
    // for(vector<int>::iterator iter=this->hv_ump_list.begin();iter!=this->hv_ump_list.end();++iter)
    // {
    //     cout<<*iter<<" ";
    // }
    // cout<<endl;
    this->build_remaining_v_num_list();
    // cout<<"hv_remaining_v_num_list: "<<endl;
    // for(vector<int>::iterator iter=this->hv_remaining_v_num_list.begin();iter!=this->hv_remaining_v_num_list.end();++iter)
    // {
    //     cout<<*iter<<' ';
    // }
    // cout<<endl;
    this->init_root_hv_searched_list();

    //double max_mem_usage = 0, vsize, rss;
    //Util::process_mem_usage(vsize, rss);
    //if(vsize > max_mem_usage)
        //max_mem_usage = vsize;

    // this->root_dcl_search(join_start_query_vid);
    // cout<<"begin to search"<<endl;
    this->qv_visited_list[join_start_query_vid]=true;
    this->non_dcl_search(join_start_query_vid,0);

}







// void Match::mark_pair(int _qv,int _dv)
// {
//     if(mark_qv_dv.find(_qv)==mark_qv_dv.end())
//     {
//         set<int> mark_dv;
//         mark_dv.insert(_dv);
//         mark_qv_dv.insert(pair<int, set<int> >(_qv,mark_dv));
//     }
//     else
//     {
//         // key _qv exists
//         this->mark_qv_dv[_qv].insert(_dv);
//     }
// }










// bool Match::tell_duplicate(int* _matched_graph)
// {
//     // sort graph vertices and tell whether duplicate
//     sort(_matched_graph,_matched_graph+this->query_v_num);
//     int matched_graph_num=this->matched_subgraphs.size();
//     if(matched_graph_num==0)
//     {
//         return false;
//     }
//     for(int graph_id=0;graph_id<matched_graph_num;++graph_id)
//     {
//         int* tmp_graph=matched_subgraphs[graph_id];
//         int tmp_duplicate=true;
//         for(int vid=0;vid<this->query_v_num;++vid)
//         {
//             if(_matched_graph[vid]!=tmp_graph[vid])
//             {
//                 tmp_duplicate=false;
//                 break;
//             }
//         }
//         if(tmp_duplicate)
//         {
//             // duplicate
//             return true;
//         }
//     }
//     // not duplicate
//     // add into subgraph list
//     this->matched_subgraphs.push_back(_matched_graph);
//     return false;
// }






