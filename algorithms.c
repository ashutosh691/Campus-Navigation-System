 #include "algorithms.h"
 #include "utils.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <float.h>
 #include <limits.h>
 
 #define INFINITY_VAL DBL_MAX
 
 //Internal Priority Queue 
 typedef struct PQNode { 
     int node_id; 
     double priority; 
     struct PQNode* next;
 } PQNode;

 typedef struct { 
     PQNode* head; 
 } PriorityQueue;
 
 static PriorityQueue* pq_create() {
     return calloc(1, sizeof(PriorityQueue));
 }

 static void pq_destroy(PriorityQueue* pq) {
     PQNode* current = pq->head;
     while (current) {
         PQNode* temp = current;
         current = current->next;
         free(temp);
     }
     free(pq);
 }

 static bool pq_is_empty(const PriorityQueue* pq) { return !pq || !pq->head; }

 static void pq_insert(PriorityQueue* pq, int node_id, double priority) {
     PQNode* new_node = malloc(sizeof(PQNode));
     new_node->node_id = node_id; 
     new_node->priority = priority;
     if (!pq->head || priority < pq->head->priority) {
         new_node->next = pq->head; pq->head = new_node;
     } else {
         PQNode* current = pq->head;
         while (current->next && current->next->priority <= priority) current = current->next;
         new_node->next = current->next; 
         current->next = new_node;
     }
 }

 static int pq_extract_min(PriorityQueue* pq) {
     if (pq_is_empty(pq)) return -1;
     PQNode* min_node = pq->head;
     int node_id = min_node->node_id;
     pq->head = min_node->next;
     free(min_node);
     return node_id;
 }
 
 static int* reconstruct_path(const int* predecessors, int start_id, int end_id, int* path_length) {
     int len = 0;
     for (int at = end_id; at != -1; at = predecessors[at]) len++;
 
     int* path = malloc(len * sizeof(int));
     if (!path) {
         *path_length = 0;
         return NULL;
     }
 
     *path_length = len;
     int current = end_id;
     for (int i = len - 1; i >= 0; i--) {
         path[i] = current;
         current = predecessors[current];
     }
 
     if (len > 0 && path[0] != start_id) {
         free(path);
         *path_length = 0;
         return NULL;
     }
     
     return path;
 }
 
 // A* HEURISTIC FUNCTION
 static double heuristic(const Graph* graph, int node_id, int end_id) {
     const Node* current = get_node(graph, node_id);
     const Node* end = get_node(graph, end_id);
 
     if (!current || !end) {
         return 0.0;
     }

     return haversine_distance(current->latitude, current->longitude,end->latitude, end->longitude);
 }
 
 // Dijkstra 
 PathResult dijkstra_shortest_path(const Graph* graph, int start_id, int end_id) {
     PathResult result = { .found = false };
     int num_nodes = get_node_count(graph);
     double* distances = calloc(num_nodes, sizeof(double));
     int* predecessors = calloc(num_nodes, sizeof(int));
     PriorityQueue* pq = pq_create();
 
     for (int i = 0; i < num_nodes; i++) {
         distances[i] = INFINITY_VAL;
         predecessors[i] = -1;
     }
     distances[start_id] = 0.0;
     pq_insert(pq, start_id, 0.0);
 
     while (!pq_is_empty(pq)) {
         int current_id = pq_extract_min(pq);
         if (current_id == end_id) break;
         const Edge* edge = get_edges(graph, current_id);
         while (edge) {
             double new_dist = distances[current_id] + edge->weight;
             if (new_dist < distances[edge->destination_id]) {
                 distances[edge->destination_id] = new_dist;
                 predecessors[edge->destination_id] = current_id;
                 pq_insert(pq, edge->destination_id, new_dist);
             }
             edge = edge->next;
         }
     }
 
     if (distances[end_id] != INFINITY_VAL) {
         result.path = reconstruct_path(predecessors, start_id, end_id, &result.path_length);
         
         if (result.path) {
             result.total_distance = distances[end_id];
             result.found = true;
         }
     }
 
     free(distances);
     free(predecessors);
     pq_destroy(pq);
     return result;
 }
 
 // A*
 PathResult a_star_shortest_path(const Graph* graph, int start_id, int end_id) {
     PathResult result = { .found = false };
     int num_nodes = get_node_count(graph);
 
     double* g_scores = calloc(num_nodes, sizeof(double));
     double* f_scores = calloc(num_nodes, sizeof(double));
     int* predecessors = calloc(num_nodes, sizeof(int));
     PriorityQueue* pq = pq_create();
 
     for (int i = 0; i < num_nodes; i++) {
         g_scores[i] = INFINITY_VAL;    //actual cost from starting
         f_scores[i] = INFINITY_VAL;    //guess + heuristic for guiding a* in a straight line
         predecessors[i] = -1;
     }
 
     g_scores[start_id] = 0.0;
     f_scores[start_id] = heuristic(graph, start_id, end_id);
     
     pq_insert(pq, start_id, f_scores[start_id]);
 
     while (!pq_is_empty(pq)) {
         int current_id = pq_extract_min(pq);
         if (current_id == end_id) break;
 
         const Edge* edge = get_edges(graph, current_id);
         while (edge) {
             int neighbor_id = edge->destination_id;
             double tentative_g_score = g_scores[current_id] + edge->weight;
 
             if (tentative_g_score < g_scores[neighbor_id]) {
                 predecessors[neighbor_id] = current_id;
                 g_scores[neighbor_id] = tentative_g_score;
                 f_scores[neighbor_id] = tentative_g_score + heuristic(graph, neighbor_id, end_id);
                 pq_insert(pq, neighbor_id, f_scores[neighbor_id]);
             }
             edge = edge->next;
         }
     }
 
     if (g_scores[end_id] != INFINITY_VAL) {
         result.path = reconstruct_path(predecessors, start_id, end_id, &result.path_length);
         
         if (result.path) {
             result.total_distance = g_scores[end_id];
             result.found = true;
         }
     }
 
     free(g_scores);
     free(f_scores);
     free(predecessors);
     pq_destroy(pq);
     return result;
 }
 
 void free_path_result(PathResult* result) {
     if (result && result->path) {
         free(result->path);
         result->path = NULL;
         result->path_length = 0;
         result->found = false;
     }
 }
 
 void print_path_result(const PathResult* result, const Graph* graph) {
     if (!result || !result->found) {
         printf("\n--- No Path Found ---\n");
         return;
     }
     printf("\n\tPath Result\n");
     printf("Total Distance: %.2f km\n", result->total_distance);
     printf("Route:\n");
     for (int i = 0; i < result->path_length; i++) {
         int node_id = result->path[i];
         const Node* node = get_node(graph, node_id);
         printf("  %d. Node %d (%s)\n", i + 1, node_id, node->name);
     }
     printf("\n");
 }
