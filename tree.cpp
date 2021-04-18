#include "tree.h"
#include "math.h"

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

RRTTree::RRTTree() {}

RRTTree::~RRTTree(){}

double distance(std::pair<double, double> pos1, std::pair<double, double> pos2){
    double x_diff = abs(pos1.first - pos2.first);
    double y_diff = abs(pos1.second - pos2.second);
    return sqrt(pow(x_diff, 2) + std::pow(y_diff, 2));    
}

// RRTNode RRTTree::nearestNeighbor(std::pair<double, double> pos, int start_idx, int end_idx){
//     ReadLock r_lock(treeLock);

//     if(start_idx < 0 || end_idx >= (int)nodes.size() || end_idx <= start_idx){
//         //return (RRTNode*) nullptr; 
//         //FIXME how to handle bad indices
//     }

//     int nearestIndex = start_idx;
//     double nearestDistance = distance(pos, nodes[start_idx].getPosition());
//     for(int i = start_idx; i < end_idx; i++){
//         double dist = distance(pos, nodes[i].getPosition());
//         if(dist < nearestDistance){
//             nearestIndex = i;
//             nearestDistance = dist;
//         }
//     }

//     return nodes[nearestIndex];
// }

void RRTTree::addNode(int p, std::pair<double, double> pos){
    //TODO synchronize this with a mutex

    std::vector<int> c;
    RRTNode newNode( (int) nodes.size(), p, c, pos);
    nodes[p].addChild(newNode.getId());
    nodes.push_back(newNode);

    return;
}

/* === Nearest Neighbor Search === */

// useful for the reduction phase
struct idx_val_pair {
	int idx;
	double val;
};

// reduction phase function
struct idx_val_pair find_min_distance(const struct idx_val_pair omp_out, const struct idx_val_pair omp_in) {
	if (omp_out.val < omp_in.val) {
		return omp_out;
	} else {
		return omp_in;
	}
}

int nearest_neighbor_search(std::pair<double, double> pos, int t, int tree_sz) {
	// set number of threads for openMP execution
	omp_set_num_threads(t);

	// declare reduction clause for finding index with nearest neighbor
	#pragma omp declare reduction \
		(min_pair : struct idx_val_pair : \
		omp_out = find_min_distance(omp_out, omp_in)) \
		initializer(omp_priv = {0, 0})

	/*
	Using openMP, each spawned thread with calculate the node of the
	tree section for which it is responsible that is closest to the
	randomly chosen node.

	Once all threads have completed this task, they will then reduce
	their index-distance pairs by comparing them and returning back
	the pair that has the smallest distance value. 
	*/
	struct idx_val_pair min = {0, INT_MAX};
	
	#pragma omp parallel for reduction(min_pair: min)
	for (int i = 0; i < tree_sz; ++i) {
		// calculate distance between random node and tree node
		double d = distance(pos, nodes[i]);

		// check if distance is less than the current minimum distance
		if (d < min.val) {
			min.idx = i;
			min.val = d;
		}
	}
}
