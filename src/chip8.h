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

#define VNUM 16

// Our implementation of a CHIP-8 system.
struct chip8_core {
     uint8_t  mem[MEM_SIZE];
     uint8_t  v[VNUM];
     uint8_t  vidmem[SCREEN_HEIGHT][SCREEN_WIDTH];

     uint8_t  dt; // delay timer
     uint8_t  st; // sound timer

     uint8_t  sp; // stack pointer
     uint16_t pc; // program counter
     
     uint16_t i;
     uint16_t stack[16];
};

int   CHIP8_Init (const char* filename);
int   CHIP8_LoadProgram (const char* filename);
char* CHIP8_BuildInstructionBuffer (unsigned long length);
void  CHIP8_Reset (void);
int   CHIP8_FetchAndDecodeOpcode (void);

#endif
