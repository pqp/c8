#include "chip8.h"
#include "platform.h"

#include <gtk/gtk.h>

#include <stdlib.h>
#include <string.h>

#define WINDOW_WIDTH_OFFSET   30
#define WINDOW_HEIGHT_OFFSET  20
#define WINDOW_BORDER_WIDTH   8

#define DRAWING_AREA_SCALE    4

static int interpreting = 0;
static int stepping = 0;

static GtkWidget* mainWindow, *drawingArea;
static GtkWidget* debugWindow;
static GtkWidget* vLabel, *coreLabel;

static char str[1024];

static gboolean
InterpreterLoop (void)
{
     if (!interpreting) {
          gtk_window_set_title(GTK_WINDOW(mainWindow), "C8 (paused)");

          return TRUE;
     }

     if (CHIP8_FetchAndDecodeOpcode() < 0) {
          exit(1);
     }

     snprintf(str, sizeof(str), "V0: %d\tV1: %d\tV2: %d\tV3: %d\t"
              "V4: %d\tV5: %d\tV6: %d\tV7: %d\n\n"
              "V8: %d\tV9: %d\tV10: %d\tV11: %d\t"
              "V12: %d\tV13: %d\tV14: %d\tV15: %d",
              core.v[0], core.v[1], core.v[2], core.v[3],
              core.v[4], core.v[5], core.v[6], core.v[7],
              core.v[8], core.v[9], core.v[10], core.v[11],
              core.v[12], core.v[13], core.v[14], core.v[15]);
     
     gtk_label_set_text(GTK_LABEL(vLabel), str);

     str[0] = '\0';

     snprintf(str, sizeof(str), "DT: %d\n\nST: %d\n\nSP: %d\n\nPC: 0x%3x\n\nI: 0x%3x\n\nStack: 0x%3xd\n\n",
              core.dt, core.st, core.sp, core.pc, core.i, core.stack[core.sp]);

     gtk_label_set_text(GTK_LABEL(coreLabel), str);
     gtk_window_set_title(GTK_WINDOW(mainWindow), "C8");

     if (stepping) {
          interpreting = 0;
          stepping = 0;
     }

     return TRUE;
}

static gboolean
Draw (GtkWidget* widget, cairo_t* cr, gpointer data)
{
     guint width, height;
     GdkRGBA white = { 255, 255, 255, 255 };

     // This is sloppy!
     cairo_scale(cr, DRAWING_AREA_SCALE, DRAWING_AREA_SCALE);

     width = gtk_widget_get_allocated_width(widget);
     height = gtk_widget_get_allocated_height(widget);

     for (int y = 0; y < SCREEN_HEIGHT; y++) {
          for (int x = 0; x < SCREEN_WIDTH; x++) {
               if (core.vidmem[y][x])
                    cairo_rectangle(cr, x, y, 1, 1); 
          }
     }
     
     gdk_cairo_set_source_rgba(cr, &white);

     cairo_fill(cr);

     return FALSE;
}

static void
Destroy (GtkApplication* app, gpointer userData)
{
     interpreting = 0;

     exit(1);
}

// I think this sucks.
static void
KeyRelease (GtkWidget* widget, GdkEvent* event, gpointer userData)
{
     switch (event->key.keyval)
     {
     case GDK_KEY_F1:
     {
          interpreting = !interpreting;
          break;
     }
     case GDK_KEY_F2:
          break;
     case GDK_KEY_2:
          pi.keys[0x1] = 0;
          break;
     case GDK_KEY_3:
          pi.keys[0x2] = 0;
          break;
     case GDK_KEY_4:
          pi.keys[0x3] = 0;
          break;
     case GDK_KEY_5:
          pi.keys[0xC] = 0;
          break;
     case GDK_KEY_w:
          pi.keys[0x4] = 0;
          break;
     case GDK_KEY_e:
          pi.keys[0x5] = 0;
          break;
     case GDK_KEY_r:
          pi.keys[0x6] = 0;
          break;
     case GDK_KEY_t:
          pi.keys[0xD] = 0;
          break;
     case GDK_KEY_s:
          pi.keys[0x7] = 0;
          break;
     case GDK_KEY_d:
          pi.keys[0x8] = 0;
          break;
     case GDK_KEY_f:
          pi.keys[0x9] = 0;
          break;
     case GDK_KEY_g:
          pi.keys[0xE] = 0;
          break;
     case GDK_KEY_x:
          pi.keys[0xA] = 0;
          break;
     case GDK_KEY_c:
          pi.keys[0x0] = 0;
          break;
     case GDK_KEY_v:
          pi.keys[0xB] = 0;
          break;
     case GDK_KEY_b:
          pi.keys[0xF] = 0;
          break;
     default:
          break;
     }
}

static void
KeyPress (GtkWidget* widget, GdkEvent* event, gpointer userData)
{
     switch (event->key.keyval)
     {
     case GDK_KEY_2:
          pi.keys[0x1] = 1;
          break;
     case GDK_KEY_3:
          pi.keys[0x2] = 1;
          break;
     case GDK_KEY_4:
          pi.keys[0x3] = 1;
          break;
     case GDK_KEY_5:
          pi.keys[0xC] = 1;
          break;
     case GDK_KEY_w:
          pi.keys[0x4] = 1;
          break;
     case GDK_KEY_e:
          pi.keys[0x5] = 1;
          break;
     case GDK_KEY_r:
          pi.keys[0x6] = 1;
          break;
     case GDK_KEY_t:
          pi.keys[0xD] = 1;
          break;
     case GDK_KEY_s:
          pi.keys[0x7] = 1;
          break;
     case GDK_KEY_d:
          pi.keys[0x8] = 1;
          break;
     case GDK_KEY_f:
          pi.keys[0x9] = 1;
          break;
     case GDK_KEY_g:
          pi.keys[0xE] = 1;
          break;
     case GDK_KEY_x:
          pi.keys[0xA] = 1;
          break;
     case GDK_KEY_c:
          pi.keys[0x0] = 1;
          break;
     case GDK_KEY_v:
          pi.keys[0xB] = 1;
          break;
     case GDK_KEY_b:
          pi.keys[0xF] = 1;
          break;
     default:
          break;
     }
}

