# Compiler
CC = gcc
CFLAGS = -Wall -Wextra -g -O2

# Directories
SRC_DIR = src
INCLUDE_DIR = include
LIB_DIR = lib
TARGET = exec/main

# Source files
SRC = $(SRC_DIR)/main.c \
      $(SRC_DIR)/socket.c \
      $(SRC_DIR)/checksum.c \
      $(SRC_DIR)/ethernet.c \
      $(SRC_DIR)/ipv4.c \
      $(SRC_DIR)/tcp.c \
      $(SRC_DIR)/tcp_state.c \
      $(LIB_DIR)/mongoose.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) -I $(INCLUDE_DIR) -I $(LIB_DIR) $(SRC)
