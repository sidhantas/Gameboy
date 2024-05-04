CC=clang
CFLAGS=-Wall -Wextra -Wpedantic -Wconversion -std=c1x  -D_THREAD_SAFE -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -I./include
LDFLAGS= -lncurses -pthread  -L/opt/homebrew/lib -lSDL2
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .
EXE := $(BIN_DIR)/gameboy
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o, $(SRC_FILES))

.PHONY: all run

all: $(EXE)

run: 
	make all && ./$(EXE) -g Tetris.gb

$(EXE): $(OBJ_FILES) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

-include $(OBJ.o=.d)

clean:
	rm -rf $(OBJ_DIR) $(EXE)
