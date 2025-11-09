/*
 * Navigation System - Interactive Version
 * Asks the user to select a map, algorithm, and a route.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 #include "graph.h"
 #include "algorithms.h"
 
 // Helper function to read a valid integer choice
 int get_int_choice(int max_choice) {
     char input[256];
     int choice = -1;
 
     if (!fgets(input, sizeof(input), stdin)) {
         return -1; // EOF or read error
     }
     choice = (int)strtol(input, NULL, 10);
 
     if (choice < 1 || choice > max_choice) {
         return -1; // Invalid choice
     }
     return choice;
 }
 
 // Helper function to get a valid node ID
 int get_node_id(int max_id) {
     char input[256];
     int node_id = -1;
 
     if (!fgets(input, sizeof(input), stdin)) {
         return -1; // EOF or read error
     }
     node_id = (int)strtol(input, NULL, 10);
 
     if (node_id < 0 || node_id > max_id) {
         return -1; // Invalid node
     }
     return node_id;
 }
 
 
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
     int num_maps = 3;
     int choice = 0;
 
     printf("Please choose a campus map to load:\n");
     for (int i = 0; i < num_maps; i++) {
         printf("  %d. %s\n", i + 1, available_maps[i]);
     }
     printf("Enter choice (1-%d): ", num_maps);
 
     choice = get_int_choice(num_maps);
 
     if (choice == -1) {
         fprintf(stderr, "❌ Invalid choice. Please enter a number between 1 and %d.\n", num_maps);
         return 1;
     }
 
     const char* chosen_map_file = available_maps[choice - 1];
 
     // --- 2. Load Road Network ---
     printf("\nAttempting to load '%s'...\n", chosen_map_file);
     // Increased default capacity, adjust as needed for your maps
     Graph* road_network = create_graph(250); 
     if (!road_network || !load_road_network(road_network, chosen_map_file)) {
         fprintf(stderr, "❌ Failed to load road network. Make sure the file exists and capacity is sufficient.\n");
         if (road_network) destroy_graph(road_network);
         return 1;
     }
     printf("✅ Map loaded successfully. (%d nodes)\n", get_node_count(road_network));
     // print_graph(road_network); // Optional: can be noisy for large graphs
 
     // --- 3. Select Algorithm (NEW SECTION) ---
     printf("\nChoose a pathfinding algorithm:\n");
     printf("  1. Dijkstra (Guaranteed shortest path)\n");
     printf("  2. A* (Optimized, usually faster)\n");
     printf("Enter choice (1-2): ");
 
     int algo_choice = get_int_choice(2);
 
     if (algo_choice == -1) {
         fprintf(stderr, "❌ Invalid algorithm choice.\n");
         destroy_graph(road_network);
         return 1;
     }
 
     // --- 4. Get Route from User ---
     int start_node = -1;
     int destination_node = -1;
     int max_node_id = get_node_count(road_network) - 1;
 
     printf("\n--- Enter Route Details ---\n");
     printf("Available Nodes: 0 to %d\n", max_node_id);
 
     while (start_node == -1) {
         printf("Enter start node: ");
         start_node = get_node_id(max_node_id);
         if (start_node == -1) {
             fprintf(stderr, "  Invalid ID. Please enter a number between 0 and %d.\n", max_node_id);
         }
     }
 
     while (destination_node == -1) {
         printf("Enter destination node: ");
         destination_node = get_node_id(max_node_id);
         if (destination_node == -1) {
             fprintf(stderr, "  Invalid ID. Please enter a number between 0 and %d.\n", max_node_id);
         }
     }
 
     // --- 5. Calculate Route (UPDATED) ---
     PathResult route_result = { .found = false };
     
     if (algo_choice == 1) {
         printf("\nCalculating route (Dijkstra) from Node %d to Node %d...\n", start_node, destination_node);
         route_result = dijkstra_shortest_path(road_network, start_node, destination_node);
     } else {
         printf("\nCalculating route (A*) from Node %d to Node %d...\n", start_node, destination_node);
         route_result = a_star_shortest_path(road_network, start_node, destination_node);
     }
     
     if (route_result.found) {
         printf("✅ Route found!\n");
         print_path_result(&route_result, road_network);
     } else {
         printf("❌ No route found to the destination!\n");
     }
 
     // --- 6. Cleanup ---
     printf("\nCleaning up resources...\n");
     free_path_result(&route_result);
     destroy_graph(road_network);
 
     printf("Program finished.\n");
     return 0;
 }
