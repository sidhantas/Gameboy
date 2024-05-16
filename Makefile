CC=clang
CFLAGS=-Wall -g -Wextra -pedantic -Wconversion -std=c1x  -D_THREAD_SAFE -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -I./include 
CFLAGS += -D SKIP_BOOT 
#CFLAGS += -D ENABLE_DEBUGGER
LDFLAGS= -lncurses -pthread  -L/opt/homebrew/lib -lSDL2
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .
EXE := $(BIN_DIR)/gameboy
SRC_FILES := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o, $(SRC_FILES))
DIRECTORIES := $(sort $(dir $(OBJ_FILES)))


.PHONY: all

all: $(EXE)


run: 
	make all && ./$(EXE) -g Tetris.gb

$(EXE): $(OBJ_FILES) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(DIRECTORIES)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $@

$(DIRECTORIES):
	mkdir $@

-include $(OBJ.o=.d)

clean:
	rm -rf $(OBJ_DIR) $(EXE)
