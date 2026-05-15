#include "model_marker.h"
#include <cmath>
#include <iostream>

ModelMarker::ModelMarker(Gtk::Fixed* parent, const glm::vec3& local_pos, float /*zoom*/,
                        MarkerType type, const glm::vec3& color, int model_index)
    : m_parent_fixed(parent)
    , m_type(type)
    , m_color(color)
    , m_local_pos(local_pos)
    , m_model_index(model_index)

{
    set_size_request(20, 20);
    add_events(Gdk::ENTER_NOTIFY_MASK |
                Gdk::LEAVE_NOTIFY_MASK |
                Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK|
                Gdk::POINTER_MOTION_MASK);
    set_can_focus(false);
    set_has_window(true);
    refresh_visibility();
}

void ModelMarker::set_marker_type(MarkerType mt) {
    m_type = mt;
    refresh_visibility();
    queue_draw();
}

void ModelMarker::refresh_visibility() {
    m_hidden = (m_type == MarkerType::unabledx ||
                   m_type == MarkerType::unabledy ||
                   m_type == MarkerType::unabledz ||
                   m_type == MarkerType::unableds);
    set_sensitive(!m_hidden);
    queue_draw();
}

void ModelMarker::set_position(double x, double y) {
    m_screen_x = x;
    m_screen_y = y;
    if (m_parent_fixed) {
        m_parent_fixed->move(*this,
                             static_cast<int>(x) - get_width()  / 2,
                             static_cast<int>(y) - get_height() / 2);
    }
    queue_draw();
}

bool ModelMarker::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    if (m_hidden) return true;

    double cx = get_width()  / 2.0;
    double cy = get_height() / 2.0;

    float brighten = m_hovered ? 1.4f : 1.0f;
    float r = std::min(1.0f, m_color.r * brighten);
    float g = std::min(1.0f, m_color.g * brighten);
    float b = std::min(1.0f, m_color.b * brighten);

    cr->set_line_width(m_hovered ? 2.5 : 2.0);
    cr->set_source_rgb(r, g, b);
    cr->arc(cx, cy, 8.0, 0.0, 2 * M_PI);
    cr->stroke_preserve();
    cr->set_source_rgba(r, g, b, m_hovered ? 0.55 : 0.3);
    cr->fill();
    return true;
}

bool ModelMarker::on_enter_notify_event(GdkEventCrossing*) {
    if (!m_hidden) { m_hovered = true; queue_draw(); }
    return false;
}
 
bool ModelMarker::on_leave_notify_event(GdkEventCrossing*) {
    m_hovered = false;
    queue_draw();
    return false;
}
 
bool ModelMarker::on_button_press_event(GdkEventButton* e) {
    if (e->button == 1) {
        if (m_type == MarkerType::center) {
            signal_clicked.emit(m_type);
        } else if (!m_hidden) {
            m_dragging    = true;
            m_drag_last_x = e->x_root;
            m_drag_last_y = e->y_root;
        }
        signal_request_focus.emit();
        return true;
    }
    return false;
}
 
bool ModelMarker::on_button_release_event(GdkEventButton* e) {
    if (e->button == 1) { m_dragging = false; return true; }
    return false;
}
 
bool ModelMarker::on_motion_notify_event(GdkEventMotion* e) {
    if (!m_dragging) return false;
    double dx = e->x_root - m_drag_last_x;
    double dy = e->y_root - m_drag_last_y;
    m_drag_last_x = e->x_root;
    m_drag_last_y = e->y_root;
    signal_dragged.emit(m_type, m_model_index, dx, dy);
    return true;
}