#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef DEBUG
#include <stdarg.h>
#endif

#include "chip8.h"
#include "platform.h"

// Bitmaps for hex digits
static uint8_t digits[] = {
     0xF0, 0x90, 0x90, 0x90, 0xF0, 
     0x20, 0x60, 0x20, 0x20, 0x70,
     0xF0, 0x10, 0xF0, 0x80, 0xF0,
     0xF0, 0x10, 0xF0, 0x10, 0xF0,
     0x90, 0x90, 0xF0, 0x10, 0x10,
     0xF0, 0x80, 0xF0, 0x10, 0xF0,
     0xF0, 0x80, 0xF0, 0x90, 0xF0,
     0xF0, 0x10, 0x20, 0x40, 0x40,
     0xF0, 0x90, 0xF0, 0x90, 0xF0,
     0xF0, 0x90, 0xF0, 0x10, 0xF0,
     0xF0, 0x90, 0xF0, 0x90, 0x90,
     0xE0, 0x90, 0xE0, 0x90, 0xE0,
     0xF0, 0x80, 0x80, 0x80, 0xF0,
     0xE0, 0x90, 0x90, 0x90, 0xE0,
     0xF0, 0x80, 0xF0, 0x80, 0xF0,
     0xF0, 0x80, 0xF0, 0x80, 0x80
};

struct chip8_core core;
struct platform_interface_t pi;

unsigned programSize;

static unsigned char* program;

static unsigned numOfInst = 0;

static unsigned last = 0;

static int
LoadProgram (const char* filename)
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

     // Should there be a program size limit?

     program = malloc(sizeof(char) * fileSize);
     if (!program) {
          printf("Failed to allocate memory for program storage.\n");
          return 0;
     }

     const int status = fread(program, fileSize, 1, programFile);

     if (status < 1) {
          printf("Failed to read program into memory.\n");
          return 0;
     }

     printf("Loaded %s (%ld bytes)..\n", filename, fileSize);

     programSize = fileSize;

     return 1;
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

static void
AppendToBuffer (char* buffer, const char* str, ...)
{
     char final[256];

     numOfInst++;

     va_list args;
     va_start(args, str);
     vsnprintf(final, sizeof(final), str, args);
     va_end(args);

     // TODO: Verify that vsnprintf works here

     if (!last)
          strncat(final, "\n", 1);

     strncat(buffer, final, strlen(final));

     // TODO: Verify that strncat does not exceed bounds of buffer
}

int
CHIP8_Main (int argc, char* argv[])
{
     if (!argv[1]) {
          printf("%s is not a proper filename.\n", argv[1]);
          return 0;
     }

     int result = LoadProgram(argv[1]);

     if (!result) {
          return 0;
     }

     return 1;
}

/* CHIP8_LoadProgramIntoRAM
   Load hex digit bitmaps and program into CHIP-8 system memory.
*/
void
CHIP8_LoadProgramIntoRAM (unsigned char* program, const unsigned programSize)
{
     void* digitDest = memcpy(core.mem, digits, sizeof(digits));

     // TODO: We need proper error handling here
     if (!digitDest) {
          printf("Failed to copy hex digits into CHIP-8 core memory.\n");
     }

     void* programDest = memcpy(core.mem+PROGRAM_LOC_OFFSET, program, programSize);

     if (!programDest) {
          printf("Failed to copy program into CHIP-8 core memory.\n");
     }

     free(program);
}

