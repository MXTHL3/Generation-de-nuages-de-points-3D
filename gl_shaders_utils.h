#pragma once

#include <epoxy/gl.h>
#include <string>

class GLShadersUtils {
public:
    GLShadersUtils();
    void compile_shader(GLuint shader, const char* name);
    void link_program(GLuint program);
    GLuint build_program(const char* vert_src, const char* frag_src);
    bool ok() const { return m_ok; }

private:
    bool m_ok = true;
};