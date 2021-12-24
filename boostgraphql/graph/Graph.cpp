#include "Graph.h"
using namespace std;


Neighbor::Neighbor()
{
	this->vid=-1;
}

Neighbor::Neighbor(int _vid)
{
	this->vid=_vid;
}

QueryVertex::QueryVertex()
{
	this->vid=-1;
	this->vlabel=-1;
}

QueryVertex::~QueryVertex()
{

}

QueryVertex::QueryVertex(int _vid,int _vlabel)
{
    this->vid=_vid;
	this->vlabel=_vlabel;
}

void QueryGraph::add_vertex(int _vid,int _vlb)
{
	this->vertices.push_back(QueryVertex(_vid,_vlb));
}

void QueryGraph::add_edge(int  _from_vid, int _to_vid)
{
	// no direction
	this->vertices[_from_vid].neighbors.push_back(Neighbor(_to_vid));
	this->vertices[_to_vid].neighbors.push_back(Neighbor(_from_vid));

}

int QueryGraph::get_qv_label(int _qvid)
{
	int qv_label=this->vertices[_qvid].vlabel;
	return qv_label;
}



Hyper_Data_Graph::Hyper_Data_Graph()
{
	this->label_num =0;
	this->hv_num=0;
	this->he_num= 0;
	this->v_num=0;
	this->hv_label_list = NULL;
	this->hv_type_list = NULL;
	this->real_vertices_list = NULL;
	this->label_to_hvs_list=NULL;
	this->hv_neighbour_group_by_label_by_hv=NULL;
	this->sc_children_list=NULL;
	this->sc_fathers_list=NULL;
	// drt = NULL;
}
Hyper_Data_Graph::~Hyper_Data_Graph()
{
	if(this->hv_label_list != NULL)
		delete []this->hv_label_list;
	if(this->hv_type_list != NULL)
		delete []this->hv_type_list;
	if(this->real_vertices_list != NULL)
		delete []this->real_vertices_list;
	if(this->label_to_hvs_list != NULL)
		delete []this->label_to_hvs_list;
	if(this->hv_neighbour_group_by_label_by_hv != NULL)
		delete []this->hv_neighbour_group_by_label_by_hv;
	if(this->sc_children_list != NULL)
		delete []this->sc_children_list;
	if(this->sc_fathers_list != NULL)
		delete []this->sc_fathers_list;
}

void Hyper_Data_Graph::addVertex(vector<int>& vals)
{
	int hv_id = vals[0];
	this->hv_label_list[hv_id] = vals[1];
	this->hv_type_list[hv_id] = vals[2];
	for(int i = 3; i < vals.size(); ++i)
	{
		this->real_vertices_list[hv_id].push_back(vals[i]);
	}
	
	this->label_to_hvs_list[this->hv_label_list[hv_id]].push_back(hv_id);
}
int Hyper_Data_Graph::contain(int e, vector <HV_Neighbour_Group_Specific_Label> *temp)
{
	int Size = temp->size();
	for(int i = 0; i < Size; i ++)
	{
		if((*temp)[i].label == e)
			return i;
	}
	return -1;
}
void Hyper_Data_Graph::addEdge(int hv_from, int hv_to)
{
	int to_label = this->hv_label_list[hv_to];
	int nei_group_pos = this->contain(to_label, &(this->hv_neighbour_group_by_label_by_hv[hv_from]));
	if(nei_group_pos != -1)
	{
		this->hv_neighbour_group_by_label_by_hv[hv_from][nei_group_pos].hv_list.push_back(hv_to);
	}
	else
	{
		HV_Neighbour_Group_Specific_Label nei_group;
		nei_group.label = to_label;
		nei_group.hv_list.push_back(hv_to);
		this->hv_neighbour_group_by_label_by_hv[hv_from].push_back(nei_group);
	}
	
	int from_label = this->hv_label_list[hv_from];
	nei_group_pos = contain(from_label, &(this->hv_neighbour_group_by_label_by_hv[hv_to]));
	if(nei_group_pos != -1)
	{
		this->hv_neighbour_group_by_label_by_hv[hv_to][nei_group_pos].hv_list.push_back(hv_from);
	}
	else
	{
		HV_Neighbour_Group_Specific_Label nei_group;
		nei_group.label = from_label;
		nei_group.hv_list.push_back(hv_from);
		this->hv_neighbour_group_by_label_by_hv[hv_to].push_back(nei_group);
	}

	this->nei_hv_vec_by_hv[hv_from].push_back(hv_to);
	this->nei_hv_vec_by_hv[hv_to].push_back(hv_from);
}

void Hyper_Data_Graph::transform()
{
	for(int i = 0; i < this->label_num; ++i)
	{
		sort(this->label_to_hvs_list[i].begin(), this->label_to_hvs_list[i].end());
	}
	for(int i = 0; i < this->hv_num; ++i)
	{
		sort(this->hv_neighbour_group_by_label_by_hv[i].begin(), this->hv_neighbour_group_by_label_by_hv[i].end());
		for(int j = 0; j < this->hv_neighbour_group_by_label_by_hv[i].size(); ++j)
		{
			vector<int>& group_hv_list = this->hv_neighbour_group_by_label_by_hv[i][j].hv_list;
			sort(group_hv_list.begin(), group_hv_list.end());
		}
	}
	for(int i = 0; i < this->hv_num; ++i)
	{
		sort(this->sc_children_list[i].begin(), this->sc_children_list[i].end());
		sort(this->sc_fathers_list[i].begin(), this->sc_fathers_list[i].end());
	}
}
void Hyper_Data_Graph::loadSE(string seFile)
{
	FILE* fp = fopen(seFile.c_str(), "r");
	char* line = NULL;
	size_t len = 0;
	vector<int> integerValues;
	//WARN: C++ getline is very different from C getline function
	getline (&line, &len, fp);
	while ( len != 0 && line[0] == 't')
	{	
		if(strcmp(line, "t # -1\n") == 0)
		{
			break;
		}
		//read the second line: vertexNum, edgeNum, vlabelMax
		getline (&line, &len, fp);
		// insert all the nodes
		
		while(getline (&line, &len, fp) && line[0] == 'v')
		{
			Util::readIntegersFromString(line,integerValues);
			this->addVertex(integerValues);
		}
		if(line[0] == 'e')
		{
		// insert all the edges
		do {
			if (len != 0)
			{
				Util::readIntegersFromString(line, integerValues);
				//cout<<"check: "<<integerValues[0]<<" "<<integerValues[1]<<endl;
				this->addEdge(integerValues[0], integerValues[1]);
			}
		} while(getline(&line, &len, fp) && line[0] == 'e');
		}
		if(strcmp(line, "t # -1\n") == 0)
		{
			break;
		}
	}
	if(line)
	{
		xfree(line);
	}
	fclose(fp);
}

