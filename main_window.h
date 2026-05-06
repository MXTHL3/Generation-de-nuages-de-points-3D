#pragma once

#include <gtkmm.h>
#include <vector>
#include <string>
#include <memory>
#include "gl.h"
#include "handle_file.h"

class MainWindow : public Gtk::Window {
public:
    explicit MainWindow(std::unique_ptr<Gl> gl);
    void add_menu_item(const std::string& menu_item,
                       const std::vector<std::string>& sub_menu_items);

private:
    Gtk::Box main_box;
    Gtk::MenuBar menubar;
    std::vector<std::vector<std::string>> menu_bar_data;
    std::unique_ptr<Gl> m_gl;
    HandleFile m_handle_file;
    void build_menubar();
};