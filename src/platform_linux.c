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

// TODO: Make this non-static?
static GtkWidget* mainWindow, *drawingArea;

static GtkWidget* vLabels[VNUM], *coreLabels[6];

static GtkWidget* codeView;
static GtkTextMark* mark;
static GtkTextBuffer* buffer;
static GtkTextIter start;

// Contents of V registers for debugger
static char regStr[1024];

// Disassembly of program code
// TODO: Need dynamically sized array here
static char disassemblyText[2048];

static void
UpdateLabels (void)
{
     // Update V register displays
     for (int i = 0; i < VNUM; i++) {
          char str[4];

          snprintf(str, sizeof(str), "%d", core.v[i]);
          gtk_label_set_text(GTK_LABEL(vLabels[i]), str);
     }

     snprintf(regStr, sizeof(regStr), "%d", core.dt);
     gtk_label_set_text(GTK_LABEL(coreLabels[0]), regStr);

     snprintf(regStr, sizeof(regStr), "%d", core.st);
     gtk_label_set_text(GTK_LABEL(coreLabels[1]), regStr);

     snprintf(regStr, sizeof(regStr), "%d", core.sp);
     gtk_label_set_text(GTK_LABEL(coreLabels[2]), regStr);

     snprintf(regStr, sizeof(regStr), "0x%03x", core.pc);
     gtk_label_set_text(GTK_LABEL(coreLabels[3]), regStr);

     snprintf(regStr, sizeof(regStr), "0x%03x", core.i);
     gtk_label_set_text(GTK_LABEL(coreLabels[4]), regStr);

     snprintf(regStr, sizeof(regStr), "0x%03x", core.stack[core.sp]);
     gtk_label_set_text(GTK_LABEL(coreLabels[5]), regStr);
}
     

static gboolean
InterpreterLoop (gpointer data)
{
     if (!interpreting) {
          gtk_window_set_title(GTK_WINDOW(mainWindow), "C8 (paused)");

          return TRUE;
     }

     // Close if interpreter core fails
     if (CHIP8_FetchAndDecodeOpcode() < 0) {
          exit(1);
     }

     UpdateLabels();
    
     gtk_window_set_title(GTK_WINDOW(mainWindow), "C8");

     if (stepping) {
          interpreting = 0;
          stepping = 0;

          /*
          // The buffer is not modified past application initialization,
          // so we should be able to just use a TextIter here?
          gtk_text_buffer_get_start_iter(buffer, &start);
          gtk_text_iter_set_line(&start, 50);//(core.pc - 0x200) / 2);

          mark = gtk_text_buffer_get_mark(buffer, "scroll");
          gtk_text_buffer_move_mark(buffer, mark, &start);
          gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(codeView), mark);
          //gtk_text_view_scroll_to_iter(codeView, &start, 0.0, TRUE, 0.5, 0.5);
          */
     }

     return TRUE;
}

static gboolean
UpdateDisplay (GtkWidget* widget, cairo_t* cr, gpointer data)
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
DestroyApplication (GtkApplication* app, gpointer userData)
{
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
ResetClicked (void)
{
     CHIP8_Reset();
     
     UpdateLabels();
}

static void
StepClicked (void)
{
     interpreting = 1;
     stepping = 1;
}

static GtkWidget*
BuildMainWindow (GtkApplication* app)
{
     GtkWidget* mainWindow, *displayFrame, *mainBox;
     GdkColor black = { 0, 0, 0, 1 };
     
     mainWindow = gtk_application_window_new(app);
     gtk_window_set_title(GTK_WINDOW(mainWindow), "C8");
     gtk_widget_set_size_request(GTK_WIDGET(mainWindow), (SCREEN_WIDTH*DRAWING_AREA_SCALE)+WINDOW_WIDTH_OFFSET, (SCREEN_HEIGHT*DRAWING_AREA_SCALE)+WINDOW_HEIGHT_OFFSET);
     gtk_window_set_resizable(GTK_WINDOW(mainWindow), FALSE);
     gtk_container_set_border_width(GTK_CONTAINER(mainWindow), WINDOW_BORDER_WIDTH);

     mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

     displayFrame = gtk_frame_new(NULL);
     gtk_frame_set_shadow_type(GTK_FRAME(displayFrame), GTK_SHADOW_IN);

     /*menuBar = gtk_menu_bar_new();
     gtk_widget_set_hexpand(menuBar, TRUE);
     gtk_widget_show(menuBar);

     menuItem = gtk_menu_item_new_with_label("Test");
     gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), menuItem);*/

     drawingArea = gtk_drawing_area_new();
     gtk_widget_set_size_request(GTK_WIDGET(drawingArea), SCREEN_WIDTH, SCREEN_HEIGHT);
     // TODO: Use non-deprecated function (adjust bg color in draw signal?)
     gtk_widget_modify_bg(GTK_WIDGET(drawingArea), GTK_STATE_NORMAL, &black);
     gtk_container_add(GTK_CONTAINER(displayFrame), drawingArea);

     //gtk_box_pack_start(GTK_BOX(mainBox), menuBar, TRUE, TRUE, 0);
     gtk_box_pack_end(GTK_BOX(mainBox), displayFrame, TRUE, TRUE, 0);

     gtk_container_add(GTK_CONTAINER(mainWindow), mainBox);

     g_signal_connect(G_OBJECT(mainWindow), "destroy", G_CALLBACK(DestroyApplication), NULL);
     g_signal_connect(G_OBJECT(mainWindow), "key-press-event", G_CALLBACK(KeyPress), NULL);
     g_signal_connect(G_OBJECT(mainWindow), "key-release-event", G_CALLBACK(KeyRelease), NULL);
     g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(UpdateDisplay), NULL);

     // FIXME
     // Kind of pointless to return this since it's in global scope
     return mainWindow;
}

