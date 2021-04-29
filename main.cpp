#include <time.h>
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
pair<int, int> sample_bmp_map(int dim_x, int dim_y, int bias, int x_end, int y_end);
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
    int bias;
    int thread_id;
};

// useful for reduction phase of nearest neighbor search
struct args_info {
    int chunk_sz;
    std::pair<int, int> q_rand;
    std::pair<int, double>* results;
    int tid;
};


/*
=================================================================================
--------------------------
* Command Line Arguments *
--------------------------
• NOTE: First argument of the config is the number of subsequent parameters
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
• config[11]: greedy bias -> (integer between 0 and 100)
=================================================================================
*/


// RRTTree: Global tree structure for storing nodes
RRTTree tree;

// for barrier sync
Barrier barrier;

// sampler threads will check this to see if the end goal has been reached
int end_flag = 0;
pthread_mutex_t tree_lock;
pthread_mutex_t end_flag_lock;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("Incorrect number of arguments: ./[program_name] [config_filename]\n");
        return 1;
    }

    // parse argument file
    vector<string> config;
    string config_filename;
    config_filename = string(argv[1]);
    config = parse_config_file(config_filename);

    // setup arguments
    string bmp_filename;
    struct timespec start, stop;
    double time;
    int stop_thresh, x_root, y_root, t, samples, m, delta, x_end, y_end, nns_t, bias;

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
    bias = stoi(config[11]);
    
    //optional config parameters for writing tree to file
    bool serializeOutput = false;
    if(config.size() >= 13){
        serializeOutput = (bool)stoi(config[12]);
    }
    string outputFilename = "out.txt";
    if(config.size() >= 14){
        outputFilename = config[13];
    }

    // setup barrier
    barrier.set_nthreads(t);

    // setup lock for end_flag
    pthread_mutex_init(&tree_lock, NULL);
    pthread_mutex_init(&end_flag_lock, NULL);

    // read in the map, find dimensions
    // NOTE: stripline, grid, and clutter are all 720x405
    bmpMap bmp_map(bmp_filename);

    unsigned int dim_x = bmp_map.get_width();
    unsigned int dim_y = bmp_map.get_height();

    // make sure the start and goal points are in map bounds
    if (x_root >= dim_x || x_root < 0) {
        printf("Invalid x_root: %d\n", x_root);
        return 1;
    } else if (y_root >= dim_y || y_root < 0) {
        printf("Invalid y_root: %d\n", y_root);
        return 1;
    } else if (x_end >= dim_x || x_end < 0) {
        printf("Invalid x_end: %d\n", x_end);
        return 1;
    } else if (y_end >= dim_y || y_end < 0) {
        printf("Invalid y_end: %d\n", y_end);
        return 1;
    }

    // make sure start and goal points are not on an obstacle
    std::pair<int, int> root (x_root, y_root);
    std::pair<int, int> goal (x_end, y_end);
    
    if (!bmp_map.checkFree(root)) {
        printf("Invalid root: obstacle interference\n");
        return 1;
    } else if (!bmp_map.checkFree(goal)) {
        printf("Invalid goal: obstacle interference\n");
        return 1;
    }

    // initialize the root of the RRT tree as the starting point
    tree.createRoot(root);

    /*
    ==================== Algorithm Start ====================
    */

    // start timer...
    if (clock_gettime(CLOCK_REALTIME, &start) == -1 ) {
        perror( "clock gettime" );
    }

    // generate threads to run sampling
    pthread_t threads[t];
    for (int i = 0; i < t; ++i) {
        struct sample_args sample_func_args = {t, m, delta, samples, nns_t, &bmp_map, dim_x, dim_y, x_end, y_end, stop_thresh, bias, i};
        int status = pthread_create(&threads[i], NULL, thread_sample, (void*) &sample_func_args);
        if (status != 0) {
            printf("[ERROR] Issue with thread create in main...\n");
            return 1;
        }
    }
    
    // wait for threads to finish
    for (int i = 0; i < t; ++i) {
        int status = pthread_join(threads[i], NULL);
        if (status != 0) {
            printf("[ERROR] Issue with thread join in main...\n");
            return 1;
        }
    }

    // stop timer...
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
    }

    /*
    ==================== Algorithm End ====================
    */

    time = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1e9;
    printf("\n================== Execution Summary ==================\n");
    printf("• Start Point: (%d, %d)\n", x_root, y_root);
    printf("• End Point: (%d, %d)\n", x_end, y_end);
    printf("• # of Sampler Threads: %d\n", t);
    printf("• # of Samples: %d\n", samples);
    printf("• Batch Size: %d\n", m);
    printf("• Delta: %d\n", delta);
    printf("• # of NNS Threads per Sample Thread: %d\n", nns_t);
    printf("• Threshold Distance: %d\n", stop_thresh);
    printf("• Greedy Bias: %d\n", bias);
    if (end_flag) {
        printf("• Goal Status: Found\n");
    } else {
        printf("• Goal Status: Not Found\n");
    }
    printf("• Execution Time: %f sec\n", time);
    printf("=======================================================\n\n");

    if(serializeOutput){
        ofstream o;
        o.open(outputFilename);
        tree.serializeTree(o);
    }

    return 0;
}


/*
============================================ Helper Functions ============================================
*/


