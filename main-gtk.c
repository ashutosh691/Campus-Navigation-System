#include <gtk/gtk.h>
#include <cairo.h> // Include the Cairo graphics library
#include <math.h>  // Include for fmin, cos, and M_PI
#include "graph.h"
#include "algorithms.h"

// Struct to hold widget pointers and shared data
typedef struct {
    GtkEntry* start_entry;
    GtkEntry* end_entry;
    GtkWidget* dijkstra_radio;
    GtkDrawingArea* drawing_area;
    GtkLabel* status_label; // For short status messages
    GtkLabel* node_list_label; // For the long list of nodes

    Graph* graph;
    PathResult path_result; // Stores the last found path

    // Bounding box of the loaded graph for coordinate mapping
    double min_lon, max_lon, min_lat, max_lat;
    
    // --- NEW ---
    // The true, corrected aspect ratio of the map
    double map_aspect_ratio; 

} AppWidgets;


// --- Coordinate Mapping ---

/**
 * Finds the min/max latitude and longitude in the graph
 * and calculates the correct map aspect ratio.
 */
static void find_graph_bounds(AppWidgets* app) {
    if (!app->graph || get_node_count(app->graph) == 0) {
        app->min_lon = app->min_lat = 0;
        app->max_lon = app->max_lat = 1;
        app->map_aspect_ratio = 1.0;
        return;
    }

    const Node* first_node = get_node(app->graph, 0);
    app->min_lon = app->max_lon = first_node->longitude;
    app->min_lat = app->max_lat = first_node->latitude;

    for (int i = 1; i < get_node_count(app->graph); i++) {
        const Node* n = get_node(app->graph, i);
        if (n->longitude < app->min_lon) app->min_lon = n->longitude;
        if (n->longitude > app->max_lon) app->max_lon = n->longitude;
        if (n->latitude < app->min_lat) app->min_lat = n->latitude;
        if (n->latitude > app->max_lat) app->max_lat = n->latitude;
    }

    // Add a little padding so nodes aren't on the exact edge
    double padding_lon = (app->max_lon - app->min_lon) * 0.05;
    double padding_lat = (app->max_lat - app->min_lat) * 0.05;
    
    if (padding_lon == 0.0) padding_lon = 0.0001; // Avoid division by zero
    if (padding_lat == 0.0) padding_lat = 0.0001; // Avoid division by zero

    app->min_lon -= padding_lon;
    app->max_lon += padding_lon;
    app->min_lat -= padding_lat;
    app->max_lat += padding_lat;
    
    // --- NEW: Calculate Correct Aspect Ratio ---
    double map_width_geo = app->max_lon - app->min_lon;
    double map_height_geo = app->max_lat - app->min_lat;
    
    // Calculate cosine correction at the average latitude
    double avg_lat_rad = (app->min_lat + app->max_lat) / 2.0 * (M_PI / 180.0);
    
    // The true aspect ratio is (width_in_degrees * cos(lat)) / (height_in_degrees)
    app->map_aspect_ratio = (map_width_geo * cos(avg_lat_rad)) / map_height_geo;
    
    if (map_height_geo == 0.0) app->map_aspect_ratio = 1.0; // Handle single line case
}

/**
 * Remaps a longitude/latitude coordinate to a normalized (0.0 to 1.0) coordinate.
 */
static void get_normalized_coords(AppWidgets* app, double lon, double lat, 
                                  double* nx, double* ny) 
{
    double map_width = app->max_lon - app->min_lon;
    double map_height = app->max_lat - app->min_lat;
    
    *nx = (map_width == 0.0) ? 0.5 : (lon - app->min_lon) / map_width;
    // Invert Y-axis: screen (0,0) is top-left, map (lat,lon) is bottom-left
    *ny = (map_height == 0.0) ? 0.5 : (1.0 - (lat - app->min_lat) / map_height);
}


// --- Drawing Function ---

/**
 * This is the main drawing callback for the GtkDrawingArea.
 * It's called every time the map needs to be redrawn.
 */
