#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace hier {
    struct Transform
    {
        glm::vec3 translate;
        glm::quat rotation;
        glm::vec3 scale;

        Transform* parent = nullptr;
    };

    Transform GetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, Transform* parent = nullptr);
    Transform GetTransform(glm::vec3 pos, glm::quat rot, glm::vec3 scale, Transform* parent = nullptr);
    glm::mat4 ToMatrix(Transform transform);
    glm::mat4 GetWorldMatrix(const Transform& transform);
    glm::mat4 GetWorldDecomposeMatrix(const Transform& transform);
    glm::vec3 GetGlobalPosition(const Transform& t);
    glm::mat3 GetGlobalRotationAndScale(const Transform& t);
    glm::vec3 GetGlobalLossyScale(const Transform& t);
    glm::quat GetGlobalRotation(const Transform& t);
    void SetGlobalPosition(Transform* t, glm::vec3 position);
    void SetGlobalRotation(Transform* t, const glm::quat& rotation);
    void SetGlobalScale(Transform* t, const glm::vec3& scale);
    glm::vec3 InverseTransformPoint(const Transform& t, glm::vec3 point);

} // hierachy

