clang src/chip8.c src/platform_common.c src/platform_linux.c -o c8 `pkg-config --cflags gtk+-3.0` -std=c99 -Wall `pkg-config --libs gtk+-3.0`
