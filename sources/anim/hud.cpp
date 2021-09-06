#include "hud.h"

#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include "util.h"
#include "BufferLayout.h"
#include "AABB.hpp"
#include "mesh.h"
#include "primitive.h"
#include "ogl.h"

using namespace color;

namespace {
    unsigned int VAO, VBO;

    ProgramPtr dotProgram;  // vec2
    ProgramPtr pointProgram; // vec4
    VertexArrayPtr pointVao;

    ProgramPtr fullscreenProgram;
    VertexArrayPtr fullscreenVao;

    ProgramPtr blitProgram;
    VertexArrayPtr blitVao;

    VertexArrayPtr lineVao;
    ProgramPtr lineProgram;

    ProgramPtr gridProgram;
}

struct PointVertex {
    glm::vec4 position;
    glm::vec3 color;
    float size;
};

struct LineVertex
{
    glm::vec4 position;
    glm::vec3 color;
};

struct Renderer2DData
{
    static const uint32_t MaxLines = 10000;
    static const uint32_t MaxLineVertices = MaxLines * 2;
    static const uint32_t MaxLineIndices = MaxLines * 6;

    static const uint32_t MaxPoints = 10000;

    std::vector<LineVertex> LineVertexBuffer;
    std::vector<PointVertex> PointVertexBuffer;
};

static Renderer2DData s_Data;

void initDot() {
    dotProgram = createProgram({"dot.vs", "dot.fs"});

    BufferLayoutDesc layoutDesc({
        { ShaderDataType::Float4, "a_position" },
        { ShaderDataType::Float3, "a_color" },
        { ShaderDataType::Float, "a_size" }
    });

    auto layout = std::make_shared<BufferLayout>(layoutDesc);

    pointProgram = createProgram({"point.vs", "point.fs", layoutDesc});

    VertexArrayDesc vaoDesc;
    vaoDesc.vertexSize = sizeof(PointVertex);
    vaoDesc.vertexCount = s_Data.MaxPoints;
    vaoDesc.layout = layout;

    pointVao = createVertexArray(vaoDesc);
}

void DrawPoint(const glm::vec4& position, const glm::vec3& color, float size) {
    s_Data.PointVertexBuffer.push_back({position, color, size});
}

void DrawPoint(const glm::mat4& transform, const glm::vec4& position, const glm::vec3& color, float size) {
    DrawPoint(transform * position, color, size);
}

void drawPoint(const glm::mat4& transform, const glm::vec3& position, const glm::vec3& color, float size) {
    DrawPoint(transform * glm::vec4(position, 1.f), color, size);
}

void FlushPointDrawing() {
    auto& buffer = s_Data.PointVertexBuffer;
    size_t size = sizeof(PointVertex) * buffer.size();
    glBindBuffer(GL_ARRAY_BUFFER, pointVao->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, buffer.data());

    bindVertexArray(pointVao);
    glDisable(GL_DEPTH_TEST);
    useProgram(pointProgram);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, buffer.size());

    buffer.clear();
}

