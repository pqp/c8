#ifndef CHIP8_H
#define CHIP8_H

// Non platform-specific CHIP-8 interpretation functionality.

void CHIP8_LoadProgramIntoRAM (const unsigned char* program, const unsigned programSize);
void CHIP8_StartExecution ( void );

#endif