static void on_draw(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data) {
    AppWidgets* app = (AppWidgets*)data;

    // 1. Draw background
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1); // Dark background
    cairo_paint(cr);

    if (!app->graph) return; // No graph loaded

    // --- NEW: Aspect-Ratio-Preserving Scaling ---
    double window_aspect_ratio = (double)width / (double)height;
    double scale_x, scale_y, offset_x, offset_y;

    if (window_aspect_ratio > app->map_aspect_ratio) {
        // Window is wider than the map (letterbox)
        scale_y = height;
        scale_x = height * app->map_aspect_ratio;
        offset_x = (width - scale_x) / 2.0;
        offset_y = 0;
    } else {
        // Window is taller than the map (pillarbox)
        scale_x = width;
        scale_y = width / app->map_aspect_ratio;
        offset_x = 0;
        offset_y = (height - scale_y) / 2.0;
    }
    // --- End of new scaling logic ---


    // 2. Draw all edges (roads)
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5); // Grey for roads
    cairo_set_line_width(cr, 1.0);
    for (int i = 0; i < get_node_count(app->graph); i++) {
        const Node* n1 = get_node(app->graph, i);
        const Edge* edge = get_edges(app->graph, n1->id);
        while (edge) {
            if (n1->id < edge->destination_id) { // Only draw edges once
                const Node* n2 = get_node(app->graph, edge->destination_id);
                double x1, y1, x2, y2, nx1, ny1, nx2, ny2;
                
                get_normalized_coords(app, n1->longitude, n1->latitude, &nx1, &ny1);
                get_normalized_coords(app, n2->longitude, n2->latitude, &nx2, &ny2);

                // Apply the new scaling
                x1 = (nx1 * scale_x) + offset_x;
                y1 = (ny1 * scale_y) + offset_y;
                x2 = (nx2 * scale_x) + offset_x;
                y2 = (ny2 * scale_y) + offset_y;
                
                cairo_move_to(cr, x1, y1);
                cairo_line_to(cr, x2, y2);
                cairo_stroke(cr);
            }
            edge = edge->next;
        }
    }

    // 3. Draw all nodes (intersections) and their names
    for (int i = 0; i < get_node_count(app->graph); i++) {
        const Node* n = get_node(app->graph, i);
        double x, y, nx, ny;
        get_normalized_coords(app, n->longitude, n->latitude, &nx, &ny);
        
        // Apply the new scaling
        x = (nx * scale_x) + offset_x;
        y = (ny * scale_y) + offset_y;
        
        // Draw the node circle
        cairo_set_source_rgb(cr, 0.2, 0.8, 1.0); // Light blue for nodes
        cairo_arc(cr, x, y, 3.0, 0, 2 * M_PI); // 3px radius circle
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
        cairo_set_line_width(cr, 0.5);
        cairo_stroke(cr);

        // Draw node name and ID
        char label_text[80];
        snprintf(label_text, sizeof(label_text), "[%d] %s", n->id, n->name);
        
        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 9.0);
        
        cairo_move_to(cr, x + 5, y + 4);
        cairo_show_text(cr, label_text);
    }

    // 4. Draw the found path (if it exists)
    if (app->path_result.found) {
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.2); // Bright red for path
        cairo_set_line_width(cr, 3.0);
        for (int i = 0; i < app->path_result.path_length - 1; i++) {
            const Node* n1 = get_node(app->graph, app->path_result.path[i]);
            const Node* n2 = get_node(app->graph, app->path_result.path[i + 1]);
            double x1, y1, x2, y2, nx1, ny1, nx2, ny2;

            get_normalized_coords(app, n1->longitude, n1->latitude, &nx1, &ny1);
            get_normalized_coords(app, n2->longitude, n2->latitude, &nx2, &ny2);
            
            // Apply the new scaling
            x1 = (nx1 * scale_x) + offset_x;
            y1 = (ny1 * scale_y) + offset_y;
            x2 = (nx2 * scale_x) + offset_x;
            y2 = (ny2 * scale_y) + offset_y;
            
            cairo_move_to(cr, x1, y1);
            cairo_line_to(cr, x2, y2);
            cairo_stroke(cr);
        }
    }
}


// --- GTK Callbacks ---

/**
 * Callback for the "Find Path" button.
 */
