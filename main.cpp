#include <cmath>
#include <random>
#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <cfloat>
#include <pthread.h>

#include "tree.h"
#include "bmp2Map.h"
#include "barrier.h"

using namespace std;


// function prototypes
void* thread_sample(void* args);
vector<string> parse_config_file(string filename);
pair<int, int> sample_bmp_map(int dim_x, int dim_y);
RRTNode create_node(int q_near_idx, pair<int, int> q_rand, int delta);

// nearest neighbor functions
double distance(std::pair<int, int> pos1, std::pair<int, int> pos2);
void* search_partition(void* args);
int nearest_neighbor_search(std::pair<int, int> q_rand, int t);


// Struct for passing arguments into the function of pthread_create()
struct sample_args {
    int t;
    int m;
    int delta;
    int samples;
    int nns_t;
    bmpMap* bmp_map;
    unsigned int dim_x;
    unsigned int dim_y;
    int x_end;
    int y_end;
    int stop_thresh;
};


/*
===================================
-----------------------
Command Line Arguments:
-----------------------
• config[0]: bitmap filename
• config[1]: x coordinate (root)
• config[2]: y coordinate (root)
• config[3]: # of sampler threads
• config[4]: # of samples
• config[5]: local sampling batch
• config[6]: delta
• config[7]: x coordinate (end)
• config[8]: y coordinate (end)
• config[9]: # of search_partition threads
• config[10]: threshold distance - determines when we're "close enough" to goal
• TODO... greediness parameters
===================================
*/


// RRTTree: Global tree structure for storing nodes
RRTTree tree;

// for barrier sync
Barrier barrier;

// sampler threads will check this to see if the end goal has been reached
int end_flag = 0;
pthread_mutex_t end_flag_lock;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "Incorrect number of arguments: [program_name] [config_filename]" << endl;
        return -1;
    }

    // parse argument file
    vector<string> config;
    string config_filename;
    config_filename = string(argv[1]);
    config = parse_config_file(config_filename);
   
    // setup arguments
    string bmp_filename;
    int stop_thresh, x_root, y_root, t, samples, m, delta, x_end, y_end, nns_t;

    bmp_filename = config[0];

    x_root = stoi(config[1]);
    y_root = stoi(config[2]);
    x_end = stoi(config[7]);
    y_end = stoi(config[8]);

    t = stoi(config[3]);
    m = stoi(config[5]);
    delta = stoi(config[6]);
    samples = stoi(config[4]);
    nns_t = stoi(config[9]);
    stop_thresh = stoi(config[10]);

    // setup barrier
    barrier.set_nthreads(t);

    // setup lock for end_flag
    pthread_mutex_init(&end_flag_lock, NULL);

    // read in the map, find dimensions
    bmpMap bmp_map(bmp_filename);

    unsigned int dim_x = bmp_map.get_width();
    unsigned int dim_y = bmp_map.get_height();

    // initialize the start state with the RRT Tree
    pair<int, int> root (x_root, y_root);
    tree.createRoot(root);

    // format arguments
    struct sample_args sample_func_args = {t, m, delta, samples, nns_t, &bmp_map, dim_x, dim_y, x_end, y_end, stop_thresh};

    // generate threads to run sampling
    pthread_t threads[t];
    for (int i = 0; i < t; ++i) {
        int status = pthread_create(&threads[i], NULL, thread_sample, (void*) &sample_func_args);
	if (status != 0) {
		printf("[ERROR] Issue with thread create in main...");
		return 1;
	}
    }
    
    // wait for threads to finish
    for (int i = 0; i < t; ++i) {
        int status = pthread_join(threads[i], NULL);
	if (status != 0) {
	    printf("[ERROR] Issue with thread join in main...");
	    return 1;
	}
    }

    printf("Program exiting...");
    return 0;
}





// ============================================ Helper Functions ============================================ //



pair<int, int> sample_bmp_map(int dim_x, int dim_y) {
    int x;
    int y;
    pair<int, int> point;

    x = rand();
    while (x >= dim_x || x < 0) {
        x = rand();
    }

    y = rand();
    while (y >= dim_y || y < 0) {
        y = rand();
    }

    point.first = x;
    point.second = y;

    return point;
}


/*
parse configuration file (contains setup arguments)
*/
vector<string> parse_config_file(string filename) {
    int argc;
    string line;
    ifstream config_file;
      
    // open file
    config_file.open(filename);

    // get argument count
    getline(config_file, line);
    argc = stoi(line);

    // create array for storing arguments as strings
    vector<string> args;
    // get arguments
    for (int i = 0; i < argc; ++i) {
        getline(config_file, line);
        args.push_back(line);
    }

    // return array of arguments
    return args;
}


/*
create new node in direction of sampled node at a distance of delta away from the nearest neighbor
*/
RRTNode create_node(int q_near_idx, pair<int, int> q_rand, int delta) {
    int q_rand_x = q_rand.first;
    int q_rand_y = q_rand.second;
    
    RRTNode rrt_node = tree.get_node(q_near_idx);
    pair<int, int> rrt_node_pos = rrt_node.getPosition();
    int rrt_node_x = rrt_node_pos.first;
    int rrt_node_y = rrt_node_pos.second;

    int dx = q_rand_x - rrt_node_x;
    int dy = q_rand_y - rrt_node_y;

    double angle = atan2(dy, dx);

    int dx_new = (int) (cos(angle) * delta);
    int dy_new = (int) (sin(angle) * delta);

    int x_new = rrt_node_x + dx_new;
    int y_new = rrt_node_y + dy_new;

    // create new node
    pair<int, int> pos_new (x_new, y_new);
    int p = q_near_idx;
    int idx = -1;   // leave this unknown for now, update when finally added to the tree
    vector<int> children;

    RRTNode new_node (idx, p, children, pos_new);
    
    return new_node;
}


