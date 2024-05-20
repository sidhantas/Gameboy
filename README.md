# Gameboy

Gameboy Emulator written for MacOS, but likely compiles on Linux as well if dependencies are set up or with minor tweaks

## Dependencies
* SDL2 (make sure `sdl2-config --cflags --libs` returns directories in terminal)
* ncurses (make sure `pkg-config --cflags --libs ncurses` returns directories in terminal)

## Build
* clone repository and run make from top directory
* Can add -O3 flag in `CFLAGS` Makefile variable for runtime optimizations
* Uncomment `CFLAGS += -D SKIP_BOOT` option in Makefile if you don't have a bootrom or would like to skip the initial Nintendo loading screen
* Uncomment `CFLAGS += -D ENABLE_DEBUGGER` option in Makefile to run the debugger

## Run

In terminal run:
```
./gameboy -g [Gameboy Rom]
```
If you get an error "No dmg present" recompile with `CFLAGS += -D SKIP_BOOT` in Makefile or get a DMG bootrom that you "legally" got from your original Nintendo Gameboy (or go [here](https://gbdev.gg8.se/files/roms/bootroms/) and download "dmg_boot.bin") and name it "dmg.bin" and put it in the top level directory

### Saves
* Saves are made in a "save" directory in the top of the project and they have .sav extensions
* Saving is done when quitting the emulator

## Unimplemented features

* Any games that don't use "No MBC", MBC1, or MBC3 will not work. However, it is fairly trivial to add an additional MBC if I find a game that I want to play with an unimplemented MBC (Copy MBC1 and refer to [this page](https://gbdev.io/pandocs/MBCs.html))
* STOP instruction has not been implemented mostly because I can't find a game that uses it anywhere

## Features that could be greatly improved
* Rendering is insanely slow because I'm refetching the tile for each pixel even if the pixel is in the same tile as the previous one and for every pixel I'm searching through all objects that could possibly exist
  * This could be fixed by setting up the entire row of pixels for objects before Mode 3 begins because after mode 2 they can't change position
* OBJ Background priority is currently determined by the pixel color and not the pixel id which looks weird and sometimes makes things visible that shouldn't be visible or vice versa
* Timer circuit is nowhere near perfect
* Setting and resetting flags is slow because I'm making a separate function call for each one (up to 4) per instruction, could probably get it down to one function
* There are functions in place to make the PPU run on it's own thread. However, when doing this I found that it lagged behind too many clock cycles for many games and didn't work and adding locks/condvars for each mode would slow it down too much. Updating the frame in the actual window is done through the main thread, however

## Tested Games
* Donkey Kong World
* Tetris
* Super Mario Land
* Super Mario Land 2
* Pokemon Red
* Legend of Zelda - Link's Awakening
