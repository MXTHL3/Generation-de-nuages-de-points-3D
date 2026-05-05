#pragma once

#include <gtkmm.h>
#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>
#include "cgal.h"

class Gl {
public:
    explicit Gl(std::unique_ptr<Cgal> scene);
    void set_scene(std::unique_ptr<Cgal> scene);
    Gtk::GLArea& widget() { return gl_area; }

private:
    Gtk::GLArea gl_area;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint shader_program = 0;
    std::vector<float> vertex_data;
    std::unique_ptr<Cgal> scene;
    float angle_y = 0.0f;
    guint tick_id = 0;

    void on_realize();
    bool on_render(const Glib::RefPtr<Gdk::GLContext>& context);
    void on_unrealize();
    void upload_geometry();
    bool on_tick(const Glib::RefPtr<Gdk::FrameClock>& clock);
};