# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Wno-unused-parameter
LDFLAGS = -lm

# Source file and executable
SRC = bwtsearch.c common.c common_bwt.c index_bwt.c
EXEC = bwtsearch

# Object files
OBJ = $(SRC:.c=.o)

# Default target
all: $(EXEC)

# Linking
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

# Compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 2>/dev/null

# Clean
clean:
	rm -f $(OBJ) $(EXEC)
