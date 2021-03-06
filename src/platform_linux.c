#include "chip8.h"
#include "platform.h"

#include <gtk/gtk.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define WINDOW_WIDTH_OFFSET   30
#define WINDOW_HEIGHT_OFFSET  20
#define WINDOW_BORDER_WIDTH   8

#define DRAWING_AREA_SCALE    4

static bool interpreting = false;
static bool stepping = false;

// TODO: Make this non-static?
static GtkWidget* debugWindow;
static GtkWidget* mainWindow, *drawingArea;

static GtkWidget* vLabels[VNUM], *coreLabels[6];

static GtkWidget* codeView;
static GtkTextMark* mark;
static GtkTextBuffer* buffer;
static GtkTextIter start;

static struct platform_keymap_t* globalKeymap;

// Contents of V registers for debugger
static char regStr[1024];

static int bufferLen = 2048;

// Disassembly of program code
// TODO: Need dynamically sized array here
static char* disassemblyText;

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
          interpreting = false;
          stepping = false;

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

static void
FileDialog (void)
{
     GtkWidget* dialog;
     GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

     // Stop interpreter while loading a new program
     interpreting = false;

     dialog = gtk_file_chooser_dialog_new("Open File",
                                          GTK_WINDOW(mainWindow),
                                          action,
                                          "Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "Open",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);

     gint result = gtk_dialog_run(GTK_DIALOG(dialog));
     if (result == GTK_RESPONSE_ACCEPT) {
          GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
          char* filename;

          filename = gtk_file_chooser_get_filename(chooser);
               
          if (CHIP8_Init(filename) < 0)
               return;

          CHIP8_Reset();
     }

     interpreting = true;

     gtk_widget_destroy(dialog);
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
     default:
     {
          int keyval = event->key.keyval;
          int keyReleased = KeyMap_Get(globalKeymap, keyval);

          if (keyReleased != -1)
               pi.keys[keyReleased] = 0;
     }
          break;
     }
}

static void
KeyPress (GtkWidget* widget, GdkEvent* event, gpointer userData)
{
     int keyval = event->key.keyval;
     int keyPressed = KeyMap_Get(globalKeymap, keyval);

     if (keyPressed != -1)
          pi.keys[keyPressed] = 1;
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
     interpreting = false;
     stepping = false;
}

static void
ToggleDebugger (void)
{
     gboolean visible = gtk_widget_get_visible(debugWindow);
     
     if (visible) {
          gtk_widget_hide(debugWindow);
     } else {
          gtk_widget_show_all(debugWindow);
     }
}

static GtkWidget*
BuildMainWindow (GtkApplication* app)
{
     GtkWidget* mainWindow, *displayFrame, *mainBox;

     GtkWidget* menuBar;
     // File menu and its different functions
     GtkWidget* fileMenu, *file, *fileLoad, *fileReset, *fileQuit;
     // View menu and the debugger toggler
     GtkWidget* viewMenu, *view, *viewDebugger;
     
     // Background color
     GdkColor black = { 0, 0, 0, 1 };
     
     // Create main window (don't make it resizable)
     mainWindow = gtk_application_window_new(app);
     gtk_window_set_title(GTK_WINDOW(mainWindow), "C8");
     gtk_window_set_resizable(GTK_WINDOW(mainWindow), FALSE);
     gtk_container_set_border_width(GTK_CONTAINER(mainWindow), WINDOW_BORDER_WIDTH);

     mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

     displayFrame = gtk_frame_new(NULL);
     gtk_frame_set_shadow_type(GTK_FRAME(displayFrame), GTK_SHADOW_IN);

     menuBar = gtk_menu_bar_new();
     gtk_widget_show(menuBar);

     // Populate menu
     fileMenu = gtk_menu_new();
     file = gtk_menu_item_new_with_label("File");
     fileLoad = gtk_menu_item_new_with_label("Load");
     fileReset = gtk_menu_item_new_with_label("Reset");
     fileQuit = gtk_menu_item_new_with_label("Quit");

     gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), fileMenu);
     gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileLoad);
     gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileReset);
     gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileQuit);
     gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), file);

     viewMenu = gtk_menu_new();
     view = gtk_menu_item_new_with_label("View");
     viewDebugger = gtk_check_menu_item_new_with_label("Debugger");
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(viewDebugger), TRUE);

     gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), viewMenu);
     gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), viewDebugger);
     gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), view);

     drawingArea = gtk_drawing_area_new();
     gtk_widget_set_size_request(GTK_WIDGET(drawingArea), SCREEN_WIDTH*4, SCREEN_HEIGHT*4);
     gtk_widget_set_hexpand(GTK_WIDGET(drawingArea), TRUE);
     gtk_widget_set_vexpand(GTK_WIDGET(drawingArea), TRUE);
     // TODO: Use non-deprecated function (adjust bg color in draw signal?)
     gtk_widget_modify_bg(GTK_WIDGET(drawingArea), GTK_STATE_NORMAL, &black);
     gtk_container_add(GTK_CONTAINER(displayFrame), drawingArea);

     gtk_box_pack_start(GTK_BOX(mainBox), menuBar, FALSE, FALSE, 0);
     gtk_box_pack_end(GTK_BOX(mainBox), displayFrame, TRUE, TRUE, 0);

     gtk_container_add(GTK_CONTAINER(mainWindow), mainBox);

     g_signal_connect(G_OBJECT(fileLoad), "activate", G_CALLBACK(FileDialog), NULL);
     g_signal_connect(G_OBJECT(fileReset), "activate", G_CALLBACK(ResetClicked), NULL);
     g_signal_connect(G_OBJECT(fileQuit), "activate", G_CALLBACK(DestroyApplication), NULL);
     g_signal_connect(G_OBJECT(viewDebugger), "activate", G_CALLBACK(ToggleDebugger), NULL);
     g_signal_connect(G_OBJECT(mainWindow), "destroy", G_CALLBACK(DestroyApplication), NULL);
     g_signal_connect(G_OBJECT(mainWindow), "key-press-event", G_CALLBACK(KeyPress), NULL);
     g_signal_connect(G_OBJECT(mainWindow), "key-release-event", G_CALLBACK(KeyRelease), NULL);
     g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(UpdateDisplay), NULL);

     return mainWindow;
}

