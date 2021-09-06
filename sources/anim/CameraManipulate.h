#pragma once

class Camera;

struct CameraManipulate
{
    void UpdateCamera(Camera& camera, float inDeltaTime);

    float lastX = 0.f;
    float lastY = 0.f;
    bool firstMouse = true;
};

