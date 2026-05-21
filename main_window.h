#pragma once

#include <gtkmm.h>
#include <vector>
#include <string>
#include <memory>
#include "gl.h"
#include "handle_file.h"
#include "model_marker.h"

class MainWindow : public Gtk::Window {
public:
    explicit MainWindow(std::unique_ptr<Gl> gl);
    void add_menu_item(const std::string& menu_item,
                       const std::vector<std::string>& sub_menu_items);

private:
    Gtk::Box main_box;
    Gtk::MenuBar menubar;
    Gtk::Label m_status_label;
    Gtk::ComboBoxText m_lidar_combo; 
    std::vector<std::vector<std::string>> menu_bar_data;
    std::unique_ptr<Gl> m_gl;
    HandleFile m_handle_file;

    const std::vector<std::string> transformation_mode = {
        "", "Translation mode", "Rotation mode", "Scale mode"
    };
    int m_tm_id = 0;   

    void update_markers();
    void update_status_label();
    void build_menubar();
};