#pragma once

#include <memory>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
#include <string>

void EnableOpenGLDebug();

struct BufferLayoutDesc;

typedef std::shared_ptr<class BufferLayout> BufferLayoutPtr;
typedef std::shared_ptr<struct Texture2D> Texture2DPtr;


struct VertexArrayDesc {
    size_t vertexSize = 0;
    size_t vertexCount = 0;
    size_t indexCount = 0;
    uint8_t* vertices = 0;
    uint8_t* indices = 0;
    BufferLayoutPtr layout = nullptr;
};

struct VertexArray
{
    GLuint vbo = 0;
    GLuint ibo = 0;
    size_t vertexCount = 0;
    size_t indexCount = 0;
    VertexArrayDesc desc;
};

typedef std::shared_ptr<struct VertexArray> VertexArrayPtr;

VertexArrayPtr createVertexArray(const VertexArrayDesc& desc);
void bindVertexArray(const VertexArrayPtr& vao);
void unbindVertexArray(const VertexArrayPtr& vao);

typedef std::shared_ptr<struct Framebuffer> FramebufferPtr;

struct Framebuffer {

    Framebuffer();
    ~Framebuffer();
    bool Create(size_t width, size_t height);
    void Destory();

    size_t width = 0;
    size_t height = 0;
    uint32_t id = 0;
    uint32_t depthBuffer = 0;

    Texture2DPtr colorTexture;
};

