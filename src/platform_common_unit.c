#include "platform.h"
#include <stdio.h>

static struct platform_keymap_t* keymap;

//
// platform_keymap_t unit test
//

int
main (int argc, char* argv[])
{
     printf("Initializing symbol table...\n");

     keymap = KeyMap_Init(2);

     printf("Inserting values into symbol table...\n");

     KeyMap_Insert(keymap, 43, 0x4);
     KeyMap_Insert(keymap, 44, 0x5);
     KeyMap_Insert(keymap, 45, 0x6);
     KeyMap_Insert(keymap, 46, 0x7);

     printf("Retrieving values from symbol table...\n");

     int val1 = KeyMap_Get(keymap, 45);
     int val2 = KeyMap_Get(keymap, 43);
     int val3 = KeyMap_Get(keymap, 44);
     int val4 = KeyMap_Get(keymap, 46);

     printf("KeyMap_Get(45): %d\n", val1);
     printf("KeyMap_Get(43): %d\n", val2);
     printf("KeyMap_Get(44): %d\n", val3);
     printf("KeyMap_Get(46): %d\n", val4);

     if (val1 == 0x6 && val2 == 0x4 && val3 == 0x5 && val4 == 0x7)
          printf("Symbol table test passed.\n");
     else
          printf("Symbol table test failed.\n");
}
