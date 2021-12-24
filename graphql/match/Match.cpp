#include "Match.h"

using namespace std;

//#define DEBUG 1
extern long bound, numofembeddings, ncalls;
extern double max_mem;

Match::Match(QueryGraph* _query, DataGraph* _data, int n_, int b_)
{


	this->query_graph=_query;
    this->data_graph=_data;
    this->query_v_num=_query->vertices.size();
    this->data_v_num=_data->vertices.size();
    // this->output_dir=_output_dir;
    

    
    this->dv_visited=(bool*)malloc(data_v_num*sizeof(bool));

    for(int di=0;di<data_v_num;++di)
    {
        this->dv_visited[di]=false;
    }

    qv_degrees=(int*)malloc(query_v_num*sizeof(int));
    for(int qi=0;qi<query_v_num;++qi)
    {
        int tmp_qv_degree=query_graph->vertices[qi].neighbors.size();
        qv_degrees[qi]=tmp_qv_degree;
    }
    
    // this->qv_select_seq_pos=-1;
    this->tmp_matched_graph=vector<int>(query_v_num);
}

Match::~Match()
{
    if(this->dv_visited!=NULL)
    {
        free(this->dv_visited);
        this->dv_visited= NULL;
    }
    if(this->qv_degrees!=NULL)
    {
        free(this->qv_degrees);
        this->qv_degrees=NULL;
    }
    if(this->qv_visited_list!=NULL)
    {
        free(this->qv_visited_list);
        this->qv_visited_list=NULL;
    }
}

bool Match::dv_qv_pre_match(DataVertex* _dv_ptr,QueryVertex* _qv_ptr)
{
    int qv_predicate_num=_qv_ptr->neighbors.size();
    int dv_predicate_num=_dv_ptr->neighbors.size();
    if(dv_predicate_num<qv_predicate_num)
        return false;

    return true;
    
}

