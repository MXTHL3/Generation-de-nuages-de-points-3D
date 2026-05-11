#pragma once
#include <gtkmm/drawingarea.h>
#include <gtkmm/fixed.h>
#include <glm/glm.hpp>
#include <sigc++/sigc++.h>

class ModelMarker : public Gtk::DrawingArea {
public:
    ModelMarker(Gtk::Fixed* parent, const glm::vec3& center_3d, float zoom);
    void set_position(double x, double y);

    sigc::signal<void()> signal_clicked;
    sigc::signal<void()> signal_request_focus;

private:
    Gtk::Fixed* m_parent_fixed;  
    bool m_hovered = false;

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_enter_notify_event(GdkEventCrossing* event) override;
    bool on_leave_notify_event(GdkEventCrossing* event) override;
    bool on_button_press_event(GdkEventButton* event) override;
};