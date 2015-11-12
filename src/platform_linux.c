#include "chip8.h"
#include "platform.h"

#include <gtk/gtk.h>

#include <stdlib.h>

#define WINDOW_WIDTH_OFFSET   30
#define WINDOW_HEIGHT_OFFSET  20
#define WINDOW_BORDER_WIDTH   8

#define DRAWING_AREA_SCALE    4

static int interpreting = 1;

static GtkWidget* window;
static GtkWidget* drawingArea;

static gboolean
InterpreterLoop (void)
{
     if (interpreting) {
          if (CHIP8_FetchAndDecodeOpcode() < 0) {
               exit(1);
          }
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
Activate (GtkApplication* app, gpointer userData)
{
     GtkWidget* frame;
     GtkWidget* statusBar;
     GtkWidget* menuBar;

     GdkColor black = { 0, 0, 0, 0 };

     window = gtk_application_window_new(app);
     gtk_window_set_title(GTK_WINDOW(window), "C8");
     gtk_widget_set_size_request(GTK_WIDGET(window), (SCREEN_WIDTH*DRAWING_AREA_SCALE)+WINDOW_WIDTH_OFFSET, (SCREEN_HEIGHT*DRAWING_AREA_SCALE)+WINDOW_HEIGHT_OFFSET);
     gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
     gtk_container_set_border_width(GTK_CONTAINER(window), WINDOW_BORDER_WIDTH);

     g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(Destroy), NULL);
     g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(KeyPress), NULL);
     g_signal_connect(G_OBJECT(window), "key-release-event", G_CALLBACK(KeyRelease), NULL);

     frame = gtk_frame_new(NULL);
     gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
     gtk_container_add(GTK_CONTAINER(window), frame);

     drawingArea = gtk_drawing_area_new();
     gtk_widget_set_size_request(GTK_WIDGET(drawingArea), SCREEN_WIDTH, SCREEN_HEIGHT);
     // TODO: Use non-deprecated function (adjust bg color in draw signal?)
     gtk_widget_modify_bg(GTK_WIDGET(drawingArea), GTK_STATE_NORMAL, &black);
     g_signal_connect (G_OBJECT(drawingArea), "draw",
                       G_CALLBACK(Draw), NULL);
     gtk_container_add(GTK_CONTAINER(frame), drawingArea);

     statusBar = gtk_statusbar_new();
     //gtk_widget_set_size_request(GTK_WIDGET(statusBar), 100, 20);
     gtk_container_add(GTK_CONTAINER(window), statusBar);

     menuBar = gtk_menu_bar_new();
     gtk_container_add(GTK_CONTAINER(window), menuBar);

     gtk_widget_show_all(window);

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
