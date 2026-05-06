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
    void load_file(const std::string& path);

private:
    Gtk::GLArea gl_area;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint shader_program = 0;
    std::vector<float> vertex_data;
    std::unique_ptr<Cgal> scene;
    float angle_x = 0.0f;
    float angle_y = 0.0f;
    bool  m_dragging = false;
    double m_last_x = 0.0;
    double m_last_y = 0.0;
    float m_zoom = 5.0f;

    void on_realize();
    bool on_render(const Glib::RefPtr<Gdk::GLContext>& context);
    void on_unrealize();
    void upload_geometry();
    bool on_button_press(GdkEventButton* e);
    bool on_button_release(GdkEventButton* e);
    bool on_motion(GdkEventMotion* e);
    bool on_key_press(GdkEventKey* e);
};