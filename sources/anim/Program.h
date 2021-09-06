#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "BufferLayout.h"

struct ProgramDesc
{
    std::string vertexFilepath;
    std::string fragemntFilepath;
    BufferLayoutDesc layout;
};

struct Program
{
    uint32_t vertex = 0;
    uint32_t framgment = 0;
    uint32_t id = 0;

    uint64_t timeStamp = 0;

    ProgramDesc desc;

    bool setUniform(const std::string& name, int32_t v) const;
    bool setUniform(const std::string& name, float v) const;
    bool setUniform(const std::string& name, const glm::vec2& v) const;
    bool setUniform(const std::string& name, const glm::vec3& v) const;
    bool setUniform(const std::string& name, const glm::vec4& v) const;
    bool setUniform(const std::string& name, const glm::vec4* v, size_t count) const;
    bool setUniform(const std::string& name, const glm::mat3& v) const;
    bool setUniform(const std::string& name, const glm::mat4& v) const;
    bool setUniform(const std::string& name, const glm::mat4* v, size_t count) const;
};

typedef std::shared_ptr<struct Program> ProgramPtr;

ProgramPtr createProgram(const ProgramDesc& desc);
void destroyProgram(ProgramPtr& program);
void useProgram(const ProgramPtr& program);
void initProgramTextures(const ProgramPtr& program, const std::initializer_list<std::string>& textureNames);
