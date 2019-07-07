#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <unordered_map>
#include <limits> //numeric_limits<double>::infinity
#include <utility> //pair, make_pair()

enum class GraphType {Dense, Sparse};

template <typename Node, typename Node_id, typename Dist_type, typename Comp>
class Graph {
public:
	Graph(GraphType type_in,
		  Dist_type init_val_in = std::numeric_limits<double>::infinity(),
		  int num_nodes=0);

	bool add_node(Node* new_node);

	void add_edge(const Node_id &left_id, const Node_id &right_id);

	void make_complete();

	void remove_edge(const Node_id &left_id, const Node_id &right_id);

	void remove_all_edges();

private:
	std::vector<Node*> nodes;
	std::unordered_map<Node_id, size_t> nodes_look_up;
	
	std::vector< std::vector<Dist_type> > adj_mat;
	std::unordered_map<Node_id, std::vector<std::pair<Node_id, Dist_type> > > adj_list;

	GraphType type;
	const Dist_type init_val;
	Comp comp;

	void expand_adj_mat();
};

#include "Graph.tpp"

#endif
