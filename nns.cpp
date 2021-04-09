/* === Nearest Neighbor Search === */

#include <omp.h>
#include <cmath>
#include <utility>
#include <limits.h>
#include <algorithm>

#include "node.hpp"

std::vector<struct Node*> tmp_tree;

// useful for the reduction phase
struct idx_val_pair {
	int idx;
	double val;
};

// finds the distance between two points
double calc_distance(double a, double b) {
	return sqrt(a * a + b * b);
}

// reduction phase function
struct idx_val_pair find_min_distance(const struct idx_val_pair omp_out, const struct idx_val_pair omp_in) {
	if (omp_out.val < omp_in.val) {
		return omp_out;
	} else {
		return omp_in;
	}
}

int nearest_neighbor_search(struct Node* node, int t, int tree_sz) {
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
		double d = calc_distance(node->val, tree[i].val);

		// check if distance is less than the current minimum distance
		if (d < min.val) {
			min.idx = i;
			min.val = d;
		}
	}
}






