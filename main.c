/*
 * Navigation System - Interactive Version
 * Asks the user to select a map and a route.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 #include "graph.h"
 #include "algorithms.h"
 
 int main() {
     printf("=======================================\n");
     printf("    Campus Navigation System\n");
     printf("=======================================\n\n");
 
     // --- 1. Map Selection ---
     const char* available_maps[] = {
         "dehradun_campus.txt",
         "bhimtal_campus.txt",
         "haldwani_campus.txt"
     };
     int num_maps = sizeof(available_maps) / sizeof(available_maps[0]);
     int choice = 0;
     char input[256];
 
     printf("Please choose a campus map to load:\n");
     for (int i = 0; i < num_maps; i++) {
         printf("  %d. %s\n", i + 1, available_maps[i]);
     }
     printf("Enter choice (1-%d): ", num_maps);
 
     if (!fgets(input, sizeof(input), stdin)) {
         fprintf(stderr, "Error reading input.\n");
         return 1;
     }
     choice = (int)strtol(input, NULL, 10);
 
     if (choice < 1 || choice > num_maps) {
         fprintf(stderr, "❌ Invalid choice. Please enter a number between 1 and %d.\n", num_maps);
         return 1;
     }
 
     const char* chosen_map_file = available_maps[choice - 1];
 
     // --- 2. Load Road Network ---
     printf("\nAttempting to load '%s'...\n", chosen_map_file);
     Graph* road_network = create_graph(100);
     if (!road_network || !load_road_network(road_network, chosen_map_file)) {
         fprintf(stderr, "❌ Failed to load road network. Make sure the file exists.\n");
         if (road_network) destroy_graph(road_network);
         return 1;
     }
     printf("✅ Map loaded successfully.\n");
     print_graph(road_network);
 
     // --- 3. Get Route from User ---
     int start_node = -1;
     int destination_node = -1;
     int max_node_id = get_node_count(road_network) - 1;
 
     printf("\nEnter start node (0-%d): ", max_node_id);
     if (fgets(input, sizeof(input), stdin)) {
         start_node = (int)strtol(input, NULL, 10);
     }
 
     printf("Enter destination node (0-%d): ", max_node_id);
     if (fgets(input, sizeof(input), stdin)) {
         destination_node = (int)strtol(input, NULL, 10);
     }
 
     // --- 4. Calculate Route ---
     PathResult route_result = { .found = false };
     printf("\nCalculating route from Node %d to Node %d...\n", start_node, destination_node);
     
     if (!is_valid_node(road_network, start_node) || !is_valid_node(road_network, destination_node)) {
          fprintf(stderr, "❌ Invalid start or destination node ID.\n");
     } else {
         route_result = dijkstra_shortest_path(road_network, start_node, destination_node);
         if (route_result.found) {
             printf("✅ Route found!\n");
             print_path_result(&route_result, road_network);
         } else {
             printf("❌ No route found to the destination!\n");
         }
     }
 
     // --- 5. Cleanup ---
     printf("\nCleaning up resources...\n");
     free_path_result(&route_result);
     destroy_graph(road_network);
 
     printf("Program finished.\n");
     return 0;
 }