#pragma once

typedef struct Widget Widget;
typedef struct GUI GUI;

void gui_widget_status_bar_add(GUI* gui, Widget* widget);

void gui_widget_add(GUI* gui, Widget* widget);

void gui_widget_fs_add(GUI* gui, Widget* widget);

void gui_widget_dialog_add(GUI* gui, Widget* widget);
