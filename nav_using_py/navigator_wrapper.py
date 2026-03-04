import ctypes
import os

# 1. Load the Shared Library
# We look for 'libnavigator.so' in the current directory
lib_path = os.path.abspath("libnavigator.so")
lib = ctypes.CDLL(lib_path)

# 2. Define C Structures in Python

class Node(ctypes.Structure):
    _fields_ = [
        ("id", ctypes.c_int),
        ("latitude", ctypes.c_double),
        ("longitude", ctypes.c_double),
        ("name", ctypes.c_char * 64)
    ]

class Edge(ctypes.Structure):
    pass

Edge._fields_ = [
    ("destination_id", ctypes.c_int),
    ("weight", ctypes.c_double),
    ("road_name", ctypes.c_char * 32),
    ("next", ctypes.POINTER(Edge))
]

class Graph(ctypes.Structure):
    _fields_ = [
        ("nodes", ctypes.POINTER(Node)),
        ("adjacency_list", ctypes.POINTER(ctypes.POINTER(Edge))),
        ("num_nodes", ctypes.c_int),
        ("num_edges", ctypes.c_int),
        ("capacity", ctypes.c_int)
    ]

class PathResult(ctypes.Structure):
    _fields_ = [
        ("path", ctypes.POINTER(ctypes.c_int)),
        ("path_length", ctypes.c_int),
        ("total_distance", ctypes.c_double),
        ("found", ctypes.c_bool)
    ]

# 3. Define Argument and Return Types for C Functions

# Graph* create_graph(int capacity);
lib.create_graph.argtypes = [ctypes.c_int]
lib.create_graph.restype = ctypes.POINTER(Graph)

# void destroy_graph(Graph* graph);
lib.destroy_graph.argtypes = [ctypes.POINTER(Graph)]

# bool load_road_network(Graph* graph, const char* filename);
lib.load_road_network.argtypes = [ctypes.POINTER(Graph), ctypes.c_char_p]
lib.load_road_network.restype = ctypes.c_bool

# const Node* get_node(const Graph* graph, int node_id);
lib.get_node.argtypes = [ctypes.POINTER(Graph), ctypes.c_int]
lib.get_node.restype = ctypes.POINTER(Node)

# PathResult dijkstra_shortest_path(const Graph* graph, int start_id, int end_id);
lib.dijkstra_shortest_path.argtypes = [ctypes.POINTER(Graph), ctypes.c_int, ctypes.c_int]
lib.dijkstra_shortest_path.restype = PathResult

# PathResult a_star_shortest_path(const Graph* graph, int start_id, int end_id);
lib.a_star_shortest_path.argtypes = [ctypes.POINTER(Graph), ctypes.c_int, ctypes.c_int]
lib.a_star_shortest_path.restype = PathResult

# void free_path_result(PathResult* result);
lib.free_path_result.argtypes = [ctypes.POINTER(PathResult)]

# Helper to get string from char array
def decode_str(char_arr):
    return char_arr.decode('utf-8')