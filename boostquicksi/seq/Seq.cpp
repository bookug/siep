#include "Seq.h"
using namespace std;


SEQ_Entry::SEQ_Entry()
{

}

SEQ_Entry::SEQ_Entry(int _parent_entry_id, int _vlabel, int _vid, int _degree)
{
    this->parent_entry_id=_parent_entry_id;
    this->vlabel=_vlabel;
    this->vid=_vid;
    this->degree=_degree;
}


SEQ::SEQ()
{
    seq_len=0;
}


void SEQ::add_seq_entry(int _parent_entry_id, int _vid, \
Weight_Graph* _weight_query_graph_ptr)
{

    int vlabel=_weight_query_graph_ptr->weight_vertices[_vid].vlabel;
    

    this->entries.push_back(SEQ_Entry(_parent_entry_id,vlabel,_vid,0));
    ++this->seq_len;
    int tmp_seq_entry_id=this->seq_len-1;
    this->vid_to_entry_id.insert(pair<int,int>(_vid,tmp_seq_entry_id));
    this->entry_id_to_vid.insert(pair<int,int>(tmp_seq_entry_id,_vid));
    
    if(_parent_entry_id==-1)
    {
        return;
    }
    
    ++this->entries[tmp_seq_entry_id].degree;
    ++this->entries[_parent_entry_id].degree;

}


int SEQ::get_entry_id_by_vid(int _vid)
{
    if(this->vid_to_entry_id.find(_vid)==this->vid_to_entry_id.end())
    {
        // vid not in seq
        return -1;
    }
    else
    {
        return this->vid_to_entry_id[_vid];
    }
}

void SEQ::add_seq_extra_edge(int _left_vid, int _right_vid)
{
    int seq_entry_id_a=this->get_entry_id_by_vid(_left_vid);
    int seq_entry_id_b=this->get_entry_id_by_vid(_right_vid);
    // if(seq_entry_id_a==-1||seq_entry_id_b==-1)
    // {
    //     cerr<< "cannot add this edge into seq"<<endl;
    // }
    if(seq_entry_id_a<seq_entry_id_b)
    {
        // add extra end entry a to entry b
        this->entries[seq_entry_id_b].extra_edges_end_entries.push_back(seq_entry_id_a);
    }
    else
    {
        // add extra end entry b to entry a
        this->entries[seq_entry_id_a].extra_edges_end_entries.push_back(seq_entry_id_b);
    }
    ++this->entries[seq_entry_id_a].degree;
    ++this->entries[seq_entry_id_b].degree;
    
}


void SEQ::print_seq()
{
    printf("tmp SEQ length: %d\n",this->seq_len);
    printf("seq entries: [parent_entry_id,vlabel,vid,degree,[extra_edge_end_entries]]\n");
    for(vector<SEQ_Entry>::iterator iter=this->entries.begin();iter!=this->entries.end();\
    ++iter)
    {
        SEQ_Entry* seq_entry_ptr=&(*iter);
        int parent_entry_id=seq_entry_ptr->parent_entry_id;
        int vlabel=seq_entry_ptr->vlabel;
        int vid=seq_entry_ptr->vid;
        int degree=seq_entry_ptr->degree;
        printf("[%d,%d,%d,%d,[",parent_entry_id,vlabel,vid,degree);
        vector<int>* extra_edge_end_entries_ptr=&(seq_entry_ptr->extra_edges_end_entries);
        for(vector<int>::iterator iter=extra_edge_end_entries_ptr->begin();\
        iter!=extra_edge_end_entries_ptr->end();++iter)
        {
            int end_entry_id=*iter;
            printf("%d,",end_entry_id);
        }
        printf("] ]\n");
    }
    printf("\n");
}



