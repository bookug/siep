#ifndef _GRAPH_GRAPH_H
#define _GRAPH_GRAPH_H

#include "../util/Util.h"
using namespace std;

class Neighbor
{
public:
	int vid;
	// no elabel
	Neighbor();
	Neighbor(int _vid);
	bool operator<(const Neighbor& _nei) const
	{

		return this->vid < _nei.vid;

	}
};



class QueryVertex
{
public:
	
	int vid;
	int vlabel;
	std::vector<Neighbor> neighbors;
	std::set<int> candidates;
	
	QueryVertex();
    ~QueryVertex();
    QueryVertex(int _vid,int _vlabel);
};

class QueryGraph
{
public:
	std::vector<QueryVertex> vertices;
	QueryGraph() { }
	~QueryGraph() { }

	void add_vertex(int _vid,int  _vlb);
	void add_edge(int _from_vid, int _to_vid);
    int get_qv_label(int _qvid);

};




struct HV_Neighbour_Group_Specific_Label
{
	int label;
	vector <int> hv_list;
    bool operator<(const HV_Neighbour_Group_Specific_Label& nei) const
    {
        return this->label < nei.label;
    }
    bool operator==(const HV_Neighbour_Group_Specific_Label& nei) const
    {
        return this->label == nei.label;
    }
};

class QDN
{
public:
    //int key;
    int num;  //number of QDC parent
    vector<int> qdcc;   //QDC children
    vector<int> qdel;   //QDE list
    void clear()
    {
        //key = -1;
        qdcc.clear();
        qdel.clear();
    }
};

// class HyperVertex
// {
// public:
//     int label;
//     int type; //0 single vertex   1 a clique   2 not a clique
//     vector<int> vertices;   //real vertices corresponding to this hypervertex
//     //se edges: undirected
//     vector<labelVlist> see;  //SE edges
//     //sc edges: only exist between same-label vertices
//     vector<int> scc;   //SC children
//     vector<int> scf;   //SC father
//     HyperVertex()
//     {
//     }
//     HyperVertex(vector<int>& vals)
//     {
//         label = vals[1];
//         type = vals[2];
//         for(int i = 3; i < vals.size(); ++i)
//         {
//             vertices.push_back(vals[i]);
//         }
//     }
// };

class Hyper_Data_Graph							//gsh
{
public:
	int label_num;					
	int hv_num;					
    int he_num;
    int v_num;
	int *hv_label_list;			//vertex label
    int *hv_type_list;         //vertex type, 0 single, 1 clique, 2 not clique
    vector<int>* real_vertices_list;   //real vertices corresponding to this hyper vertex
	vector <int> *label_to_hvs_list;		//inverse label list
	vector <HV_Neighbour_Group_Specific_Label> *hv_neighbour_group_by_label_by_hv;	//SE edges
    vector<int>* nei_hv_vec_by_hv;
    vector<int> *sc_children_list;   //SC children
    vector<int> *sc_fathers_list;   //SC father
    // vector<QDN>* drt;   //DRT for QDN, vector<QDN> (an array of all hyper vertices in GSH) for each query node

    vector<int> v_num_by_hv;
    vector<int> actual_degree_by_hv;

	Hyper_Data_Graph();

	~Hyper_Data_Graph();

	void addVertex(vector<int>& vals);
	int contain(int e, vector <HV_Neighbour_Group_Specific_Label> *temp);
    void addEdge(int hv_from, int hv_to);

    void transform();
    void loadSE(string seFile);
    void loadSC(string scFile);
    void count_v_num();
    void gen_hv_actual_degree();

    void build(string seFile, string scFile);

    int get_hv_label(int _hv_id);
    int get_hv_v_num(int _hv_id);
    int get_hv_actual_degree(int _hv_id);

};

#endif

