#include "chip8.h"
#include <gtk/gtk.h>

static cairo_surface_t* surface = NULL;

//chip8_key
void
Platform_KeyPressed (void)
{
}

void
Platform_ClearDisplay (void)
{
}

void
Platform_UpdateDisplay (void)
{
}

static void
button_click (GtkWidget *widget, gpointer data)
{
     g_print("Hello, world!\n");
}

static void
clear_surface (void)
{
     cairo_t* cr;

     cr = cairo_create(surface);

     cairo_set_source_rgb(cr, 0, 0, 0);
     cairo_paint(cr);

     cairo_destroy(cr);
}

static gboolean
configure_event (GtkWidget *widget, GdkEventConfigure* event, gpointer data)
{
     if (surface)
          cairo_surface_destroy(surface);

     surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
                                                 CAIRO_CONTENT_COLOR,
                                                 gtk_widget_get_allocated_width(widget),
                                                 gtk_widget_get_allocated_height(widget));

     clear_surface();

     return TRUE;
}

static gboolean
draw (GtkWidget* widget, cairo_t* cr, gpointer data)
{
     cairo_set_source_surface(cr, surface, 0, 0);
     cairo_paint(cr);

     return FALSE;
}

static void
activate (GtkApplication* app, gpointer user_data)
{
     GtkWidget* window;
     GtkWidget* frame;
     GtkWidget* drawing_area;

     window = gtk_application_window_new(app);
     gtk_window_set_title(GTK_WINDOW(window), "Window");
     gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

     gtk_container_set_border_width(GTK_CONTAINER(window), 8);

     frame = gtk_frame_new(NULL);
     gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
     gtk_container_add(GTK_CONTAINER(window), frame);

     drawing_area = gtk_drawing_area_new();
     // TODO: Define width and height of CHIP-8 screen
     gtk_widget_set_size_request(drawing_area, 64, 32);
     gtk_container_add(GTK_CONTAINER(frame), drawing_area);

     g_signal_connect(drawing_area, "draw", G_CALLBACK(draw), NULL);
     g_signal_connect(drawing_area, "configure-event", G_CALLBACK(configure_event), NULL);

     gtk_widget_show_all(window);
}

int
main (int argc, char* argv[])
{
     GtkApplication* app;
     int status;
     
     if (!CHIP8_Main(argc, argv)) {
          return 0;
     }

     // TODO: Create a proper application ID 
     app = gtk_application_new("com.test.c8", G_APPLICATION_FLAGS_NONE);
     g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
     status = g_application_run(G_APPLICATION(app), argc, argv);
     g_object_unref(app);

     return status;
}
