# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11

# --- GTK specific flags ---
GTK_CFLAGS = $(shell pkg-config --cflags gtk4)
GTK_LIBS = $(shell pkg-config --libs gtk4)

# --- Source Files ---

# 1. Common Files (Logic used by BOTH GUI and Terminal)
SRCS_COMMON = graph.c algorithms.c utils.c
OBJS_COMMON = $(SRCS_COMMON:.c=.o)

# 2. GUI Specific Files
SRCS_GUI = main-gtk.c
OBJS_GUI = $(SRCS_GUI:.c=.o)
TARGET_GUI = navigator-gui

# 3. Terminal/CLI Specific Files
# Updated to use main.c
SRCS_CLI = main.c
OBJS_CLI = $(SRCS_CLI:.c=.o)
TARGET_CLI = navigator-cli

# --- Build Rules ---

# Default target: build BOTH executables
all: $(TARGET_GUI) $(TARGET_CLI)

# Shortcut targets to build only one version
gui: $(TARGET_GUI)
cli: $(TARGET_CLI)

# --- Linking Rules ---

# Rule to link the GUI executable
# Links: main-gtk.o + common objects + GTK libs + math
$(TARGET_GUI): $(OBJS_GUI) $(OBJS_COMMON)
	$(CC) $(CFLAGS) -o $@ $^ $(GTK_LIBS) -lm

# Rule to link the Terminal executable
# Links: main.o + common objects + math (No GTK)
$(TARGET_CLI): $(OBJS_CLI) $(OBJS_COMMON)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# --- Compilation Rules ---

# Special rule for main-gtk.c: NEEDS GTK_CFLAGS
main-gtk.o: main-gtk.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

# General rule for all other .c files (main.c, graph.c, etc.)
# These compile with standard flags
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Clean ---

# Removes all object files and BOTH executables
clean:
	rm -f *.o $(TARGET_GUI) $(TARGET_CLI)

.PHONY: all clean gui cli
