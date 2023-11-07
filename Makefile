CC=clang
CFLAGS= -g -Wall -Werror -w -D_XOPEN_SOURCE_EXTENDED 
LDFLAGS= -lncurses -framework Foundation -framework Cocoa -framework ApplicationServices -framework Carbon -pthread 
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .
EXE := $(BIN_DIR)/gameboy
SRC_C := $(wildcard $(SRC_DIR)/*.c)
SRC_M := $(wildcard $(SRC_DIR)/*.m)
OBJ_M := $(SRC_M:$(SRC_DIR)/%.m=$(OBJ_DIR)/%.o)
OBJ_C := $(SRC_C:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all

all: $(EXE)

$(EXE): $(OBJ_C) $(OBJ_M) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.m | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

-include $(OBJ.o=.d)

clean:
	rm -rf $(OBJ_DIR) $(EXE)