void Hyper_Data_Graph::loadSC(string scFile)
{
	FILE* fp = fopen(scFile.c_str(), "r");
	char* line = NULL;
	size_t len = 0;
	vector<int> integerValues;
	getline (&line, &len, fp);
	while ( len != 0 && line[0] == 't')
	{
		if(strcmp(line, "t # -1\n") == 0)
		{
			break;
		}
		getline (&line, &len, fp);
		while(getline(&line, &len, fp) && line[0] == 'e')
		{
			if (len != 0)
			{
				Util::readIntegersFromString(line, integerValues);
				int src_hvid = integerValues[0], dst_hvid = integerValues[1];
				this->sc_children_list[src_hvid].push_back(dst_hvid);
				this->sc_fathers_list[dst_hvid].push_back(src_hvid);
			}
		}
		if(strcmp(line, "t # -1\n") == 0)
		{
			break;
		}
	}
	if(line)
	{
		xfree(line);
	}
	fclose(fp);
}

void Hyper_Data_Graph::count_v_num()
{
	for(int hv_id=0;hv_id<this->hv_num;++hv_id)
	{
		int hv_v_num=this->real_vertices_list[hv_id].size();
		this->v_num_by_hv.push_back(hv_v_num);
		this->v_num+=hv_v_num;
	}
}


void Hyper_Data_Graph::build(string seFile, string scFile)
{

	FILE* fp = fopen(seFile.c_str(), "r");
	char* line = NULL;
	size_t len = 0;
	vector<int> integerValues;
	getline (&line, &len, fp);
	while ( len != 0 && line[0] == 't')
	{
		if(strcmp(line, "t # -1\n") == 0)
		{
			break;
		}
		getline (&line, &len, fp);
		while(getline (&line, &len, fp) && line[0] == 'v')
		{

			Util::readIntegersFromString(line,integerValues);
			this->label_num = max(this->label_num, integerValues[1]);
			this->hv_num++;
		}
		do{
			if (len != 0)
			{
				this->he_num++;
			}
		}while(getline(&line, &len, fp) && line[0] == 'e');
		if(strcmp(line, "t # -1\n") == 0)
		{
			break;
		}
	}
	if(line)
	{
		xfree(line);
	}
	fclose(fp);
	//initialize
	//cout<<"numVertex: "<<numVertex<<endl;
	this->label_num++;
	this->hv_label_list = new int[this->hv_num];
	this->hv_type_list = new int[this->hv_num];
	this->real_vertices_list=new vector<int>[this->hv_num];
	this->label_to_hvs_list = new vector<int>[this->label_num];
	this->hv_neighbour_group_by_label_by_hv = new vector<HV_Neighbour_Group_Specific_Label>[this->hv_num];
	this->nei_hv_vec_by_hv=new vector<int>[this->hv_num];
	this->sc_children_list = new vector<int>[this->hv_num];
	this->sc_fathers_list = new vector<int>[this->hv_num];

	this->loadSE(seFile);

	this->loadSC(scFile);
	this->transform();
	this->count_v_num();
}


int Hyper_Data_Graph::get_hv_label(int _hv_id)
{
	return this->hv_label_list[_hv_id];
}

int Hyper_Data_Graph::get_hv_v_num(int _hv_id)
{
	return this->v_num_by_hv[_hv_id];
}

int Hyper_Data_Graph::get_hv_actual_degree(int _hv_id)
{
	int hv_actual_degree=-1;
	int hv_v_num=this->get_hv_v_num(_hv_id);

	// degree from this hyper vertex
	int hv_type=this->hv_type_list[_hv_id];
	if(hv_type==0)
		hv_actual_degree=0;
	else if(hv_type==1)
		hv_actual_degree=hv_v_num-1;
	else if(hv_type==2)
		hv_actual_degree=0;
	
	// degree from hv_neighbor
	vector<HV_Neighbour_Group_Specific_Label>* neighbour_group_vec_ptr=&(this->hv_neighbour_group_by_label_by_hv[_hv_id]);
	for(vector<HV_Neighbour_Group_Specific_Label>::iterator group_iter=neighbour_group_vec_ptr->begin(); group_iter!=neighbour_group_vec_ptr->end();++group_iter)
	{
		vector<int>* group_hv_list_ptr= &((*group_iter).hv_list);
		for(vector<int>::iterator iter=group_hv_list_ptr->begin();iter!=group_hv_list_ptr->end();++iter)
		{
			int hv_id=(*iter);
			int hv_v_num=this->get_hv_v_num(hv_id);
			hv_actual_degree+=hv_v_num;
		}
	}
	return hv_actual_degree;

}