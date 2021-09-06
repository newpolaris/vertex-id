#pragma once

#include <imgui.h>
#include <glm/glm.hpp>

namespace ImGui {
    bool InputQuat(const char* label, glm::quat* quat, bool slideInput);
    bool InputScale(const char* label, glm::vec3* vec, bool slideInput);
}
