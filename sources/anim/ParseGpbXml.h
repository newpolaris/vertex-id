#include "pugixml.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>

namespace pugi {
    class xpath_node;
}

struct GpbMesh 
{
    bool parse(pugi::xpath_node& node);

    std::string id;

    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 center;
    float radius;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<uint32_t> indices;
};

struct ParseGpbXml
{
    bool parse(const std::string& filename);
    std::vector<GpbMesh> meshes;
};

