#include "imgui_input.h"
#include <glm/gtx/quaternion.hpp>

namespace ImGui {
    bool InputQuat(const char* label, glm::quat* quat, bool slideInput) 
    {
        // https://gamedev.stackexchange.com/questions/183771/euler-angle-and-quaternion-conversion-become-weird-when-yaw-is-bigger-than-90-de
        // https://www.andre-gaschler.com/rotationconverter/
        // zyx order
        glm::vec3 angle = glm::eulerAngles(*quat);
        // https://stackoverflow.com/questions/2084970/how-to-get-rid-of-minus-sign-from-signed-zero/14015445#14015445
        // remove some negative zero
        angle += glm::vec3(0.f);
        angle = glm::degrees(angle);

        bool edit = false;
        if (slideInput) 
            edit = ImGui::SliderFloat3(label, (float*)&angle, -180.f, 180.f);
        else
            edit = ImGui::InputFloat3(label, (float*)&angle);

        if (edit) {
            bool angleLimit = (angle.y < -90.f || angle.y > 90.f);
            // angle 제한은 에디트 중에만 수행
            if (angleLimit && !ImGui::IsItemDeactivatedAfterEdit())
                angle.y = glm::clamp(angle.y, -90.f, 90.f);
            *quat = glm::normalize(glm::quat(glm::radians(angle)));
            return true;
        }
        return false;
    }

    bool InputScale(const char* label, glm::vec3* vec, bool slideInput)
    {
        bool edit = false;
        if (slideInput)
            edit = ImGui::SliderFloat3(label, (float*)vec, 0.1f, 10.f);
        else
            edit = ImGui::InputFloat3(label, (float*)vec);

        if (glm::abs(vec->x) < FLT_EPSILON)
            vec->x = 0.001f;
        if (glm::abs(vec->y) < FLT_EPSILON)
            vec->y = 0.001f;
        if (glm::abs(vec->z) < FLT_EPSILON)
            vec->z = 0.001f;
        return edit;
    }
}
