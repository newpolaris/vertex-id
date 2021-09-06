#pragma once

#include <vector>
#include <glm/glm.hpp>

struct PrimitiveVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

std::vector<PrimitiveVertex> cubeMesh();
std::vector<PrimitiveVertex> PlaneMesh(float size = 1.f, float res = 7.f, float UVScale = 1);
std::vector<PrimitiveVertex> SphereMesh(float radius = 1.f, size_t meshResolution = 3);