static GtkWidget*
BuildDebugWindow (GtkApplication* app)
{
     GtkWidget* debugWindow;
     GtkWidget* vFrame, *coreFrame;

     GtkWidget* buttonBox, *pauseButton, *resetButton, *stepButton;
     GtkWidget* scroll, *pcListBox; 
     GtkWidget* debugGrid, *vGrid, *coreGrid, *scrollGrid;

     debugWindow = gtk_application_window_new(app);
     gtk_widget_set_size_request(GTK_WIDGET(debugWindow), 600, 400);
     gtk_window_set_title(GTK_WINDOW(debugWindow), "Debugger");
     gtk_window_set_resizable(GTK_WINDOW(debugWindow), FALSE);

     // Create window and label grids
     debugGrid  = gtk_grid_new();
     vGrid      = gtk_grid_new();
     coreGrid   = gtk_grid_new();
     scrollGrid = gtk_grid_new();

     vFrame    = gtk_frame_new(NULL);
     coreFrame = gtk_frame_new(NULL);

     // Create buttonbox and buttons
     buttonBox = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);

     pauseButton = gtk_button_new_with_label("Pause/Resume");
     resetButton = gtk_button_new_with_label("Reset");
     stepButton = gtk_button_new_with_label("Step");

     gtk_container_add(GTK_CONTAINER(buttonBox), pauseButton);
     gtk_container_add(GTK_CONTAINER(buttonBox), resetButton);
     gtk_container_add(GTK_CONTAINER(buttonBox), stepButton);

     // Create scrolled window for textview
     scroll = gtk_scrolled_window_new(NULL, NULL);
     gtk_widget_set_hexpand(scroll, TRUE);
     gtk_widget_set_vexpand(scroll, TRUE);
     gtk_container_add(GTK_CONTAINER(scroll), scrollGrid);

     PangoFontDescription* fontDesc;
     fontDesc = pango_font_description_from_string("Monospace 12");

     // Create code textview
     codeView = gtk_text_view_new();
     gtk_widget_set_hexpand(codeView, TRUE);
     gtk_widget_modify_font(codeView, fontDesc);
     gtk_text_view_set_editable(GTK_TEXT_VIEW(codeView), FALSE);

     // Adjust font size for program counter labels.
     fontDesc = pango_font_description_from_string("Monospace 10");

     mark = gtk_text_mark_new(NULL, FALSE);

     pcListBox = gtk_list_box_new();

     // Make program counter labels and insert them into listbox.
     for (int i = PROGRAM_LOC_OFFSET; i < PROGRAM_LOC_OFFSET+programSize; i += 2) {
          char str[12]; 

          snprintf(str, sizeof(str), "0x%03x  ", i);
          GtkWidget* label = gtk_label_new(str);
          gtk_widget_modify_font(label, fontDesc);

          gtk_list_box_insert(GTK_LIST_BOX(pcListBox), label, -1);
     }

     // Make V register labels
     for (int i = 0; i < VNUM; i++) {
          char v[4];

          snprintf(v, sizeof(v), "V%d", i);
          GtkWidget* label = gtk_label_new(v);
          gtk_label_set_width_chars(GTK_LABEL(label), 10);

          gtk_grid_attach(GTK_GRID(vGrid), label, 0, i, 1, 1);
     }

     // Make V register value labels
     for (int i = 0; i < VNUM; i++) {
          vLabels[i] = gtk_label_new("0");
          gtk_label_set_width_chars(GTK_LABEL(vLabels[i]), 10);

          gtk_grid_attach(GTK_GRID(vGrid), vLabels[i], 1, i, 1, 1);
     }

     // This is ugly.
     GtkWidget* label = gtk_label_new("DT");
     gtk_label_set_width_chars(GTK_LABEL(label), 10);
     gtk_grid_attach(GTK_GRID(coreGrid), label, 0, 0, 1, 1);

     label = gtk_label_new("ST");
     gtk_label_set_width_chars(GTK_LABEL(label), 10);
     gtk_grid_attach(GTK_GRID(coreGrid), label, 1, 0, 1, 1);

     label = gtk_label_new("SP");
     gtk_label_set_width_chars(GTK_LABEL(label), 10);
     gtk_grid_attach(GTK_GRID(coreGrid), label, 2, 0, 1, 1);

     label = gtk_label_new("PC");
     gtk_label_set_width_chars(GTK_LABEL(label), 10);
     gtk_grid_attach(GTK_GRID(coreGrid), label, 3, 0, 1, 1);

     label = gtk_label_new("I");
     gtk_label_set_width_chars(GTK_LABEL(label), 10);
     gtk_grid_attach(GTK_GRID(coreGrid), label, 4, 0, 1, 1);

     label = gtk_label_new("Stack");
     gtk_label_set_width_chars(GTK_LABEL(label), 10);
     gtk_grid_attach(GTK_GRID(coreGrid), label, 5, 0, 1, 1);

     for (int i = 0; i < 6; i++) {
          coreLabels[i] = gtk_label_new("0");

          gtk_grid_attach(GTK_GRID(coreGrid), coreLabels[i], i, 1, 1, 1);
     }
     
     gtk_container_add(GTK_CONTAINER(vFrame), vGrid);
     gtk_container_add(GTK_CONTAINER(coreFrame), coreGrid);

     pango_font_description_free(fontDesc);

     // Attach widgets to debug window grid
     gtk_grid_attach(GTK_GRID(debugGrid), buttonBox, 0, 0, 1, 1);
     gtk_grid_attach(GTK_GRID(debugGrid), coreFrame, 1, 0, 1, 1);
     gtk_grid_attach(GTK_GRID(debugGrid), vFrame,    0, 1, 1, 1);
     gtk_grid_attach(GTK_GRID(debugGrid), scroll,    1, 1, 1, 1);

     // Attach widgets to scrolled window grid
     gtk_grid_attach(GTK_GRID(scrollGrid), pcListBox, 0, 0, 1, 1);
     gtk_grid_attach(GTK_GRID(scrollGrid), codeView,  1, 0, 1, 1); 

     gtk_container_add(GTK_CONTAINER(debugWindow), debugGrid);

     g_signal_connect(G_OBJECT(pauseButton), "clicked", G_CALLBACK(PauseClicked), NULL);
     g_signal_connect(G_OBJECT(resetButton), "clicked", G_CALLBACK(ResetClicked), NULL);
     g_signal_connect(G_OBJECT(stepButton), "clicked", G_CALLBACK(StepClicked), NULL);

     return debugWindow;
}

static void
Activate (GtkApplication* app, gpointer userData)
{
     GtkWidget* menuBar, *menuItem;

     //
     // Build and activate main interpreter window.
     //

     mainWindow = BuildMainWindow(app);
     gtk_widget_show_all(mainWindow);

     //
     // Build and activate debugger window.
     //

     GtkWidget* debugWindow = BuildDebugWindow(app);

     // Write program disassembly to text view
     buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codeView));
     gtk_text_buffer_set_text(buffer, disassemblyText, sizeof(char) * strlen(disassemblyText));

     gtk_widget_show_all(debugWindow);


     g_timeout_add(1,  InterpreterLoop, NULL);
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

     CHIP8_BuildInstructionBuffer(disassemblyText);  

     // TODO: Create a proper application ID 
     app = gtk_application_new("com.test.c8", G_APPLICATION_FLAGS_NONE);
     g_signal_connect(app, "activate", G_CALLBACK(Activate), NULL);
     status = g_application_run(G_APPLICATION(app), 0, NULL);
     g_object_unref(app);

     return 0;
}