void
CHIP8_BuildInstructionBuffer (char* buffer)
{
     for (int i = PROGRAM_LOC_OFFSET; i < PROGRAM_LOC_OFFSET+programSize; i += 2) {
          uint16_t opcode = 0;
          opcode |= core.mem[i];
          opcode <<= 8;
          opcode |= core.mem[i + 1];

          // If there isn't another instruction,
          // don't add a newline char to this line in the buffer.
          if (i + 2 >= (PROGRAM_LOC_OFFSET+programSize))
               last = 1;
          else
               last = 0;

          const unsigned x       = (opcode & 0x0F00) >> 8;
          const unsigned y       = (opcode & 0x00F0) >> 4;
          const unsigned data    = (opcode & 0x00FF);
          const unsigned address = (opcode & 0x0FFF);

          switch (opcode)
          {
          case 0x00E0:
               AppendToBuffer(buffer, "CLS");
               break;
               
          case 0x00EE:
               AppendToBuffer(buffer, "RET");
               break;
          }

          switch (opcode & 0xF000)
          {
          default:
               AppendToBuffer(buffer, "UNKNOWN OPCODE");
               break;
          case 0x1000:
               AppendToBuffer(buffer, "JP 0x%03x", address);
               break;
          case 0x2000:
               AppendToBuffer(buffer, "CALL 0x%03x", address);
               break;
          case 0x3000:
               AppendToBuffer(buffer, "SE V%d, 0x%03x (%d)", x, data, data);
               break;
          case 0x4000:
               AppendToBuffer(buffer, "SNE V%d, 0x%03x (%d)", x, data, data);
               break;
          case 0x5000:
               AppendToBuffer(buffer, "SE V%d, V%d", x, y);
               break;
          case 0x6000:
               AppendToBuffer(buffer, "LD V%d, 0x%03x (%d)", x, data, data);
               break;
          case 0x7000:
               AppendToBuffer(buffer, "ADD V%d, 0x%03x (%d)", x, data, data);
               break;
          case 0x8000:
               switch (opcode & 0x000F)
               {
               case 0x0:
                    AppendToBuffer(buffer, "LD V%d, V%d", x, y);
                    break;
               case 0x1:
                    AppendToBuffer(buffer, "OR V%d, V%d", x, y);
                    break;
               case 0x2:
                    AppendToBuffer(buffer, "AND V%d, V%d", x, y);
                    break;
               case 0x3:
                    AppendToBuffer(buffer, "XOR V%d, V%d", x, y);
                    break;
               case 0x4:
                    AppendToBuffer(buffer, "ADD V%d, V%d", x, y);
                    break;
               case 0x5:
                    AppendToBuffer(buffer, "SUB V%d, V%d", x, y);
                    break;
               case 0x6:
                    AppendToBuffer(buffer, "0x8xy6: NOT IMPLEMENTED");
                    break;
               case 0x7:
                    AppendToBuffer(buffer, "SUBN V%d, V%d", x, y);
                    break;
               case 0xE:
                    AppendToBuffer(buffer, "0x8xyE: NOT DOCUMENTED");
                    break;
               default:
                    AppendToBuffer(buffer, "0x8xy?: NOT DOCUMENTED");
                    break;
               }
               break;
          case 0x9000:
               AppendToBuffer(buffer, "SNE V%d, V%d", x, y);
               break;
          case 0xA000:
               AppendToBuffer(buffer, "LD I, 0x%03x", core.i, address);
               break;
          case 0xB000:
               AppendToBuffer(buffer, "JP V0, 0x%03x", address);
               break;
          case 0xC000:
               AppendToBuffer(buffer, "RND V%d, 0x%03x (%d)", x, data, data);
               break;
          case 0xD000:
               AppendToBuffer(buffer, "DRW V%d, V%d, %d", x, y, opcode & 0x000F);
               break;
          case 0xE000:
               switch (data)
               {
               case 0x9E:
                    AppendToBuffer(buffer, "SKP V%d", x);
                    break;
               case 0xA1:
                    AppendToBuffer(buffer, "SKNP V%d", x);
                    break;
               }
          case 0xF000:
               switch (data)
               {
               case 0x07:
                    AppendToBuffer(buffer, "LD V%d, DT", x);
                    break;
               case 0x0A:
                    AppendToBuffer(buffer, "LD V%d, key", x);
                    break;
               case 0x15:
                    AppendToBuffer(buffer, "LD DT, V%d", x);
                    break;
               case 0x18:
                    AppendToBuffer(buffer, "LD ST, V%x", x);
                    break;
               case 0x1E:
                    AppendToBuffer(buffer, "ADD I, V%d", x);
                    break;
               case 0x29:
                    AppendToBuffer(buffer, "LD F, V%d", x);
                    break;
               case 0x33:
                    AppendToBuffer(buffer, "LD B, V%d", x);
                    break;
               }
          }

     }

     // after we build the table

     printf("Disassembled %d instructions (out of %d total instructions).\n", numOfInst, programSize / 2);
}

void
CHIP8_StartExecution (void)
{
     CHIP8_LoadProgramIntoRAM(program, programSize);

     core.pc = PROGRAM_LOC_OFFSET;

     srand(time(NULL)); // I know rand() sucks and all, but...
}

