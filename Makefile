CC = gcc
CFLAGS = -Wall -Wextra -g -O2
TARGET = capture_bytes
SRC = capture_bytes.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)
