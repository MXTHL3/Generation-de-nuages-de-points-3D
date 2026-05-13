#include "gl.h"
#include "gl_shaders_utils.h"
#include "cgal_shape.h"
#include <iostream>

static const char* VERTEX_SHADER_SRC = R"glsl(
    #version 330
    layout(location = 0) in vec3 position;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(position, 1.0);
    }
)glsl";

static const char* FRAGMENT_SHADER_SRC = R"glsl(
    #version 330
    uniform vec3 u_color;
    out vec4 outputColor;
    void main() {
        outputColor = vec4(u_color, 1.0);
    }
)glsl";

static constexpr float OFFSET_STEP = 1.5f;


static void wire_markers_to_center(
    std::vector<std::unique_ptr<ModelMarker>>& markers,
    ModelMarker* center,
    size_t first_axis_idx)
{
    for (size_t i = first_axis_idx; i < first_axis_idx + 4 && i < markers.size(); ++i)
        markers[i]->set_center_marker(center);
}

Gl::Gl(std::unique_ptr<Cgal> default_scene) {
    default_scene->build_cube_mesh();
    default_scene->set_offset(0.0f, 0.0f, 0.0f);
    m_scenes.push_back(std::move(default_scene));

    gl_area.set_hexpand(true);
    gl_area.set_vexpand(true);
    gl_area.set_has_depth_buffer(true);
    gl_area.set_has_stencil_buffer(false);
    gl_area.set_auto_render(false);

    gl_area.signal_realize().connect(sigc::mem_fun(*this, &Gl::on_realize));
    gl_area.signal_render().connect(sigc::mem_fun(*this, &Gl::on_render), false);
    gl_area.signal_unrealize().connect(sigc::mem_fun(*this, &Gl::on_unrealize), false);

    m_overlay.add_events(Gdk::BUTTON_PRESS_MASK |
                     Gdk::BUTTON_RELEASE_MASK |
                     Gdk::POINTER_MOTION_MASK);

    m_overlay.signal_button_press_event().connect(sigc::mem_fun(*this, &Gl::on_button_press));
    m_overlay.signal_button_release_event().connect(sigc::mem_fun(*this, &Gl::on_button_release));
    m_overlay.signal_motion_notify_event().connect(sigc::mem_fun(*this, &Gl::on_motion));
    m_overlay.signal_key_press_event().connect(sigc::mem_fun(*this, &Gl::on_key_press));

    gl_area.set_can_focus(true);
    gl_area.grab_focus();
    gl_area.show();

    m_overlay.add(gl_area);

    m_fixed.set_hexpand(true);
    m_fixed.set_vexpand(true);
    m_overlay.add_overlay(m_fixed);
    m_overlay.set_overlay_pass_through(m_fixed, false);
    m_overlay.set_can_focus(true);
    m_overlay.grab_focus();

    m_fixed.signal_draw().connect(sigc::mem_fun(*this, &Gl::on_fixed_draw), false);

    add_center_marker(0);
    ModelMarker* center0 = m_markers.back().get();  
    size_t first_axis = m_markers.size();            

    auto tx_rx = std::make_unique<ModelMarker>(
        &m_fixed, glm::vec3(2.0f, 0.0f, 0.0f), m_zoom, MarkerType::unabledx, glm::vec3(0.0f, 0.0f, 1.0f));
    auto ty_ry = std::make_unique<ModelMarker>(
        &m_fixed, glm::vec3(0.0f, 2.0f, 0.0f), m_zoom, MarkerType::unabledy, glm::vec3(0.0f, 0.0f, 1.0f));
    auto tz_rz = std::make_unique<ModelMarker>(
        &m_fixed, glm::vec3(0.0f, 0.0f, -2.0f), m_zoom, MarkerType::unabledz, glm::vec3(0.0f, 0.0f, 1.0f));
    auto s = std::make_unique<ModelMarker>(
        &m_fixed, glm::vec3(2.0f, 2.0f, -2.0f), m_zoom, MarkerType::unableds, glm::vec3(0.0f, 0.0f, 1.0f));

    m_fixed.put(*tx_rx, 0, 0);
    m_fixed.put(*ty_ry, 0, 0);
    m_fixed.put(*tz_rz, 0, 0);
    m_fixed.put(*s, 0, 0);

    m_markers.push_back(std::move(tx_rx));
    m_markers.push_back(std::move(ty_ry));
    m_markers.push_back(std::move(tz_rz));
    m_markers.push_back(std::move(s));

    wire_markers_to_center(m_markers, center0, first_axis);

    m_fixed.show_all();
    m_overlay.show();
}

void Gl::add_center_marker(int model_index) {
    glm::vec3 pos(0.0f);
    if (model_index > 0) {
        float off = OFFSET_STEP * static_cast<float>(model_index);
        pos = glm::vec3(off, off, off);
    }
    auto marker = std::make_unique<ModelMarker>(
        &m_fixed, pos, m_zoom, MarkerType::center, glm::vec3(0.0f, 1.0f, 0.0f)
    );
    marker->signal_clicked.connect([this](MarkerType type) {
        signal_marker_clicked.emit(type);
    });
    marker->signal_request_focus.connect([this]() { gl_area.grab_focus(); });
    m_fixed.put(*marker, 0, 0);
    m_markers.push_back(std::move(marker));
    (void)model_index;
}

void Gl::add_overlay_widget(Gtk::Widget& w) {
    m_overlay.add_overlay(w);
    m_overlay.set_overlay_pass_through(w, true);
}

void Gl::load_file(const std::string& path) {
    if (gl_area.get_realized())
        gl_area.make_current();

    m_load_count++;
    float off = OFFSET_STEP * static_cast<float>(m_load_count);

    auto scene = std::make_unique<CgalShape>();
    scene->build_mesh_from_file(path);
    scene->set_offset(off, off, off);
    m_scenes.push_back(std::move(scene));

    add_center_marker(m_load_count);
    ModelMarker* centerN = m_markers.back().get();
    size_t first_axis = m_markers.size();

    auto tx_rx = std::make_unique<ModelMarker>(
        &m_fixed, glm::vec3(off + 2.0f, off + 0.0f, off + 0.0f), m_zoom, MarkerType::unabledx, glm::vec3(0.0f, 0.0f, 1.0f));
    auto ty_ry = std::make_unique<ModelMarker>(
        &m_fixed, glm::vec3(off + 0.0f, off + 2.0f, off + 0.0f), m_zoom, MarkerType::unabledy, glm::vec3(0.0f, 0.0f, 1.0f));
    auto tz_rz = std::make_unique<ModelMarker>(
        &m_fixed, glm::vec3(off + 0.0f, off + 0.0f, off - 2.0f), m_zoom, MarkerType::unabledz, glm::vec3(0.0f, 0.0f, 1.0f));
    auto s = std::make_unique<ModelMarker>(
        &m_fixed, glm::vec3(off + 2.0f, off + 2.0f, off - 2.0f), m_zoom, MarkerType::unableds, glm::vec3(0.0f, 0.0f, 1.0f));

    m_fixed.put(*tx_rx, 0, 0);
    m_fixed.put(*ty_ry, 0, 0);
    m_fixed.put(*tz_rz, 0, 0);
    m_fixed.put(*s, 0, 0);

    m_markers.push_back(std::move(tx_rx));
    m_markers.push_back(std::move(ty_ry));
    m_markers.push_back(std::move(tz_rz));
    m_markers.push_back(std::move(s));

    wire_markers_to_center(m_markers, centerN, first_axis);

    m_fixed.show_all();

    rebuild_vertex_data();
    upload_vertex_data();
    update_markers_positions();
    gl_area.queue_render();
}

void Gl::rebuild_vertex_data() {
    vertex_data.clear();
    for (const auto& scene : m_scenes) {
        auto vd = scene->to_vertex_data();
        vertex_data.insert(vertex_data.end(), vd.begin(), vd.end());
    }
}

void Gl::upload_vertex_data() {
    if (vao == 0 || vbo == 0) return;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_data.size() * sizeof(float),
                 vertex_data.data(),
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void Gl::on_realize() {
    try {
        gl_area.make_current();
        gl_area.throw_if_error();
    } catch (const Gdk::GLError& ex) {
        std::cerr << "Erreur GLArea: " << ex.what() << std::endl;
        return;
    }

    GLShadersUtils su;
    shader_program = su.build_program(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
    if (!shader_program) { std::cerr << "Shader program failed!\n"; return; }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    rebuild_vertex_data();
    upload_vertex_data();

    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    update_markers_positions();
}

bool Gl::on_render(const Glib::RefPtr<Gdk::GLContext>&) {
    int w = gl_area.get_allocated_width();
    int h = gl_area.get_allocated_height();
    if (w <= 0 || h <= 0) return true;

    glViewport(0, 0, w, h);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, m_zoom),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    GLint color_loc = glGetUniformLocation(shader_program, "u_color");
    GLsizei vertex_count = static_cast<GLsizei>(vertex_data.size() / 3);

    glBindVertexArray(vao);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
    glUniform3f(color_loc, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    glDisable(GL_POLYGON_OFFSET_FILL);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUniform3f(color_loc, 0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPointSize(1.0f);
    glUniform3f(color_loc, 0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_POINTS, 0, vertex_count);

    glBindVertexArray(0);

    update_markers_positions();

    return true;
}

void Gl::on_unrealize() {
    gl_area.make_current();
    if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao != 0) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (shader_program != 0) { glDeleteProgram(shader_program); shader_program = 0; }
}

bool Gl::on_fixed_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    for (const auto& marker : m_markers) {
        ModelMarker* center = marker->get_center_marker();
        if (!center) continue; 

        MarkerType t = marker->get_marker_type();
        if (t == MarkerType::unabledx || t == MarkerType::unabledy ||
            t == MarkerType::unabledz || t == MarkerType::unableds)
            continue;

        double x0 = marker->screen_x();
        double y0 = marker->screen_y();
        double x1 = center->screen_x();
        double y1 = center->screen_y();

        cr->set_source_rgba(0.0, 0.0, 0.8, 0.55);
        cr->set_line_width(1.2);

        std::vector<double> dashes = {6.0, 4.0};
        cr->set_dash(dashes, 0.0);

        cr->move_to(x0, y0);
        cr->line_to(x1, y1);
        cr->stroke();
    }

    cr->unset_dash();
    return false;   
}

bool Gl::on_button_press(GdkEventButton* e) {
    if (e->button == 1) { m_dragging = true; m_last_x = e->x; m_last_y = e->y; }
    return true;
}

bool Gl::on_button_release(GdkEventButton* e) {
    if (e->button == 1) m_dragging = false;
    return true;
}

bool Gl::on_motion(GdkEventMotion* e) {
    if (!m_dragging) return true;
    double dx = e->x - m_last_x;
    double dy = e->y - m_last_y;
    m_last_x = e->x; m_last_y = e->y;
    angle_y += static_cast<float>(dx) * 0.01f;
    angle_x += static_cast<float>(dy) * 0.01f;
    update_markers_positions();
    gl_area.queue_render();
    return true;
}

bool Gl::on_key_press(GdkEventKey* e) {
    switch (e->keyval) {
        case GDK_KEY_plus:  case GDK_KEY_KP_Add:      m_zoom -= 1.0f; break;
        case GDK_KEY_minus: case GDK_KEY_KP_Subtract:  m_zoom += 1.0f; break;
        default: return false;
    }
    m_zoom = std::clamp(m_zoom, 1.0f, 150.0f);
    update_markers_positions();
    gl_area.queue_render();
    return true;
}

std::pair<double, double> Gl::project_to_2d(const glm::vec3& point_3d) {
    int w = gl_area.get_allocated_width();
    int h = gl_area.get_allocated_height();
    if (w <= 0 || h <= 0) return {0.0, 0.0};

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, m_zoom),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

    glm::vec4 clip = projection * view * model * glm::vec4(point_3d, 1.0f);

    if (std::abs(clip.w) < 1e-6f) return {w / 2.0, h / 2.0};
    glm::vec3 ndc = glm::vec3(clip) / clip.w;

    double x = (ndc.x * 0.5 + 0.5) * w;
    double y = (1.0 - (ndc.y * 0.5 + 0.5)) * h;

    return {x, y};
}

void Gl::update_markers_positions() {
    for (auto& marker : m_markers) {
        auto [x, y] = project_to_2d(marker->world_position());
        marker->set_position(x, y);
    }
    m_fixed.queue_draw();
}

void Gl::focus_gl_area() {
    gl_area.grab_focus();
}