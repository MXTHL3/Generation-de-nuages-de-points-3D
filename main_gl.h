#pragma once

#include <gtkmm.h>
#include <epoxy/gl.h>

class MainGL : public Gtk::GLArea {
    public:
        MainGL();
    
        private:
        void on_realize() override;
        void on_unrealize() override;
        void on_resize(int width, int height) override;
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context) override;
};