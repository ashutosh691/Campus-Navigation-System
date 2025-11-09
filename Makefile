# Compiler and Flags
CC = gcc
# CFLAGS: -Wall (all warnings), -Wextra (more warnings), -g (debug symbols), -std=c11 (C standard)
CFLAGS = -Wall -Wextra -g -std=c11

# --- GTK specific flags ---
# Use pkg-config to get the correct flags for GTK 4
GTK_CFLAGS = $(shell pkg-config --cflags gtk4)
GTK_LIBS = $(shell pkg-config --libs gtk4)

# --- Source Files ---
# All .c files needed for the GUI
SRCS = main-gtk.c graph.c algorithms.c utils.c
# Automatically generate object file names (e.g., main-gtk.o, graph.o)
OBJS = $(SRCS:.c=.o)

# --- Target Executable ---
TARGET = navigator-gui

# --- Build Rules ---

# Default target: build the GUI executable
all: $(TARGET)

# Rule to link the GUI executable
# Needs all object files ($^), the GTK libraries, and the math library (-lm)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(GTK_LIBS) -lm

# --- Compilation Rules ---

# Special rule for main-gtk.c: it NEEDS the GTK_CFLAGS
main-gtk.o: main-gtk.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

# General rule for all other .c files
# These don't need the GTK flags for compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove all generated files
clean:
	rm -f *.o $(TARGET)

# Phony target declaration (tells make that 'all' and 'clean' aren't files)
.PHONY: all clean