static gboolean
Redraw (gpointer data)
{
     gtk_widget_queue_resize(drawingArea);

     return TRUE;
}

static void
PauseClicked (void)
{
     interpreting = !interpreting;
}

static void
StepClicked (void)
{
     interpreting = 1;
     stepping = 1;
}

static void
Activate (GtkApplication* app, gpointer userData)
{
     GtkWidget* frame;
     GdkColor black = { 0, 0, 0, 0 };

     GtkWidget* scroll, *codeView;
     GtkWidget* debugGrid, *labelGrid;

     GtkWidget* buttonBox;
     GtkWidget* pauseButton, *stepButton;

     mainWindow = gtk_application_window_new(app);
     gtk_window_set_title(GTK_WINDOW(mainWindow), "C8");
     gtk_widget_set_size_request(GTK_WIDGET(mainWindow), (SCREEN_WIDTH*DRAWING_AREA_SCALE)+WINDOW_WIDTH_OFFSET, (SCREEN_HEIGHT*DRAWING_AREA_SCALE)+WINDOW_HEIGHT_OFFSET);
     gtk_window_set_resizable(GTK_WINDOW(mainWindow), FALSE);
     gtk_container_set_border_width(GTK_CONTAINER(mainWindow), WINDOW_BORDER_WIDTH);

     g_signal_connect(G_OBJECT(mainWindow), "destroy", G_CALLBACK(Destroy), NULL);
     g_signal_connect(G_OBJECT(mainWindow), "key-press-event", G_CALLBACK(KeyPress), NULL);
     g_signal_connect(G_OBJECT(mainWindow), "key-release-event", G_CALLBACK(KeyRelease), NULL);

     frame = gtk_frame_new(NULL);
     gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
     gtk_container_add(GTK_CONTAINER(mainWindow), frame);

     drawingArea = gtk_drawing_area_new();
     gtk_widget_set_size_request(GTK_WIDGET(drawingArea), SCREEN_WIDTH, SCREEN_HEIGHT);
     // TODO: Use non-deprecated function (adjust bg color in draw signal?)
     gtk_widget_modify_bg(GTK_WIDGET(drawingArea), GTK_STATE_NORMAL, &black);
     g_signal_connect (G_OBJECT(drawingArea), "draw",
                       G_CALLBACK(Draw), NULL);
     gtk_container_add(GTK_CONTAINER(frame), drawingArea);

     gtk_widget_show_all(mainWindow);

     debugWindow = gtk_application_window_new(app);
     gtk_widget_set_size_request(GTK_WIDGET(debugWindow), 600, 400);
     gtk_window_set_title(GTK_WINDOW(debugWindow), "Debugger");

     // Create window and label grids
     debugGrid = gtk_grid_new();
     labelGrid = gtk_grid_new();
     gtk_container_add(GTK_CONTAINER(debugWindow), debugGrid);

     // Create buttonbox and buttons
     buttonBox = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);

     pauseButton = gtk_button_new_with_label("Pause/Resume");
     stepButton = gtk_button_new_with_label("Step");
     gtk_container_add(GTK_CONTAINER(buttonBox), pauseButton);
     gtk_container_add(GTK_CONTAINER(buttonBox), stepButton);

     g_signal_connect(G_OBJECT(pauseButton), "clicked", G_CALLBACK(PauseClicked), NULL);
     g_signal_connect(G_OBJECT(stepButton), "clicked", G_CALLBACK(StepClicked), NULL);

     vLabel = gtk_label_new("");
     coreLabel = gtk_label_new("");

     // Create scrolled window for textview
     scroll = gtk_scrolled_window_new(NULL, NULL);
     gtk_widget_set_hexpand(scroll, TRUE);
     gtk_widget_set_vexpand(scroll, TRUE);

     // Create code textview
     codeView = gtk_text_view_new();
     gtk_text_view_set_editable(GTK_TEXT_VIEW (codeView), FALSE);
     gtk_container_add(GTK_CONTAINER(scroll), codeView);

     // Attach widgets to debug window grid
     gtk_grid_attach(debugGrid, buttonBox, 0, 0, 1, 1);
     gtk_grid_attach(debugGrid, coreLabel, 0, 1, 1, 1);
     gtk_grid_attach(debugGrid, labelGrid, 1, 0, 1, 1);
     gtk_grid_attach(debugGrid, scroll,    1, 1, 1, 1);

     // Attach labels to label grid
     gtk_grid_attach(labelGrid, vLabel, 0, 0, 1, 1);

     gtk_widget_show_all(debugWindow);

     g_timeout_add(1, InterpreterLoop, NULL);
     g_timeout_add(17, Redraw, NULL);
}

int
main (int argc, char* argv[])
{
     GtkApplication* app;
     int status;
     
     if (!CHIP8_Main(argc, argv)) {
          return 0;
     }

     CHIP8_StartExecution();

     // TODO: Create a proper application ID 
     app = gtk_application_new("com.test.c8", G_APPLICATION_FLAGS_NONE);
     g_signal_connect(app, "activate", G_CALLBACK(Activate), NULL);
     status = g_application_run(G_APPLICATION(app), 0, NULL);
     g_object_unref(app);

     return 0;
}
