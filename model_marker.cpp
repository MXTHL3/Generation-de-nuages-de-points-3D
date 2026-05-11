#include "model_marker.h"
#include <cmath>
#include <iostream>

ModelMarker::ModelMarker(Gtk::Fixed* parent, const glm::vec3& /*center_3d*/, float /*zoom*/)
    : m_parent_fixed(parent)
{
    set_size_request(20, 20);
    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK | Gdk::BUTTON_PRESS_MASK);
    set_can_focus(false);
    set_has_window(true);
    add_events(Gdk::ENTER_NOTIFY_MASK |
        Gdk::LEAVE_NOTIFY_MASK |
        Gdk::BUTTON_PRESS_MASK);
}

void ModelMarker::set_position(double x, double y) {
    if (m_parent_fixed) {
        m_parent_fixed->move(*this,
                             static_cast<int>(x) - get_width()  / 2,
                             static_cast<int>(y) - get_height() / 2);
    }
    queue_draw();
}

bool ModelMarker::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    double center_x = get_width()  / 2.0;
    double center_y = get_height() / 2.0;

    cr->set_source_rgb(0.0, m_hovered ? 0.5 : 1.0, 0.0);
    cr->set_line_width(2.0);
    cr->arc(center_x, center_y, 8.0, 0.0, 2 * M_PI);
    cr->stroke_preserve();
    cr->set_source_rgba(0.0, 1.0, 0.0, 0.3);
    cr->fill();
    return true;
}

bool ModelMarker::on_enter_notify_event(GdkEventCrossing*) {
    m_hovered = true;
    queue_draw();
    return false;
}

bool ModelMarker::on_leave_notify_event(GdkEventCrossing*) {
    m_hovered = false;
    queue_draw();
    return false;
}

bool ModelMarker::on_button_press_event(GdkEventButton* e) {
    if (e->button == 1) {
        std::cout << "marker cliqué" << std::endl;
        signal_clicked.emit();
        signal_request_focus.emit();
        return true;
    }
    return false;
}