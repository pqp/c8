#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdbool.h> 
#include <GL/glew.h>

#include "chip8.h"
#include "platform.h"

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "lib/glew32.lib")

#define DRAWING_AREA_SCALE 4

#define VIEW_WIDTH  SCREEN_WIDTH*DRAWING_AREA_SCALE
#define VIEW_HEIGHT SCREEN_HEIGHT*DRAWING_AREA_SCALE 

bool running = true;

static HDC dc;

static struct platform_keymap_t* globalKeymap;

static void
InitGL (void)
{
     glViewport(0.0f, 0.0f, VIEW_WIDTH, VIEW_HEIGHT);

     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     glOrtho(0.0, VIEW_WIDTH, VIEW_HEIGHT, 0.0f, 1.0f, -1.0f);

     glMatrixMode(GL_MODELVIEW);
     glLoadIdentity();

     glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

static void
DrawPixel (const float x, const float y)
{
     glBegin(GL_QUADS);

     glColor3f(1.0f, 1.0f, 1.0);

     glVertex2f(x, y);
     glVertex2f(x, y + DRAWING_AREA_SCALE);
     glVertex2f(x + DRAWING_AREA_SCALE, y + DRAWING_AREA_SCALE);
     glVertex2f(x + DRAWING_AREA_SCALE, y);

     glEnd();
}

static void
UpdateDisplay (void)
{
     for (int y = 0; y < VIEW_HEIGHT; y++) {
          for (int x = 0; x < VIEW_WIDTH; x++) {
               if (core.vidmem[y][x]) {
                    DrawPixel(x*DRAWING_AREA_SCALE, y*DRAWING_AREA_SCALE);
               } 
          }
     }
}

LRESULT CALLBACK
WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     switch (message)
     {
     case WM_CREATE:
     {
          PIXELFORMATDESCRIPTOR pfd =
          {
               sizeof(PIXELFORMATDESCRIPTOR),
               1,
               PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
               PFD_TYPE_RGBA,
               32,
               0, 0, 0, 0, 0, 0,
               0,
               0,
               0,
               0, 0, 0, 0,
               24,
               8,
               0,
               PFD_MAIN_PLANE,
               0,
               0, 0, 0
          };

          dc = GetDC(hWnd);

          int pixelFormat = ChoosePixelFormat(dc, &pfd);
          SetPixelFormat(dc, pixelFormat, &pfd);

          HGLRC context = wglCreateContext(dc);
          wglMakeCurrent(dc, context);

          GLenum err = glewInit();
          if (err != GLEW_OK) {
          }

          if (!GLEW_VERSION_2_0) {
               fprintf(stderr, "OpenGL 2.0 isn't available.\n");
          }

          InitGL();
     }
          break;
     case WM_CLOSE:
          running = false;
          break;
     case WM_KEYDOWN:
     {
          int keyPressed = KeyMap_Get(globalKeymap, wParam);

          if (keyPressed != -1)
               pi.keys[keyPressed] = 1;
     }
          break;
     case WM_KEYUP:
     {
          int keyReleased = KeyMap_Get(globalKeymap, wParam);

          if (keyReleased != -1)
               pi.keys[keyReleased] = 0;
     }
     break;
     default:
          return DefWindowProc(hWnd, message, wParam, lParam);
     }

     return 0;
}

static void
InterpreterThread (void* id)
{
     while (1) {
          if (CHIP8_FetchAndDecodeOpcode() < 0) {
          }

          Sleep(1);
     }
}

static void
InitKeymap (void)
{
     globalKeymap = KeyMap_Init(32);

     KeyMap_Insert(globalKeymap, '2', 0x1);
     KeyMap_Insert(globalKeymap, '3', 0x2);
     KeyMap_Insert(globalKeymap, '4', 0x3);
     KeyMap_Insert(globalKeymap, '5', 0xC);
     KeyMap_Insert(globalKeymap, 'W', 0x4);
     KeyMap_Insert(globalKeymap, 'E', 0x5);
     KeyMap_Insert(globalKeymap, 'R', 0x6);
     KeyMap_Insert(globalKeymap, 'T', 0xD);
     KeyMap_Insert(globalKeymap, 'S', 0x7);
     KeyMap_Insert(globalKeymap, 'D', 0x8);
     KeyMap_Insert(globalKeymap, 'F', 0x9);
     KeyMap_Insert(globalKeymap, 'G', 0xE);
     KeyMap_Insert(globalKeymap, 'X', 0xA);
     KeyMap_Insert(globalKeymap, 'C', 0x0);
     KeyMap_Insert(globalKeymap, 'V', 0xB);
     KeyMap_Insert(globalKeymap, 'B', 0xF);
}

int 
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
     MSG msg = {0};
     WNDCLASS wc = {0};
     RECT wr;
     unsigned threadNum;

     wr.left   = 0;
     wr.right  = VIEW_WIDTH;
     wr.top    = 0;
     wr.bottom = VIEW_HEIGHT;

     wc.lpfnWndProc = WndProc;
     wc.hInstance   = hInstance;
     wc.lpszClassName = "C8WinClass";

     if (!RegisterClass(&wc))
          return 0;

     AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

     long style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
     /*
     style ^= WS_THICKFRAME;
     style ^= WS_MAXIMIZEBOX;
     */

     CreateWindow(wc.lpszClassName, "C8", style, 0, 0, (SCREEN_WIDTH * 4)+30, (SCREEN_HEIGHT * 4) + 30,
                  0, 0, hInstance, 0);

     freopen("stdout.txt", "w", stdout);
     freopen("stderr.txt", "w", stderr);

     if (!CHIP8_Init(lpCmdLine)) {
          running = false;
          return;
     }

     CHIP8_Reset();

     InitKeymap();

     _beginthread(InterpreterThread, 0, NULL);

     // TODO: formatting?
     while (running)
     {
          PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
          TranslateMessage(&msg);
          DispatchMessage(&msg);

          glClear(GL_COLOR_BUFFER_BIT);

          UpdateDisplay();

          SwapBuffers(dc);
     }

     return 0;
}
