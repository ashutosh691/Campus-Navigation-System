# C Campus Navigation System

This is a lightweight, command-line navigation system written in C. It calculates the shortest path between two points on a campus map using Dijkstra's algorithm.

## Features
- **Interactive Menu**: Allows users to choose from multiple campus maps at runtime.
- **Dynamic Routing**: Prompts the user for a start and destination node after loading a map.
- **Dijkstra's Algorithm**: Implements Dijkstra's shortest path algorithm to find the most efficient route.
- **Customizable Maps**: Can load any campus map from a `.txt` file as long as it follows the specified format.
- **Modular Design**: The code is separated into logical modules:
    - `graph`: Manages the core graph data structure.
    - `algorithms`: Contains the pathfinding logic.
    - `utils`: Provides helper functions.
    - `main`: Handles user interaction and orchestrates the program flow.

## How to Compile and Run

### Prerequisites
- A C compiler like **GCC** or **Clang**.
- The **`make`** build tool.
- On macOS, you can get these by installing the Xcode Command Line Tools: `xcode-select --install`.

### Steps
1.  **Clone the repository** (or ensure all `.c`, `.h`, `.txt`, and the `Makefile` are in the same directory).
2.  **Compile the program** by running the `make` command in your terminal:
    ```bash
    make
    ```
3.  **Run the executable**:
    ```bash
    ./navigator
    ```
4.  **Clean up generated files** (optional):
    ```bash
    make clean
    ```