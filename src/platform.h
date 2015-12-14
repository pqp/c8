#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h> 

// Interface between CHIP-8 interpreter and platform layer.

struct platform_interface_t {
     unsigned keys[16];
     // video memory?
};

extern struct chip8_core core;
extern struct platform_interface_t pi;

extern unsigned programSize;

#endif
