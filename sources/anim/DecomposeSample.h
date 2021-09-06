#pragma once

#include <Application.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include "camera.h"
#include "CameraManipulate.h"
#include "transform_hier.h"

class DecomposeSample : public Application
{
	void Initialize();
	void Update(float inDeltaTime);
	void Render(float inAspectRatio);
	void Shutdown();
    void ImGui(nk_context* inContext);

public:

    hier::Transform m_A, m_B;

    float m_Rotation = 0.0f;
    Camera m_Camera;
    CameraManipulate m_CameraControl;
};

