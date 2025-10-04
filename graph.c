/*
 * Graph Data Structure Implementation
 * Reduced to core creation, modification, and loading functionality.
 */

 #include "graph.h"
 #include "utils.h" 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 Graph* create_graph(int capacity) {
     if (capacity <= 0) return NULL;
     Graph* graph = malloc(sizeof(Graph));
     if (!graph) return NULL;
     graph->nodes = calloc(capacity, sizeof(Node));
     graph->adjacency_list = calloc(capacity, sizeof(Edge*));
     if (!graph->nodes || !graph->adjacency_list) {
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
     if (!graph || graph->num_nodes >= graph->capacity) return -1;
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
     if (!graph || !is_valid_node(graph, source_id) || !is_valid_node(graph, destination_id)) return false;
     Edge* new_edge = malloc(sizeof(Edge));
     if (!new_edge) return false;
     new_edge->destination_id = destination_id;
     new_edge->weight = weight;
     // Basic road name assignment, ignoring the road_name parameter in this version
     snprintf(new_edge->road_name, sizeof(new_edge->road_name), "Road");
     new_edge->next = graph->adjacency_list[source_id];
     graph->adjacency_list[source_id] = new_edge;
     graph->num_edges++;
     return true;
 }
 
 bool add_bidirectional_edge(Graph* graph, int node1_id, int node2_id, double weight, const char* road_name) {
     if (!add_edge(graph, node1_id, node2_id, weight, road_name)) return false;
     if (!add_edge(graph, node2_id, node1_id, weight, road_name)) return false;
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
     if (!graph) return;
     printf("--- Graph Info (Nodes: %d, Edges: %d) ---\n", graph->num_nodes, graph->num_edges);
     for (int i = 0; i < graph->num_nodes; i++) {
         const Node* n = &graph->nodes[i];
         printf("Node %d: '%s'\n", n->id, n->name);
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
     printf("----------------------------------------\n");
 }
 
 bool load_road_network(Graph* graph, const char* filename) {
     if (!graph || !filename) return false;
     FILE* file = fopen(filename, "r");
     if (!file) return false;
 
     char line[256];
     int file_nodes_count = 0, file_edges_count = 0;
 
     while (fgets(line, sizeof(line), file)) {
         if (line[0] == '#' || line[0] == '\n') continue;
         if (sscanf(line, "%d %d", &file_nodes_count, &file_edges_count) == 2) break;
     }
     if (file_nodes_count > graph->capacity) {
         fclose(file);
         return false;
     }
 
     for (int i = 0; i < file_nodes_count; ) {
         if (!fgets(line, sizeof(line), file)) { fclose(file); return false; }
         if (line[0] == '#' || line[0] == '\n') continue;
         double lat, lon;
         char name[64] = "";
         sscanf(line, "%lf %lf %63[^\n]", &lat, &lon, name);
         add_node(graph, lat, lon, name);
         i++;
     }
 
     for (int i = 0; i < file_edges_count; ) {
         if (!fgets(line, sizeof(line), file)) { fclose(file); return false; }
         if (line[0] == '#' || line[0] == '\n') continue;
         int source, dest;
         double weight = -1.0;
         sscanf(line, "%d %d %lf", &source, &dest, &weight);
         if (weight <= 0) {
             const Node* n1 = get_node(graph, source);
             const Node* n2 = get_node(graph, dest);
             weight = haversine_distance(n1->latitude, n1->longitude, n2->latitude, n2->longitude);
         }
         add_bidirectional_edge(graph, source, dest, weight, NULL);
         i++;
     }
     fclose(file);
     return true;
 }