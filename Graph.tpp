template <typename Node, typename Node_id, typename Dist_type, typename Comp>
Graph<Node, Node_id, Dist_type, Comp>::Graph(GraphType type_in, Dist_type init_val_in, int num_nodes) 
	: type{type_in}, init_val{init_val_in} {
	nodes.reserve(num_nodes);
	if (type == GraphType::Dense) {
		adj_mat.reserve(num_nodes);
	}
	comp = Comp();
}

template <typename Node, typename Node_id, typename Dist_type, typename Comp>
bool Graph<Node, Node_id, Dist_type, Comp>::add_node(Node* new_node) {
	if (nodes_look_up.find(new_node->id) == nodes_look_up.end()) {
		nodes.push_back(new_node);
		nodes_look_up[new_node->id] = nodes.size()-1; //it's index
		return true;
	}
	return false;
}

template <typename Node, typename Node_id, typename Dist_type, typename Comp>
void Graph<Node, Node_id, Dist_type, Comp>::add_edge(const Node_id &left_id, const Node_id &right_id) {
	unsigned left_ind = nodes_look_up[left_id];
	unsigned right_ind = nodes_look_up[right_id];
	size_t num_nodes = nodes.size();
	switch(type) {
		case GraphType::Dense: {
			expand_adj_mat();
			adj_mat[right_ind][left_ind] = adj_mat[left_ind][right_ind] = comp(nodes[right_ind], nodes[left_ind]);
			break;
		}
		case GraphType::Sparse: {
			Dist_type dist = comp(nodes[right_ind], nodes[left_ind]);
			auto &left_list = adj_list[left_id];
			for (auto it = left_list.begin(); it != left_list.end(); ++it) {
				if (it->first == right_ind) {
					return;
				}
			}
			left_list.push_back(right_id, dist);
			auto &right_list = adj_list[right_id];
			right_list.push_back(left_id, dist);
			break;
		}
	}
}

template <typename Node, typename Node_id, typename Dist_type, typename Comp>
void Graph<Node, Node_id, Dist_type, Comp>::make_complete() {
	size_t num_nodes = nodes.size();
	switch(type) {
		case GraphType::Dense: {
			expand_adj_mat();
			for (size_t row = 0; row < num_nodes; ++row) {
				for (size_t col = row+1; col < num_nodes; ++col) {
					adj_mat[row][col] = adj_mat[col][row] = comp(nodes[row], nodes[col]);
				}
			}
			break;
		}
		case GraphType::Sparse: {
			remove_all_edges();
			for (size_t row = 0; row < num_nodes; ++row) {
				const Node_id &row_id = nodes[row]->id;
				auto &row_list = adj_list[row_id];
				for (size_t col = row+1; col < num_nodes; ++col) {
					const Node_id &col_id = nodes[col]->id;
					auto &col_list = adj_list[col_id];
					Dist_type dist = comp(nodes[row], nodes[col]);
					row_list.emplace_back(col_id, dist);
					col_list.emplace_back(row_id, dist);
				}
			}
			break;
		}
	}
}

template <typename Node, typename Node_id, typename Dist_type, typename Comp>
void Graph<Node, Node_id, Dist_type, Comp>::expand_adj_mat() {
	size_t num_nodes = nodes.size();
	if (adj_mat.size() < num_nodes) {
		adj_mat.resize(num_nodes);
		for (auto it = adj_mat.begin(); it != adj_mat.end(); ++it) {
			it->resize(num_nodes, init_val);
		}
	}
}

template <typename Node, typename Node_id, typename Dist_type, typename Comp>
void Graph<Node, Node_id, Dist_type, Comp>::remove_edge(const Node_id &left_id, const Node_id &right_id) {
	unsigned left_ind = nodes_look_up[left_id];
	unsigned right_ind = nodes_look_up[right_id];
	switch(type) {
		case GraphType::Dense: {
			adj_mat[left_ind][right_ind] = adj_mat[right_ind][left_ind] = init_val;
			break;
		}
		case GraphType::Sparse: {
			bool found_edge = false;
			auto &left_list = adj_list[left_id];
			for (auto it = left_list.begin(); it != left_list.end(); ++it) {
				if (it->first == right_ind) {
					left_list.erase(it);
					found_edge = true;
					break;
				}
			}
			if (found_edge) {
				auto &right_list = adj_list[right_id];
				for (auto it = right_list.begin(); it != right_list.end(); ++it) {
					if (it->first == left_ind) {
						right_list.erase(it);
						break;
					}
				}
			}
			break;
		}
	}
}

template <typename Node, typename Node_id, typename Dist_type, typename Comp>
void Graph<Node, Node_id, Dist_type, Comp>::remove_all_edges() {
	size_t num_nodes = nodes.size();
	switch(type) {
		case GraphType::Dense: {
			for (size_t row = 0; row < num_nodes; ++row) {
				for (size_t col = 0; col < num_nodes; ++col) {
					adj_mat[row][col] = init_val;
				}
			}
			break;
		}
		case GraphType::Sparse: {
			for (auto it = adj_list.begin(); it != adj_list.end(); ++it) {
				it->second.clear();
			}
			break;
		}
	}
}
