# Compiler and Flags
CC = gcc
# -fPIC is required for creating shared libraries
CFLAGS = -Wall -Wextra -g -std=c11 -fPIC

# Source Files (Note: main.c and main-gtk.c are EXCLUDED)
# We only want the backend logic.
SRCS = graph.c algorithms.c utils.c
OBJS = $(SRCS:.c=.o)

# Target Shared Library
LIB_NAME = libnavigator.so

# Build Rules
all: $(LIB_NAME)

# Link the object files into a shared library
$(LIB_NAME): $(OBJS)
	$(CC) -shared -o $@ $^ -lm

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(LIB_NAME)