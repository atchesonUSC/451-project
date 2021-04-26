#include "tree.h"
#include <cmath>
#include <cfloat>
#include <cstdio>


// ========== Miscellaneous ========== //

/*
double distance(std::pair<int, int> pos1, std::pair<int, int> pos2) {
    int dx = std::abs(pos1.first - pos2.first);
    int dy = std::abs(pos1.second - pos2.second);

    return std::sqrt(std::pow(dx, 2) + std::pow(dy, 2));    
}
*/


// ========== RRTNode ========== //

RRTNode::RRTNode(int idx, int p, std::vector<int> c, std::pair<int, int> pos) : idx(idx), parent(p), children(c), position(pos) {}

RRTNode::RRTNode(const RRTNode& rhs)
:idx(rhs.idx)
, parent(rhs.parent)
, children(rhs.children)
, position(rhs.position) {

}

RRTNode& RRTNode::operator=(const RRTNode& rhs) {
	// 1. First check that we're not self-assigning
    if (&rhs != this) {
        this->idx = rhs.idx;
		int parent = rhs.parent;
		this->parent = parent;

		int pos1 = rhs.position.first;
		int pos2 = rhs.position.second;
		this->position = std::make_pair(pos1, pos2);
        
		this->children.clear();
        for (std::size_t i = 0; i < rhs.children.size(); ++i) {
            this->children.push_back(rhs.children[i]);
        }
    }

    return *this;
}

int RRTNode::getIdx(){
    return idx;
}

int RRTNode::getParent(){
    return parent;
}

std::vector<int> RRTNode::getChildren(){
    return children;
}

void RRTNode::setParent(int p){
    parent = p;
}

void RRTNode::addChild(int c){
    children.push_back(c);
}

std::pair<int, int> RRTNode::getPosition(){
    return position;
}

void RRTNode::setIdx(int idx) {
	this->idx = idx;
}

// ========== RRTTree ========== //

RRTTree::RRTTree() {
        // initialize the goal node index
        int goal_node_idx = -1;

        // initialize the mutex
	if (pthread_mutex_init(&tree_lock, NULL) != 0) {
		printf("Error while initializing mutex");
	}
}

RRTTree::~RRTTree() {}

void RRTTree::createRoot(std::pair<int, int> start_pos) {
	std::vector<int> empty;
	RRTNode root = RRTNode(0, -1, empty, start_pos);
	nodes.push_back(root);
}

void RRTTree::addNode(RRTNode new_node){
	int parent_idx, new_node_idx;
	
	// critical section
	pthread_mutex_lock(&tree_lock);

	parent_idx = new_node.getParent();
	new_node_idx = this->nodes.size();
	new_node.setIdx(new_node_idx);

    nodes[parent_idx].addChild(new_node_idx);
    nodes.push_back(new_node);

	pthread_mutex_unlock(&tree_lock);
}

RRTNode RRTTree::get_node(int idx) {
	return nodes[idx];
}

int RRTTree::get_size() {
	return nodes.size();
}

void RRTTree::print_tree() {
    for (int i = 0; i < this->get_size(); ++i) {
        std::pair<int, int> pos = nodes[i].getPosition();
        int x = pos.first;
        int y = pos.second;
    
        printf("(%d, %d)\n", x, y);
    }
}

/*
void* search_partition(void* args) {
	// get arguments
	struct args_info* data = (struct args_info*) args;
	
	std::cerr << "start of search" << std::endl;

	int chunk_sz = data->chunk_sz;
	std::pair<int, int> q_rand = data->q_rand;
	std::pair<int, double>* results = data->results;
	std::vector<RRTNode> nodes (data->tree_nodes);

	// get the thread's ID
	unsigned long int tid = (unsigned long int) pthread_self();

	// find the starting index
	int start = tid * chunk_sz;

	// iterate over the chunk
	for (int i = start; i < start+chunk_sz; ++i) {
		// calculate distance between q_rand and node in partition
		double d = distance(q_rand, nodes[i].getPosition());

		// compare to current value in results
		if (results[i].second > d) {
			std::pair<int, double> updated (i, d);
			results[tid] = updated;
		}
	}

	pthread_exit(NULL);
}
*/

/*
// new implementation for potential nearest neighbor search using only pthreads
int RRTTree::nearest_neighbor_search_new(std::pair<int, int> q_rand, int t) {
	// number of nodes in tree
	int chunk_sz = get_size() / t;

	// setup array for searching threads
	pthread_t threads[t];

	// setup array for storing nearest neighbor from each thread's partition
	std::pair<int, double> results[t];

	// initiliaze results array
	for (int i = 0; i < t; ++i) {
		std::pair<int, double> tmp (-1, FLT_MAX);
		results[i] = tmp;
	}

	struct args_info search_partition_args = {chunk_sz, q_rand, results, nodes};
	// have each thread search its partition
	std::cerr << "made in NNS before create\n";
	for (int i = 0; i < chunk_sz; ++i) {
		int status = pthread_create(&threads[i], NULL, &search_partition, (void*) &search_partition_args);
		if (status != 0) {
			std::cerr << "did not create properly" << std::endl;
		}
	}

	std::cerr << "made in NNS after create\n";
	// wait for all threads to finish
	for (int i = 0; i < t; ++i) {
		std::cerr << "joining thread..." << std::endl;
		int status = pthread_join(threads[i], NULL);
		if (status != 0) {
			printf("[ERROR] Issue with thread join in nearest neighbor...");
			return 1;
		}
	}

	// after all threads finish, scan over the results and find smallest distance, return the index
	int smallest_idx = results[0].first;
	double smallest_d = results[0].second;
	
	for (int i = 0; i < t; ++i) {
		if (results[i].second < smallest_d) {
			smallest_idx = results[i].first;
			smallest_d = results[i].second;
		}
	}

	return smallest_idx;
}
*/
