#include "chip8.h"
#include "platform.h"

#include <gtk/gtk.h>

#include <stdlib.h>
#include <pthread.h>

static int interpreting = 1;

static GtkWidget* window;
static GtkWidget* drawingArea;

//chip8_key
void
Platform_KeyPressed (void)
{
}

void
Platform_ClearDisplay (void)
{
}

static void*
InterpreterLoop (void* threadID)
{
     while (interpreting) {
          CHIP8_FetchAndDecodeOpcode();
     }

     pthread_exit(NULL);
}

static gboolean
Draw (GtkWidget* widget, cairo_t* cr, gpointer data)
{
     guint width, height;
     GdkRGBA color;

     // This is sloppy!
     cairo_scale(cr, 4, 4);

     width = gtk_widget_get_allocated_width (widget);
     height = gtk_widget_get_allocated_height (widget);

     for (int y = 0; y < SCREEN_HEIGHT; y++) {
          for (int x = 0; x < SCREEN_WIDTH; x++) {
               // if (core.vidmem[y][x])
               cairo_rectangle(cr, x, y, 1, 1); 
          }
     }
     
     gtk_style_context_get_color (gtk_widget_get_style_context (widget),
                                  0,
                                  &color);
     gdk_cairo_set_source_rgba (cr, &color);

     cairo_fill (cr);

     return FALSE;
}

static void
Destroy (GtkApplication* app, gpointer userData)
{
     interpreting = 0;

     exit(1);
}

static void
KeyRelease (GtkWidget* widget, GdkEvent* event, gpointer userData)
{
     switch (event->key.keyval)
     {
     case GDK_KEY_F1:
     {
          interpreting = !interpreting;

          if (interpreting) {
               pthread_t thread;
               pthread_create(&thread, NULL, InterpreterLoop, 0);
          }

          break;
     }
     case GDK_KEY_W:
          pi.keys[0] = 1;
          break;
     case GDK_KEY_E:
          break;
     case GDK_KEY_R:
          break;
     case GDK_KEY_T:
          break;
     case GDK_KEY_S:
          break;
     case GDK_KEY_D:
          break;
     case GDK_KEY_F:
          break;
     case GDK_KEY_G:
          break;
     case GDK_KEY_X:
          break;
     case GDK_KEY_C:
          break;
     case GDK_KEY_V:
          break;
     case GDK_KEY_B:
          break;
     default:
          break;
     }
}

static void
Redraw (gpointer data)
{
     gtk_widget_queue_resize(drawingArea);

     //ClearKeys();
}

static void
Activate (GtkApplication* app, gpointer userData)
{
     GtkWidget* frame;

     window = gtk_application_window_new(app);
     gtk_window_set_title(GTK_WINDOW(window), "C8");
     gtk_window_set_default_size(GTK_WINDOW(window), SCREEN_WIDTH*4, SCREEN_HEIGHT*4);
     //gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
     g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(Destroy), NULL);
     g_signal_connect(G_OBJECT(window), "key-release-event", G_CALLBACK(KeyRelease), NULL);
     gtk_container_set_border_width(GTK_CONTAINER(window), 8);

     frame = gtk_frame_new(NULL);
     gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
     gtk_container_add(GTK_CONTAINER(window), frame);

     drawingArea = gtk_drawing_area_new();
     gtk_widget_set_size_request(drawingArea, SCREEN_WIDTH, SCREEN_HEIGHT);
     g_signal_connect (G_OBJECT(drawingArea), "draw",
                       G_CALLBACK(Draw), NULL);
     gtk_container_add(GTK_CONTAINER(frame), drawingArea);

     gtk_widget_show_all(window);

     g_timeout_add(17, Redraw, NULL);
}

int
main (int argc, char* argv[])
{
     GtkApplication* app;
     pthread_t iThread;
     int status;
     
     if (!CHIP8_Main(argc, argv)) {
          return 0;
     }

     CHIP8_StartExecution();
     pthread_create(&iThread, NULL, InterpreterLoop, 0);

     // TODO: Create a proper application ID 
     app = gtk_application_new("com.test.c8", G_APPLICATION_FLAGS_NONE);
     g_signal_connect(app, "activate", G_CALLBACK(Activate), NULL);
     status = g_application_run(G_APPLICATION(app), argc, argv);
     g_object_unref(app);

     pthread_exit(NULL);

     return 0;
}