void DrawCircle(glm::vec2 center, glm::vec2 viewport, glm::vec4 color) {
    useProgram(dotProgram);
    dotProgram->setUniform("u_center", center);
    dotProgram->setUniform("u_viewport", viewport);
    dotProgram->setUniform("u_color", color);

    bindVertexArray(fullscreenVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void initFullscreen() {
    struct FullscreenVertex { float x, y, tu, tv; };

    BufferLayoutDesc layoutDesc({
        { ShaderDataType::Float2, "a_position" },
        { ShaderDataType::Float2, "a_texcoord" }
    });

    std::vector<FullscreenVertex> vertices {
        { -1, -1, 0, 0 },
        {  3, -1, 2, 0 },
        { -1,  3, 0, 2 },
    };

    fullscreenProgram = createProgram({"fullscreen.vs", "fullscreen.fs", layoutDesc});

    initProgramTextures(fullscreenProgram, {"u_texture"});

    auto layout = std::make_shared<BufferLayout>(layoutDesc);

    VertexArrayDesc vaoDesc;
    vaoDesc.vertexSize = sizeof(FullscreenVertex);
    vaoDesc.vertexCount = vertices.size();
    vaoDesc.layout = layout;
    vaoDesc.vertices = (uint8_t*)vertices.data();

    fullscreenVao = createVertexArray(vaoDesc);
}

void drawFullscreen() {
    useProgram(fullscreenProgram);

    bindVertexArray(fullscreenVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void initBlit() 
{
    struct BlitVertex {
        glm::vec3 position;
        glm::vec2 texcoord;
    };

    BufferLayoutDesc layoutDesc({
        { ShaderDataType::Float3, "a_position" },
        { ShaderDataType::Float2, "a_texcoord" }
    });

    std::vector<BlitVertex> vertices {
        { { -1, -1, 0 }, {0, 0} },
        { { +1, -1, 0 }, {1, 0} },
        { { -1, +1, 0 }, {0, 1} },
        { { -1, +1, 0 }, {0, 1} },
        { { +1, -1, 0 }, {1, 0} },
        { { +1, +1, 0 }, {1, 1} }
    };

    blitProgram = createProgram({"blit.vs", "blit.fs", layoutDesc});

    initProgramTextures(blitProgram, {"u_texture"});

    auto layout = std::make_shared<BufferLayout>(layoutDesc);

    VertexArrayDesc vaoDesc;
    vaoDesc.vertexSize = sizeof(BlitVertex);
    vaoDesc.vertexCount = vertices.size();
    vaoDesc.layout = layout;
    vaoDesc.vertices = (uint8_t*)vertices.data();

    blitVao = createVertexArray(vaoDesc);
}

void DrawBlit(const glm::mat4& transform) {
    useProgram(blitProgram);
    blitProgram->setUniform("u_transform", transform);

    bindVertexArray(blitVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void DrawLine(const glm::mat4& transform, const glm::vec4& p0, const glm::vec4& p1, const glm::vec3& color) {
    s_Data.LineVertexBuffer.push_back({transform*p0, color});
    s_Data.LineVertexBuffer.push_back({transform*p1, color});
}

void DrawLine(const glm::vec4& p0, const glm::vec4& p1, const glm::vec3& color) {
    s_Data.LineVertexBuffer.push_back({p0, color});
    s_Data.LineVertexBuffer.push_back({p1, color});
}

void DrawAABB(const AABB& aabb, const glm::mat4 transform) {
    DrawAABB(aabb, transform, glm::vec4(1.f));
}

void initGrid() 
{
    BufferLayoutDesc layoutDesc({
        { ShaderDataType::Float3, "a_position" },
        { ShaderDataType::Float2, "a_texcoord" }
    });

    gridProgram = createProgram({"Assets/grid.vert", "Assets/grid.frag", layoutDesc});
}

void initLine() {
    BufferLayoutDesc layoutDesc({
        { ShaderDataType::Float4, "a_position" },
        { ShaderDataType::Float3, "a_color" }
    });

    auto layout = std::make_shared<BufferLayout>(layoutDesc);

    lineProgram = createProgram({"line.vs", "line.fs", layoutDesc});

    size_t maxVertexCount = s_Data.MaxLineVertices;
    size_t maxIndexCount = s_Data.MaxLineIndices;

    VertexArrayDesc vaoDesc;
    vaoDesc.vertexSize = sizeof(LineVertex);
    vaoDesc.vertexCount = maxVertexCount;
    vaoDesc.indexCount = maxIndexCount;
    vaoDesc.layout = layout;

    lineVao = createVertexArray(vaoDesc);

    std::vector<uint32_t> indices(maxIndexCount);
    for (size_t i = 0; i < maxIndexCount; i++)
        indices[i] = i;
    size_t indexSize = sizeof(uint32_t)*maxIndexCount;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineVao->ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexSize, indices.data());
}

void FlushLineDrawing() {
    auto& buffer = s_Data.LineVertexBuffer;
    size_t size = sizeof(LineVertex) * buffer.size();
    glBindBuffer(GL_ARRAY_BUFFER, lineVao->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, buffer.data());

    glBindVertexArray(lineVao->id);
    glDisable(GL_DEPTH_TEST);
    // NOTE: core context requires below 1.0
    glLineWidth(1.0);
    useProgram(lineProgram);
    glDrawElements(GL_LINES, buffer.size(), GL_UNSIGNED_INT, 0);

    buffer.clear();
}

void DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color) {

    const auto& min = aabb.getMin();
    const auto& max = aabb.getMax();

    auto cornerPts = {
        glm::vec3{ min.x, min.y, max.z },
        glm::vec3{ min.x, max.y, max.z },
        glm::vec3{ max.x, max.y, max.z },
        glm::vec3{ max.x, min.y, max.z },

        glm::vec3{ min.x, min.y, min.z },
        glm::vec3{ min.x, max.y, min.z },
        glm::vec3{ max.x, max.y, min.z },
        glm::vec3{ max.x, min.y, min.z },
    };

    std::vector<glm::vec4> corners;
    for (auto& pts : cornerPts) {
        corners.push_back(transform * glm::vec4(pts, 1.f));
    }

    // À§¿¡ 4°³³¢¸®
    for (uint32_t i = 0; i < 4; i++)
        DrawLine(corners[i], corners[(i + 1) % 4], color);

    // ¾Æ·¡ 4°³ ³¢¸®
    for (uint32_t i = 0; i < 4; i++)
        DrawLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);

    // À§¾Æ·¡ ÀÕ´Â ¼±
    for (uint32_t i = 0; i < 4; i++)
        DrawLine(corners[i], corners[i + 4], color);
}

ProgramPtr primitiveProgram;
VertexArrayPtr sphereVao;

ProgramPtr cubeProgram;
VertexArrayPtr cubeVao;

void initPrimitive() {
    BufferLayoutDesc layoutDesc({
        { ShaderDataType::Float3, "a_position" },
        { ShaderDataType::Float3, "a_normal" },
        { ShaderDataType::Float2, "a_texcoord" }
    });

    auto layout = std::make_shared<BufferLayout>(layoutDesc);

    primitiveProgram = createProgram({"primitive.vs", "primitive.fs", layoutDesc});

    auto sphere = SphereMesh(1.f, 8);

    VertexArrayDesc sphereDesc;
    sphereDesc.vertexSize = sizeof(PrimitiveVertex);
    sphereDesc.vertexCount = sphere.size();
    sphereDesc.vertices = (uint8_t*)sphere.data();
    sphereDesc.layout = layout;

    sphereVao = createVertexArray(sphereDesc);
}

void initCube()
{
    BufferLayoutDesc layoutDesc({
        { ShaderDataType::Float3, "a_position" },
        { ShaderDataType::Float3, "a_color" },
    });

    auto layout = std::make_shared<BufferLayout>(layoutDesc);

    cubeProgram = createProgram({"Assets/cube.vert", "Assets/cube.frag", layoutDesc});

    struct ColorVertex {
        glm::vec3 pos, col;
    };

    auto cube = std::vector<ColorVertex> {
        {glm::vec3(1, 1, 1), glm::vec3(1, 1, 1)}, // 0
        {glm::vec3(-1, 1, 1), glm::vec3(0, 1, 1)}, // 1
        {glm::vec3(-1, -1, 1), glm::vec3(0, 0, 1)}, // 2
        {glm::vec3(1, -1, 1), glm::vec3(1, 0, 1)}, // 3
        {glm::vec3(1, -1, -1), glm::vec3(1, 0, 0)}, // 4
        {glm::vec3(-1, -1, -1), glm::vec3(0, 0, 0)}, // 5
        {glm::vec3(-1, 1, -1), glm::vec3(0, 1, 0)}, // 6
        {glm::vec3(1, 1, -1), glm::vec3(1, 1, 0)}, // 7
    };
    uint32_t indices[24] = {
        0, 1, 2, 3,                 // Front face
        7, 4, 5, 6,                 // Back face
        6, 5, 2, 1,                 // Left face
        7, 0, 3, 4,                 // Right face
        7, 6, 1, 0,                 // Top face
        3, 2, 5, 4,                 // Bottom face
    };

    VertexArrayDesc cubeDesc;
    cubeDesc.vertexSize = sizeof(ColorVertex);
    cubeDesc.vertexCount = cube.size();
    cubeDesc.vertices = (uint8_t*)cube.data();
    cubeDesc.indexCount = 24;
    cubeDesc.indices = (uint8_t*)indices;
    cubeDesc.layout = layout;

    cubeVao = createVertexArray(cubeDesc);
}

void DrawSphere(const glm::mat4& transform, const glm::vec3& position, const float scale) {
    auto scaleM = glm::scale(glm::mat4(1.f), glm::vec3(scale));
    auto translateM = glm::translate(glm::mat4(1.f), position);
    auto transformM = transform * translateM * scaleM;

    glEnable(GL_DEPTH_TEST);
    bindVertexArray(sphereVao);
    useProgram(primitiveProgram);
    primitiveProgram->setUniform("u_transform", transformM);
    glDrawArrays(GL_TRIANGLES, 0, sphereVao->vertexCount);
    glDisable(GL_DEPTH_TEST);
}

void DrawCube(const glm::mat4& transform, const glm::vec3& position, const float scale) {
    auto scaleM = glm::scale(glm::mat4(1.f), glm::vec3(scale));
    auto translateM = glm::translate(glm::mat4(1.f), position);
    auto transformM = transform * translateM * scaleM;

    glEnable(GL_DEPTH_TEST);
    bindVertexArray(cubeVao);
    useProgram(primitiveProgram);
    primitiveProgram->setUniform("u_transform", transformM);
    glDrawElements(GL_QUADS, cubeVao->indexCount, GL_UNSIGNED_INT, 0);
    glDisable(GL_DEPTH_TEST);
}

void DrawCube(const glm::mat4& transform, const glm::vec3& position, const glm::vec3& scale) {
    auto scaleM = glm::scale(glm::mat4(1.f), scale);
    auto translateM = glm::translate(glm::mat4(1.f), position);
    auto transformM = transform * translateM * scaleM;

    glEnable(GL_DEPTH_TEST);
    bindVertexArray(cubeVao);
    useProgram(cubeProgram);
    cubeProgram->setUniform("u_transform", transformM);
    glDrawElements(GL_QUADS, cubeVao->indexCount, GL_UNSIGNED_INT, 0);
    glDisable(GL_DEPTH_TEST);
}

void DrawAxis(const glm::mat4& transform)
{
    DrawLine(transform, glm::vec4(-1.f, 0.f, 0.f, 1.0), glm::vec4(1.f, 0.0f, 0.f, 1.f), RED);
    DrawLine(transform, glm::vec4(0.f, -1.f, 0.f, 1.0), glm::vec4(0.f, 1.0f, 0.f, 1.f), GREEN);
    DrawLine(transform, glm::vec4(0.f, 0.f, 1.f, 1.0), glm::vec4(0.f, 0.0f, -1.f, 1.f), BLUE);
}

void DrawGrid(const glm::mat4& viewProjection)
{
    float gridScale = 16.025f, gridSize = 0.025f;
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(16.0f));

    glDisable(GL_CULL_FACE);
    useProgram(gridProgram);

    gridProgram->setUniform("u_MVP", viewProjection * model);
    gridProgram->setUniform("u_Scale", gridScale);
    gridProgram->setUniform("u_Res", gridSize);

    bindVertexArray(blitVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_CULL_FACE);
}

void initHUD() {
    initDot();
    initLine();
    initBlit();
    initGrid();
    initFullscreen();
    initPrimitive();
    initCube();
}

void FlushDrawing() {
    FlushLineDrawing();
    FlushPointDrawing();
}

