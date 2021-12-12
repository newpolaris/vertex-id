#define _CRT_SECURE_NO_WARNINGS

#include "ogl.h"

#include <vector>
#include <sstream>

#include "util.h"
#include "BufferLayout.h"
#include "mesh.h"
#include "Program.h"
#include "Texture2D.h"

GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
    switch (type)
    {
    case ShaderDataType::Float:    return GL_FLOAT;
    case ShaderDataType::Float2:   return GL_FLOAT;
    case ShaderDataType::Float3:   return GL_FLOAT;
    case ShaderDataType::Float4:   return GL_FLOAT;
    case ShaderDataType::Mat3:     return GL_FLOAT;
    case ShaderDataType::Mat4:     return GL_FLOAT;
    case ShaderDataType::Int:      return GL_INT;
    case ShaderDataType::Int2:     return GL_INT;
    case ShaderDataType::Int3:     return GL_INT;
    case ShaderDataType::Int4:     return GL_INT;
    case ShaderDataType::Bool:     return GL_BOOL;
    }
    return 0;
}

VertexArrayPtr createVertexArray(const VertexArrayDesc& desc) {
    size_t vertexCount = desc.vertexCount;
    size_t indexCount = desc.indexCount;

    GLuint ibo = 0;
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);

    const size_t vertexBytes = desc.vertexSize * vertexCount;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBytes, desc.vertices, GL_DYNAMIC_DRAW);

    if (indexCount > 0) {
        glGenBuffers(1, &ibo);
        const size_t indexBytes = sizeof(uint32_t) * indexCount;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBytes, desc.indices, GL_DYNAMIC_DRAW);
    } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    auto size = desc.layout->getSize();
    const auto& elements = desc.layout->getElements();

    for (size_t i = 0; i < elements.size(); i++) {
        const auto& e = elements[i];
        auto compCnt = e.Count;
        auto offset = e.Offset;
        auto dataType = ShaderDataTypeToOpenGLBaseType(e.Type);
        auto glBaseType = ShaderDataTypeToOpenGLBaseType(e.Type);
        glEnableVertexAttribArray(i);
        if (glBaseType == GL_INT)
            glVertexAttribIPointer(i, compCnt, dataType, size, (const void*)offset);
        else
            glVertexAttribPointer(i, compCnt, dataType, GL_FALSE, size, (const void*)offset);
    }

    auto vtxArr = std::make_shared<VertexArray>();
    if (vtxArr) {
        vtxArr->vbo = vbo;
        vtxArr->ibo = ibo;
        vtxArr->vertexCount = vertexCount;
        vtxArr->indexCount = indexCount;
        vtxArr->desc = desc;
    }
    return vtxArr;
}

void bindVertexArray(const VertexArrayPtr& vao) 
{
    assert(vao);

    GLuint vbo = vao->vbo;
    GLuint ibo = vao->ibo;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    auto size = vao->desc.layout->getSize();
    const auto& elements = vao->desc.layout->getElements();

    for (size_t i = 0; i < elements.size(); i++) {
        const auto& e = elements[i];
        auto compCnt = e.Count;
        auto offset = e.Offset;
        auto dataType = ShaderDataTypeToOpenGLBaseType(e.Type);
        auto glBaseType = ShaderDataTypeToOpenGLBaseType(e.Type);
        glEnableVertexAttribArray(i);
        if (glBaseType == GL_INT)
            glVertexAttribIPointer(i, compCnt, dataType, size, (const void*)offset);
        else
            glVertexAttribPointer(i, compCnt, dataType, GL_FALSE, size, (const void*)offset);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void unbindVertexArray(const VertexArrayPtr& vao) 
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    auto size = vao->desc.layout->getSize();
    const auto& elements = vao->desc.layout->getElements();
    for (size_t i = 0; i < elements.size(); i++)
        glDisableVertexAttribArray(i);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Framebuffer::Framebuffer()
{
}

Framebuffer::~Framebuffer()
{
    Destory();
}

bool Framebuffer::Create(size_t w, size_t h)
{
    Destory();

    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    colorTexture = createTexture(w, h, nullptr);

    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture->_id, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    width = w;
    height = h;

    return true;
}

void Framebuffer::Destory()
{
    glDeleteFramebuffers(1, &id);
    glDeleteRenderbuffers(1, &depthBuffer);
    if (colorTexture) {
        glDeleteTextures(1, &colorTexture->_id);
        colorTexture = 0;
    }
    id = 0;
    depthBuffer = 0;
}
