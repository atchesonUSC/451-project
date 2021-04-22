#include "tree.h"
#include <cmath>
#include <cfloat>
#include <cstdio>


// ========== Miscellaneous ========== //

double distance(std::pair<int, int> pos1, std::pair<int, int> pos2) {
    int dx = std::abs(pos1.first - pos2.first);
    int dy = std::abs(pos1.second - pos2.second);
    return std::sqrt(std::pow(dx, 2) + std::pow(dy, 2));    
}

/*
// reduction phase function
struct idx_val_pair distance_redux(const struct idx_val_pair omp_out, const struct idx_val_pair omp_in) {
	if (omp_out.val < omp_in.val) {
		return omp_out;
	} else {
		return omp_in;
	}
}
*/

// ========== RRTNode ========== //

RRTNode::RRTNode(int idx, int p, std::vector<int> c, std::pair<int, int> pos) : idx(idx), parent(p), children(c), position(pos) {}

RRTNode::RRTNode(const RRTNode& rhs)
:idx(rhs.getIdx())
, parent(rhs.getParent())
, children(rhs.getChildren())
, position(rhs.getPosition()){

}

RRTNode& RRTNode::operator=(const RRTNode& rhs) {
	// 1. First check that we're not self-assigning
    if (&rhs != this)
    {
        this->setIdx(rhs.getIdx());
		int parent = rhs.getParent();
		this->setParent(parent);

		int pos1 = rhs.getPosition().first;
		int pos2 = rhs.getPosition().second;
		this->position = std::make_pair(pos1, pos2);
        
		this->children.clear();
        for (std::size_t i = 0; i < rhs.getChildren().size(); ++i) {
            this->children.push_back(rhs.getChildren()[i]);
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
	// initialize the mutex
	if (pthread_mutex_init(&tree_lock, NULL) != 0) {
		//return 1;
	}
	printf("rrt tree constructor");
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

/*
int RRTTree::nearest_neighbor_search(std::pair<int, int> p, int t) {
	// set number of threads for openMP execution
	omp_set_num_threads(t);

	// declare reduction clause for finding index with nearest neighbor
	#pragma omp declare reduction \
		(min_pair : struct idx_val_pair : \
		omp_out = distance_redux(omp_out, omp_in)) \
		initializer(omp_priv = {0, 0})

	
	//Using openMP, each spawned thread with calculate the node of the
	//tree section for which it is responsible that is closest to the
	//randomly chosen node.

	//Once all threads have completed this task, they will then reduce
	//their index-distance pairs by comparing them and returning back
	//the pair that has the smallest distance value. 
	
	struct idx_val_pair min {0, INT_MAX};
	
	#pragma omp parallel for reduction(min_pair: min)
	for (int i = 0; i < this->nodes.size(); ++i) {
		
		// calculate distance between sampled node and tree node
		double d = distance(p, this->nodes[i].getPosition());

		// check if distance is less than the current minimum distance
		if (d < min.val) {
			min.idx = i;
			min.val = d;
		}
	}

	// maybe we add in something here to compare to locally sampled nodes...

	// return index of the nearest neigbor in the nodes vector
	return min.idx;
}
*/

void* search_partition(void* args) {
	// get arguments
	struct args_info* data = (args_info*) args;

	int chunk_sz = data->chunk_sz;
	std::pair<int, int> q_rand = data->q_rand;
	std::pair<int, double>* results = data->results;
	std::vector<RRTNode> nodes (data->tree_nodes);

	// get the thread's ID
	int tid = pthread_self();

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

	struct args_info search_partition_args {chunk_sz, q_rand, results, nodes};
	// have each thread search its partition
	for (int i = 0; i < chunk_sz; ++i) {
		pthread_create(&threads[i], NULL, search_partition, (void*) &search_partition_args);
	}

	// wait for all threads to finish
	for (int i = 0; i < t; ++i) {
		pthread_join(threads[i], NULL);
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
