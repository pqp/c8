#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef DEBUG
#include <stdarg.h>
#endif

#include "chip8.h"

// 4KB of memory
#define MEM_SIZE 4096

// Programs start at this location in memory.
#define PROGRAM_LOC_OFFSET 512

// Our implementation of a CHIP-8 system.
struct chip8_core {
     uint8_t  mem[MEM_SIZE];
     uint8_t  v[16];

     uint8_t  dt; // delay timer
     uint8_t  st; // sound timer

     uint8_t  sp; // stack pointer
     uint16_t pc; // program counter
     
     uint16_t i;
     uint16_t stack[16];
} core;

/* CHIP8_LoadProgramIntoRAM
   Load program into CHIP-8 core given program character buffer.
*/
void
CHIP8_LoadProgramIntoRAM (const unsigned char* program, const unsigned programSize)
{
     void* dest = memcpy(core.mem+PROGRAM_LOC_OFFSET, program, programSize);

     if (!dest) {
          printf("Failed to copy program into CHIP-8 core memory.\n");
     }
}

static void
printd (char* str, ...)
{
     #ifdef DEBUG
     char final[256];
     va_list args;
     va_start(args, str);
     vsnprintf(final, sizeof(final), str, args);
     va_end(args);
    
     printf("PC 0x%x: %s", core.pc, final);
     #endif
}