/*
performs sampling operations
*/
void* thread_sample(void* args_struct) {
    struct sample_args* args;
    int samples, t, m, delta, dim_x, dim_y, nns_t, stop_thresh, x_end, y_end;

    // setup arguments
    args = (struct sample_args*) args_struct;

    bmpMap local_map(*(args->bmp_map));

    t = args->t;
    m = args->m;
    delta = args->delta;
    samples = args->samples;
    dim_x = args->dim_x;
    dim_y = args->dim_y;
    nns_t = args->nns_t;
    x_end = args->x_end;
    y_end = args->y_end;
    stop_thresh = args->stop_thresh;
    pair<int, int> end_pos (x_end, y_end);

    // begin sampling
    int i = 0;
    while (i < samples / t){
 	// check the end_flag
 	int end_flag_raised = 0;
 	pthread_mutex_lock(&end_flag_lock);
	if (end_flag) {
		cout << "End flag has been raised" << endl;
		end_flag_raised = 1;
	}
	pthread_mutex_unlock(&end_flag_lock);
	if (end_flag_raised) {
		pthread_exit(NULL);
	}
 
        // container for storing locally-sampled nodes, track current batch m size
	int batch_m = m;
        vector<RRTNode> local_bin;

        // sync threads
        barrier.wait();

        for (int j = 0; j < m; ++j) {
            // sample random location on map
            pair<int, int> q_rand;
            q_rand = sample_bmp_map(dim_x, dim_y);

            // find nearest neighbor to q_rand
            int neighbor_idx = nearest_neighbor_search(q_rand, nns_t);

            // create new node for tree
            RRTNode new_node = create_node(neighbor_idx, q_rand, delta);

            // similarity check: find nearest neighbor to q_new, if the same nearest neighbor it is not too similar
            pair<int, int> q_new (new_node.getPosition());
            int neighbor_idx2 = nearest_neighbor_search(q_new, nns_t);
            
            // if valid and not too similar pushback
            if (local_map.checkFree(q_new) && neighbor_idx == neighbor_idx2) {
		// check that node is within bounds of map
		pair<int, int> pos (new_node.getPosition());
		int x = pos.first;
		int y = pos.second;

		if ((dim_x > x && x >= 0) && (dim_y > y && y >= 0)) {
                	local_bin.push_back(new_node);
		} else {
			batch_m--;
		}
            } else {
		batch_m--;
	    }
        }

        // sync threads
        barrier.wait();

        // Add valid nodes to global bin
        for (int i = 0; i < batch_m; ++i) {
            tree.addNode(local_bin[i]);
	    
	    // check if we're close enough to the goal
	    pair<int, int> node_pos = local_bin[i].getPosition();
	    int d = distance(node_pos, end_pos);
	    if (d < stop_thresh) {
		pthread_mutex_lock(&end_flag_lock);
		end_flag = 1;
		pthread_mutex_unlock(&end_flag_lock);
	    }
        }
    }

    cout << "end of thread sampling..." << endl;
    pthread_exit(NULL);
}


/*
finding distance between two points
*/
double distance(std::pair<int, int> pos1, std::pair<int, int> pos2) {
    int dx = std::abs(pos1.first - pos2.first);
    int dy = std::abs(pos1.second - pos2.second);

    return std::sqrt(std::pow(dx, 2) + std::pow(dy, 2));    
}


/*
search partition of graph nodes
*/
void* search_partition(void* args) {
	// get arguments
	struct args_info* data = (struct args_info*) args;

	int chunk_sz = data->chunk_sz;
	std::pair<int, int> q_rand = data->q_rand;
	std::pair<int, double>* results = data->results;
	std::vector<RRTNode> tree_nodes (data->tree_nodes);

	// get the thread's ID
	unsigned long int tid = (unsigned long int) pthread_self();

	// find the starting index
	int start = tid * chunk_sz;

	// iterate over the chunk
	for (int i = start; i < start+chunk_sz; ++i) {
		// calculate distance between q_rand and node in partition
		double d = distance(q_rand, tree_nodes[i].getPosition());

		// compare to current value in results
		if (results[tid].second > d) {
			std::pair<int, double> new_nearest (i, d);
			results[tid] = new_nearest;
		}
	}
	pthread_exit(NULL);
}


/*
perform the nearest neighbor search
*/
int nearest_neighbor_search(std::pair<int, int> q_rand, int t) {
	// number of nodes in tree
	int chunk_sz = tree.get_size() / t;

	// setup array for searching threads
	pthread_t threads[t];

	// setup array for storing nearest neighbor from each thread's partition
	std::pair<int, double> results[t];

	// initiliaze results array
	for (int i = 0; i < t; ++i) {
		std::pair<int, double> default_nearest (-1, FLT_MAX);
		results[i] = default_nearest;
	}

	// have each thread search its partition
	struct args_info search_partition_args = {chunk_sz, q_rand, results, tree.nodes};
	for (int i = 0; i < t; ++i) {
		int status = pthread_create(&threads[i], NULL, search_partition, (void*) &search_partition_args);
		if (status != 0) {
			std::cerr << "did not create properly" << std::endl;
		}
	}

	// wait for all threads to finish
	for (int i = 0; i < t; ++i) {
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