bool Match::dv_qv_label_match(DataVertex* _dv,QueryVertex* _qv)
{
    if(_dv->vlabel==_qv->vlabel)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Match::add_candidate(int _q_vid,int _d_vid)
{
    // add _d_vid to candidate list of _q_vid
    this->query_graph->vertices[_q_vid].candidates.insert(_d_vid);
}

void Match::init_candidates(int _q_vid)
{
    // candidates for each query vetex, matching predicates
    QueryVertex* q_vertex_ptr=&(this->query_graph->vertices[_q_vid]);
    for(int data_vid=0;data_vid<this->data_v_num;++data_vid)
    {
        bool tmp_pre_match= dv_qv_pre_match(&this->data_graph->vertices[data_vid],q_vertex_ptr);
        if(!tmp_pre_match)
        {
            continue;
        }

        bool tmp_label_match= dv_qv_label_match(&this->data_graph->vertices[data_vid],q_vertex_ptr);
        if(!tmp_label_match)
        {
            continue;
        }
        // add to candidate list
        add_candidate(_q_vid,data_vid);
    }
}

//void Match::prepare_for_search()
//{
//    bool* qv_visited=(bool*)malloc(query_v_num*sizeof(bool));
//    for(int qi=0;qi<query_v_num;++qi)
//    {
//        qv_visited[qi]=false;
//    }
//
//    this->qv_select_seq=(int*)malloc(query_v_num*sizeof(int));
//    // choose remaining vertex with least degree
//    for(int i=0;i<this->query_v_num;++i)
//    {
//        this->qv_select_seq[i]=construct_next_qv(qv_visited);
//    }
//    this->qv_select_seq_pos=0;
//    free(qv_visited);
//
//}
//
//int Match::construct_next_qv(bool* qv_visited)
//{
//    // choose vertex with least degree in remaining qurey vertices
//    int min_degree_v=-1;
//    int min_degree=1e8;
//    for(int qv_i=0;qv_i<this->query_v_num;++qv_i)
//    {
//        if(qv_visited[qv_i])
//        {
//            continue;
//        }
//        int tmp_degree=qv_degrees[qv_i];
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
// void Match::init_dv_matched_seq()
// {
//     // each dv match a qv
//     this->dv_matched_seq=(int*)malloc(this->query_v_num,sizeof(int));
//     for(int i=0;i<this->query_v_num;++i)
//     {
//         dv_matched_seq[i]=-1;
//     }
//     this->dv_matched_seq_pos=0;
// }

int Match::get_first_qv()
{
// min candidate set, max degree
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
// connectivity, min cand set
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

bool Match::exist_query_edge(int _qv_a,int _qv_b)
{
    bool edge_found=false;
    std::vector<Neighbor>* a_nei_vec_ptr =&this->query_graph->vertices[_qv_a].neighbors;
    int a_nei_num=a_nei_vec_ptr->size();
    for(int a_nei_pos=0;a_nei_pos<a_nei_num;++a_nei_pos)
    {
        if((*a_nei_vec_ptr)[a_nei_pos].vid==_qv_b)
        {
            edge_found=true;
            break;
        }
    }
    return edge_found;
}

bool Match::exist_data_edge(int _dv_a, int _dv_b)
{
    bool edge_found=false;
    std::vector<Neighbor>* a_nei_vec_ptr =&this->data_graph->vertices[_dv_a].neighbors;
    int a_nei_num=(*a_nei_vec_ptr).size();
    for(int a_nei_pos=0;a_nei_pos<a_nei_num;++a_nei_pos)
    {
        if((*a_nei_vec_ptr)[a_nei_pos].vid==_dv_b)
        {
            edge_found=true;
            break;
        }
    }
    return edge_found;
}

bool Match::check(int _qv,int _dv)
{
    // check all _qv-related edges in temp graph
    if(this->qv2dv.empty())
    {
        return true;
    }
    // qv2dv is not empty
    for(map<int,int>::iterator it=qv2dv.begin();it!=qv2dv.end();++it)
    {
        int tmp_qv=it->first;
        int tmp_dv=it->second;
        if(exist_query_edge(_qv,tmp_qv))
        {
            if(!exist_data_edge(_dv,tmp_dv))
            {
                return false;
            }
        }
    }
    return true;
}

bool Match::tell_duplicate(int* _matched_graph)
{
    // sort graph vertices and tell whether duplicate
    sort(_matched_graph,_matched_graph+this->query_v_num);
    int matched_graph_num=this->matched_subgraphs.size();
    if(matched_graph_num==0)
    {
        return false;
    }
    for(int graph_id=0;graph_id<matched_graph_num;++graph_id)
    {
        int* tmp_graph=matched_subgraphs[graph_id];
        int tmp_duplicate=true;
        for(int vid=0;vid<this->query_v_num;++vid)
        {
            if(_matched_graph[vid]!=tmp_graph[vid])
            {
                tmp_duplicate=false;
                break;
            }
        }
        if(tmp_duplicate)
        {
            // duplicate
            return true;
        }
    }
    // not duplicate
    // add into subgraph list
    this->matched_subgraphs.push_back(_matched_graph);
    return false;
}


void Match::output_subgraph()
{
#ifdef PRINT_RESULT
    io_ptr->output(tmp_matched_graph,this->query_v_num);
#endif
}

void Match::get_subgraph_matched()
{
    // get a subgraph match
    // might duplicate
    //int* tmp_matched_graph=(int*)malloc(this->query_v_num*sizeof(int));
    map<int,int>::iterator iter = qv2dv.begin();
    for(int i=0;i<this->query_v_num;++i)
    {
        tmp_matched_graph[i]=iter->second;
        ++iter;
    }
    //bool duplicate_graph=tell_duplicate(tmp_matched_graph);
    bool duplicate_graph = false;
    if(!duplicate_graph)
    {
        // this subgraph is matcded and unique
        // vertices in this subgraph is sorted
        output_subgraph();
        // cout<<"output a match"<<endl;
	//cerr<<"OK:"<<numofembeddings<<endl;
	    numofembeddings++;	// jiangyan add
    }
}

void Match::search(int _qv,int _matched_q_num)
{
    ++ncalls;
	if (numofembeddings > bound) return;	// jiangyan add
    if(_matched_q_num==this->query_v_num)
    {
        // get a subgraph match successfully
        // cout<<"one matched graph, check duplicate"<<endl;
        this->get_subgraph_matched();
        double vsize, rss;
		Util::process_mem_usage(vsize, rss);
		if (vsize > max_mem)
			max_mem = vsize;
        return;
    }
    set<int>* qv_candidates= &this->query_graph->vertices[_qv].candidates;
    
    for(set<int>::iterator iter=(*qv_candidates).begin();iter!=(*qv_candidates).end();++iter)
    {
        int cand_dv=*iter;
        if(dv_visited[cand_dv])
        {
            continue;
        }
        bool cand_satisfy=this->check(_qv,cand_dv);
        if(cand_satisfy)
        { 
            // a step deeper
            dv_visited[cand_dv]=true;
            this->qv2dv.insert(pair<int,int>(_qv,cand_dv));
	    
	    if(_matched_q_num==this->query_v_num-1)
	    {
		search(-1,this->query_v_num);
	    }
	    else
	    {
                int next_qv=this->get_next_qv();
	        this->qv_visited_list[next_qv]=true;
                search(next_qv,_matched_q_num+1);
	        this->qv_visited_list[next_qv]=false;
	    }
            qv2dv.erase(_qv);
            dv_visited[cand_dv]=false;
        }
if (numofembeddings > bound) return;	// jiangyan add
    }
}

void Match::match(IO* io,int _nei_radius,int _global_refine_level)
{
    this->io_ptr=io;
    
    if(this->query_v_num>this->data_v_num)
    {
        //cerr<<"query vertices > data vertices"<<endl;
        return;
    }

    // cout<<"query vertex num: "<<this->query_v_num<<endl;
    // cout<<"data vertex num: "<<this->data_v_num<<endl;
    for(int query_vid=0;query_vid<this->query_v_num;++query_vid)
    {
        this->init_candidates(query_vid);
    }
    //cout<<"candidates initialized"<<endl;

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
    // cout<<endl;
    // cout<<"local filter begins"<<endl;
    this->local_filter(_nei_radius);
    // cout<<"after local filter"<<endl;
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
    // cout<<endl;

    this->global_refine(_global_refine_level);

    // cout<<"after global refine"<<endl;
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
    // cout<<endl;

    //construct_qv_select_seq();

    this->qv_visited_list=(bool*)malloc(this->query_v_num*sizeof(bool));
    for(int qv_i=0;qv_i<this->query_v_num;++qv_i)
    {
    	this->qv_visited_list[qv_i]=false;
    }
    int start_qv=this->get_first_qv();

    double vsize, rss;
    Util::process_mem_usage(vsize, rss);
    if (vsize > max_mem)
        max_mem = vsize;
    this->qv_visited_list[start_qv]=true;
    search(start_qv,0);

}

// void Match::add_qv_nei_profile(int _root_qv,int _nei_radius,set<int>* _qv_nei_profile_ptr,
// bool* _qv_visited)
// {
//     _qv_visited[_root_qv]=true;
//     int root_qv_label=this->query_graph->vertices[_root_qv].vlabel;
//     (*_qv_nei_profile_ptr).insert(root_qv_label);
//     if(_nei_radius==0)
//     {
//         return;
//     }
//     vector<Neighbor>* nei_vec_ptr=&this->query_graph->vertices[_root_qv].neighbors;
//     int rt_qv_nei_num=(*nei_vec_ptr).size();
//     for(int nei_pos=0;nei_pos<rt_qv_nei_num;++nei_pos)
//     {
//         int nei_qv_id=(*nei_vec_ptr)[nei_pos].vid;
//         if(!_qv_visited[nei_qv_id])
//         {
//             add_qv_nei_profile(nei_qv_id,_nei_radius-1,_qv_nei_profile_ptr,_qv_visited);
//         }
//     }
// }

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
    this->add_qv_nei_profile(&qv_next_hop,_nei_radius-1,_qv_profile,_qv_visited);
    // set<int>().swap(qv_next_hop);
}

void Match::add_dv_nei_profile(set<int>* _dv_this_hop,int _nei_radius,set<int>* _dv_profile,
bool* _dv_visited)
{
    // cout<<"_nei_radius: "<<_nei_radius<<endl;
    // cout<<"dv this hop: ";
    // for(set<int>::iterator iter=_dv_this_hop->begin();iter!=_dv_this_hop->end();++iter)
    // {
    //     cout<<*iter<<" ";
    // }
    // cout<<endl;

    for(set<int>::iterator iter=(*_dv_this_hop).begin();iter!=(*_dv_this_hop).end();++iter)
    {
        int tmp_dv=*iter;
        // cout<<"tmp_dv: "<<tmp_dv<<endl;
        _dv_visited[tmp_dv]=true;
        int tmp_dv_label=this->data_graph->vertices[tmp_dv].vlabel;
        (*_dv_profile).insert(tmp_dv_label);
    }
    if(_nei_radius==0)
    {
        return;
    }

    // next hop
    set<int> dv_next_hop;
    for(set<int>::iterator iter=(*_dv_this_hop).begin();iter!=(*_dv_this_hop).end();++iter)
    {
        int this_hop_dv=*iter;
        vector<Neighbor>* neighbors_ptr=&this->data_graph->vertices[this_hop_dv].neighbors;
        int nei_num=(*neighbors_ptr).size();
        for(int nei_pos=0;nei_pos<nei_num;++nei_pos)
        {
            int nei_dvid=(*neighbors_ptr)[nei_pos].vid;
            if(!_dv_visited[nei_dvid])
            {
                dv_next_hop.insert(nei_dvid);
            }
        }
    }
    add_dv_nei_profile(&dv_next_hop,_nei_radius-1,_dv_profile,_dv_visited);
    // set<int>().swap(dv_next_hop);
}

bool Match::dv_prof_include_qv_prof(set<int>* _dv_profile, set<int>* _qv_profile)
{
    set<int>::iterator q_iter=(*_qv_profile).begin();
    set<int>::iterator d_iter=(*_dv_profile).begin();
    set<int>::iterator q_end=(*_qv_profile).end();
    set<int>::iterator d_end=(*_dv_profile).end();
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
    // reduce candidates of _qv

    set<int> qv_profile;
    bool* qv_visited=(bool*)malloc(this->query_v_num*sizeof(bool));
    for(int i=0;i<this->query_v_num;++i)
    {
        qv_visited[i]=false;
    }
    
    set<int> qv_this_hop;
    qv_this_hop.insert(_qv);
    // cout<<"gen profile of qv: "<<_qv<<endl;
    add_qv_nei_profile(&qv_this_hop,_nei_radius,&qv_profile,qv_visited);

    free(qv_visited);



    // cout<<"qv "<<_qv<<" profile: ";
    // for(set<int>::iterator iter=qv_profile.begin();iter!=qv_profile.end();++iter)
    // {
    //     cout<<*iter<<" ";
    // }
    // cout<<endl;


    set<int>* candidates_ptr=&this->query_graph->vertices[_qv].candidates;
    int cand_num=(*candidates_ptr).size();
    //printf("candidate num of qv %d : %d\n",_qv,cand_num);
    // for each candidate of _qv, if its profile does not include _qv profile, remove it
    set<int> new_candidates;
    for(set<int>::iterator iter=(*candidates_ptr).begin();iter!=(*candidates_ptr).end();++iter)
    {
        int cand_dv=*iter;
        set<int> cand_dv_profile;
        bool* dv_visited=(bool*)malloc(this->data_v_num*sizeof(bool));
        for(int i=0;i<this->data_v_num;++i)
        {
            dv_visited[i]=false;
        }
        set<int> dv_this_hop;
        dv_this_hop.insert(cand_dv);
        // cout<<"gen profile of cand_dv: "<<cand_dv<<endl;
        add_dv_nei_profile(&dv_this_hop,_nei_radius,&cand_dv_profile,dv_visited);

        // cout<<"cand_dv "<<cand_dv<<" profile: ";
        // for(set<int>::iterator iter=cand_dv_profile.begin();iter!=cand_dv_profile.end();++iter)
        // {
        //     cout<<*iter<<" ";
        // }
        // cout<<endl;

        // whether dv profile includes qv profile
        bool cand_semi_match= this->dv_prof_include_qv_prof(&cand_dv_profile,&qv_profile);
        // cout<<"candidate semi match: "<<cand_semi_match<<endl;
        if(cand_semi_match)
        {
            new_candidates.insert(cand_dv);
        }
        set<int>().swap(cand_dv_profile);
        set<int>().swap(dv_this_hop);
        free(dv_visited);
    }
    // set<int>().swap(this->query_graph->vertices[_qv].candidates);
    this->query_graph->vertices[_qv].candidates=new_candidates;

    
    set<int>().swap(qv_profile);
    set<int>().swap(qv_this_hop);
    // cout<<"========="<<endl;
}

void Match::local_filter(int _nei_radius)
{
    for(int qv=0;qv<this->query_v_num;++qv)
    {
        this->qv_local_filter(qv,_nei_radius);
    }
}

bool Match::exist_equal(set<int>* _dv_set, set<int>* _cand_set)
{
    set<int>::iterator dv_set_iter=(*_dv_set).begin();
    set<int>::iterator cand_set_iter=(*_cand_set).begin();
    set<int>::iterator dv_set_end=(*_dv_set).end();
    set<int>::iterator cand_set_end=(*_cand_set).end();
    while(dv_set_iter!=dv_set_end&&cand_set_iter!=cand_set_end)
    {
        int dv_set_ele=*dv_set_iter;
        int cand_set_ele=*cand_set_iter;
        if(dv_set_ele==cand_set_ele)
            return true;
        else if(dv_set_ele<cand_set_ele)
        {
            ++dv_set_iter;
        }
        else if(dv_set_ele>cand_set_ele)
        {
            ++cand_set_iter;
        }
    }
    return false;
}

bool Match::bipart_semi_perf(int _qv,int _dv)
{
    vector<Neighbor>* qv_neighbors=&this->query_graph->vertices[_qv].neighbors;
    set<int> qv_nei_ids;
    int qv_nei_num=(*qv_neighbors).size();
    for(int pos=0;pos<qv_nei_num;++pos)
    {
        int tmp_nei_qvid=(*qv_neighbors)[pos].vid;
        qv_nei_ids.insert(tmp_nei_qvid);
    }
    vector<Neighbor>* dv_neighbors=&this->data_graph->vertices[_dv].neighbors;
    set<int> dv_nei_ids;
    int dv_nei_num=(*dv_neighbors).size();
    for(int pos=0;pos<dv_nei_num;++pos)
    {
        int tmp_nei_dvid=(*dv_neighbors)[pos].vid;
        dv_nei_ids.insert(tmp_nei_dvid);
    }
    for(set<int>::iterator iter=qv_nei_ids.begin();iter!=qv_nei_ids.end();++iter)
    {
        int nei_qv=*iter;
        set<int>* nei_qv_candidates=&this->query_graph->vertices[nei_qv].candidates;
        bool dvs_satisfy_nei_qv=exist_equal(&dv_nei_ids,nei_qv_candidates);
        if(!dvs_satisfy_nei_qv)
        {
            // _dv is not candidate of _qv
            return false;
        }
    }
    return true;
}

bool Match::is_candidate(int _qv, int _dv)
{
    set<int>* candidates=&this->query_graph->vertices[_qv].candidates;
    for(set<int>::iterator iter=(*candidates).begin();iter!=(*candidates).end();++iter)
    {
        int tmp_cand=*iter;
        if(_dv==tmp_cand)
            return true;
    }
    return false;
}

void Match::mark_pair(int _qv,int _dv)
{
    if(mark_qv_dv.find(_qv)==mark_qv_dv.end())
    {
        set<int> mark_dv;
        mark_dv.insert(_dv);
        mark_qv_dv.insert(pair<int, set<int> >(_qv,mark_dv));
    }
    else
    {
        // key _qv exists
        this->mark_qv_dv[_qv].insert(_dv);
    }  
}

void Match::mark_potential(int _rt_qv,int _rt_dv)
{
    vector<Neighbor>* q_neighbors=&this->query_graph->vertices[_rt_qv].neighbors;
    set<int> nei_qvs;
    for(vector<Neighbor>::iterator iter=(*q_neighbors).begin();iter!=(*q_neighbors).end();++iter)
    {
        int nei_qv=(*iter).vid;
        nei_qvs.insert(nei_qv);
    }
    vector<Neighbor>* d_neighbors=&this->data_graph->vertices[_rt_dv].neighbors;
    set<int> nei_dvs;   
    for(vector<Neighbor>::iterator iter=(*d_neighbors).begin();iter!=(*d_neighbors).end();++iter)
    {
        int nei_dv=(*iter).vid;
        nei_dvs.insert(nei_dv);
    }
    for(set<int>::iterator q_iter=nei_qvs.begin();q_iter!=nei_qvs.end();++q_iter)
    {
        int nei_qv=*q_iter;
        if(nei_qv==_rt_qv)
        {
            // cannot link to oneself
            continue;
        }
        for(set<int>::iterator d_iter=nei_dvs.begin();d_iter!=nei_dvs.end();++d_iter)
        {
            int nei_dv=*d_iter;
            bool is_cand=is_candidate(nei_qv,nei_dv);
            if(is_cand)
            {
                mark_pair(nei_qv,nei_dv);
            }
        }
    }
}

void Match::global_refine_search(int _global_refine_level)
{
    if(_global_refine_level==0)
        return;
    if(this->mark_qv_dv.empty())
    {
        // no mark
        return;
    }
    for(int qv=0;qv<this->query_v_num;++qv)
    {
        if(this->mark_qv_dv.find(qv)==mark_qv_dv.end())
        {
            continue;
        }
        set<int>* marked_cand=&this->mark_qv_dv.at(qv);
        for(set<int>::iterator iter=(*marked_cand).begin();iter!=(*marked_cand).end();++iter)
        {
            int cand_dv=*iter;
            bool semi_perf_match=bipart_semi_perf(qv,cand_dv);
            if(semi_perf_match)
            {
                continue;
            }
            else
            {
                // cand_dv is not candidate of qv
                this->query_graph->vertices[qv].candidates.erase(cand_dv);
                //  potential changes
                mark_potential(qv,cand_dv);
            }   
        }
        // unmark (qv,*) 
        set<int>().swap(*marked_cand);
        this->mark_qv_dv.erase(qv);
    }

    global_refine_search(_global_refine_level-1);
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
        this->mark_qv_dv.insert(pair<int,set<int> >(qv,qv_candidates));
    }
    
    this->global_refine_search(_global_refine_level);
    for(map<int,set<int> >::iterator iter_out=this->mark_qv_dv.begin();iter_out!=mark_qv_dv.end();++iter_out)
    {
        set<int>().swap((*iter_out).second);
    }
    map<int,set<int> >().swap(this->mark_qv_dv);
}