static void on_find_path_clicked(GtkWidget* widget, gpointer data) {
    AppWidgets* app = (AppWidgets*)data;

    // Clear the old path before finding a new one
    free_path_result(&app->path_result);

    const char* start_text = gtk_editable_get_text(GTK_EDITABLE(app->start_entry));
    const char* end_text = gtk_editable_get_text(GTK_EDITABLE(app->end_entry));
    int start_node = atoi(start_text);
    int end_node = atoi(end_text);

    if (!app->graph) {
        gtk_label_set_text(app->status_label, "Error: No map loaded.");
        return;
    }
    if (!is_valid_node(app->graph, start_node) || !is_valid_node(app->graph, end_node)) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Error: Invalid node ID. Use 0-%d.", get_node_count(app->graph) - 1);
        gtk_label_set_text(app->status_label, buffer);
        return;
    }

    gboolean use_dijkstra = gtk_check_button_get_active(GTK_CHECK_BUTTON(app->dijkstra_radio));
    const char* algo_name = use_dijkstra ? "Dijkstra" : "A*";

    if (use_dijkstra) {
        app->path_result = dijkstra_shortest_path(app->graph, start_node, end_node);
    } else {
        app->path_result = a_star_shortest_path(app->graph, start_node, end_node);
    }
    
    if (app->path_result.found) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Path found (%s): %.2f km", algo_name, app->path_result.total_distance);
        gtk_label_set_text(app->status_label, buffer);
    } else {
        gtk_label_set_text(app->status_label, "No path found between these locations.");
    }
    
    // IMPORTANT: Force the drawing area to redraw itself
    gtk_widget_queue_draw(GTK_WIDGET(app->drawing_area));
}

/**
 * Loads the single, hard-coded default map.
 */
static void load_default_map(gpointer data) {
    AppWidgets* app = (AppWidgets*)data;
    const char* map_file = "dehradun_campus.txt"; // Hard-coded map
    
    // Clear old graph and path
    if (app->graph) {
        destroy_graph(app->graph);
        app->graph = NULL;
    }
    free_path_result(&app->path_result);
    gtk_label_set_text(app->node_list_label, ""); // Clear old node list
    
    app->graph = create_graph(250); // Set a reasonable default capacity
    if (!app->graph) {
        gtk_label_set_text(app->status_label, "Error: Could not allocate memory for graph.");
        return;
    }
    
    if (load_road_network(app->graph, map_file)) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Loaded '%s'. Ready (Nodes 0-%d).",
                 map_file, get_node_count(app->graph) - 1);
        gtk_label_set_text(app->status_label, buffer);
        
        // Find the new map's boundaries and aspect ratio
        find_graph_bounds(app);

        // --- Populate the node list ---
        GString* list_str = g_string_new("");
        for (int i = 0; i < get_node_count(app->graph); i++) {
            const Node* node = get_node(app->graph, i);
            g_string_append_printf(list_str, "[%d] %s\n", node->id, node->name);
        }
        gtk_label_set_text(app->node_list_label, list_str->str);
        g_string_free(list_str, TRUE);

    } else {
        gtk_label_set_text(app->status_label, "Error: Failed to load 'dehradun_campus.txt'.");
        destroy_graph(app->graph);
        app->graph = NULL;
    }
    
    // Redraw the map area with the new graph (or clear it if failed)
    gtk_widget_queue_draw(GTK_WIDGET(app->drawing_area));
}

/**
 * Frees all allocated memory when the window is closed.
 */
static void on_window_destroy(GtkWidget* widget, gpointer data) {
    AppWidgets* app = (AppWidgets*)data;
    if (app->graph) {
        destroy_graph(app->graph);
    }
    free_path_result(&app->path_result);
    g_slice_free(AppWidgets, app);
}

/**
 * Main function to build the application UI.
 */
