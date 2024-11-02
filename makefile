# Compiler settings
CC = gcc
CFLAGS = -Wall -g

# Source files
SRCS = memory_manager.c bitmap.c
OBJS = memory_manager.o bitmap.o

# Library name
LIB = memory_manager.a

# Default target
all: $(LIB)

# Compile source files into object files
memory_manager.o: memory_manager.c memory_manager.h common.h
	$(CC) $(CFLAGS) -c memory_manager.c

bitmap.o: bitmap.c common.h
	$(CC) $(CFLAGS) -c bitmap.c

# Create static library
$(LIB): $(OBJS)
	ar rcs $(LIB) $(OBJS)

# Clean target
clean:
	rm -f $(OBJS) $(LIB)

.PHONY: all clean