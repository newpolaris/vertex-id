#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct StaticVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct AnimatedVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
    int32_t index[4] = { 0, 0, 0, 0 };
    float weight[4] = { 0.f, 0.f, 0.f, 0.f };
};

struct Submesh {
    uint32_t baseIndex = 0;
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    uint32_t baseVertex = 0;

    uint32_t MaterialIndex = 0;
    glm::mat4 Transform;
};

struct SubmeshName {
    std::string NodeName, MeshName;
};

struct MeshData {
    std::vector<StaticVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<Submesh> submeshes;
};

MeshData ObjLoader(const std::string& filepath);

std::string getFileAsString(const std::string& name);

void trace(const char* format...);

template <typename T>
void writeBinary(const std::string& name, const std::vector<T>& data) {
    FILE* file = fopen(name.c_str(), "wb");
    fwrite(data.data(), 1, data.size() * sizeof(T), file);
    fclose(file);
}

template <typename T>
std::vector<T> readBinary(const std::string& name) {
    std::vector<T> data;
    FILE* file = fopen(name.c_str(), "rb");
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    size_t count = len / sizeof(T);
    assert(len % sizeof(T) == 0);
    fseek(file, 0, SEEK_SET);
    data.resize(count);
    fread(data.data(), 1, data.size() * sizeof(T), file);
    fclose(file);
    return data;
}

template <typename T>
std::vector<T> ReadBuffer(unsigned char* ptr, uint32_t length) {
    assert(length % sizeof(T) == 0);
    uint32_t size = length / sizeof(T);
    std::vector<T> data(size);
    memcpy(data.data(), ptr, length);
    return data;
}

