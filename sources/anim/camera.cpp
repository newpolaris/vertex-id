#include "camera.h"

glm::mat4 Camera::GetProjectionMatrix() const
{
    if (isPerspective)
        return GetPerspectiveMatrix();
    else 
        return GetOrthogonalMatrix();
}

glm::mat4 Camera::GetOrthogonalMatrix() const
{
    const float tanHalfFovy = tan(glm::radians(Zoom) / static_cast<float>(2));
    const float sx = static_cast<float>(0.1) * Aspect / tanHalfFovy;
    const float sy = static_cast<float>(0.1) / (tanHalfFovy);

    return glm::ortho(-sx, sx, -sy, sy, -15.f, 15.f);
}

glm::mat4 Camera::GetPerspectiveMatrix() const
{
    // pass projection matrix to shader (note that in this case it could change every frame)
    glm::mat4 projection = glm::perspective(glm::radians(Zoom), Aspect, 0.1f, 100.0f);
    return projection;
}
