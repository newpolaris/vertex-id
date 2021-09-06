#include "CameraManipulate.h"

#include <imgui.h>

#include "camera.h"

void CameraManipulate::UpdateCamera(Camera& camera, float inDeltaTime)
{
    // TODO: replace with MouseDelta (ShowDemoWindowMisc())
    bool bViewportPanelMouseOver = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
    if (bViewportPanelMouseOver)
        return;

    if (ImGui::IsMouseDown(1) && ImGui::IsMousePosValid()) {
        ImVec2 pos = ImGui::GetMousePos();

        if (firstMouse) {
            lastX = (float)pos.x;
            lastY = (float)pos.y;
            firstMouse = false;
        }

        float xoffset = (float)(pos.x - lastX);
        float yoffset = (float)(lastY - pos.y); // reversed since y-coordinates go from bottom to top

        lastX = (float)pos.x;
        lastY = (float)pos.y;

        camera.ProcessMouseMovement(xoffset, yoffset, true);
    }
    if (ImGui::IsMouseReleased(1)) {
        firstMouse = true;
    }

    auto keyPress = [](char c) -> bool {
        return ImGui::GetKeyPressedAmount(c, 0.05f, 0.020f);
    };

    if (keyPress('W')) camera.ProcessKeyboard(FORWARD, inDeltaTime);
    if (keyPress('S')) camera.ProcessKeyboard(BACKWARD, inDeltaTime);
    if (keyPress('A')) camera.ProcessKeyboard(LEFT, inDeltaTime);
    if (keyPress('D')) camera.ProcessKeyboard(RIGHT, inDeltaTime);
    if (keyPress('E')) camera.ProcessKeyboard(UP, inDeltaTime);
    if (keyPress('Q')) camera.ProcessKeyboard(DOWN, inDeltaTime);

    camera.ProcessMouseScroll(ImGui::GetIO().MouseWheel);
}
