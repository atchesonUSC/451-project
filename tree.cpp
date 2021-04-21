#include "tree.h"
#include "math.h"


// ========== Miscellaneous ========== //

double distance(std::pair<double, double> pos1, std::pair<double, double> pos2) {
    double dx = abs(pos1.first - pos2.first);
    double dy = abs(pos1.second - pos2.second);
    return sqrt(std::pow(dx, 2) + std::pow(dy, 2));    
}

// reduction phase function
struct idx_val_pair distance_redux(const struct idx_val_pair omp_out, const struct idx_val_pair omp_in) {
	if (omp_out.val < omp_in.val) {
		return omp_out;
	} else {
		return omp_in;
	}
}

// ========== RRTNode ========== //

RRTNode::RRTNode(int idx, int p, std::vector<int> c, std::pair<double, double> pos) : id(idx), parent(p), children(c), position(pos) {}

int RRTNode::getId(){
    return id;
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

std::pair<double, double> RRTNode::getPosition(){
    return position;
}

void RRTNode::setIdx(int idx) {
	this->idx = idx;
}

// ========== RRTTree ========== //

RRTTree::RRTTree() {
	// initialize the mutex
	if (pthread_mutex_init(&tree_lock, nullptr) != 0) {
		return 1;
	}
}

RRTTree::~RRTTree() {}

void RRTTree::createRoot(std::pair<double, double> start_pos) {
	vector<int> empty;
	RRTNode root = RRTNode(0, -1, empty, start_pos);
	nodes.push_back(root);
}

void RRTTree::addNode(RRTNode new_node){
	int parent_idx, new_node_idx;
	
	// critical section
	pthread_mutex_lock(&tree_lock);

	parent_idx = new_node.getParent();
	new_node_idx = tree.size();
	new_node.setIdx(new_node_idx);

    nodes[parent_idx].addChild(new_node_idx);
    nodes.push_back(new_node);

	pthread_mutex_unlock(&tree_lock);
}

RRTNode RRTTree::get_node(int idx) {
	return nodes[idx];
}

int RRTTREE::nearest_neighbor_search(std::pair<double, double> p, int t) {
	// set number of threads for openMP execution
	omp_set_num_threads(t);

	// declare reduction clause for finding index with nearest neighbor
	#pragma omp declare reduction \
		(min_pair : struct idx_val_pair : \
		omp_out = distance_redux(omp_out, omp_in)) \
		initializer(omp_priv = {0, 0})

	/*
	Using openMP, each spawned thread with calculate the node of the
	tree section for which it is responsible that is closest to the
	randomly chosen node.

	Once all threads have completed this task, they will then reduce
	their index-distance pairs by comparing them and returning back
	the pair that has the smallest distance value. 
	*/
	struct idx_val_pair min {0, INT_MAX};
	
	#pragma omp parallel for reduction(min_pair: min)
	for (int i = 0; i < nodes.size(); ++i) {
		
		// calculate distance between sampled node and tree node
		double d = distance(p, nodes[i].getPosition());

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
