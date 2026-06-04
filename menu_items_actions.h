#pragma once

#include <gtkmm.h>
#include <memory>
#include "gl.h"
#include "handle_file.h"

class MenuItemsActions {
protected:
    Gtk::MenuItem* _sub = nullptr;   
    std::unique_ptr<Gl> m_gl;
    HandleFile m_handle_file;

    virtual Gtk::Window& as_window() = 0;
    virtual void update_markers() = 0;

    void open_3d_model();
    void load_scan();
    void capture_image();
    void launch_scan();
    void display_cloud();
    void generate_dataset();
    void generate_dataset_from_json();
    void launch_recognition();
    void to_fullscreen();
    void exit_app();
};