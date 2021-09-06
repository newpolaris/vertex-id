#include "DecomposeSample.h"
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>
#include <ImGuizmo.h>

#include "hud.h"
#include "transform_hier.h"
#include "imgui_input.h"

static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
{
    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat orientation;
    glm::decompose(transform, scale, orientation, translation, skew, perspective);

    return {translation, orientation, scale};
}

void DecomposeSample::Initialize() 
{
    m_Camera.Pitch = -30;
    m_Camera.updateCameraVectors();
    m_Camera.Position = glm::vec3(0.f, 15.f, 20.f);

    m_A = hier::GetTransform(glm::vec3(0.f, 5.f, 0.f), glm::vec3(0.f), glm::vec3(2, 0.25, 0.25));
    m_B = hier::GetTransform(glm::vec3(2.f, 2.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1, 1, 1), &m_A);
}

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
static bool useSnap = false;
static bool displayGizmo = true;
static float snap[3] = {1.f, 1.f, 1.f};
static float bounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
static float boundsSnap[] = {0.1f, 0.1f, 0.1f};
static bool boundSizing = false;
static bool boundSizingSnap = false;
static bool worldDecomp = false;
static bool slideInput = false;

static int selectTransform = 0;

void DecomposeSample::Update(float inDeltaTime) 
{
    m_Rotation += inDeltaTime * 45.0f;
    while (m_Rotation > 360.0f) {
        m_Rotation -= 360.0f;
    }
    m_CameraControl.UpdateCamera(m_Camera, inDeltaTime);

    if (ImGui::IsKeyPressed('1'))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed('2'))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed('3'))
        mCurrentGizmoOperation = ImGuizmo::SCALE;
}

glm::mat4 GetWorldMatrix(const hier::Transform& transform, bool bDecomp) {
    if (bDecomp)
        return GetWorldDecomposeMatrix(transform);
    return GetWorldMatrix(transform);
}

void DecomposeSample::Render(float inAspectRatio) 
{
    m_Camera.SetAspectRatio(inAspectRatio);

    auto view = m_Camera.GetViewMatrix();
    auto projection = m_Camera.GetProjectionMatrix();
    auto viewProjection = projection * view;
    DrawGrid(viewProjection);

    auto rotation = glm::angleAxis(glm::radians(m_Rotation), glm::vec3(0.f, 1.f, 0.f));
    auto model = glm::toMat4(rotation);

    auto Cube = [&](const glm::mat4 viewProj, const glm::mat4& model) {
        DrawCube(viewProj * model, glm::vec3(0.f), glm::vec3(1.f));
    };

    Cube(viewProjection, GetWorldMatrix(m_A, worldDecomp));
    Cube(viewProjection, GetWorldMatrix(m_B, worldDecomp));
}

void DecomposeSample::Shutdown() {
}

