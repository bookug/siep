/*=============================================================================
# Filename: Graph.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-10-24 23:01
# Description: 
=============================================================================*/

#include "Graph.h"

using namespace std;

void 
DGraph::addVertex(LABEL _vlb)
{
	this->vertices.push_back(Vertex(_vlb));
}

void 
DGraph::addEdge(VID _from, VID _to, LABEL _elb)
{
	this->vertices[_from].out.push_back(DNeighbor(_to, _elb));
	//vector<DNeighbor>& out = this->vertices[_from].out;
	//cout<<"size: "<<out.size()<<endl;
	//DNeighbor tn(_to, _elb);
	//out.push_back(tn);
	this->vertices[_to].in.push_back(DNeighbor(_from, _elb));
    //NOTICE: TurboISO handles undirected graphs by default
	//this->vertices[_to].out.push_back(DNeighbor(_from, _elb));
	//this->vertices[_from].in.push_back(DNeighbor(_to, _elb));
}

void 
DGraph::transformToCSR()
{
	this->vertex_num = this->vertices.size();
	this->vertex_value = new int[this->vertex_num];
	this->row_offset_in = new int[this->vertex_num+1];
	this->row_offset_out = new int[this->vertex_num+1];
	this->row_offset_in[0] = this->row_offset_out[0] = 0;
	
	int in_edge_num = 0, out_edge_num = 0, i, j, in_label_num = 0, out_label_num = 0;
	for(i = 0; i < this->vertex_num; ++i)
	{
		this->vertex_value[i] = this->vertices[i].label;
		int insize = this->vertices[i].in.size(), outsize = this->vertices[i].out.size();
		in_edge_num += insize;
		out_edge_num += outsize;
		set<int> lbs;
		for(j = 0; j < insize; ++j)
		{
			lbs.insert(this->vertices[i].in[j].elb);
		}
		in_label_num += lbs.size();
		this->row_offset_in[i+1] = this->row_offset_in[i] + lbs.size();

		lbs.clear();
		for(j = 0; j < outsize; ++j)
		{
			lbs.insert(this->vertices[i].out[j].elb);
		}
		out_label_num += lbs.size();
		this->row_offset_out[i+1] = this->row_offset_out[i] + lbs.size();
	}
	this->column_index_in = new int[in_edge_num];
	this->edge_value_in = new int[in_label_num];
	this->edge_offset_in = new int[in_label_num+1];

	this->column_index_out = new int[out_edge_num];
	this->edge_value_out = new int[out_label_num];
	this->edge_offset_out = new int[out_label_num+1];

	int ink = 0, inpos = 0, outk = 0, outpos = 0;
	for(i = 0; i < this->vertex_num; ++i)
	{
		int insize = this->vertices[i].in.size(), outsize = this->vertices[i].out.size();
		//sort on label, when label is identical, sort on VID
		sort(this->vertices[i].in.begin(), this->vertices[i].in.end());
		sort(this->vertices[i].out.begin(), this->vertices[i].out.end());
		for(j = 0; j < insize; ++j, ++inpos)
		{
			DNeighbor& tn = this->vertices[i].in[j];
			this->column_index_in[inpos] = tn.vid;
			if(j == 0 || tn.elb != this->vertices[i].in[j-1].elb)
			{
				this->edge_value_in[ink] = tn.elb;
				this->edge_offset_in[ink] = inpos;
				ink++;
			}
		}
		//below are for out neighbors
		for(j = 0; j < outsize; ++j, ++outpos)
		{
			DNeighbor& tn = this->vertices[i].out[j];
			this->column_index_out[outpos] = tn.vid;
			if(j == 0 || tn.elb != this->vertices[i].out[j-1].elb)
			{
				this->edge_value_out[outk] = tn.elb;
				this->edge_offset_out[outk] = outpos;
				outk++;
			}
		}
	}
	this->edge_offset_in[in_label_num] = in_edge_num;
	this->edge_offset_out[out_label_num] = out_edge_num;

	//now we can release the memory of original structure 
	this->vertices.clear();

	//to construct inverse label list
	Element* elelist = new Element[this->vertex_num];
	for(i = 0; i <this->vertex_num; ++i)
	{
		elelist[i].id = i;
		elelist[i].label = this->vertex_value[i];
	}
	sort(elelist, elelist+this->vertex_num);

	int label_num = 0;
	for(i = 0; i <this->vertex_num; ++i)
	{
		if(i == 0 || elelist[i].label != elelist[i-1].label)
		{
			label_num++;
		}
	}

	this->label_num = label_num;
	this->inverse_label = new int[label_num];
	this->inverse_offset = new int[label_num+1];
	this->inverse_vertex = new int[this->vertex_num];
	j = 0;
	for(i = 0; i <this->vertex_num; ++i)
	{
		this->inverse_vertex[i] = elelist[i].id;
		if(i == 0 || elelist[i].label != elelist[i-1].label)
		{
			this->inverse_label[j] = elelist[i].label;
			this->inverse_offset[j] = i;
			++j;
		}
	}
	this->inverse_offset[label_num] = this->vertex_num;

	delete[] elelist;
}

bool 
DGraph::isEdgeContained(VID from, VID to, LABEL label)
{
	vector<DNeighbor>& out = this->vertices[from].out;
	for(int i = 0; i < out.size(); ++i)
	{
		if(out[i].vid == to && out[i].elb == label)
		{
			return true;
		}
	}
	return false;
}

