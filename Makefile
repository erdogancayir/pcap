# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread -DDEBUG_MODE -g3
LDFLAGS = -lpcap

# Directories
SRC_DIR = source
OBJ_DIR = obj

# Source and object files
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))


BASE_FLAGS = -Wall -Wextra -Iinclude -pthread
DEBUG_FLAGS = -g3 -DDEBUG_MODE
CFLAGS = $(BASE_FLAGS)


# Target executable
TARGET = packet_sniffer

# Default build
all: $(TARGET)

# Link object files into the final binary
$(TARGET): $(OBJS)
	@echo "🔗 Linking target..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each source file into object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "🛠 Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean