#include "gl.h"
#include "gl_shaders_utils.h"
#include "cgal_shape.h"
#include "noise_model.h"
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

static constexpr float OFFSET_STEP      = 1.5f;
static constexpr float DRAG_SENSITIVITY = 0.02f;

static void wire_markers_to_center(
    std::vector<std::unique_ptr<ModelMarker>>& markers,
    ModelMarker* center,
    size_t first_axis_idx)
{
    for (size_t i = first_axis_idx; i < first_axis_idx + 4 && i < markers.size(); ++i)
        markers[i]->set_center_marker(center);
}

void Gl::connect_marker_signals(ModelMarker* marker) {
    marker->signal_clicked.connect([this](MarkerType type) {
        signal_marker_clicked.emit(type);
    });
    marker->signal_request_focus.connect([this]() { gl_area.grab_focus(); });
    marker->signal_dragged.connect(sigc::mem_fun(*this, &Gl::on_marker_dragged));
}

void Gl::on_marker_dragged(MarkerType type, int idx, double dx, double dy) {
    if (idx < 0 || idx >= static_cast<int>(m_transforms.size())) return;
    ModelTransform& t = m_transforms[idx];
    switch (type) {
        case MarkerType::tx: t.pos_x += static_cast<float>(dx) * DRAG_SENSITIVITY; break;
        case MarkerType::ty: t.pos_y -= static_cast<float>(dy) * DRAG_SENSITIVITY; break;
        case MarkerType::tz: t.pos_z -= static_cast<float>(dx) * DRAG_SENSITIVITY; break;
        case MarkerType::rx: t.angle_x += static_cast<float>(dy) * DRAG_SENSITIVITY; break;
        case MarkerType::ry: t.angle_y += static_cast<float>(dx) * DRAG_SENSITIVITY; break;
        case MarkerType::rz: t.angle_z -= static_cast<float>(dx) * DRAG_SENSITIVITY; break;
        case MarkerType::s:
            t.scale += static_cast<float>(dx - dy) * DRAG_SENSITIVITY * 0.5f;
            if (t.scale < 0.01f) t.scale = 0.01f;
            break;
        default: return;
    }
    update_markers_positions();
    gl_area.queue_render();
}

