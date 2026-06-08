#pragma once

#include <gtkmm.h>
#include <memory>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include "gl.h"
#include "handle_file.h"
#include "scene_utils.h"
#include "asset_manager.h"

class MenuItemsActions {
protected:
    Gtk::MenuItem* _sub = nullptr;   
    std::unique_ptr<Gl> m_gl;
    HandleFile m_handle_file;

    virtual Gtk::Window& as_window() = 0;
    virtual void update_markers() = 0;

    void open_3d_model();
    void load_scan();
    void load_json_scene();
    void capture_image();
    void launch_scan();
    void display_cloud();
    void generate_dataset();
    void generate_dataset_from_json();
    void launch_recognition();
    void to_fullscreen();
    void open_docs();
    void exit_app();
};