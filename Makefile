# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11

# --- GTK specific flags ---
GTK_CFLAGS = $(shell pkg-config --cflags gtk4)
GTK_LIBS = $(shell pkg-config --libs gtk4)

# --- Command-line version ---
CLI_SRCS = main.c graph.c algorithms.c utils.c
CLI_OBJS = $(CLI_SRCS:.c=.o)
CLI_TARGET = navigator

# --- GUI version ---
GUI_NON_GTK_SRCS = graph.c algorithms.c utils.c
GUI_NON_GTK_OBJS = $(GUI_NON_GTK_SRCS:.c=.o)
GUI_TARGET = navigator-gui

# --- Build Rules ---

# Default target: build both versions
all: $(CLI_TARGET) $(GUI_TARGET)

# Rule to build the command-line version
$(CLI_TARGET): $(CLI_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Rule to build the GUI version
$(GUI_TARGET): main-gtk.o $(GUI_NON_GTK_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(GTK_LIBS) -lm

# Target specifically for the GUI
gui: $(GUI_TARGET)

# --- Compilation Rules ---

# This is the NEW rule specifically for main-gtk.c
# It adds the GTK flags during compilation.
main-gtk.o: main-gtk.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

# This is the general rule for all other .c files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove all generated files
clean:
	rm -f *.o $(CLI_TARGET) $(GUI_TARGET)

# Phony target declaration
.PHONY: all gui clean
