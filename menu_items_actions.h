#pragma once

#include <gtkmm.h>
#include <memory>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include "menu_items_actions.h"
#include "gl.h"
#include "handle_file.h"
#include "pipeline.h"
#include "asset_manager.h"
#include "scene_utils.h"
#include "noise_model.h"
#include "point_cloud_exporter.h"
#include "lidar_scanner.h"      
#include "logger.h"

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
    void reset_scene();
    void scanner_settings();
    void exit_app();
};