static void on_app_activate(GtkApplication* app, gpointer user_data) {
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Campus Navigation System (Visual)");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);

    // Use a Paned widget to make controls and map resizable
    GtkWidget* paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_window_set_child(GTK_WINDOW(window), paned);

    // --- 1. Left-side Control Panel ---
    GtkWidget* controls_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(controls_box, 15);
    gtk_widget_set_margin_end(controls_box, 15);
    gtk_widget_set_margin_top(controls_box, 15);
    gtk_widget_set_margin_bottom(controls_box, 15);
    gtk_paned_set_start_child(GTK_PANED(paned), controls_box);
    gtk_paned_set_resize_start_child(GTK_PANED(paned), FALSE);
    gtk_paned_set_shrink_start_child(GTK_PANED(paned), FALSE);

    // Allocate the struct to hold our widget pointers
    AppWidgets* widgets = g_slice_new0(AppWidgets);
    widgets->graph = NULL;
    widgets->path_result.found = false;
    widgets->map_aspect_ratio = 1.0; // Default

    // Title Label
    GtkWidget* title_label = gtk_label_new("Campus Navigator");
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "label { font-size: 1.4em; font-weight: bold; }", -1);
    gtk_style_context_add_provider(gtk_widget_get_style_context(title_label),
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
    gtk_box_append(GTK_BOX(controls_box), title_label);


    // Input Grid
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_widget_set_margin_top(grid, 20);
    gtk_box_append(GTK_BOX(controls_box), grid);

    widgets->start_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(widgets->start_entry, "e.g. 0");
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Start Node:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(widgets->start_entry), 1, 0, 1, 1);

    widgets->end_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(widgets->end_entry, "e.g. 14");
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("End Node:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(widgets->end_entry), 1, 1, 1, 1);

    // Algorithm selection
    GtkWidget* algo_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(algo_box, 20);
    gtk_box_append(GTK_BOX(controls_box), algo_box);
    
    gtk_box_append(GTK_BOX(algo_box), gtk_label_new("Algorithm:"));
    widgets->dijkstra_radio = gtk_check_button_new_with_label("Dijkstra (Slow, Complete)");
    GtkWidget* a_star_radio = gtk_check_button_new_with_label("A* (Fast, Optimized)");
    gtk_check_button_set_group(GTK_CHECK_BUTTON(a_star_radio), GTK_CHECK_BUTTON(widgets->dijkstra_radio));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(a_star_radio), TRUE); // Default to A*
    gtk_box_append(GTK_BOX(algo_box), widgets->dijkstra_radio);
    gtk_box_append(GTK_BOX(algo_box), a_star_radio);

    // Find Path Button
    GtkWidget* find_button = gtk_button_new_with_label("Find Shortest Path");
    gtk_widget_set_margin_top(find_button, 20);
    gtk_box_append(GTK_BOX(controls_box), find_button);

    // Status Label
    widgets->status_label = GTK_LABEL(gtk_label_new("Loading map..."));
    gtk_label_set_wrap(widgets->status_label, TRUE);
    gtk_widget_set_margin_top(widgets->status_label, 15);
    gtk_box_append(GTK_BOX(controls_box), GTK_WIDGET(widgets->status_label));
    
    // Scrollable Node List
    GtkWidget* list_label = gtk_label_new("Available Nodes:");
    gtk_widget_set_halign(list_label, GTK_ALIGN_START);
    gtk_widget_set_margin_top(list_label, 15);
    gtk_box_append(GTK_BOX(controls_box), list_label);

    GtkWidget* list_scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(list_scroll_window, TRUE); // Allow list to fill space
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list_scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    widgets->node_list_label = GTK_LABEL(gtk_label_new(""));
    gtk_label_set_xalign(widgets->node_list_label, 0.0); // Align text left
    gtk_label_set_yalign(widgets->node_list_label, 0.0); // Align text top
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(list_scroll_window), GTK_WIDGET(widgets->node_list_label));
    gtk_box_append(GTK_BOX(controls_box), list_scroll_window);

    // --- 2. Right-side Drawing Area ---
    widgets->drawing_area = GTK_DRAWING_AREA(gtk_drawing_area_new());
    gtk_widget_set_hexpand(GTK_WIDGET(widgets->drawing_area), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(widgets->drawing_area), TRUE);
    gtk_drawing_area_set_draw_func(widgets->drawing_area, on_draw, widgets, NULL);
    gtk_paned_set_end_child(GTK_PANED(paned), GTK_WIDGET(widgets->drawing_area));
    gtk_paned_set_resize_end_child(GTK_PANED(paned), TRUE);
    gtk_paned_set_shrink_end_child(GTK_PANED(paned), FALSE);

    // Set initial pane position
    gtk_paned_set_position(GTK_PANED(paned), 320); // Give controls panel fixed width

    // --- Connect Signals ---
    g_signal_connect(find_button, "clicked", G_CALLBACK(on_find_path_clicked), widgets);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), widgets);
    
    // Initial map load
    load_default_map(widgets);

    gtk_window_present(GTK_WINDOW(window));
}

/**
 * Entry point of the application.
 */
int main(int argc, char** argv) {
    GtkApplication* app = gtk_application_new("com.campus.navigator.visual", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