static GtkWidget*
BuildDebugWindow (GtkApplication* app)
{
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
     // TODO: replace deprecated modify_font
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

     g_signal_connect(G_OBJECT(debugWindow), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
     g_signal_connect(G_OBJECT(pauseButton), "clicked", G_CALLBACK(PauseClicked), NULL);
     g_signal_connect(G_OBJECT(resetButton), "clicked", G_CALLBACK(ResetClicked), NULL);
     g_signal_connect(G_OBJECT(stepButton), "clicked", G_CALLBACK(StepClicked), NULL);

     return debugWindow;
}

static void
Activate (GtkApplication* app, gpointer userData)
{
     //
     // Build and activate main interpreter window.
     //

     mainWindow = BuildMainWindow(app);
     gtk_widget_show_all(mainWindow);

     //
     // Build and activate debugger window.
     //

     debugWindow = BuildDebugWindow(app);
     gtk_widget_show_all(debugWindow);

     // Write program disassembly to text view
     buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codeView));
     gtk_text_buffer_set_text(buffer, disassemblyText, strlen(disassemblyText));

     g_timeout_add(1,  InterpreterLoop, NULL);
     g_timeout_add(17, Redraw, NULL);
}

static void
InitKeymap (void)
{
     globalKeymap = KeyMap_Init(32);

     KeyMap_Insert(globalKeymap, GDK_KEY_2, 0x1);
     KeyMap_Insert(globalKeymap, GDK_KEY_3, 0x2);
     KeyMap_Insert(globalKeymap, GDK_KEY_4, 0x3);
     KeyMap_Insert(globalKeymap, GDK_KEY_5, 0xC);
     KeyMap_Insert(globalKeymap, GDK_KEY_w, 0x4);
     KeyMap_Insert(globalKeymap, GDK_KEY_e, 0x5);
     KeyMap_Insert(globalKeymap, GDK_KEY_r, 0x6);
     KeyMap_Insert(globalKeymap, GDK_KEY_t, 0xD);
     KeyMap_Insert(globalKeymap, GDK_KEY_s, 0x7);
     KeyMap_Insert(globalKeymap, GDK_KEY_d, 0x8);
     KeyMap_Insert(globalKeymap, GDK_KEY_f, 0x9);
     KeyMap_Insert(globalKeymap, GDK_KEY_g, 0xE);
     KeyMap_Insert(globalKeymap, GDK_KEY_x, 0xA);
     KeyMap_Insert(globalKeymap, GDK_KEY_c, 0x0);
     KeyMap_Insert(globalKeymap, GDK_KEY_v, 0xB);
     KeyMap_Insert(globalKeymap, GDK_KEY_b, 0xF);
}

int
main (int argc, char* argv[])
{
     GtkApplication* app;
     int status;
     
     if (!CHIP8_Init(argv[1])) {
          return 0;
     }

     InitKeymap();

     CHIP8_Reset();

     // Only needed for debugger
     disassemblyText = CHIP8_BuildInstructionBuffer(bufferLen);

     // TODO: Create a proper application ID 
     app = gtk_application_new("com.test.c8", G_APPLICATION_FLAGS_NONE);
     g_signal_connect(app, "activate", G_CALLBACK(Activate), NULL);
     status = g_application_run(G_APPLICATION(app), 0, NULL);
     g_object_unref(app);

     return 0;
}
