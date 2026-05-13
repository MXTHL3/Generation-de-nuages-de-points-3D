#pragma once
#include <gtkmm/drawingarea.h>
#include <gtkmm/fixed.h>
#include <glm/glm.hpp>
#include <sigc++/sigc++.h>

enum class MarkerType {
    unabledx, unabledy, unabledz, unableds, center, tx, ty, tz, rx, ry, rz, s
};

class ModelMarker : public Gtk::DrawingArea {
public:
    ModelMarker(Gtk::Fixed* parent, const glm::vec3& world_pos, float zoom, MarkerType type,
                const glm::vec3& color);
    void set_position(double x, double y);
    MarkerType get_marker_type() { return m_type; };
    void set_marker_type(MarkerType mt);
    const glm::vec3& world_position() const { return m_world_pos; }

    void set_center_marker(ModelMarker* center) { m_center_marker = center; }
    ModelMarker* get_center_marker() const { return m_center_marker; }
 
    double screen_x() const { return m_screen_x; }
    double screen_y() const { return m_screen_y; }

    sigc::signal<void(MarkerType)> signal_clicked;
    sigc::signal<void()> signal_request_focus;

private:
    Gtk::Fixed* m_parent_fixed;
    ModelMarker* m_center_marker = nullptr;
    MarkerType m_type;
    glm::vec3 m_color;
    glm::vec3 m_world_pos;
    bool m_hovered = false;
    double m_screen_x = 0.0;
    double m_screen_y = 0.0;

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_enter_notify_event(GdkEventCrossing* event) override;
    bool on_leave_notify_event(GdkEventCrossing* event) override;
    bool on_button_press_event(GdkEventButton* event) override;
    void refresh_visibility();
};