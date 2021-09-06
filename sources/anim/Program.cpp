#include "Program.h"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include "util.h"

namespace {
    // update check
    std::vector<ProgramPtr> g_ProgramList;
}

GLuint createShader(GLenum type, const char* shaderCode)
{
    GLuint id = glCreateShader(type);
    if (id == 0)
        return 0;

    glShaderSource(id, 1, &shaderCode, 0);
    glCompileShader(id);

    GLint compiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint length = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> buffer(length + 1);
        glGetShaderInfoLog(id, length, 0, buffer.data());
        trace("%s (%d) %s\n", __FILE__, __LINE__, buffer.data());
        glDeleteShader(id);
        return 0;
    }
    return id;
}

GLuint createProgram(GLuint vertex, GLuint fragment, const BufferLayoutDesc& layout) {
    GLuint id = glCreateProgram();

    GLint status = 0;
    if (vertex != 0) {
        glAttachShader(id, vertex);
        if (fragment != 0)
            glAttachShader(id, fragment);

        const auto& elements = layout.Elements;
        for (size_t i = 0; i < elements.size(); i++)
            glBindAttribLocation(id, i, elements[i].Name.c_str());

        glLinkProgram(id);
        glGetProgramiv(id, GL_LINK_STATUS, &status);

        if (status == GL_FALSE) {
            const uint32_t kBufferSize = 512u;
            char log[kBufferSize];
            glGetProgramInfoLog(id, sizeof(log), nullptr, log);
            trace("%s:%d %d: %s", __FILE__, __LINE__, status, log);
            return 0;
        }
    }

    if (status == GL_FALSE) {
        glDeleteProgram(id);
        id = 0;
        return id;
    }

    return id;
}

ProgramPtr createProgram(const ProgramDesc& desc) {
    auto vtxShader = getFileAsString(desc.vertexFilepath);
    auto frgShader = getFileAsString(desc.fragemntFilepath);

    GLuint vs = createShader(GL_VERTEX_SHADER, vtxShader.c_str());
    GLuint fs = createShader(GL_FRAGMENT_SHADER, frgShader.c_str());

    assert(vs != 0);
    assert(fs != 0);

    GLuint id = createProgram(vs, fs, desc.layout);

    auto program = std::make_shared<Program>();
    program->vertex = vs;
    program->framgment = fs;
    program->id = id;
    program->desc = desc;
    program->timeStamp = 0;

    g_ProgramList.push_back(program);

    return program;
}

void destroyProgram(ProgramPtr& program) {
    auto it = std::find_if(g_ProgramList.begin(), g_ProgramList.end(),
        [&](const ProgramPtr& a) { return program == a; });
    g_ProgramList.erase(it);

    glDeleteProgram(program->id);
    glDeleteShader(program->vertex);
    glDeleteShader(program->framgment);

    program->vertex = 0;
    program->framgment = 0;
    program->id = 0;

    program = nullptr;
}

void useProgram(const ProgramPtr& program) {
    assert(program);
    if (program->id == 0) {
        trace("program id is zero %s\n", program->desc.vertexFilepath.c_str());
        return;
    }
    glUseProgram(program->id);
}

// register tmu 
void initProgramTextures(const ProgramPtr& program, const std::initializer_list<std::string>& textureNames) {
    useProgram(program);

    GLint tmu = 0;
    for (auto texture : textureNames) {
        program->setUniform(texture.c_str(), tmu++);
    }
    glUseProgram(0);
}

bool Program::setUniform(const std::string& name, const glm::vec2& v) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());

    if(-1 == loc)
    {
        printf("ProgramShader : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniform2fv(loc, 1, glm::value_ptr(v));
    return true;
}

bool Program::setUniform(const std::string &name, const glm::vec3 &v) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());

    if(-1 == loc)
    {
        printf("ProgramShader : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniform3fv(loc, 1, glm::value_ptr(v));
    return true;
}

bool Program::setUniform(const std::string &name, const glm::vec4 &v) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());

    if(-1 == loc)
    {
        printf("ProgramShader : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniform4fv(loc, 1, glm::value_ptr(v));
    return true;
}

bool Program::setUniform(const std::string &name, int32_t v) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());

    if(-1 == loc)
    {
        printf("Program : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniform1i(loc, v);
    return true;
}


bool Program::setUniform(const std::string &name, float v) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());

    if(-1 == loc)
    {
        printf("Program : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniform1f(loc, v);
    return true;
}

bool Program::setUniform(const std::string& name, const glm::vec4* v, size_t count) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());
    if (-1 == loc)
    {
        printf("Program : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    if (count == 0)
        return true;

    glUniform4fv(loc, count, glm::value_ptr(*v));
    return true;
}

bool Program::setUniform(const std::string& name, const glm::mat3& v) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());

    if(-1 == loc)
    {
        printf("Program : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v));
    return true;
}

bool Program::setUniform(const std::string &name, const glm::mat4 &v) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());

    if(-1 == loc)
    {
        printf("Program : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v));
    return true;
}

bool Program::setUniform(const std::string &name, const glm::mat4* v, size_t count) const
{
    GLint loc = glGetUniformLocation(id, name.c_str());

    if(-1 == loc)
    {
        printf("Program : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniformMatrix4fv(loc, count, GL_FALSE, (const GLfloat*)&v[0]);
    return true;
}

