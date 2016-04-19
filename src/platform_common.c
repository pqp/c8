#include "platform.h"
#include <stdlib.h>
#include <assert.h>

/* 

   --
   -- platform_keymap_t
   --
   
   A naive, elementary symbol table, implemented with an array. 

   Assumes that there aren't any duplicates. So don't add any!
*/

struct platform_keymap_t*
KeyMap_Init (unsigned length)
{
     struct platform_keymap_t* keymap = malloc(sizeof(struct platform_keymap_t));
     keymap->pairs = malloc(sizeof(int)*length);
     keymap->n = 0;
     keymap->length = length;

     return keymap;
}

// TODO: Implement binary search
int
KeyMap_Get (struct platform_keymap_t* keymap, int key)
{
     for (int i = 0; i < keymap->n; i++) {
          if (keymap->pairs[i].key == key)
               return keymap->pairs[i].value;
     }

     // What should I do here?
     return -1;
}

static struct platform_keymap_t*
resize (struct platform_keymap_t* keymap, unsigned length)
{
     struct platform_keymap_t* newkeymap;
     newkeymap = malloc(sizeof(struct platform_keymap_t));
     newkeymap->pairs = malloc(sizeof(int)* (keymap->length * 2));
     newkeymap->length *= 2;

     for (int i = 0; i < keymap->length; i++) {
          newkeymap->pairs[i] = keymap->pairs[i];
     }

     free(keymap);
     
     return newkeymap;
}

void
KeyMap_Insert (struct platform_keymap_t* keymap, int key, int value)
{
     int i = keymap->n;
     struct platform_keyvalue_t* pair = &keymap->pairs[i];

     if (pair != NULL) {
          keymap->pairs[i].key = key;
          keymap->pairs[i].value = value;
          keymap->n++;
     } else {
          keymap = resize(keymap, 2*keymap->n);
          keymap->pairs[i].key = key;
          keymap->n++;
     }

     return;
}