void DecomposeSample::ImGui(nk_context* inContext) {

    ImGui::Begin("Control");
    ImGui::Text("Camera:");
    ImGui::InputFloat3("Pos", (float*)&m_Camera.Position);
    ImGui::Separator();

    ImGui::Checkbox("Display Gizmo", &displayGizmo);
    ImGui::Checkbox("Decompose", &worldDecomp);
    ImGui::Checkbox("Slide Input", &slideInput);
    ImGui::Separator();

    ImGui::Text("Guizmo Option:");
    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
        mCurrentGizmoOperation = ImGuizmo::SCALE;

    if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
        mCurrentGizmoMode = ImGuizmo::LOCAL;
    ImGui::SameLine();
    if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
        mCurrentGizmoMode = ImGuizmo::WORLD;

#if 0
    ImGui::Checkbox("", &useSnap);
    ImGui::SameLine();
    switch (mCurrentGizmoOperation)
    {
    case ImGuizmo::TRANSLATE:
        ImGui::InputFloat3("Snap", &snap[0]);
        break;
    case ImGuizmo::ROTATE:
        ImGui::InputFloat("Angle Snap", &snap[0]);
        break;
    case ImGuizmo::SCALE:
        ImGui::InputFloat("Scale Snap", &snap[0]);
        break;
    }
    ImGui::Checkbox("Bound Sizing", &boundSizing);
    if (boundSizing)
    {
        ImGui::PushID(3);
        ImGui::Checkbox("", &boundSizingSnap);
        ImGui::SameLine();
        ImGui::InputFloat3("Snap", boundsSnap);
        ImGui::PopID();
    }
#endif
    ImGui::Separator();

    ImGui::Text("Selected Node:");
    if (ImGui::RadioButton("Empty", selectTransform == -1))
        selectTransform = -1;
    ImGui::SameLine();
    if (ImGui::RadioButton("Parent", selectTransform == 0))
        selectTransform = 0;
    ImGui::SameLine();
    if (ImGui::RadioButton("Child", selectTransform == 1))
        selectTransform = 1;

    ImGui::Separator();


    ImGui::Text("World Transform Edit:");

    hier::Transform* transform = nullptr;
    if (selectTransform == 0)
        transform = &m_A;
    else if (selectTransform == 1)
        transform = &m_B;
    if (transform != nullptr)
    {
        hier::Transform* select = transform;
        glm::vec3 translate = hier::GetGlobalPosition(*select);
        if (ImGui::InputFloat3("Translate", (float*)&translate))
            hier::SetGlobalPosition(select, translate);
        glm::quat rotateQuat = hier::GetGlobalRotation(*select);
        if (ImGui::InputQuat("Rotate", &rotateQuat, slideInput)) {
            hier::SetGlobalRotation(select, rotateQuat);
        }
        glm::vec3 scale = GetGlobalLossyScale(*select);
        if (ImGui::InputFloat3("Scale", (float*)&scale)) {
            hier::SetGlobalScale(select, scale);
        }
    }
    ImGui::End();

    ImGui::Begin("Tree");
    ImGui::SetNextTreeNodeOpen(true);
    if (ImGui::TreeNode("Parent")) {
        ImGui::InputFloat3("Tr", (float*)&m_A.translate);
        ImGui::InputQuat("Rt", &m_A.rotation, slideInput);
        ImGui::InputScale("Sc", &m_A.scale, slideInput);
        ImGui::SetNextTreeNodeOpen(true);
        if (ImGui::TreeNode("Child")) {
            ImGui::InputFloat3("Tr", (float*)&m_B.translate);
            ImGui::InputQuat("Rt", &m_B.rotation, slideInput);
            ImGui::InputScale("Sc", &m_B.scale, slideInput);
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::Text("glm::decompose");
    if (transform != nullptr)
    {
        hier::Transform* select = transform;
        glm::mat4 transform = hier::GetWorldMatrix(*select);
        glm::mat4 localTransform = hier::ToMatrix(*select);
        {
            auto [translation, rotation, scale] = GetTransformDecomposition(transform);
            glm::vec3 rotate = glm::degrees(glm::eulerAngles(glm::normalize(rotation)));
            ImGui::Text(" World Transform");
            ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
            ImGui::Text("  Rotate: %.2f, %.2f, %.2f", rotate.x, rotate.y, rotate.z);
            ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
        }
        {
            auto [translation, rotation, scale] = GetTransformDecomposition(localTransform);
            glm::vec3 rotate = glm::degrees(glm::eulerAngles(glm::normalize(rotation)));
            ImGui::Text(" Local Transform");
            ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
            ImGui::Text("  Rotate: %.2f, %.2f, %.2f", rotate.x, rotate.y, rotate.z);
            ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
        }
    }

    ImGui::End();

    if (transform && displayGizmo) {
        auto entityTransform = hier::GetWorldMatrix(*transform);

        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        float rw = (float)ImGui::GetWindowWidth();
        float rh = (float)ImGui::GetWindowHeight();
        ImGuizmo::SetOrthographic(false);

        ImGuizmo::Manipulate(glm::value_ptr(m_Camera.GetViewMatrix()),
            glm::value_ptr(m_Camera.GetProjectionMatrix()),
            mCurrentGizmoOperation,
            mCurrentGizmoMode,
            glm::value_ptr(entityTransform),
            NULL,
            useSnap ? &snap[0] : NULL,
            boundSizing ? bounds : NULL,
            boundSizingSnap ? boundsSnap : NULL);

        glm::mat4 inverse = glm::mat4(1.f);
        if (transform->parent)
            inverse = glm::inverse(hier::GetWorldMatrix(*transform->parent));
        entityTransform = inverse * entityTransform;

    #if 0
        glm::vec3 translate, rotate, scale;
        ImGuizmo::DecomposeMatrixToComponents(
            glm::value_ptr(entityTransform),
            glm::value_ptr(translate),
            glm::value_ptr(rotate),
            glm::value_ptr(scale));

        glm::quat rotationQuat = glm::normalize(glm::quat(glm::radians(rotate)));
    #else
        auto [translate, rotationQuat, scale] = GetTransformDecomposition(entityTransform);
    #endif

        *transform = hier::GetTransform(translate, glm::normalize(rotationQuat), scale, transform->parent);
    }
}
