#include "gl.h"
#include "gl_shaders_utils.h"
#include <iostream>
#include <stdexcept>

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

Gl::Gl(std::unique_ptr<Cgal> scene_)
    : scene(std::move(scene_))
{
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

    gl_area.signal_button_press_event().connect(
        sigc::mem_fun(*this, &Gl::on_button_press));
    gl_area.signal_button_release_event().connect(
        sigc::mem_fun(*this, &Gl::on_button_release));
    gl_area.signal_motion_notify_event().connect(
        sigc::mem_fun(*this, &Gl::on_motion));

    gl_area.add_events(Gdk::BUTTON_PRESS_MASK |
                       Gdk::BUTTON_RELEASE_MASK |
                       Gdk::POINTER_MOTION_MASK |
                       Gdk::SCROLL_MASK);          

    gl_area.signal_key_press_event().connect(
        sigc::mem_fun(*this, &Gl::on_key_press));
    gl_area.set_can_focus(true);
    gl_area.grab_focus();

    gl_area.show();
}

void Gl::set_scene(std::unique_ptr<Cgal> new_scene)
{
    scene = std::move(new_scene);
    if (gl_area.get_realized()) {
        gl_area.make_current();
        upload_geometry();
        gl_area.queue_render();
    }
}

void Gl::on_realize()
{
    try {
        gl_area.make_current();
        gl_area.throw_if_error();
    }
    catch (const Gdk::GLError& ex) {
        std::cerr << "Erreur GLArea: " << ex.what() << std::endl;
        return;
    }

    GLShadersUtils su;

    shader_program = su.build_program(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
    if (!shader_program) {
        std::cerr << "Shader program failed!\n";
        return;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    upload_geometry();

    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
}

void Gl::upload_geometry()
{
    if (!scene || vao == 0 || vbo == 0)
        return;

    scene->build_cube_mesh();
    vertex_data = scene->to_vertex_data();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    if (vertex_data.empty()) {
        std::cerr << "No vertex data!\n";
        return;
    }

    glBufferData(
        GL_ARRAY_BUFFER,
        vertex_data.size() * sizeof(float),
        vertex_data.data(),
        GL_STATIC_DRAW
    );

    std::cout << "vertex count = " << vertex_data.size() / 6 << std::endl;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Gl::load_file(const std::string& path)
{
    if (gl_area.get_realized())
        gl_area.make_current();

    scene->build_mesh_from_file(path);
    vertex_data = scene->to_vertex_data();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_data.size() * sizeof(float),
                 vertex_data.data(),
                 GL_STATIC_DRAW);
    glBindVertexArray(0);

    gl_area.queue_render();
}

bool Gl::on_render(const Glib::RefPtr<Gdk::GLContext>&)
{
    int w = gl_area.get_allocated_width();
    int h = gl_area.get_allocated_height();
    if (w <= 0 || h <= 0)
        return true;

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
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

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

    return true;
}

void Gl::on_unrealize()
{
    gl_area.make_current();

    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    if (shader_program != 0) {
        glDeleteProgram(shader_program);
        shader_program = 0;
    }
}

bool Gl::on_button_press(GdkEventButton* e)
{
    if (e->button == 1) {
        m_dragging = true;
        m_last_x = e->x;
        m_last_y = e->y;
    }
    return true;
}

bool Gl::on_button_release(GdkEventButton* e)
{
    if (e->button == 1)
        m_dragging = false;
    return true;
}

bool Gl::on_motion(GdkEventMotion* e)
{
    if (!m_dragging) return true;

    double dx = e->x - m_last_x;
    double dy = e->y - m_last_y;
    m_last_x = e->x;
    m_last_y = e->y;

    angle_y += static_cast<float>(dx) * 0.01f;
    angle_x += static_cast<float>(dy) * 0.01f;

    gl_area.queue_render();
    return true;
}

bool Gl::on_key_press(GdkEventKey* e)
{
    switch (e->keyval) {
        case GDK_KEY_plus:
        case GDK_KEY_KP_Add:
            m_zoom -= 0.3f;
            break;
        case GDK_KEY_minus:
        case GDK_KEY_KP_Subtract:
            m_zoom += 0.3f;
            break;
        default:
            return false;
    }

    m_zoom = std::clamp(m_zoom, 1.0f, 20.0f);
    gl_area.queue_render();
    return true;
}