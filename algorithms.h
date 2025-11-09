/*
 * Pathfinding Algorithms for Navigation System
 */

 #ifndef ALGORITHMS_H
 #define ALGORITHMS_H
 
 #include "graph.h"
 #include <stdbool.h>
 
 typedef struct {
     int* path;
     int path_length;
     double total_distance;
     bool found;
 } PathResult;
 
 // Core Pathfinding API
 PathResult dijkstra_shortest_path(const Graph* graph, int start_id, int end_id);
 PathResult a_star_shortest_path(const Graph* graph, int start_id, int end_id); // <-- NEW
 
 // Result Handling
 void free_path_result(PathResult* result);
 void print_path_result(const PathResult* result, const Graph* graph);
 
 #endif // ALGORITHMS_H
