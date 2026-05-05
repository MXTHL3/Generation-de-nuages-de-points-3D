#pragma once

#include <gtkmm.h>
#include <vector>
#include <string>

#include "main_gl.h"

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    void add_menu_item(std::string menu_item, std::vector<std::string> sub_menu_items);

private:
    Gtk::Box main_box;
    Gtk::Box draw_box;
    Gtk::MenuBar menubar;
    MainGL main_gl;
    std::vector<std::vector<std::string>> menu_bar;
};