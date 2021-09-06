#pragma once

#include <vector>
#include <glm/glm.hpp>

struct NVGcontext;

struct RenderHUD
{
    struct LineParm
    {
        glm::vec3 start;
        glm::vec3 end;
        float width;
        glm::vec4 color;
    };

    struct CircleParm
    {
        glm::vec3 position;
        float radius;
        glm::vec4 color;
    };

    RenderHUD(NVGcontext* context);
    void BeginFrame(const glm::mat4& transform, const glm::vec2& viewport);
    void EndFrame();
    void FlushDrawings();

    void Circle(const glm::vec3& position, float radius, const glm::vec4& color);
    void Line(const glm::vec3& start, const glm::vec3& end, float width, const glm::vec4& color);

    NVGcontext* vg = nullptr; 

    glm::vec2 _viewport = glm::vec2(0.f);
    glm::mat4 _transform = glm::mat4(0.f);

    std::vector<LineParm> _lines;
    std::vector<CircleParm> _circles;
};

