#ifndef PLATFORM_H
#define PLATFORM_H

// Interface between CHIP-8 interpreter and platform layer.

// Screen dimensions.
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

struct platform_interface_t {
     unsigned keys[15];
};

void Platform_KeyPressed (void);
void Platform_ClearDisplay (void);
void Platform_UpdateDisplay (void);

extern struct platform_interface_t pi;

#endif
