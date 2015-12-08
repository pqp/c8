#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

/*
  chip8.h

  Non platform-specific CHIP-8 interpretation functionality.
*/

// 4KB of memory
#define MEM_SIZE 4096

// Programs start at this location in memory.
#define PROGRAM_LOC_OFFSET 512

// Screen dimensions.
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

// Our implementation of a CHIP-8 system.
struct chip8_core {
     uint8_t  mem[MEM_SIZE];
     uint8_t  v[16];
     uint8_t  vidmem[SCREEN_HEIGHT][SCREEN_WIDTH];

     uint8_t  dt; // delay timer
     uint8_t  st; // sound timer

     uint8_t  sp; // stack pointer
     uint16_t pc; // program counter
     
     uint16_t i;
     uint16_t stack[16];
};

int  CHIP8_Main (int argc, char* argv[]);
void CHIP8_LoadProgramIntoRAM (unsigned char* program, const unsigned programSize);
void CHIP8_BuildInstructionTable (char* buffer);
void CHIP8_StartExecution (void);
int  CHIP8_FetchAndDecodeOpcode (void);

#endif
