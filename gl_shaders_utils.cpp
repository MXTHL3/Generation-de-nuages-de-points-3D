#include "gl_shaders_utils.h"
#include <iostream>

// Source : https://pageperso.lis-lab.fr/~edouard.thiel/ens/prog-gra/pgra02-cm/

GLShadersUtils::GLShadersUtils() {}

void GLShadersUtils::compile_shader(GLuint shader, const char* name)
{
    std::cout << "Compile " << name << " shader...\n";
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) m_ok = false;

    GLsizei maxLength = 2048, length;
    char infoLog[2048];
    glGetShaderInfoLog(shader, maxLength, &length, infoLog);

    if (length == 0) return;
    if (isCompiled == GL_TRUE)
        std::cout << "Compilation messages:\n";
    else
        std::cout << "### Compilation errors:\n";
    std::cout << infoLog << std::endl;
}

void GLShadersUtils::link_program(GLuint program)
{
    std::cout << "Link program...\n";
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) m_ok = false;

    GLsizei maxLength = 2048, length;
    char infoLog[2048];
    glGetProgramInfoLog(program, maxLength, &length, infoLog);

    if (length == 0) return;
    if (status == GL_TRUE)
        std::cout << "Linking messages:\n";
    else
        std::cout << "### Linking errors:\n";
    std::cout << infoLog << std::endl;
}

GLuint GLShadersUtils::build_program(const char* vert_src, const char* frag_src)
{
    m_ok = true;

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vert_src, nullptr);
    compile_shader(vs, "vertex");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &frag_src, nullptr);
    compile_shader(fs, "fragment");

    if (!m_ok) {
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    link_program(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!m_ok) {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}