glm::mat4 Gl::make_model_matrix(const ModelTransform& tr) const {
    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(tr.pos_x, tr.pos_y, tr.pos_z));
    m = glm::rotate(m, tr.angle_x, glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::rotate(m, tr.angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::rotate(m, tr.angle_z, glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::scale(m, glm::vec3(tr.scale));
    return m;
}

Gl::Gl(std::unique_ptr<Cgal> default_scene) {
    default_scene->build_cube_mesh();
    m_scenes.push_back(std::move(default_scene));
    m_transforms.push_back({});

    gl_area.set_hexpand(true);
    gl_area.set_vexpand(true);
    gl_area.set_has_depth_buffer(true);
    gl_area.set_has_stencil_buffer(false);
    gl_area.set_auto_render(false);

    gl_area.signal_realize().connect(sigc::mem_fun(*this, &Gl::on_realize));
    gl_area.signal_render().connect(sigc::mem_fun(*this, &Gl::on_render), false);
    gl_area.signal_unrealize().connect(sigc::mem_fun(*this, &Gl::on_unrealize), false);

    gl_area.add_events(Gdk::BUTTON_PRESS_MASK |
                       Gdk::BUTTON_RELEASE_MASK |
                       Gdk::POINTER_MOTION_MASK);
    gl_area.signal_button_press_event().connect(sigc::mem_fun(*this, &Gl::on_button_press));
    gl_area.signal_button_release_event().connect(sigc::mem_fun(*this, &Gl::on_button_release));
    gl_area.signal_motion_notify_event().connect(sigc::mem_fun(*this, &Gl::on_motion));
    m_overlay.signal_key_press_event().connect(sigc::mem_fun(*this, &Gl::on_key_press));

    gl_area.set_can_focus(true);
    gl_area.grab_focus();
    gl_area.show();

    m_overlay.add(gl_area);

    m_fixed.set_hexpand(true);
    m_fixed.set_vexpand(true);
    m_overlay.add_overlay(m_fixed);
    m_overlay.set_overlay_pass_through(m_fixed, true);
    m_overlay.set_can_focus(true);
    m_overlay.grab_focus();

    m_fixed.signal_draw().connect(sigc::mem_fun(*this, &Gl::on_fixed_draw), false);

    size_t first_axis = m_markers.size();

    auto tx_rx = std::make_unique<ModelMarker>(&m_fixed, glm::vec3( 1.25f,  0.0f,  0.0f), m_zoom, MarkerType::unabledx, glm::vec3(0.0f, 0.0f, 1.0f), 0);
    auto ty_ry = std::make_unique<ModelMarker>(&m_fixed, glm::vec3( 0.0f,  1.25f,  0.0f), m_zoom, MarkerType::unabledy, glm::vec3(0.0f, 0.0f, 1.0f), 0);
    auto tz_rz = std::make_unique<ModelMarker>(&m_fixed, glm::vec3( 0.0f,  0.0f, -1.25f), m_zoom, MarkerType::unabledz, glm::vec3(0.0f, 0.0f, 1.0f), 0);
    auto s = std::make_unique<ModelMarker>(&m_fixed, glm::vec3( 1.25f,  1.25f, -1.25f), m_zoom, MarkerType::unableds, glm::vec3(0.0f, 0.0f, 1.0f), 0);

    connect_marker_signals(tx_rx.get());
    connect_marker_signals(ty_ry.get());
    connect_marker_signals(tz_rz.get());
    connect_marker_signals(s.get());

    m_fixed.put(*tx_rx, 0, 0);
    m_fixed.put(*ty_ry, 0, 0);
    m_fixed.put(*tz_rz, 0, 0);
    m_fixed.put(*s, 0, 0);

    m_markers.push_back(std::move(tx_rx));
    m_markers.push_back(std::move(ty_ry));
    m_markers.push_back(std::move(tz_rz));
    m_markers.push_back(std::move(s));

    add_center_marker(0);
    ModelMarker* center0 = m_markers.back().get();

    wire_markers_to_center(m_markers, center0, first_axis);

    m_fixed.show_all();
    m_overlay.show();
}

void Gl::add_center_marker(int model_index) {
    auto marker = std::make_unique<ModelMarker>(
        &m_fixed, glm::vec3(0.0f), m_zoom,
        MarkerType::center, glm::vec3(0.0f, 1.0f, 0.0f), model_index);
    connect_marker_signals(marker.get());
    m_fixed.put(*marker, 0, 0);
    m_markers.push_back(std::move(marker));
}

void Gl::add_overlay_widget(Gtk::Widget& w) {
    m_overlay.add_overlay(w);
    m_overlay.set_overlay_pass_through(w, true);
}

void Gl::build_grid(float size, float step)
{
    std::vector<float> grid;
    float half = size * 0.5f;

    for (float z = -half; z <= half; z += step) {
        grid.push_back(-half); grid.push_back(0.0f); grid.push_back(z);
        grid.push_back( half); grid.push_back(0.0f); grid.push_back(z);
    }

    for (float x = -half; x <= half; x += step) {
        grid.push_back(x); grid.push_back(0.0f); grid.push_back(-half);
        grid.push_back(x); grid.push_back(0.0f); grid.push_back( half);
    }

    grid_vertex_count = static_cast<GLsizei>(grid.size() / 3);

    glGenVertexArrays(1, &vao_grid);
    glGenBuffers(1, &vbo_grid);

    glBindVertexArray(vao_grid);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);

    glBufferData(GL_ARRAY_BUFFER,
                 grid.size() * sizeof(float),
                 grid.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Gl::load_file(const std::string& path) {
    if (gl_area.get_realized()) gl_area.make_current();

    m_load_count++;
    int midx = m_load_count;

    auto scene = std::make_unique<CgalShape>();
    scene->build_mesh_from_file(path);
    m_scenes.push_back(std::move(scene));
    m_transforms.push_back({});

    add_center_marker(midx);
    ModelMarker* centerN = m_markers.back().get();
    size_t first_axis = m_markers.size();

    auto tx_rx = std::make_unique<ModelMarker>(&m_fixed, glm::vec3( 1.25f,  0.0f,  0.0f), m_zoom, MarkerType::unabledx, glm::vec3(0.0f, 0.0f, 1.0f), midx);
    auto ty_ry = std::make_unique<ModelMarker>(&m_fixed, glm::vec3( 0.0f,  1.25f,  0.0f), m_zoom, MarkerType::unabledy, glm::vec3(0.0f, 0.0f, 1.0f), midx);
    auto tz_rz = std::make_unique<ModelMarker>(&m_fixed, glm::vec3( 0.0f,  0.0f, -1.25f), m_zoom, MarkerType::unabledz, glm::vec3(0.0f, 0.0f, 1.0f), midx);
    auto s = std::make_unique<ModelMarker>(&m_fixed, glm::vec3( 1.25f,  1.25f, -1.25f), m_zoom, MarkerType::unableds, glm::vec3(0.0f, 0.0f, 1.0f), midx);

    connect_marker_signals(tx_rx.get());
    connect_marker_signals(ty_ry.get());
    connect_marker_signals(tz_rz.get());
    connect_marker_signals(s.get());

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

void Gl::load_scan(const std::string& path) {
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    std::vector<float> cloud_data;

    if (ext == ".ply") {
        std::ifstream f(path);
        if (!f.is_open()) {
            std::cerr << "Impossible d'ouvrir : " << path << "\n";
            return;
        }

        std::string line;
        bool in_header = true;
        int n_vertices = 0;

        while (in_header && std::getline(f, line)) {
            if (line.rfind("element vertex", 0) == 0)
                n_vertices = std::stoi(line.substr(15));
            else if (line == "end_header")
                in_header = false;
        }

        cloud_data.reserve(n_vertices * 3);
        float x, y, z;
        while (f >> x >> y >> z) {
            cloud_data.push_back(x);
            cloud_data.push_back(y);
            cloud_data.push_back(z);
        }

    } else if (ext == ".las" || ext == ".laz") {
        pdal::Options opts;
        opts.add("filename", path);

        pdal::LasReader reader;
        reader.setOptions(opts);

        pdal::PointTable table;
        reader.prepare(table);
        pdal::PointViewSet viewSet = reader.execute(table);

        for (const auto& view : viewSet) {
            cloud_data.reserve(cloud_data.size() + view->size() * 3);
            for (pdal::PointId i = 0; i < view->size(); ++i) {
                cloud_data.push_back(static_cast<float>(view->getFieldAs<double>(pdal::Dimension::Id::X, i)));
                cloud_data.push_back(static_cast<float>(view->getFieldAs<double>(pdal::Dimension::Id::Y, i)));
                cloud_data.push_back(static_cast<float>(view->getFieldAs<double>(pdal::Dimension::Id::Z, i)));
            }
        }

    } else {
        std::cerr << "Format de nuage non supporté : " << ext << "\n";
        return;
    }

    m_cloud_point_count = static_cast<int>(cloud_data.size() / 3);

    if (gl_area.get_realized())
        gl_area.make_current();

    glBindVertexArray(vao_cloud);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cloud);
    glBufferData(GL_ARRAY_BUFFER,
                 cloud_data.size() * sizeof(float),
                 cloud_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_show_point_cloud = true;
    gl_area.queue_render();

    std::cout << "Scan chargé : " << m_cloud_point_count << " points <- " << path << "\n";
}

void Gl::set_transform(int idx, const ModelTransform& tr) {
    if (idx >= 0 && idx < static_cast<int>(m_transforms.size()))
        m_transforms[idx] = tr;
}

void Gl::set_camera(float pos_x, float pos_y, float pos_z, float rx, float ry, float zoom) {
    float dist = std::sqrt(pos_x*pos_x + pos_y*pos_y + pos_z*pos_z);
    m_zoom  = (dist > 0.01f) ? dist : zoom;

    angle_x = rx;
    angle_y = ry;

    gl_area.queue_render();
}

void Gl::capture_image(const std::string& path) {
    if (!gl_area.get_realized()) return;
    gl_area.make_current();

    int w = gl_area.get_allocated_width();
    int h = gl_area.get_allocated_height();
    if (w <= 0 || h <= 0) return;

    gl_area.queue_render();

    std::vector<unsigned char> pixels(w * h * 3);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    std::vector<unsigned char> flipped(w * h * 3);
    for (int row = 0; row < h; ++row)
        std::memcpy(flipped.data() + row * w * 3, pixels.data() + (h - 1 - row) * w * 3, w * 3);

    FILE* fp = std::fopen(path.c_str(), "wb");
    if (!fp) {
        std::cerr << "Impossible d'ouvrir pour écriture : " << path << "\n";
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop info  = png_create_info_struct(png);

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        std::fclose(fp);
        std::cerr << "Erreur PNG lors de la capture\n";
        return;
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, w, h, 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    for (int row = 0; row < h; ++row)
        png_write_row(png, flipped.data() + row * w * 3);

    png_write_end(png, nullptr);
    png_destroy_write_struct(&png, &info);
    std::fclose(fp);

    std::cout << "Image capturée : " << path << " (" << w << "x" << h << ")\n";
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
                 vertex_data.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
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
    glGenBuffers(1, &vbo_cloud);

    glGenVertexArrays(1, &vao_cloud);
    glBindVertexArray(vao_cloud);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cloud);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

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

    build_grid(100.0f, 5.0f);
    update_markers_positions();

    gl_area.grab_focus();
}

bool Gl::on_render(const Glib::RefPtr<Gdk::GLContext>&) {
    int w = gl_area.get_allocated_width();
    int h = gl_area.get_allocated_height();
    if (w <= 0 || h <= 0) return true;

    glViewport(0, 0, w, h);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    glm::mat4 cam = glm::rotate(glm::mat4(1.0f), angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
    cam = glm::rotate(cam, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, m_zoom),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

    GLint model_loc = glGetUniformLocation(shader_program, "model");
    GLint color_loc = glGetUniformLocation(shader_program, "u_color");
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao_grid);
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(cam));
    glUniform3f(color_loc, 0.7f, 0.2f, 0.9f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_LINES, 0, grid_vertex_count);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);

    glBindVertexArray(vao);

    GLint offset_verts = 0;
    for (size_t i = 0; i < m_scenes.size(); ++i) {
        const ModelTransform& tr = m_transforms[i];

        float off = tr.use_offset ? OFFSET_STEP * static_cast<float>(i) : 0.0f;
        glm::mat4 model = cam;
        model = glm::translate(model, glm::vec3(off, off, off));
        model = model * make_model_matrix(tr);

        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

        auto vd = m_scenes[i]->to_vertex_data();
        GLsizei vc = static_cast<GLsizei>(vd.size() / 3);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        glUniform3f(color_loc, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, offset_verts, vc);
        glDisable(GL_POLYGON_OFFSET_FILL);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUniform3f(color_loc, 0.0f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, offset_verts, vc);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform3f(color_loc, 0.0f, 0.0f, 0.0f);
        glDrawArrays(GL_POINTS, offset_verts, vc);

        offset_verts += vc;
    }

    if (m_show_point_cloud && m_cloud_point_count > 0) {
        glm::mat4 identity_model = cam;
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(identity_model));
        glUniform3f(color_loc, 1.0f, 0.9f, 0.0f);

        glBindVertexArray(vao_cloud);
        glPointSize(3.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_POINTS, 0, m_cloud_point_count);
        glPointSize(1.0f);
    }

    glBindVertexArray(0);
    update_markers_positions();
    return true;
}

void Gl::on_unrealize() {
    gl_area.make_current();
    if (vbo_grid != 0) { glDeleteBuffers(1, &vbo_grid); vbo_grid = 0; }
    if (vao_grid != 0) { glDeleteVertexArrays(1, &vao_grid); vao_grid = 0; }
    if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vbo_cloud != 0) { glDeleteBuffers(1, &vbo_cloud); vbo_cloud = 0; }
    if (vao_cloud != 0) { glDeleteVertexArrays(1, &vao_cloud); vao_cloud = 0; }
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

        cr->set_source_rgba(0.0, 0.0, 0.8, 0.55);
        cr->set_line_width(1.2);
        std::vector<double> dashes = {6.0, 4.0};
        cr->set_dash(dashes, 0.0);
        cr->move_to(marker->screen_x(), marker->screen_y());
        cr->line_to(center->screen_x(),  center->screen_y());
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
    return project_to_2d(point_3d, ModelTransform{});
}

std::pair<double, double> Gl::project_to_2d(const glm::vec3& local_pos,
                                             const ModelTransform& tr) {
    int w = gl_area.get_allocated_width();
    int h = gl_area.get_allocated_height();
    if (w <= 0 || h <= 0) return {0.0, 0.0};

    glm::mat4 cam = glm::rotate(glm::mat4(1.0f), angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
    cam = glm::rotate(cam, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, m_zoom),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

    glm::mat4 model_mat = cam * make_model_matrix(tr);
    glm::vec4 clip = projection * view * model_mat * glm::vec4(local_pos, 1.0f);

    if (std::abs(clip.w) < 1e-6f) return {w / 2.0, h / 2.0};
    glm::vec3 ndc = glm::vec3(clip) / clip.w;

    double x = (ndc.x * 0.5 + 0.5) * w;
    double y = (1.0 - (ndc.y * 0.5 + 0.5)) * h;
    return {x, y};
}

void Gl::update_markers_positions() {
    for (auto& marker : m_markers) {
        int midx = marker->model_index();

        int w = gl_area.get_allocated_width();
        int h = gl_area.get_allocated_height();
        if (w <= 0 || h <= 0) { marker->set_position(0, 0); continue; }

        glm::mat4 cam = glm::rotate(glm::mat4(1.0f), angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
        cam = glm::rotate(cam, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, m_zoom),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        float aspect = static_cast<float>(w) / static_cast<float>(h);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

        const ModelTransform& tr = (midx < static_cast<int>(m_transforms.size()))
                                    ? m_transforms[midx] : ModelTransform{};

        float off = tr.use_offset ? OFFSET_STEP * static_cast<float>(midx) : 0.0f;                            
        glm::mat4 model = cam;
        model = glm::translate(model, glm::vec3(off, off, off));
        model = model * make_model_matrix(tr);

        glm::vec4 clip = proj * view * model * glm::vec4(marker->local_position(), 1.0f);
        if (std::abs(clip.w) < 1e-6f) { marker->set_position(w / 2.0, h / 2.0); continue; }
        glm::vec3 ndc = glm::vec3(clip) / clip.w;

        double x = (ndc.x * 0.5 + 0.5) * w;
        double y = (1.0 - (ndc.y * 0.5 + 0.5)) * h;
        marker->set_position(x, y);
    }
    m_fixed.queue_draw();
}

glm::vec3 Gl::get_camera_world_position() const {
    glm::vec4 cam_pos(0.0f, 0.0f, m_zoom, 1.0f);
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
    rot = glm::rotate(rot, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec4 world = rot * cam_pos;
    return glm::vec3(world);
}

void Gl::run_scan(const std::string& lidar_config_path, const std::string& output_path) {
    std::shared_ptr<Lidar> lidar;
    try {
        if (m_lidar_override)
            lidar = m_lidar_override;
        else
            lidar = LidarFactory::createFromJsonConfig(lidar_config_path);
    } catch (const std::exception& e) {
        std::cerr << "Erreur chargement LiDAR : " << e.what() << "\n";
        return;
    }
    std::cout << "LiDAR chargé : " << lidar->m_lasers.size() << " lasers, "
              << "h_step=" << lidar->m_h_step[0] << "\n";

    glm::vec3 cam_glm = get_camera_world_position();

    double angle_y_2 = std::fmod(static_cast<double>(glm::degrees(angle_y)), 360.0);
    double sign;

    if (angle_y_2 >= 90 && angle_y_2 <= 270) {
        sign = 1;
    } else {
        sign = -1;
    }

    Point3 cam_pos(sign * cam_glm.x, sign * cam_glm.y, cam_glm.z);

    std::cout << "Position caméra : " << -cam_glm.x << " " << -cam_glm.y << " " << cam_glm.z << "\n";
    std::cout << "Angle x : " << glm::degrees(angle_x) << " Angle y : " << angle_y_2 << std::endl; 
    std::cout << "Signe : " << sign << std::endl;

    Scene scene;
    int total_tris = 0;
    for (size_t i = 0; i < m_scenes.size(); ++i) {
        const ModelTransform& tr = m_transforms[i];
        Transform3 cgal_tr = model_transform_to_cgal(tr);
        float off = tr.use_offset ? 1.5f * static_cast<float>(i) : 0.0f;

        auto obj = std::make_shared<Object>();
        const SurfaceMesh& sm = m_scenes[i]->mesh();

        for (auto face : sm.faces()) {
            std::vector<Point3> pts;
            for (auto v : CGAL::vertices_around_face(sm.halfedge(face), sm))
                pts.push_back(sm.point(v));

            if (pts.size() == 3) {
                obj->m_triangles.push_back(Triangle3(
                    cgal_tr.transform(pts[0]) + Vector3(off, off, off),
                    cgal_tr.transform(pts[1]) + Vector3(off, off, off),
                    cgal_tr.transform(pts[2]) + Vector3(off, off, off)));
                total_tris++;
            }
        }

        auto entity = std::make_shared<StaticEntity>("mesh_" + std::to_string(i), obj, Pose(), "");
        scene.addEntity(entity);
    }
    scene.build();
    std::cout << "Scène : " << total_tris << " triangles\n";

    if (total_tris == 0) {
        std::cerr << "Aucun triangle dans la scène !\n";
        return;
    }
    
    glm::vec3 forward = glm::normalize(-cam_glm);
    double pitch_deg = std::asin(static_cast<double>(forward.y)) * 180.0 / M_PI;
    double yaw_deg   = std::atan2(static_cast<double>(forward.z),
                                static_cast<double>(forward.x)) * 180.0 / M_PI;

    std::vector<Point3> cloud;
    const double h_step_deg = lidar->m_h_step[0];
    int ray_count = 0;

    for (double w = 0.0; w < 360.0; w += 20.0) {
        Pose scanner_pose(cam_pos, pitch_deg, yaw_deg + w, 0.0);
        std::unique_ptr<NoiseModel> noise_model;
        if (lidar->m_model.find("os2") != std::string::npos ||
            lidar->m_model.find("OS2") != std::string::npos) {
            noise_model = std::make_unique<OusterOS2Noise>();
            SIM_INFO("Modèle de bruit OusterOS2 activé");
        } 
        else {
            noise_model = std::make_unique<NullNoiseModel>();
            SIM_INFO("Modèle de bruit nul (distances parfaites)");
        }
        LidarEntity scanner(lidar, 0, 360.0, scanner_pose, std::move(noise_model));

        for (double h = 0.0; h < 360.0; h += h_step_deg) {
            std::vector<Ray3> rays = scanner.scan(h);
            ray_count += rays.size();
            for (const Ray3& ray : rays) {
                auto hit = scene.intersect(ray);
                if (hit && hit->distance >= lidar->m_min_dist && hit->distance <= lidar->m_max_dist)
                    cloud.push_back(hit->point);
            }
        }
    }

    std::cout << "Rayons lancés : " << ray_count << "\n";
    std::cout << "Points scannés : " << cloud.size() << "\n";

    if (cloud.empty()) {
        std::cerr << "Aucun point — vérifier que la scène est dans le champ du LiDAR.\n";
        return;
    }

    std::string ext = output_path.substr(output_path.rfind('.'));
    std::unique_ptr<IPointCloudExporter> exporter;
    if (ext == ".ply")       exporter = std::make_unique<PlyExporter>();
    else if (ext == ".las")  exporter = std::make_unique<LasExporter>();
    else { std::cerr << "Format non supporté : " << ext << "\n"; return; }

    try {
        exporter->save(output_path, cloud);
        std::cout << "Exporté : " << output_path << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Erreur export : " << e.what() << "\n";
        return;
    }

    load_scan(output_path);
}

static std::shared_ptr<Lidar> ensure_override(std::shared_ptr<Lidar>& ov, const std::string& config_path) {
    if (!ov) {
        try { ov = LidarFactory::createFromJsonConfig(config_path); }
        catch (...) {
            ov = std::make_shared<Lidar>("custom", 1.0, 1000.0, std::vector<double>{0.703125, 0.3515626, 0.1757825}, 0.01);
        }
    }
    return ov;
}

void Gl::lidar_override_set_min(double v) {
    ensure_override(m_lidar_override, m_lidar_config)->m_min_dist = v;
}
void Gl::lidar_override_set_max(double v) {
    ensure_override(m_lidar_override, m_lidar_config)->m_max_dist = v;
}
void Gl::lidar_override_set_hstep(size_t idx, double v) {
    auto& hs = ensure_override(m_lidar_override, m_lidar_config)->m_h_step;
    if (idx < hs.size()) hs[idx] = v;
}
void Gl::lidar_override_set_accuracy(double v) {
    ensure_override(m_lidar_override, m_lidar_config)->m_accuracy = v;
}

void Gl::reset_scene() {
    if (gl_area.get_realized()) gl_area.make_current();

    if (m_scenes.size() > 1)
        m_scenes.erase(m_scenes.begin() + 1, m_scenes.end());

    if (m_transforms.size() > 1)
        m_transforms.erase(m_transforms.begin() + 1, m_transforms.end());

    constexpr size_t MARKERS_PER_MODEL = 5;
    constexpr size_t CUBE_MARKERS = MARKERS_PER_MODEL; 

    for (size_t i = CUBE_MARKERS; i < m_markers.size(); ++i)
        m_fixed.remove(*m_markers[i]);

    if (m_markers.size() > CUBE_MARKERS)
        m_markers.erase(m_markers.begin() + CUBE_MARKERS, m_markers.end());

    m_load_count = 0;

    rebuild_vertex_data();
    upload_vertex_data();
    update_markers_positions();
    gl_area.queue_render();
}

void Gl::focus_gl_area() {
    gl_area.grab_focus();
}