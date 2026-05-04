#pragma once

#include <gtkmm.h>
#include <vector>
#include <string>

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    void add_menu_item(std::string menu_item, std::vector<std::string> sub_menu_items);

private:
    Gtk::Box main_box;
    Gtk::MenuBar menubar;
    std::vector<std::vector<std::string>> menu_bar;
};