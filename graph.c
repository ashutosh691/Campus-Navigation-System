/*
 * Graph Data Structure Implementation
 */

 #include "graph.h"
 #include "utils.h"  // For haversine_distance
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 Graph* create_graph(int capacity) {
     if (capacity <= 0) {
         fprintf(stderr, "[Graph Error] create_graph: Invalid capacity %d\n", capacity);
         return NULL;
     }
     
     Graph* graph = malloc(sizeof(Graph));
     if (!graph) {
         fprintf(stderr, "[Graph Error] create_graph: Failed to allocate memory for graph struct\n");
         return NULL;
     }
     
     graph->nodes = calloc(capacity, sizeof(Node));
     graph->adjacency_list = calloc(capacity, sizeof(Edge*));
     
     if (!graph->nodes || !graph->adjacency_list) {
         fprintf(stderr, "[Graph Error] create_graph: Failed to allocate memory for node/adjacency lists\n");
         free(graph->nodes);
         free(graph->adjacency_list);
         free(graph);
         return NULL;
     }
     
     graph->num_nodes = 0;
     graph->num_edges = 0;
     graph->capacity = capacity;
     return graph;
 }
 
 void destroy_graph(Graph* graph) {
     if (!graph) return;
     
     for (int i = 0; i < graph->num_nodes; i++) {
         Edge* current = graph->adjacency_list[i];
         while (current) {
             Edge* temp = current;
             current = current->next;
             free(temp);
         }
     }
     
     free(graph->nodes);
     free(graph->adjacency_list);
     free(graph);
 }
 
 int add_node(Graph* graph, double latitude, double longitude, const char* name) {
     if (!graph) return -1;
     if (graph->num_nodes >= graph->capacity) {
         fprintf(stderr, "[Graph Error] add_node: Graph is full (capacity %d)\n", graph->capacity);
         return -1;
     }
     
     int node_id = graph->num_nodes;
     graph->nodes[node_id].id = node_id;
     graph->nodes[node_id].latitude = latitude;
     graph->nodes[node_id].longitude = longitude;
     
     strncpy(graph->nodes[node_id].name, name, sizeof(graph->nodes[node_id].name) - 1);
     graph->nodes[node_id].name[sizeof(graph->nodes[node_id].name) - 1] = '\0';
     
     graph->adjacency_list[node_id] = NULL;
     graph->num_nodes++;
     return node_id;
 }
 
 bool add_edge(Graph* graph, int source_id, int destination_id, double weight, const char* road_name) {
     if (!graph || !is_valid_node(graph, source_id) || !is_valid_node(graph, destination_id)) {
         fprintf(stderr, "[Graph Error] add_edge: Invalid source (%d) or destination (%d)\n", source_id, destination_id);
         return false;
     }
     
     Edge* new_edge = malloc(sizeof(Edge));
     if (!new_edge) {
         fprintf(stderr, "[Graph Error] add_edge: Failed to allocate memory for new edge\n");
         return false;
     }
     
     new_edge->destination_id = destination_id;
     new_edge->weight = weight;
     
     if (road_name) {
         strncpy(new_edge->road_name, road_name, sizeof(new_edge->road_name) - 1);
         new_edge->road_name[sizeof(new_edge->road_name) - 1] = '\0';
     } else {
         snprintf(new_edge->road_name, sizeof(new_edge->road_name), "Path");
     }
     
     // Insert at the head of the linked list
     new_edge->next = graph->adjacency_list[source_id];
     graph->adjacency_list[source_id] = new_edge;
     
     graph->num_edges++;
     return true;
 }
 
 bool add_bidirectional_edge(Graph* graph, int node1_id, int node2_id, double weight, const char* road_name) {
     // Add both directions
     if (!add_edge(graph, node1_id, node2_id, weight, road_name)) return false;
     if (!add_edge(graph, node2_id, node1_id, weight, road_name)) {
         // Technically should clean up the first edge, but this is rare
         return false;
     }
     return true;
 }
 
 const Node* get_node(const Graph* graph, int node_id) {
     if (!is_valid_node(graph, node_id)) return NULL;
     return &graph->nodes[node_id];
 }
 
 const Edge* get_edges(const Graph* graph, int node_id) {
     if (!is_valid_node(graph, node_id)) return NULL;
     return graph->adjacency_list[node_id];
 }
 
 bool is_valid_node(const Graph* graph, int node_id) {
     return graph && node_id >= 0 && node_id < graph->num_nodes;
 }
 
 int get_node_count(const Graph* graph) {
     return graph ? graph->num_nodes : 0;
 }
 
 void print_graph(const Graph* graph) {
     if (!graph) {
         printf("Graph is NULL.\n");
         return;
     }
     printf("--- Graph Info (Nodes: %d, Edges: %d, Capacity: %d) ---\n", 
            graph->num_nodes, graph->num_edges, graph->capacity);
     for (int i = 0; i < graph->num_nodes; i++) {
         const Node* n = &graph->nodes[i];
         printf("Node %d: '%s' (%.5f, %.5f)\n", n->id, n->name, n->latitude, n->longitude);
         const Edge* edge = graph->adjacency_list[i];
         if (edge) {
             printf("  -> Edges: ");
             while (edge) {
                 printf("[%d](%.2fkm) ", edge->destination_id, edge->weight);
                 edge = edge->next;
             }
             printf("\n");
         }
     }
     printf("------------------------------------------------------\n");
 }
 
 bool load_road_network(Graph* graph, const char* filename) {
     if (!graph || !filename) {
         fprintf(stderr, "[Graph Error] load_road_network: Graph or filename is NULL.\n");
         return false;
     }
     
     FILE* file = fopen(filename, "r");
     if (!file) {
         fprintf(stderr, "[Graph Error] load_road_network: Could not open file '%s'.\n", filename);
         return false;
     }
 
     char line[256];
     int file_nodes_count = 0, file_edges_count = 0;
 
     // Read header (num_nodes num_edges)
     while (fgets(line, sizeof(line), file)) {
         if (line[0] == '#' || line[0] == '\n') continue; // Skip comments/blank lines
         if (sscanf(line, "%d %d", &file_nodes_count, &file_edges_count) == 2) {
             break;
         }
     }
 
     if (file_nodes_count == 0) {
          fprintf(stderr, "[Graph Error] load_road_network: Failed to read node/edge count header from '%s'.\n", filename);
          fclose(file);
          return false;
     }
 
     if (file_nodes_count > graph->capacity) {
         fprintf(stderr, "[Graph Error] load_road_network: Map requires %d nodes, but graph capacity is only %d.\n", 
                 file_nodes_count, graph->capacity);
         fclose(file);
         return false;
     }
 
     // Read nodes
     int nodes_read = 0;
     while (nodes_read < file_nodes_count && fgets(line, sizeof(line), file)) {
         if (line[0] == '#' || line[0] == '\n') continue;
         
         double lat, lon;
         char name[64] = "";
         // Use sscanf to parse the line
         if (sscanf(line, "%lf %lf %63[^\n]", &lat, &lon, name) >= 2) {
             if (add_node(graph, lat, lon, name) == -1) {
                 fprintf(stderr, "[Graph Error] load_road_network: Failed to add node.\n");
                 fclose(file);
                 return false;
             }
             nodes_read++;
         } else {
              fprintf(stderr, "[Graph Error] load_road_network: Malformed node line: %s", line);
         }
     }
 
     if (nodes_read != file_nodes_count) {
         fprintf(stderr, "[Graph Error] load_road_network: Expected %d nodes, but only read %d.\n", 
                 file_nodes_count, nodes_read);
         fclose(file);
         return false;
     }
 
     // Read edges
     int edges_read = 0;
     while (edges_read < file_edges_count && fgets(line, sizeof(line), file)) {
         if (line[0] == '#' || line[0] == '\n') continue;
         
         int source, dest;
         double weight = 0.0; // Default to 0, which triggers auto-calc
         
         int items_scanned = sscanf(line, "%d %d %lf", &source, &dest, &weight);
         
         if (items_scanned >= 2) {
             if (weight <= 0) { // If weight is 0 or not provided, calculate it
                 const Node* n1 = get_node(graph, source);
                 const Node* n2 = get_node(graph, dest);
                 if (!n1 || !n2) {
                     fprintf(stderr, "[Graph Error] load_road_network: Invalid node IDs (%d, %d) in edge line.\n", source, dest);
                     continue;
                 }
                 weight = haversine_distance(n1->latitude, n1->longitude, n2->latitude, n2->longitude);
             }
             if (!add_bidirectional_edge(graph, source, dest, weight, NULL)) {
                 fprintf(stderr, "[Graph Error] load_road_network: Failed to add edge (%d, %d).\n", source, dest);
             }
             edges_read++;
         } else {
             fprintf(stderr, "[Graph Error] load_road_network: Malformed edge line: %s", line);
         }
     }
 
     if (edges_read != file_edges_count) {
         fprintf(stderr, "[Graph Error] load_road_network: Expected %d edges, but only read %d.\n", 
                 file_edges_count, edges_read);
         // This is not a fatal error, so we don't return false
     }
 
     fclose(file);
     return true;
 }