int
CHIP8_FetchAndDecodeOpcode (void)
{
     // Fetch instruction.
     uint16_t opcode = 0;
     opcode |= core.mem[core.pc];
     opcode <<= 8;
     opcode |= core.mem[core.pc + 1];

     if (!opcode) {
          printd("NULL opcode! Halting.\n");
          return -1;
     }

     const unsigned x       = (opcode & 0x0F00) >> 8;
     const unsigned y       = (opcode & 0x00F0) >> 4;
     const unsigned data    = (opcode & 0x00FF);
     const unsigned address = (opcode & 0x0FFF);

     switch (opcode)
     {
     case 0x00E0:
          /*
            00E0 - CLS
            Clear the display.
          */

          printd("0x00E0: Clear the display.\n");

          for (int y = 0; y < SCREEN_HEIGHT; y++) {
               for (int x = 0; x < SCREEN_WIDTH; x++) {
                    core.vidmem[y][x] = 0;
               }
          }

          break;
               
     case 0x00EE:
          /*
            00EE - RET
            Return from a subroutine.
          */

          printd("0x00EE: Return from a subroutine(return to stack[%d], address 0x%03x)\n", core.sp - 1, core.stack[core.sp-1]);
                    
          if (core.sp - 1 < 0) {
               printf("0x00EE: Stack underflow. Halting!\n");
               return -1;
          }
        
          core.pc = core.stack[--core.sp];
          return 0; // We don't want to increment the program counter,
                    // otherwise we'll skip the instruction pointed to by the stack.
     }

     // Execute instruction.
     switch (opcode & 0xF000)
     {
     case 0x0000:
          break;
     case 0x1000:
          /*
            1nnn - JP addr
            Jump to location nnn.
          */
     {

          printd("0x1000: Jump to 0x%03x\n", address);

          core.pc = address;
          return 0;
     }
     case 0x2000:
          /*
            2nnn - CALL addr
            Call subroutine at nnn.
          */

     {
          printd("0x2000: Call subroutine at 0x%03x (put 0x%03x on stack[%d])\n", address, core.pc, core.sp);
               
          core.stack[core.sp++] = core.pc;
          core.pc = address;

          return 0;
     }
     case 0x3000:
          /*
            3xkk - SE Vx, byte
            Skip next instruction if Vx = kk.
          */

     {

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
          printd("0x7000: Set V%d(%d) = V%d + %d (%d)\n", x, core.v[x], x, data, core.v[x] + data);

          core.v[x] += data;
          break;
     }
     case 0x8000:
     {
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

               printd("0x8xy5: Set V%d(%d) = V%d(%d) - V%d(%d), set VF(%d) = NOT borrow", x, core.v[x], x, core.v[x], y, core.v[y], core.v[0xF]);

               if (core.v[x] > core.v[y])
                    core.v[0xF] = 1;
               else
                    core.v[0xF] = 0;
                    
               core.v[x] -= core.v[y];
               break;
          case 0x6:
               printd("0x8xy6: Not implemented or documented\n");
               return -1;
               break;
          case 0x7:
               /*
                 8xy7 - SUBN Vx, Vy
                 Set Vx = Vy - Vx, set VF = NOT borrow.
               */

               if (core.v[y] > core.v[x])
                    core.v[0xF] = 1;
               else
                    core.v[0xF] = 0;

               core.v[x] = core.v[y] - core.v[x];
               printd("0x8xy7: Not documented\n");
               break;
          case 0xE:
               printd("Set V%d(%d) = V%d(%d) SHL 1 (%d)", x, core.v[x], x, core.v[x], (core.v[x] >> 7) & 1);
               core.v[x] = (core.v[x] >> 7) & 1;
               break;
          default:
               printd("Unknown 0x8000 variation 0x%03x.\n", opcode & 0x000F);
               return -1;
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
          printd("0xAnnn: Set I(0x%03x) = nnn(0x%03x)\n", core.i, opcode & 0x0FFF);
          core.i = (opcode & 0x0FFF);
          break;
     }
     case 0xB000:
          /*
            Bnnn - JP V0, addr
            Jump to location nnn + V0.
          */
     {
          printd("0xBnnn: Not documented\n");
          core.pc = (opcode & 0x0FFF) + core.v[0];
          return 0; // Don't increment the program counter!
     }
     case 0xC000:
          /*
            Cxkk - RND Vx, byte
            Set Vx = random byte AND kk.
          */
     {
          printd("0xC000: Set V%d(%d) = random byte and kk(%d).\n", data);
          break;
     }
     case 0xD000:
          /*
            Dxyn - DRW Vx, Vy, nibble
            Display n-byte sprite starting at memory location 
            I at (Vx, Vy), set VF = collision.
          */
     {
          const unsigned n = (opcode & 0x000F);
          
          printd("0xD000: Draw %d byte sprite at V%d(%d), V%d(%d)\n", n, x, core.v[x], y, core.v[y]);
          
          for (int i = 0; i < n; i++) {
               // byte = row, bit = col

               uint8_t row = core.mem[core.i+i];

               int sx = core.v[x];
               int sy = core.v[y];

               // Iterate through each bit in byte
               int k = 0;
               for (int j = 7; j > 0; j--) {
                    int bit = (row >> j) & 1;
                    if (bit) {

                         // Don't try to write outside of video memory
                         // (eventually sprites should wrap around
                         // to opposite side of display)
                         if (((sy+i < 0) || (sy+i > SCREEN_HEIGHT-1)) ||
                             ((sx+k < 0) || (sx+k > SCREEN_WIDTH-1))) {
                              break;
                         }

                         core.vidmem[sy+i][sx+k] ^= 1;

                         // If pixel is erased, set collision flag
                         if (!core.vidmem[sy+i][sx+k])
                              core.v[0xF] = 1;
                         else
                              core.v[0xF] = 0;
                    }
                    k++;
               }
               
          }
          break;
     }
     case 0xE000:
     {
          switch (opcode & 0x00FF)
          {
          case 0x9E:
               /* 
                  Ex9E - SKP Vx
                  Skip next instruction if key with the value of Vx is pressed.
               */
                    
               printd("0xEx9E: Skip next instruction if key with value of V%d(%d) is pressed\n", x, core.v[x]);
               if (pi.keys[core.v[x]])
                    core.pc += 2;

               break;
          case 0xA1:
               /*
                 ExA1 - SKNP Vx
                 Skip next instruction if key with the value of Vx is not pressed.
               */
                      
               printd("0xExA1: Skip next instruction if key with the value of V%d(%d) is not pressed (%d)\n", x, core.v[x], !pi.keys[core.v[x]]);
               if (!pi.keys[core.v[x]])
                    core.pc += 2;

               break;
          }
          break;
     }
     case 0xF000:
     {
          switch (opcode & 0x00FF)
          {
          case 0x07:
               /*
                 Fx07 - LD Vx, DT
                 Set Vx = delay timer value. 
               */

               printd("0xFx07: Set V%d(%d) = delay timer(%d).\n", x, core.v[x], core.dt);
               core.v[x] = core.dt;
               break;
          case 0x0A:
               /*
                 Fx0A - LD Vx, K
                 Wait for a key press, store the value of the key in Vx. 
               */

               printd("0xFx0A: Wait for key press, store value of key (%d) in V%d(%d).\n");

               for (int i = 0; i < 15; i++) {
                    if (pi.keys[i]) core.v[x] = i;
               }

               break;
          case 0x15:
               /*
                 Fx15 - LD DT, Vx
                 Set delay timer = Vx.
               */

               printd("0xFx15: Set delay timer(%d) = V%d(%d)\n", core.dt, x, core.v[x]);
               core.dt = core.v[x];
               break;
          case 0x18:
               printd("0xFx18: Set sound timer(%d) = V%d(%d)\n", core.st, x, core.v[x]);
               core.st = core.v[x];
               break;
          case 0x1E:
               printd("0xFx1E: Set I(0x%03x) = I(%d) + V%d(%d)\n", core.i, core.i, x, core.v[x]);
               core.i += core.v[x];
               break;
          case 0x29:
               printd("0xFx29: Set I(0x%03x) = %d sprite address (0x%03x)\n", core.i, core.v[x], (core.v[x] * 5));
               core.i = core.v[x] * 5;
               break;
          case 0x33:
               printd("0xFx33: Store BCD representation of V%d(%d) in memory locations I(0x%03x), I+1(0x%03x), I+2(0x%03x)\n", x, core.v[x], core.i, core.i+1, core.i+2);
               core.mem[core.i]   = (core.v[x] % 1000) / 100;
               core.mem[core.i+1] = (core.v[x] % 100) / 10;
               core.mem[core.i+2] = (core.v[x] % 10);
               printd("0xFx33: Wrote %d at 0x%03x\n", (core.v[x] % 1000) / 100, core.i);
               printd("0xFx33: Wrote %d at 0x%03x\n", (core.v[x] % 100) / 10, core.i+1);
               printd("0xFx33: Wrote %d at 0x%03x\n", (core.v[x] % 10), core.i+2);
               break;
          case 0x55:
               printd("0xFx55: Store registers V0 through V%d in memory starting at location 0x%03x\n", x, core.i);
               for (int j = 0; j <= x; j++) {
                    printd("0xFx55: Store V%d(%d) at 0x%03x\n", j, core.v[j], core.i+j);
                    core.mem[core.i+j] = core.v[j];
               }
               break;
          case 0x65:
          {
               printd("0xFx65: Read registers V0 through V%d from memory starting at location 0x%03x\n", x, core.i);
               for (int j = 0; j <= x; j++) {
                    printd("0xFx65: Read %d into V%d\n", core.mem[core.i+j], j);
                    core.v[j] = core.mem[core.i+j];
               }

               break;
          }
          default:
               printd("Unknown 0xF000 variation 0x%2x\n", opcode & 0x00FF);
               break;
          }
          break;
     }
     default:
          printd("Unknown opcode 0x%1x\n", opcode & 0xF000);
          return -1;
          break;
     }

     core.pc += 2;

     if (core.dt > 0) {
          printd("Decrement delay timer (%d)\n", core.dt - 1);
          core.dt--;
     }

     return 0;
}
