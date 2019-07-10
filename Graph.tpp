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
	size_t left_ind = nodes_look_up[left_id];
	size_t right_ind = nodes_look_up[right_id];
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
			left_list.emplace_back(right_id, dist);
			auto &right_list = adj_list[right_id];
			right_list.emplace_back(left_id, dist);
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
	size_t left_ind = nodes_look_up[left_id];
	size_t right_ind = nodes_look_up[right_id];
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

template <typename Node, typename Node_id, typename Dist_type, typename Comp>
Dist_type Graph<Node, Node_id, Dist_type, Comp>::find_MST(std::vector<std::pair<Node*, Node*> > &edges,
														bool record_edges) {

	Dist_type total_dist = 0;

	size_t num_nodes = nodes.size();
	std::vector<MST_Location> MST;
	MST.resize(num_nodes, MST_Location{init_val, false, nullptr});

	//add first node into MST
	MST_Location &first = MST.front();
	first.distance= 0;
	first.visited = true;

	//set first node as the node newly added to MST
	size_t newly_added = 0;

	for (size_t i = 1; i < num_nodes; ++i) {
		Dist_type closest_dist = init_val;
		size_t closest_node = 0; //place holder, will always be assigned new value

		// update distance changes due to newly_added node
		switch(type) {
			case GraphType::Dense: {
				//for every node other than the first
				for (size_t j = 1; j < num_nodes; ++j) {
					// if visited, skip
					MST_Location &other = MST[j];
					if (other.visited) {
						continue;
					}

					Dist_type dist = other.distance;
					Dist_type new_dist = adj_mat[newly_added][j];
					//if next_dist < dist
					if (comp(new_dist, dist)) {
						other.distance = new_dist;
						other.precede = nodes[newly_added];
						dist = new_dist;
					}
					
					// update closest_dist and closest_node
					//if dist < closest_dist
					if (comp(dist, closest_dist)) {
						closest_dist = dist;
						closest_node = j;
					}
				}
			}
			break;
			case GraphType::Sparse: {
				//for every node connected to newly_added node
				const Node_id &newly_added_id = nodes[newly_added]->id;
				const auto &connections = adj_list[newly_added_id];
				for (auto it = connections.begin(); it != connections.end(); ++it) {
					const Node_id &other_id = it->first;
					size_t other_ind = nodes_look_up[other_id];

					// if visited, skip
					MST_Location &other = MST[other_ind];
					if (other.visited) {
						continue;
					}
					
					Dist_type dist = other.distance;
					Dist_type new_dist = it->second;
					//if new_dist < dist 
					if (comp(new_dist, dist)) {
						other.distance = new_dist;
						other.precede = nodes[newly_added];
						dist = new_dist;
					}
				}

				// update closest_dist and closest_node
				for (size_t i = 1; i < num_nodes; ++i) {
					// if visited, skip
					MST_Location &current = MST[i];
					if (current.visited) {
						continue;
					}

					Dist_type dist = current.distance;
					//if dist < closest_dist 
					if (comp(dist, closest_dist)) {
						closest_dist = dist;
						closest_node = i;
					}	
				}
			}
		}

		// add closest to MST, update 'newly_added' and total_dist
		MST[closest_node].visited = true;
		newly_added = closest_node;
		total_dist += closest_dist;
	}

	if (record_edges) {
		for (size_t i = 1; i < num_nodes; ++i) {
			edges.emplace_back(nodes[i], MST[i].precede);
		}
	}

	return total_dist;
}


template <typename Node, typename Node_id, typename Dist_type, typename Comp>
Dist_type Graph<Node, Node_id, Dist_type, Comp>::find_MST() {
	std::vector<std::pair<Node*, Node*>> temp;
	return find_MST(temp, false);
}