void
CHIP8_StartExecution ( void )
{
     uint16_t opcode = 0;

     for (core.pc = PROGRAM_LOC_OFFSET;;) {
          // Fetch instruction.
          opcode = 0;
          opcode |= core.mem[core.pc];
          opcode <<= 8;
          opcode |= core.mem[core.pc + 1];

          if (!opcode) {
               printd("NULL opcode! Halting.\n");
               return;
          }

          switch (opcode)
          {
          case 0x00E0:
               /*
                 00E0 - CLS
                 Clear the display.
               */

               //Platform_ClearDisplay();
               printd("0x00E0: Clear the display.\n");

               break;
               
          case 0x00EE:
               /*
                 00EE - RET
                 Return from a subroutine.
               */

               printd("0x00EE: Return from a subroutine.\n");
                    
               core.pc = core.stack[--core.sp];
               continue;
          }

          // Execute instruction.
          switch (opcode & 0xF000)
          {
          case 0x1000:
               /*
                 1nnn - JP addr
                 Jump to location nnn.
               */
          {
               const int address = (opcode & 0x0FFF);

               printd("0x1000: jump to %3x\n", address);

               core.pc = address;
               break;
          }
          case 0x2000:
               /*
                 2nnn - CALL addr
                 Call subroutine at nnn.
               */

          {
               const int address = (opcode & 0x0FFF);

               printd("0x2000: Call subroutine at 0x%3x\n", address);
               
               core.stack[core.sp++] = core.pc;
               core.pc = address;

               continue;
          }
          case 0x3000:
               /*
                 3xkk - SE Vx, byte
                 Skip next instruction if Vx = kk.
               */

          {
               const int x    = (opcode & 0x0F00) >> 8;
               const int data = (opcode & 0x00FF);

               printd("0x3000: Skip next instruction if V%d(%d) = %d\n", x, core.v[x], data);

               if (core.v[x] == data)
                    core.pc += 2;

               break;
          }
          case 0x4000:
               /*
                 4xkk - SNE Vx, byte
                 Skip next instruction if Vx != kk.
               */
          {
               const int x    = (opcode & 0x0F00) >> 8;
               const int data = (opcode & 0x00FF);

               printd("0x4000: Skip next instruction if V%d(%d) != %d\n", x, core.v[x], data);

               if (core.v[x] != data)
                    core.pc += 2;
               
               break;
          }
          case 0x5000:
               /*
                 5xy0 - SE Vx, Vy
                 Skip next instruction if Vx = Vy.
               */
          {
               const int x = (opcode & 0x0F00) >> 8;
               const int y = (opcode & 0x00F0) >> 4;

               printd("0x5000: Skip next instruction if V%d(%d) = V%d(%d)\n", x, core.v[x], y, core.v[y]);

               if (core.v[x] == core.v[y])
                    core.pc += 2;

               break;
          }
          case 0x6000:
               /*
                 6xkk - LD Vx, byte
                 Set Vx = kk.
               */
          {
               const int x    = (opcode & 0x0F00) >> 8;
               const int data = (opcode & 0x00FF);

               printd("0x6000: Set V%d(%d) = %d\n", x, core.v[x], data);

               core.v[x] = data;

               break;
          }
          case 0x7000:
               /*
                 7xkk - ADD Vx, byte
                 Set Vx = Vx + kk.
               */
          {
               const int x    = (opcode & 0x0F00) >> 8;
               const int data = (opcode & 0x00FF);

               printd("0x7000: Set V%d(%d) = V%d + %d (%d)\n", x, core.v[x], x, data, core.v[x] + data);

               core.v[x] += data;
               break;
          }
          case 0x8000:
          {
               const int x = (opcode & 0x0F00) >> 8;
               const int y = (opcode & 0x00F0) >> 4;

               switch (opcode & 0x000F)
               {
               case 0x0:
                    /*
                      8xy0 - LD Vx, Vy
                      Stores the value of register Vy in register Vx.
                    */

                    printd("0x8xy0: Store the value of V%d(%d) in V%d(%d).\n", y, core.v[y], x, core.v[x]);
                    core.v[x] = core.v[y];
                    break;
               case 0x1:
                    /*
                      8xy1 - OR Vx, Vy
                      Set Vx = Vx OR Vy.
                    */

                    printd("0x8xy1: Set V%d = V%d(%d) | V%d(%d) = %d\n", x, x, core.v[x], y, core.v[y], core.v[x] | core.v[y]);
                    core.v[x] = (core.v[x] | core.v[y]);
                    break;
               case 0x2:
                    /*
                      8xy2 - AND Vx, Vy
                      Set Vx = Vx AND Vy.
                    */
                    
                    printd("0x8xy2: Set V%d = V%d(%d) & V%d(%d) = %d\n", x, x, core.v[x], y, core.v[y], core.v[x] & core.v[y]);
                    core.v[x] = (core.v[x] & core.v[y]);
                    break;
               case 0x3:
                    /*
                      8xy3 - XOR Vx, Vy
                      Set Vx = Vx XOR Vy.
                    */
                    
                    printd("0x8xy3: Set V%d = V%d(%d) ^ V%d(%d) = %d\n", x, x, core.v[x], y, core.v[y], core.v[x] ^ core.v[y]);
                    core.v[x] = (core.v[x] ^ core.v[y]);
                    break;
               case 0x4:
                    /*
                      8xy4 - ADD Vx, Vy
                      Set Vx = Vx + Vy, set VF = carry
                    */
               {
                    const int sum = core.v[x] + core.v[y];

                    printd("0x8xy4: Set V%d(%d) = V%d(%d) + V%d(%d), set VF(%d) = carry\n", x, core.v[x], x, core.v[x], y, core.v[y], core.v[0xF]);

                    core.v[x] = (uint8_t)sum;
                    if (sum > 255)
                         core.v[0xF] = 1;
                    else
                         core.v[0xF] = 0;
                    
                    break;
               }
               case 0x5:
                    /*
                      8xy5 - SUB Vx, Vy
                      Set Vx = Vx - Vy, set VF = NOT borrow.
                    */

                    if (core.v[x] > core.v[y])
                         core.v[0xF] = 1;
                    else
                         core.v[0xF] = 0;
                    
                    core.v[x] -= core.v[y];
                    break;
               case 0x6:
                    break;
               case 0x7:
                    break;
               case 0xE:
                    break;
               default:
                    printd("Unknown 0x8000 variation 0x%3x.\n", opcode & 0x000F);
                    break;
               }
               break;
          }
          case 0x9000:
               /*
                 9xy0 - SNE Vx, Vy
                 Skip next instruction if Vx != Vy.
               */
          {
               const int x = (opcode & 0x0F00) >> 8;
               const int y = (opcode & 0x00F0) >> 4;

               printd("0x9xy0: Skip next instruction if V%d(%d) != V%d(%d).\n", x, core.v[x], y, core.v[y]);

               if (core.v[x] != core.v[y])
                    core.pc += 2;
               
               break;
          }
          case 0xA000:
               /*
                 Annn - LD I, addr
                 Set I = nnn.
               */
          {
               printd("0xAnnn: Set I(0x%3x) = nnn(0x%3x)\n", core.i, opcode & 0x0FFF);
               core.i = (opcode & 0x0FFF);
               break;
          }
          case 0xB000:
               /*
                 Bnnn - JP V0, addr
                 Jump to location nnn + V0.
               */
          {
               core.pc = (opcode & 0x0FFF) + core.v[0];
               continue; // Don't increment the program counter!
          }
          case 0xC000:
               /*
                 Cxkk - RND Vx, byte
                 Set Vx = random byte AND kk.
               */
          {
               printd("0xC000: Set Vx = random byte and kk.\n");
               break;
          }
          case 0xD000:
               /*
                 Dxyn - DRW Vx, Vy, nibble
                 Display n-byte sprite starting at memory location 
                 I at (Vx, Vy), set VF = collision.
               */
          {
               printd("0xD000: Draw sprite\n");
               break;
          }
          case 0xE000:
          {
               const int x = (opcode & 0x0F00) >> 8;

               switch (opcode & 0x00FF)
               {
               case 0x9E:
                    /* 
                       Ex9E - SKP Vx
                       Skip next instruction if key with the value of Vx is pressed.
                    */
                    
                    ;

                    /* if (Platform_KeyPressed(core.v[x]))
                            core.pc += 2;
                    */
                    break;
               case 0xA1:
                    /*
                      ExA1 - SKNP Vx
                      Skip next instruction if key with the value of Vx is not pressed.
                    */
                      
                    ;

                    /* if (!Platform_KeyPressed(core.v[x]))
                            core.pc += 2;
                    */
                    break;
               }
               break;
          }
          case 0xF000:
          {
               const int x = (opcode & 0x0F00) >> 8;

               switch (opcode & 0x00FF)
               {
               case 0x07:
                    /* Set Vx = delay timer value. */
                    printd("0xFx07: Set V%d(%d) = delay timer(%d).\n", x, core.v[x], core.dt);
                    core.v[x] = core.dt;
                    break;
               case 0x0A:
                    printd("0xFx0A: Wait for key press, store value of key (%d) in V%d(%d).\n");
                    // core.v[x] = Platform_KeyPressed();
                    break;
               case 0x15:
                    printd("0xFx15: Set delay timer(%d) = V%d(%d)\n", core.dt, x, core.v[x]);
                    core.dt = core.v[x];
                    break;
               case 0x18:
                    printd("0xFx18: Set sound timer(%d) = V%d(%d)\n", core.st, x, core.v[x]);
                    core.st = core.v[x];
                    break;
               case 0x1E:
                    printd("Set I(%d) = I(%d) + V%d(%d)\n", core.i, core.i, x, core.v[x]);
                    core.i += core.v[x];
                    break;
               default:
                    printd("Unknown 0xF000 variation 0x%2x\n", opcode & 0x00FF);
                    break;
               }
               break;
          }
          default:
               printd("Unknown opcode 0x%1x\n", opcode & 0xF000);
               break;
          }
          core.pc += 2;
     }
}
