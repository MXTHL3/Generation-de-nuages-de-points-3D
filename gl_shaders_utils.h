#pragma once

#include <epoxy/gl.h>
#include <string>

/// @brief Utilitaire de compilation et d'édition de liens des shaders OpenGL.
/// Compile les shaders vertex et fragment, les lie en un programme GLSL
/// et rapporte les erreurs de compilation/liaison sur stdout.
class GLShadersUtils {
public:
    GLShadersUtils();

    /// @brief Compile un shader OpenGL et affiche les éventuelles erreurs.
    /// @param shader Handle OpenGL du shader à compiler.
    /// @param name Nom affiché dans les messages de log (ex. "vertex").
    void compile_shader(GLuint shader, const char* name);

    /// @brief Lie les shaders attachés en un programme exécutable.
    /// @param program Handle du programme OpenGL à lier.
    void link_program(GLuint program);

    /// @brief Compile et lie un programme GLSL complet depuis les sources.
    /// @param vert_src Code source GLSL du vertex shader.
    /// @param frag_src Code source GLSL du fragment shader.
    /// @return Handle du programme lié, 0 en cas d'erreur.
    GLuint build_program(const char* vert_src, const char* frag_src);

    /// @brief Indique si la dernière opération s'est terminée sans erreur.
    /// @return true si compilation et liaison ont réussi.
    bool ok() const { return m_ok; }

private:
    bool m_ok = true; ///< false dès qu'une erreur de compilation ou de liaison survient.
};