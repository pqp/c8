#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h> 

// Interface between CHIP-8 interpreter and platform layer.

struct platform_interface_t {
     unsigned keys[16];
     // video memory?
};


struct platform_keyvalue_t {
     int key;
     int value;
};

struct platform_keymap_t {
     struct platform_keyvalue_t *pairs;
     int n;
     int length;
};

struct platform_keymap_t* KeyMap_Init (unsigned length);
int  KeyMap_Get (struct platform_keymap_t* keymap, int key);
void KeyMap_Insert (struct platform_keymap_t* keymap, int key, int value);

extern struct chip8_core core;
extern struct platform_interface_t pi;

extern unsigned programSize;

#endif
