#include <gtk/gtk.h>
#include "graph.h"
#include "algorithms.h"

// A struct to hold pointers to our GTK widgets and graph data
// so we can access them easily in callback functions.
typedef struct {
    GtkDropDown* map_selector;
    GtkEntry* start_entry;
    GtkEntry* end_entry;
    GtkLabel* result_label;
    Graph* graph;
} AppWidgets;

// The list of map files for the dropdown menu.
const char* available_maps[] = {
    "dehradun_campus.txt",
    "bhimtal_campus.txt",
    "haldwani_campus.txt"
};

// Callback function for the "Find Path" button
static void on_find_path_clicked(GtkWidget* widget, gpointer data) {
    AppWidgets* app = (AppWidgets*)data;

    // --- 1. Get User Input ---
    const char* start_text = gtk_editable_get_text(GTK_EDITABLE(app->start_entry));
    const char* end_text = gtk_editable_get_text(GTK_EDITABLE(app->end_entry));
    int start_node = atoi(start_text);
    int end_node = atoi(end_text);

    // --- 2. Validate Input and Graph ---
    if (!app->graph) {
        gtk_label_set_text(app->result_label, "Error: No map loaded.");
        return;
    }
    if (!is_valid_node(app->graph, start_node) || !is_valid_node(app->graph, end_node)) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Error: Invalid node ID.\nPlease use 0-%d.", get_node_count(app->graph) - 1);
        gtk_label_set_text(app->result_label, buffer);
        return;
    }

    // --- 3. Run Algorithm ---
    PathResult result = dijkstra_shortest_path(app->graph, start_node, end_node);
    
    // --- 4. Display Result ---
    if (result.found) {
        GString* result_str = g_string_new("");
        g_string_append_printf(result_str, "Path Found! (%.2f km)\n\n", result.total_distance);
        for (int i = 0; i < result.path_length; i++) {
            const Node* node = get_node(app->graph, result.path[i]);
            g_string_append_printf(result_str, "%d. %s\n", i + 1, node->name);
        }
        gtk_label_set_text(app->result_label, result_str->str);
        g_string_free(result_str, TRUE);
    } else {
        gtk_label_set_text(app->result_label, "No path found between these locations.");
    }
    
    free_path_result(&result);
}

// Callback function for when the map selection changes
static void on_map_selected(GObject* object, GParamSpec* pspec, gpointer data) {
    AppWidgets* app = (AppWidgets*)data;
    guint selected_pos = gtk_string_list_get_string(GTK_STRING_LIST(gtk_drop_down_get_model(app->map_selector)), gtk_drop_down_get_selected(app->map_selector)) ? gtk_drop_down_get_selected(app->map_selector) : 0;
    
    // Free old graph if it exists
    if (app->graph) {
        destroy_graph(app->graph);
    }
    
    app->graph = create_graph(200); // Create a new graph
    if (load_road_network(app->graph, available_maps[selected_pos])) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Loaded '%s'.\n%d nodes, %d edges.",
                 available_maps[selected_pos], get_node_count(app->graph), app->graph->num_edges);
        gtk_label_set_text(app->result_label, buffer);
    } else {
        gtk_label_set_text(app->result_label, "Error: Failed to load map file.");
        destroy_graph(app->graph);
        app->graph = NULL;
    }
}

// Main function to build and run the GTK application
static void on_app_activate(GtkApplication* app, gpointer user_data) {
    // Create main window
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Campus Navigation System");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);

    // Create a vertical box to hold all widgets
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Store widgets in our struct
    AppWidgets* widgets = g_slice_new(AppWidgets);
    widgets->graph = NULL;

    // --- UI Elements ---
    // Map Selector Dropdown
    widgets->map_selector = GTK_DROP_DOWN(gtk_drop_down_new_from_strings(available_maps));
    gtk_box_append(GTK_BOX(main_box), GTK_WIDGET(widgets->map_selector));

    // Start Node Entry
    GtkWidget* start_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(main_box), start_box);
    gtk_box_append(GTK_BOX(start_box), gtk_label_new("Start Node:"));
    widgets->start_entry = GTK_ENTRY(gtk_entry_new());
    gtk_box_append(GTK_BOX(start_box), GTK_WIDGET(widgets->start_entry));

    // End Node Entry
    GtkWidget* end_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(main_box), end_box);
    gtk_box_append(GTK_BOX(end_box), gtk_label_new("End Node:  "));
    widgets->end_entry = GTK_ENTRY(gtk_entry_new());
    gtk_box_append(GTK_BOX(end_box), GTK_WIDGET(widgets->end_entry));

    // Find Path Button
    GtkWidget* find_button = gtk_button_new_with_label("Find Shortest Path");
    gtk_widget_set_halign(find_button, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), find_button);

    // Result Label
    widgets->result_label = GTK_LABEL(gtk_label_new("Please select a map to begin."));
    gtk_label_set_wrap(widgets->result_label, TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(widgets->result_label), TRUE);
    gtk_box_append(GTK_BOX(main_box), GTK_WIDGET(widgets->result_label));

    // --- Connect Signals ---
    g_signal_connect(widgets->map_selector, "notify::selected", G_CALLBACK(on_map_selected), widgets);
    g_signal_connect(find_button, "clicked", G_CALLBACK(on_find_path_clicked), widgets);
    
    // Initial map load
    on_map_selected(NULL, NULL, widgets);

    gtk_widget_show(window);
}

int main(int argc, char** argv) {
    GtkApplication* app = gtk_application_new("com.campus.navigator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
