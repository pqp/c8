#ifndef CHIP8_H
#define CHIP8_H

// Non platform-specific CHIP-8 interpretation functionality.

int  CHIP8_Main (int argc, char* argv[]);
void CHIP8_LoadProgramIntoRAM (const unsigned char* program, const unsigned programSize);
int  CHIP8_VidBit (const int x, const int y);
void CHIP8_StartExecution (void);
void CHIP8_FetchAndDecodeOpcode (void);

#endif
