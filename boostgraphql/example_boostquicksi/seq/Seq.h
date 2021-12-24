#ifndef _SEQ_H
#define _SEQ_H
#include "../util/Util.h"
#include "../graph/Graph.h"

class SEQ_Entry
{
public:
    int parent_entry_id;
    int vlabel;
    int vid;
    int degree;
    vector<int> extra_edges_end_entries;

    SEQ_Entry();
    SEQ_Entry(int _parent_entry_id, int _vlabel, int _vid, int _degree);

};



class SEQ
{
public:
    vector<SEQ_Entry> entries;
    int seq_len;
    map<int,int> vid_to_entry_id;
    map<int,int> entry_id_to_vid;

    SEQ();
    // for the first entry, its parent entry id is -1 and its initial degree is 0
    void add_seq_entry(int _parent_entry_id, int _vid, Weight_Graph* _weight_query_graph_ptr);
    int get_entry_id_by_vid(int _vid);
    void add_seq_extra_edge(int _left_vid, int _right_vid);
    void print_seq();
};

#endif