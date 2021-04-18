#include <vector>
#include <utility>
#include <omp.h>
#include <cmath>
#include <limits.h>
#include <algorithm>

// ========== Miscellaneous items ========== //

// useful for the reduction phase
struct idx_val_pair {
    int idx;
    double val;
};

// reduction phase function
struct idx_val_pair distance_redux(const struct idx_val_pair omp_out, const struct idx_val_pair omp_in);

// ========== RRTNode class ========== //

class RRTNode {
public:
    RRTNode(int idx, int p, std::vector<int> c, std::pair<double, double> pos);
    int getId();
    int getParent();
    void setParent(int p);
    std::vector<int> getChildren();
    void addChild(int child_idx);
    std::pair<double, double> getPosition();
    
private:
    int id; 			          //index of RRTNode in tree vector
    int parent; 		          //index of parent in tree vector;
    std::vector<int> children;    //indices of children in tree vector;
    std::pair<double, double> position;
};

// ========== RRTTree class ========== //

class RRTTree {
public:
    RRTTree();
    ~RRTTree();
    void createRoot(std::pair<double, double> start_pos); //FIXME create root somewhere
    RRTNode nearest_neighbor_search(std::pair<double, double> pos, int openmp_thread_count);
    void addNode(int p, std::pair<double, double> pos);
    int size();

private:
    std::vector<RRTNode> nodes;
    Lock treeLock;
};
