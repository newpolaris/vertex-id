#include "transform_hier.h"

#include <glm/gtx/quaternion.hpp>
#include "decompose.h"

namespace hier {

    Transform GetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, Transform* parent)
    {
        Transform form;
        form.parent = parent;
        form.translate = pos;
        form.rotation = glm::normalize(glm::quat(glm::radians(rot)));
        form.scale = scale;
        return form;
    }

    Transform GetTransform(glm::vec3 pos, glm::quat rot, glm::vec3 scale, Transform* parent)
    {
        Transform form;
        form.parent = parent;
        form.translate = pos;
        form.rotation = rot;
        form.scale = scale;
        return form;
    }

    glm::mat4 ToMatrix(Transform transform) {
        // First, extract the rotation basis of the transform
        glm::vec3 x = transform.rotation * glm::vec3(1, 0, 0); // Vec3 * Quat (right glm::vec3)
        glm::vec3 y = transform.rotation * glm::vec3(0, 1, 0); // Vec3 * Quat (up glm::vec3)
        glm::vec3 z = transform.rotation * glm::vec3(0, 0, 1); // Vec3 * Quat (forward glm::vec3)

        // Next, scale the basis glm::vec3s
        x = x * transform.scale.x; // glm::vec3 * float
        y = y * transform.scale.y; // glm::vec3 * float
        z = z * transform.scale.z; // glm::vec3 * float

        // Extract the position of the transform
        glm::vec3 t = transform.translate;

        return glm::mat4(
            x.x, x.y, x.z, 0, // X basis (& Scale)
            y.x, y.y, y.z, 0, // Y basis (& scale)
            z.x, z.y, z.z, 0, // Z basis (& scale)
            t.x, t.y, t.z, 1  // Position
        );
    }

    glm::mat4 GetWorldMatrix(const Transform& transform) {
        glm::mat4 localMatrix = ToMatrix(transform);
        glm::mat4 worldMatrix = localMatrix;

        if (transform.parent != NULL) {
            glm::mat4 parentMatrix = GetWorldMatrix(*transform.parent);
            worldMatrix = parentMatrix * localMatrix;
        }
        return worldMatrix;
    }

    glm::mat4 GetWorldDecomposeMatrix(const Transform& transform) 
    {
        glm::mat4 mat = GetWorldMatrix(transform);

        // col-major to row-major 
        mat = glm::transpose(mat);

        AffineParts parts;
        decomp_affine(*(HMatrix*)&mat, &parts);

        glm::vec3 trans(parts.t.x, parts.t.y, parts.t.z);
        glm::quat rot = *(glm::quat*)&parts.q;
        rot = glm::quat(rot.w, rot.x, rot.y, rot.z);
        glm::vec3 s(parts.k.x, parts.k.y, parts.k.z);
        Transform newtransfrom = GetTransform(trans, glm::normalize(rot), s, nullptr);
        return GetWorldMatrix(newtransfrom);
    }

    glm::quat GetGlobalRotation(const Transform& t)
    {
        glm::quat rotation = t.rotation;

        Transform *iterator = t.parent;
        while (iterator != nullptr) {
            rotation = iterator->rotation * rotation;
            iterator = iterator->parent;
        }
        return rotation;
    }

    glm::vec3 GetGlobalPosition(const Transform& t)
    {
        glm::vec3 worldPos = t.translate;

        Transform *iterator = t.parent;
        while (iterator != nullptr) {
            worldPos = iterator->scale * worldPos;
            worldPos = iterator->rotation * worldPos;
            worldPos += iterator->translate;
            iterator = iterator->parent;
        }
        return worldPos;
    }

    glm::mat3 GetGlobalRotationAndScale(const Transform& t) 
    {
        glm::mat3 scaleMat(
            t.scale.x, 0.f, 0.f,
            0.f, t.scale.y, 0.f,
            0.f, 0.f, t.scale.z);
        glm::mat3 rotationMat = glm::toMat3(t.rotation);
        glm::mat3 worldRS = rotationMat * scaleMat;

        if (t.parent != NULL) {
            glm::mat3 parentRS = GetGlobalRotationAndScale(*t.parent);
            worldRS = parentRS * worldRS;
        }
        return worldRS;
    }

    glm::vec3 GetGlobalLossyScale(const Transform& t)
    {
        // Find inverse global rotation (rotation only) of transform
        glm::quat rotation = GetGlobalRotation(t);
        glm::mat3 invRotation = glm::toMat3(glm::inverse(rotation));

        // Find global rotation and scale of transform
        glm::mat3 scaleAndRotation = GetGlobalRotationAndScale(t);

        // Remove global rotation from rotation & scale
        glm::mat3 scaleAndSkew = invRotation * scaleAndRotation;
        return glm::vec3(scaleAndSkew[0][0], scaleAndSkew[1][1], scaleAndSkew[2][2]);
    }

    void SetGlobalRotation(Transform* t, const glm::quat& rotation) {
        if (t->parent == NULL) {
            t->rotation = rotation;
            return;
        }

        glm::quat parentGlobal = GetGlobalRotation(*t->parent);
        glm::quat invParentGlobal = glm::inverse(parentGlobal);

        t->rotation = invParentGlobal * rotation;
    }

    glm::vec3 InverseTransformPoint(const Transform& t, glm::vec3 point) {
        // Recursive function, apply inverse of parent transform first
        if (t.parent != NULL) {
            point = InverseTransformPoint(*t.parent, point);
        }

        // First, apply the inverse translation of the transform
        point = point - t.translate;

        // Next, apply the inverse rotation of the transform
        glm::quat invRot = glm::inverse(t.rotation);
        // point = point * invRot;
        point = invRot * point;

        // Finally, apply the inverse scale
        point = point / t.scale; // Component wise vector division
        return point;
    }

    void SetGlobalPosition(Transform* t, glm::vec3 position) 
    {
        if (t->parent != NULL) {
            position = InverseTransformPoint(*t->parent, position);
        }

        t->translate = position;
    }

    void SetGlobalScaleFromRotationScaleMatrix(Transform* t, const glm::mat3& rsMat) 
    {
        // Reset scale to 1, do this so we can get the  global rotation and scale 
        // without the scale of this transform but with the scale of all parents
        t->scale = glm::vec3(1, 1, 1); 

        // Find inverse global matrix (scale of all parent tranforms, not this one)
        glm::mat3 globalRS = GetGlobalRotationAndScale(*t);
        glm::mat3 inverseRS = glm::inverse(globalRS);

        // Bring the rotation scale matrix into local space
        glm::mat3 localRS = inverseRS * rsMat;
        
        // Main diagonal is the new scale
        t->scale = glm::vec3(localRS[0][0], localRS[1][1], localRS[2][2]);
    }

    void SetGlobalScale(Transform* t, const glm::vec3& scale) {
        glm::quat globalRotation = GetGlobalRotation(*t);

        glm::vec3 x = globalRotation * glm::vec3(scale.x, 0, 0);
        glm::vec3 y = globalRotation * glm::vec3(0, scale.y, 0);
        glm::vec3 z = globalRotation * glm::vec3(0, 0, scale.z);
        
        glm::mat3 rotationAndScaleMat = glm::mat3(
            x.x, x.y, x.z,
            y.x, y.y, y.z,
            z.x, z.y, z.z
        );

        SetGlobalScaleFromRotationScaleMatrix(t, rotationAndScaleMat);
    }

} // namespace hier
