 #ifndef GRAPH_H
 #define GRAPH_H
 
 #include <stdbool.h>
 
 typedef struct {
     int id;
     double latitude;
     double longitude;
     char name[60];
 } Node;
 
 typedef struct Edge {
     int destination_id;
     double weight;
     char road_name[30];
     struct Edge* next;
 } Edge;
 
 typedef struct {
     Node* nodes;
     Edge** adjacency_list;
     int num_nodes;
     int num_edges;
     int capacity;
 } Graph;
 
 // Lifecycle Management
 Graph* create_graph(int capacity);
 void destroy_graph(Graph* graph);
 
 // Modification
 int add_node(Graph* graph, double latitude, double longitude, const char* name);
 bool add_edge(Graph* graph, int source_id, int destination_id, double weight, const char* road_name);
 bool add_bidirectional_edge(Graph* graph, int node1_id, int node2_id, double weight, const char* road_name);
 
 // Information & Queries
 const Node* get_node(const Graph* graph, int node_id);
 const Edge* get_edges(const Graph* graph, int node_id);
 bool is_valid_node(const Graph* graph, int node_id);
 int get_node_count(const Graph* graph);
 void print_graph(const Graph* graph);
 
 // File I/O
 bool load_road_network(Graph* graph, const char* filename);
 
 #endif // GRAPH_H
