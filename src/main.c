#include <stdio.h>
#include <assert.h>

#include "chip8.h"

unsigned char program[512];

static int
LoadProgram (const char* filename, unsigned* programSize)
{
     FILE* programFile;

     printf("Opening %s...\n", filename);

     programFile = fopen(filename, "r");

     if (!programFile) {
          printf("Couldn't open %s.\n", filename);
          return 0;
     }

     fseek(programFile, 0, SEEK_END);
     const long fileSize = ftell(programFile);
     rewind(programFile);

     const int status = fread(&program, fileSize, 1, programFile);

     if (status < 1) {
          printf("Failed to read program into memory.\n");
          return 0;
     }

     *programSize = fileSize;

     return 1;
}

int
main (int argc, char* argv[])
{
     unsigned programSize;

     int result = LoadProgram("pong.ch8", &programSize);

     if (!result) {
          return 0;
     }

     CHIP8_LoadProgramIntoRAM(program, programSize);
     CHIP8_StartExecution();

     return 0;
}
