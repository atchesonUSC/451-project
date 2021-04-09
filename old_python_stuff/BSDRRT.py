"""
Bulk Synchronous Distributed RRT Class Definition
"""


import threading


class Node:
    def __init__(self):
        self.x = ...
        self.y = ...
        self.parent = ...

    def get_parent(self):
        return self.parent

    def get_position(self):
        return self.x, self.y

    def set_parent(self, p):
        self.parent = p

    def set_position(self, x, y):
        self.x = x
        self.y = y


class Tree:
    def __init__(self):
        self.nodes = []
        self.edges = []

    def add_node(self, n):
        self.nodes.append(n)

    def add_edge(self, e):
        self.edges.append(e)

    # this function would draw the path we've found from goal to start points
    def trace_back(self):
        return 0


# global tree structure
tree = Tree()


def get_random_node(space):
    return 0


def find_nearest_neighbor(space, q_rand):
    return 0


def extend_tree(q_near, q_rand, delta):
    return 0


def too_similar(q_near, q_new):
    return 0


# check if we run into a collision
def is_valid(q_new):
    return 0


"""
n = number of iterations
p = number of threads
m = batch size
delta = edge length of connected nodes
"""


def sample(n, p, m, delta, config_space):
    i = 0
    while i < n/p:
        # create local storage for holding newly-sampled points
        local_container = []

        # perform m local samples
        for j in range(0, m):

            # sample a random value, returns an (x, y) coordinate
            q_rand = get_random_node(config_space)

            # find nearest neighbor to q_rand, returns a Node object pointer
            q_near = find_nearest_neighbor(config_space, q_rand)

            # extend the RRT tree by creating new point, returns Node object pointer
            q_new = extend_tree(q_near, q_rand, delta)

            if not too_similar(q_near, q_new) and is_valid(q_new):
                q_new.set_parent(q_near)
                local_container.append((q_near, q_new))

        # add nodes to the tree
        for node_pair in local_container:
            tree.add_node(node_pair[1])
            tree.add_edge(node_pair)
            i += 1
