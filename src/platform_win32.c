#include <windows.h>
#include "chip8.h"

int
main (int argc, char* argv[])
{
     if (!CHIP8_Init(argv[1])) {
          return 0;
     }

     CHIP8_StartExecution();

     while (1) {
          if (CHIP8_FetchAndDecodeOpcode() < 0) {
          }
     }
     
     return 0;
}
