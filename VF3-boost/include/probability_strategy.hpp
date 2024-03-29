// #ifndef PROBABILITY_STRATEGY_HPP
// #define PROBABILITY_STRATEGY_HPP

// #include "argraph.hpp"
// typedef int data_t;

// class NodeProbabilityStrategy{  
// public:	
// 	double* degree; //Array of degree probability
// 	double* out_deg; //Array of "out" degree probability
// 	double* in_deg; //Array of "in" degree probability
// 	int degree_size, out_deg_size, in_deg_size;
// 	map<data_t,double> labels; //Map of labels probability
// 	virtual void evaluateProbabilities(ARGraph<data_t,Empty>* g2);
  
// 	NodeProbabilityStrategy(ARGraph<data_t,Empty>* g2){
// 		evaluateProbabilities(g2);
// 	}
// 	~NodeProbabilityStrategy(){
// 		delete[] out_deg;
// 		delete[] in_deg;
// 		delete[] degree;
// 	}
// 	virtual double getProbability(ARGraph<data_t,Empty> *g1, node_id id) = 0;
// };

// class UniformProbability{
// public:
// 	UniformProbability(ARGraph<data_t, Empty>* source){}
// 	double getProbability(ARGraph<data_t,Empty>* g, node_id id) {
// 		return 1/g->NodeCount();
// 	}
// };

// // template<typename Node, typename Edge>
// // class IsoNodeProbability : public NodeProbabilityStrategy<Node,Edge>{
// // public:
// //   IsoNodeProbability(ARGraph<Node,Edge>* source):NodeProbabilityStrategy<Node,Edge>(source){}
// //   double getProbability(ARGraph<Node,Edge>* g, node_id id);
// //   virtual ~IsoNodeProbability(){}
// // };

// class SubIsoNodeProbability : public NodeProbabilityStrategy{
// public:
//   SubIsoNodeProbability(ARGraph<data_t,Empty>* source):NodeProbabilityStrategy(source){}
//   double getProbability(ARGraph<data_t,Empty>* g, node_id id);
//   virtual ~SubIsoNodeProbability(){}
// };

// void NodeProbabilityStrategy::evaluateProbabilities(ARGraph<data_t,Empty>* source){
//   int i;
//   int node_count;
//   data_t attr;
  
//   map<data_t,int> labelsmap;
  
//   if(!source)
//     return;
  
//   out_deg_size = source->OutMaxDegree()+1;
//   in_deg_size = source->InMaxDegree()+1;
//   degree_size = source->MaxDegree()+1;
//   node_count = source->NodeCount();
  
  
//   vector<int> out_deg_counter(out_deg_size,0);
//   vector<int> in_deg_counter(in_deg_size,0);
//   vector<int> total_deg_counter(degree_size,0);
  
//   out_deg = new double[out_deg_size];
//   in_deg = new double[in_deg_size];
//   degree = new double[degree_size];
  
//   for (i = 0; i < node_count; i++){
//     //Reading attributes
//     int out_deg = source->OutEdgeCount(i);
//     int in_deg = source->InEdgeCount(i);
//     int total_deg = in_deg + out_deg;
//     attr = source->GetNodeAttr(i);
    
//     out_deg_counter[out_deg]++;
//     in_deg_counter[in_deg]++;
//     total_deg_counter[total_deg]++;
    
//     if(labelsmap.count(attr)){
//       labelsmap[attr]++;
//     }else{
//       labelsmap[attr] = 1;
//     }
//   }
  
//   for(i = 0; i < out_deg_size; i++)
//     out_deg[i] = ((double)out_deg_counter[i])/node_count;
  
//   for(i = 0; i < in_deg_size; i++)
//     in_deg[i] = ((double)in_deg_counter[i])/node_count;
  
//   for(i = 0; i < degree_size; i++)
//     degree[i] = ((double)total_deg_counter[i])/node_count;
  
// 	for(typename std::map<data_t,int>::iterator iter = labelsmap.begin(); iter != labelsmap.end(); ++iter) {
// 		data_t n = iter->first;
// 		labels[n] = ((double)labelsmap[n])/node_count;
// 	}
// }

// // template<typename Node, typename Edge>
// // double IsoNodeProbability<Node,Edge>::getProbability(ARGraph<Node,Edge>* g, node_id id){
// //   double node_p;
// //   int node_out_deg, node_in_deg, node_deg;
// //   Node node_attr;
// //   node_out_deg = g->OutEdgeCount(id);
// //   node_in_deg = g->InEdgeCount(id);
// //   node_deg = node_out_deg + node_in_deg;
// //   node_attr = g->GetNodeAttr(id);
// //   node_p = this->out_deg[node_out_deg]*this->in_deg[node_in_deg]*this->degree[node_deg]*this->labels[node_attr];
// //   return node_p;
  
// // }

// double SubIsoNodeProbability::getProbability(ARGraph<data_t,Empty>* g, node_id id){
//   double node_p;
//   int node_out_deg, node_in_deg, node_deg;
//   data_t node_attr;
//   node_out_deg = g->OutEdgeCount(id);
//   node_in_deg = g->InEdgeCount(id);
//   node_deg = node_out_deg + node_in_deg;
//   node_attr = g->GetNodeAttr(id);
  
//   //Sum of probability
//   double out_deg_p_sum, in_deg_p_sum, tot_deg_p_sum;
//   out_deg_p_sum = in_deg_p_sum = tot_deg_p_sum = 0;
  
//   for (int i = this->out_deg_size-1; i >= node_out_deg; i--){
//     out_deg_p_sum += this->out_deg[i];
//   }
  
//   for (int i = this->in_deg_size-1; i >= node_in_deg; i--){
//     in_deg_p_sum += this->in_deg[i];
//   }
  
//   for (int i = this->degree_size-1; i >= node_deg; i--){
//     tot_deg_p_sum += this->degree[i];
//   }
  
//   node_p = out_deg_p_sum*in_deg_p_sum*tot_deg_p_sum*this->labels[node_attr];
  
//   return node_p;
// }


// #endif
