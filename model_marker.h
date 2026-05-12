#pragma once
#include <gtkmm/drawingarea.h>
#include <gtkmm/fixed.h>
#include <glm/glm.hpp>
#include <sigc++/sigc++.h>

enum class MarkerType {
    unabled,
    center,
    tx, ty, tz,
    rx, ry, rz,
    s
};

class ModelMarker : public Gtk::DrawingArea {
public:
    ModelMarker(Gtk::Fixed* parent, const glm::vec3& world_pos, float zoom, MarkerType type,
                const glm::vec3& color);
    void set_position(double x, double y);
    const glm::vec3& world_position() const { return m_world_pos; }

    sigc::signal<void(MarkerType)> signal_clicked;
    sigc::signal<void()> signal_request_focus;

private:
    Gtk::Fixed* m_parent_fixed;
    MarkerType m_type;
    glm::vec3 m_color;
    glm::vec3 m_world_pos;
    bool m_hovered = false;

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_enter_notify_event(GdkEventCrossing* event) override;
    bool on_leave_notify_event(GdkEventCrossing* event) override;
    bool on_button_press_event(GdkEventButton* event) override;
};