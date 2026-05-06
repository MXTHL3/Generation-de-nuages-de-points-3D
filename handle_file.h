#pragma once

#include <gtkmm.h>
#include <iostream>
#include <vector>
#include <string>

class HandleFile {
public:
    void open_model(Gtk::Window& parent, std::function<void(const std::string&)> on_loaded);
    const std::vector<std::string>& loaded_models() const { return m_loaded_models; }
private:
    std::vector<std::string> m_loaded_models;
};