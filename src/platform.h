#ifndef PLATFORM_H
#define PLATFORM_H

// Interface between CHIP-8 interpreter and platform layer.

void Platform_KeyPressed (void);
void Platform_ClearDisplay (void);
void Platform_UpdateDisplay (void);

#endif
