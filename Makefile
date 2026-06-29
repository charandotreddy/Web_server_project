CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = src
BIN_DIR = bin
TARGET = $(BIN_DIR)/server.out

all:$(TARGET)

$(TARGET): $(SRC_DIR)/server.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/server.c -o $(TARGET)

clean:
	rm -rf $(BIN_DIR)
