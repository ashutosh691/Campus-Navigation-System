# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11

# Project Files
SRCS = main.c graph.c algorithms.c utils.c

# Object files are derived from the source files
OBJS = $(SRCS:.c=.o)

# The name of the final executable
TARGET = navigator

# Default target: builds the entire project
all: $(TARGET)

# Linking rule: combines all object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Compilation rule: compiles each .c file into a .o object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: removes all generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony target declaration
.PHONY: all clean