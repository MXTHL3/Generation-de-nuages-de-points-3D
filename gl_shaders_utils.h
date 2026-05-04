#pragma once

class GLShadersUtils {
public:
    GLShadersUtils();
    void compile_shader (GLuint shader, const char* name);
    void link_program (GLuint program); 
}