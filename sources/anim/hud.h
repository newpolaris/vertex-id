#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "AABB.hpp"
#include "Program.h"

namespace color {
    const auto RED = glm::vec3(1.f, 0.f, 0.f);
    const auto GREEN = glm::vec3(0.f, 1.f, 0.f);
    const auto BLUE = glm::vec3(0.f, 0.f, 1.f);
    const auto YELLO = glm::vec3(1.f, 1.f, 0.f);
    const auto MAGENTA = glm::vec3(1.f, 0.f, 1.f);
    const auto WHITE = glm::vec3(1.0, 1.0, 1.0);
}

void initHUD();

void DrawBlit(const glm::mat4& transform);
void drawFullscreen();
void drawGridBackground(glm::vec2 viewport);
void DrawCircle(glm::vec2 center, glm::vec2 viewport, glm::vec4 color);
void DrawAxis(const glm::mat4& transform);
void DrawGrid(const glm::mat4& viewProjection);

void DrawLine(const glm::mat4& transform, const glm::vec4& p0, const glm::vec4& p1, const glm::vec3& color);
void DrawLine(const glm::vec4& p0, const glm::vec4& p1, const glm::vec3& color);
void DrawAABB(const AABB& aabb,const glm::mat4 transform);
void DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color);
void FlushDrawing();

void DrawPoint(const glm::vec4& position, const glm::vec3& color, float size);
void DrawPoint(const glm::mat4& transform, const glm::vec4& position, const glm::vec3& color, float size);

void DrawSphere(const glm::mat4& transform, const glm::vec3& position, const float scale);
void DrawCube(const glm::mat4& transform, const glm::vec3& position, const float scale);
void DrawCube(const glm::mat4& transform, const glm::vec3& position, const glm::vec3& scale);

extern ProgramPtr fontProgram;
