Campus Navigation System

This project is a C-based graphical navigation application that finds the shortest path between two points on a university campus map. It uses a graph data structure to represent the campus, the Haversine formula to calculate real-world distances, and provides both Dijkstra's and A* algorithms for pathfinding.

The application is built with GTK 4 for a modern, visual, and interactive user interface that displays the campus map and the calculated route.

Features

Visual Map Display: Renders the campus map (nodes and edges) in a resizable window using the Cairo 2D graphics library.

Correct Scaling: The map preserves its real-world aspect ratio, ensuring it never looks stretched or distorted, regardless of window size.

Dual Algorithms: Allows the user to select between:

Dijkstra's Algorithm: Guarantees the shortest path.

*A (A-star) Algorithm:** A faster, optimized algorithm that uses a Haversine distance heuristic.

Interactive Controls:

Displays a scrollable list of all available nodes (locations) and their names.

Provides simple text entry for start and destination nodes.

Highlights the shortest path in red directly on the map.

Displays the total path distance in kilometers.

Data-Driven: Loads campus layout, node locations (latitude/longitude), and connections from a simple .txt file.

Dependencies

To build and run this application, you will need:

A C compiler (e.g., gcc)

The make build tool

The GTK 4 development libraries (e.g., libgtk-4-dev on Ubuntu/Debian)

How to Build

A Makefile is provided for easy compilation.

Make sure all dependencies (especially libgtk-4-dev) are installed.

Open your terminal in the project's root directory.

Run the make command:

make


This will compile all the .c files and create a single executable file named navigator-gui.

How to Run

Ensure the dehradun_campus.txt file is in the same directory as the executable.

Run the application from your terminal:

./navigator-gui


How It Works

On Startup: The application loads the dehradun_campus.txt file into the Graph data structure.

File Loading (graph.c):

The node and edge counts are read.

Each node (latitude, longitude, name) is loaded into an array.

Each edge (connection) is read. The Haversine distance is automatically calculated as the edge's "weight" (in km) using the utils.c functions.

Display (main-gtk.c):

The full list of nodes is shown in the side panel.

The on_draw function uses Cairo to render all nodes and edges onto the map canvas, correctly scaling the coordinates to fit the window.

Pathfinding (main-gtk.c -> algorithms.c):

The user enters a Start and End node ID and clicks "Find Shortest Path."

The selected algorithm (Dijkstra or A*) is called.

*A Heuristic:** The A* algorithm is guided by the heuristic() function, which provides a "straight-line" (Haversine) distance from any node to the destination, allowing it to find the path much more efficiently.

The resulting PathResult (containing the list of nodes and total distance) is returned.

Result: The path is drawn in red on the map, and the total distance is reported in the status label.

File Structure

main-gtk.c: The main application file. Contains all GTK 4 UI code, event callbacks (button clicks), and the Cairo drawing logic.

graph.h / graph.c: Defines the Graph, Node, and Edge data structures. Handles creating/destroying the graph and loading it from the .txt file.

algorithms.h / algorithms.c: Implements the dijkstra_shortest_path and a_star_shortest_path algorithms, as well as the internal priority queue.

utils.h / utils.c: Contains the haversine_distance formula and math constants (PI, EARTH_RADIUS_KM).

dehradun_campus.txt: The map data file for the Graphic Era campus.

Makefile: The build script.
