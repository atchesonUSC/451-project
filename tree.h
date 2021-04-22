#include <vector>
#include <utility>
#include <omp.h>
#include <cmath>
#include <limits.h>
#include <pthread.h>
#include <algorithm>


// ========== Miscellaneous items ========== //

// useful for the reduction phase
struct args_info {
    int chunk_sz;
    std::pair<int, int> q_rand;
    std::pair<int, double>* results;
};

// reduction phase function
struct idx_val_pair distance_redux(const struct idx_val_pair omp_out, const struct idx_val_pair omp_in);



// ========== RRTNode class ========== //

class RRTNode {
public:
    RRTNode(int idx, int p, std::vector<int> c, std::pair<int, int> pos);
    RRTNode(const RRTNode& rhs);
    RRTNode& operator=(const RRTNode& rhs);
    int getParent();
    void setParent(int p);
    std::vector<int> getChildren();
    void addChild(int child_idx);
    std::pair<int, int> getPosition();
    int getIdx();
    void setIdx(int idx);
    
private:
    int idx; 			          //index of RRTNode in tree vector
    int parent; 		          //index of parent in tree vector;
    std::vector<int> children;    //indices of children in tree vector;
    std::pair<int, int> position;
};



// ========== RRTTree class ========== //

class RRTTree {
public:
    RRTTree();
    ~RRTTree();
    void createRoot(std::pair<int, int> start_pos);
    int nearest_neighbor_search(std::pair<int, int> pos, int openmp_thread_count);
    void addNode(RRTNode new_node);
    RRTNode get_node(int idx);
    int get_size();

private:
    std::vector<RRTNode> nodes;
    pthread_mutex_t tree_lock;
};