/*
grab a point from the bitmap
*/
std::pair<int, int> sample_bmp_map(int dim_x, int dim_y, int bias, int x_end, int y_end) {
    int x;
    int y;
    int random_num;
    std::pair<int, int> point;

    random_num = (int) (100.0 * rand() / (RAND_MAX + 1.0)) + 1;
    
    if (random_num < bias) {
        // sample the goal location
        x = x_end;
        y = y_end;
    } else {
        // sample random location
        x = rand() % dim_x;
        y = rand() % dim_y;
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
   
    // printf("q_rand_x = %d\nq_rand_y = %d\n", q_rand.first, q_rand.second);
 
    RRTNode rrt_node = tree.get_node(q_near_idx);
    pair<int, int> rrt_node_pos = rrt_node.getPosition();
    int rrt_node_x = rrt_node_pos.first;
    int rrt_node_y = rrt_node_pos.second;

    // printf("rrt_node_x = %d\nrrt_node_y = %d\n", rrt_node_x, rrt_node_y);

    int dx = q_rand_x - rrt_node_x;
    int dy = q_rand_y - rrt_node_y;

    double angle = atan2(dy, dx);

    int dx_new = (int) (cos(angle) * delta);
    int dy_new = (int) (sin(angle) * delta);

    int x_new = rrt_node_x + dx_new;
    int y_new = rrt_node_y + dy_new;

    // printf("x_new = %d\ny_new = %d\n", x_new, y_new);

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
    int samples, t, m, delta, dim_x, dim_y, nns_t, stop_thresh, x_end, y_end, bias, thread_id;

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
    bias = args->bias;
    thread_id = args->thread_id;
    pair<int, int> end_pos (x_end, y_end);

    // begin sampling
    int sampler_count = 0;
    while (sampler_count < samples / t) {

        // check the end_flag
        int end_flag_raised = 0;
	barrier.wait();
        pthread_mutex_lock(&end_flag_lock);
        if (end_flag) {
            end_flag_raised = 1;
        }
        pthread_mutex_unlock(&end_flag_lock);
        
        // if end flag raised, no more sampling needed
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
            std:pair<int, int> q_rand;
            q_rand = sample_bmp_map(dim_x, dim_y, bias, x_end, y_end);

            // find nearest neighbor to q_rand
            int neighbor_idx = nearest_neighbor_search(q_rand, nns_t);

            // create new node for tree
            RRTNode new_node = create_node(neighbor_idx, q_rand, delta);
            
            // similarity check: find nearest neighbor to q_new, if the same nearest neighbor it is not too similar
            std::pair<int, int> q_new (new_node.getPosition());
            int neighbor_idx2 = nearest_neighbor_search(q_new, nns_t);
            
            // if valid and not too similar pushback
            int x = q_new.first;
            int y = q_new.second;

            if ((dim_x > x && x >= 0) && (dim_y > y && y >= 0)) {
                if (local_map.checkFree(q_new) && (neighbor_idx == neighbor_idx2)) {
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
                
            // check if we're close enough to the goal
            pair<int, int> node_pos = local_bin[i].getPosition();
            int d = distance(node_pos, end_pos);
            
            pthread_mutex_lock(&tree_lock);
            if (d < stop_thresh) {
		cerr << "distance: " << d << "\n";
                end_flag = 1;
            }
            tree.addNode(local_bin[i], end_flag);
            pthread_mutex_unlock(&tree_lock);
        }
        // update outer sampling loop count
        sampler_count += batch_m;
    }
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
	std::vector<RRTNode> tree_nodes = tree.nodes;

	// get the thread's ID
	int tid = data->tid;

	// find the starting index
	int start = tid * chunk_sz;
	
    // iterate over the chunk
	for (int i = start; i < start+chunk_sz; ++i) {
        if(tree_nodes.size() > i) {
            // calculate distance between q_rand and node in partition
		    double d = distance(q_rand, tree_nodes[i].getPosition());
            
            // compare to current value in results
            if (results[tid].second > d) {
                std::pair<int, double> new_nearest (i, d);
                results[tid] = new_nearest;
            }
        }
	}
	pthread_exit(NULL);
}


/*
perform the nearest neighbor search
*/
int nearest_neighbor_search(std::pair<int, int> q_rand, int t) {
    if (t < tree.get_size()) {	
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
        for (int i = 0; i < t; ++i) {
            struct args_info search_partition_args = {chunk_sz, q_rand, results, i};
            int status = pthread_create(&threads[i], NULL, search_partition, (void*) &search_partition_args);
            if (status != 0) {
                std::cerr << "[ERROR] Issue with thread create in nearest neigbor" << std::endl;
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
    } else {
        int tree_sz = tree.get_size();

        // number of nodes in tree
        int chunk_sz = 1;

        // setup array for searching threads
        pthread_t threads[tree_sz];

        // setup array for storing nearest neighbor from each thread's partition
        std::pair<int, double> results[tree_sz];

        // initiliaze results array
        for (int i = 0; i < tree_sz; ++i) {
            std::pair<int, double> default_nearest (-1, FLT_MAX);
            results[i] = default_nearest;
        }

        // have each thread search its partition
        for (int i = 0; i < tree_sz; ++i) {
                    struct args_info search_partition_args = {chunk_sz, q_rand, results, i};
            int status = pthread_create(&threads[i], NULL, search_partition, (void*) &search_partition_args);
            if (status != 0) {
                std::cerr << "did not create properly" << std::endl;
            }
        }

        // wait for all threads to finish
        for (int i = 0; i < tree_sz; ++i) {
            int status = pthread_join(threads[i], NULL);
            if (status != 0) {
                printf("[ERROR] Issue with thread join in nearest neighbor...");
                return 1;
            }
        }

        // after all threads finish, scan over the results and find smallest distance, return the index
        int smallest_idx = results[0].first;
        double smallest_d = results[0].second;
        
        for (int i = 0; i < tree_sz; ++i) {
            if (results[i].second < smallest_d) {
                smallest_idx = results[i].first;
                smallest_d = results[i].second;
            }
        }
        return smallest_idx;
    }
}
