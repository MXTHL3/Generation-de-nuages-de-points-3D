#pragma once

#include <gtkmm.h>
#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>
#include "cgal.h"
#include "model_marker.h"

class Gl {
public:
    explicit Gl(std::unique_ptr<Cgal> default_scene);
    Gtk::Widget& widget() { return m_overlay; }
    void load_file(const std::string& path);
    void add_overlay_widget(Gtk::Widget& w);
    std::vector<std::unique_ptr<ModelMarker>>& get_markers() { return m_markers; };
    sigc::signal<void(MarkerType)> signal_marker_clicked;

private:
    Gtk::Overlay m_overlay;
    Gtk::GLArea gl_area;
    Gtk::Fixed m_fixed;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint shader_program = 0;
    std::vector<float> vertex_data;
    std::vector<std::unique_ptr<Cgal>> m_scenes;
    std::vector<std::unique_ptr<ModelMarker>> m_markers;

    int m_load_count = 0;
    float angle_x = 0.0f;
    float angle_y = 0.0f;
    bool m_dragging = false;
    double m_last_x = 0.0;
    double m_last_y = 0.0;
    float m_zoom = 5.0f;

    void rebuild_vertex_data();
    void upload_vertex_data();
    void on_realize();
    bool on_render(const Glib::RefPtr<Gdk::GLContext>& context);
    void on_unrealize();
    bool on_button_press(GdkEventButton* e);
    bool on_button_release(GdkEventButton* e);
    bool on_motion(GdkEventMotion* e);
    bool on_key_press(GdkEventKey* e);
    void update_markers_positions();
    void focus_gl_area();
    std::pair<double, double> project_to_2d(const glm::vec3& point_3d);
    void add_center_marker(int model_index);
    bool on_fixed_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};