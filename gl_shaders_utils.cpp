// Source : https://pageperso.lis-lab.fr/~edouard.thiel/ens/prog-gra/pgra02-cm/

GLShadersUtils() {}

void GLShadersUtils::compile_shader (GLuint shader, const char* name)
{
    std::cout << "Compile " << name << " shader...\n";
    glCompileShader (shader);

    GLint isCompiled = 0;
    glGetShaderiv (shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) m_ok = false;

    GLsizei maxLength = 2048, length;
    char infoLog[maxLength];
    glGetShaderInfoLog (shader, maxLength, &length, infoLog);

    if (length == 0) return;
    if (isCompiled == GL_TRUE)
         std::cout << "Compilation messages:\n";
    else std::cout << "### Compilation errors:\n";
    std::cout << infoLog << std::endl;
}

void GLShadersUtils::link_program (GLuint program)
{
    std::cout << "Link program...\n";
    glLinkProgram (program);

    GLint status;
    glGetProgramiv (program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) m_ok = false;

    GLsizei maxLength = 2048, length;
    char infoLog[maxLength];
    glGetProgramInfoLog (program, maxLength, &length, infoLog);

    if (length == 0) return;
    if (status == GL_TRUE)
         std::cout << "Linking messages:\n";
    else std::cout << "### Linking errors:\n";
    std::cout << infoLog << std::endl;
}

