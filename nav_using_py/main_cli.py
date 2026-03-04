import os
import ctypes
import webbrowser
import sys
from navigator_wrapper import lib, Graph, Node, PathResult, decode_str

# Configuration
MAP_FILE = "dehradun_campus.txt"
OUTPUT_HTML = "campus_map.html"

def get_valid_int(prompt, max_val):
    while True:
        try:
            val = int(input(prompt))
            if 0 <= val < max_val:
                return val
            print(f"Error: Please enter a number between 0 and {max_val-1}.")
        except ValueError:
            print("Error: Invalid input. Please enter a number.")

def main():
    print("")
    print("   Campus Navigator")
    print("")

    # 1. Initialize C Graph
    print(f"Loading C Library from: {os.path.abspath('libnavigator.so')}")
    graph = lib.create_graph(250)
    if not graph:
        print("Error: Failed to create C Graph memory.")
        return

    # 2. Load Data
    if not os.path.exists(MAP_FILE):
        print(f"Error: File '{MAP_FILE}' not found.")
        return

    print(f"Loading map file: {MAP_FILE}...")
    encoded_filename = MAP_FILE.encode('utf-8')
    success = lib.load_road_network(graph, encoded_filename)

    if not success:
        print("Error: Failed to load map data via C library.")
        return

    num_nodes = graph.contents.num_nodes
    print(f"Map Loaded Successfully! ({num_nodes} nodes)")

    # 3. Display Nodes
    print("\n--- Available Locations ---")
    for i in range(num_nodes):
        node_ptr = lib.get_node(graph, i)
        if node_ptr:
            name = decode_str(node_ptr.contents.name)
            print(f"  [{i}] {name}")
    print("---------------------------")

    # 4. Get User Input
    start_id = get_valid_int("\nEnter Start Node ID: ", num_nodes)
    end_id = get_valid_int("Enter End Node ID:   ", num_nodes)

    print("\nSelect Algorithm:")
    print("  1. Dijkstra (Standard)")
    print("  2. A* (Optimized)")
    algo_choice = input("Enter choice (1 or 2): ").strip()

    # 5. Run C Algorithm
    print("\nCalculating path in C...")
    if algo_choice == "2":
        result = lib.a_star_shortest_path(graph, start_id, end_id)
        method = "A*"
    else:
        result = lib.dijkstra_shortest_path(graph, start_id, end_id)
        method = "Dijkstra"

    if not result.found:
        print("\nNo path found between these locations.")
        lib.free_path_result(ctypes.byref(result))
        lib.destroy_graph(graph)
        return

    # 6. Extract Data
    path_ids = []
    for i in range(result.path_length):
        path_ids.append(result.path[i])
    dist_km = result.total_distance

    print(f" Route Found via {method}!")
    print(f"   Total Distance: {dist_km:.3f} km")
    
    # 7. Generate Map
    generate_map(graph, path_ids, dist_km)
    
    # Cleanup
    lib.free_path_result(ctypes.byref(result))
    lib.destroy_graph(graph)

def generate_map(graph, path_ids, distance):
    import folium
    
    # Get start node for centering
    start_node = lib.get_node(graph, path_ids[0])
    center_lat = start_node.contents.latitude
    center_lon = start_node.contents.longitude

    m = folium.Map(location=[center_lat, center_lon], zoom_start=18)

    # Draw Path
    route_coords = []
    for pid in path_ids:
        n = lib.get_node(graph, pid)
        route_coords.append([n.contents.latitude, n.contents.longitude])
        
        # Small marker for intermediate nodes
        folium.CircleMarker(
            location=[n.contents.latitude, n.contents.longitude],
            radius=3, color="red", fill=True, popup=decode_str(n.contents.name)
        ).add_to(m)

    folium.PolyLine(route_coords, color="red", weight=5, opacity=0.8).add_to(m)

    # Start/End Markers
    start_n = lib.get_node(graph, path_ids[0])
    end_n = lib.get_node(graph, path_ids[-1])

    folium.Marker(
        [start_n.contents.latitude, start_n.contents.longitude],
        popup=f"Start: {decode_str(start_n.contents.name)}",
        icon=folium.Icon(color='green', icon='play')
    ).add_to(m)

    folium.Marker(
        [end_n.contents.latitude, end_n.contents.longitude],
        popup=f"End: {decode_str(end_n.contents.name)}\nDist: {distance:.2f}km",
        icon=folium.Icon(color='black', icon='stop')
    ).add_to(m)

    # Save and Open
    output_path = os.path.abspath(OUTPUT_HTML)
    m.save(output_path)
    print(f"\nMap generated: {output_path}")
    print("Opening in browser...")
    webbrowser.open('file://' + output_path)

if __name__ == "__main__":
    main()