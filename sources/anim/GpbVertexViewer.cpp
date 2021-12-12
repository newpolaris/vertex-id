#include "GpbVertexViewer.h"

#include <glad/glad.h>
#include <nanovg.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <string>

#include "json.hpp"
#include "ParseGpbXml.h"

#include "sample/Draw.h"
#include "sample/Uniform.h"
#include "sample/Shader.h"
#include "sample/IndexBuffer.h"
#include "sample/Texture.h"
#include "sample/Mesh.h"
#include "transform_hier.h"
#include "imgui_input.h"
#include "hud.h"

typedef std::vector<uint32_t> GridColType;

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
static bool useSnap = false;
static bool displayGizmo = true;
static float snap[3] = {1.f, 1.f, 1.f};
static float bounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
static float boundsSnap[] = {0.1f, 0.1f, 0.1f};
static bool boundSizing = false;
static bool boundSizingSnap = false;
static bool showVertexIDs = true;
static float fontSize = 15.f;

template <typename T, typename U>
void memcpy(std::vector<T>& dest, const std::vector<U>& src)
{
    static_assert(sizeof(T) == sizeof(U));
    dest.resize(src.size());
    std::memcpy(dest.data(), src.data(), sizeof(T)*src.size());
}

void MeshFromGpbXml(Mesh& outMesh, GpbMesh& attributes)
{
    std::vector<vec3>& positions = outMesh.GetPosition();
    std::vector<vec3>& normals = outMesh.GetNormal();
    std::vector<vec2>& texcoords = outMesh.GetTexCoord();
    std::vector<unsigned int>& indices = outMesh.GetIndices();

    memcpy(positions, attributes.positions);
    memcpy(normals, attributes.normals);
    memcpy(texcoords, attributes.texcoords);
    memcpy(indices, attributes.indices);

    outMesh.UpdateOpenGLBuffers();
}

glm::vec2 GetScreenPos(const glm::mat4& mvp, const glm::mat4& sp, const glm::vec4& p) 
{
    glm::vec4 ndc = mvp * p;
    ndc /= ndc.w;
    return glm::vec2(sp * ndc);
};

struct Context {
    ImDrawList* mDrawList;
    float mWidth = 0.f;
    float mHeight = 0.f;
    float mX = 0.f;
    float mY = 0.f;
    float mXMax = 0.f;
    float mYMax = 0.f;
    glm::mat4 mModel = glm::mat4(1.f);
    glm::mat4 mViewProject = glm::mat4(1.f);
    glm::mat4 mMVP = glm::mat4(1.f);
} gContext;

void SetRect(float x, float y, float width, float height) {
    gContext.mX = x;
    gContext.mY = y;
    gContext.mWidth = width;
    gContext.mHeight = height;
    gContext.mXMax = gContext.mX + gContext.mWidth;
    gContext.mYMax = gContext.mY + gContext.mXMax;
}

ImVec2 worldToPos(const glm::vec3& worldPos, const glm::mat4& mat) {
    glm::vec4 trans = mat * glm::vec4(worldPos, 1.0);
    trans *= 0.5f / trans.w;
    trans += glm::vec4(0.5f, 0.5f, 0.0f, 0.f);
    trans.y = 1.f - trans.y;
    trans.x *= gContext.mWidth;
    trans.y *= gContext.mHeight;
    trans.x += gContext.mX;
    trans.y += gContext.mY;
    return ImVec2(trans.x, trans.y);
}

glm::mat4 GetViewportTransform(glm::vec2 viewport) {
    auto w = viewport.x;
    auto h = viewport.y;

    // depth 0, 1 (not used :)
    return glm::mat4{
        w / 2, 0, 0, 0,
        0, -h / 2, 0, 0,
        0, 0, 1 - 0, 0,
        w / 2, h / 2, 0, 1
    };
}

void BeginFrame() {
    ImGuiIO& io = ImGui::GetIO();

    ImU32 flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::Begin("gizmo2", NULL, flags);
    gContext.mDrawList = ImGui::GetWindowDrawList();
    ImGui::End();
    ImGui::PopStyleColor(2);
}

void GpbVertexViewer::Initialize() {
    mShader = new Shader("Shaders/static.vert", "Shaders/flat.frag");
    mDisplayTexture = new Texture("Assets/uv.png");

    // LoadMeshes("Assets/captest5.xml");
    // LoadMeshes("Assets/custom_blendshape.xml");
    // LoadMeshes("Assets/RightUpper.xml");
    // LoadMeshes("Assets/RightU.xml");
    // LoadMeshes("Assets/eyelashesns.xml");
    LoadMeshes("Assets/parteyelashes.xml");

    mCamera.Pitch = -30;
    mCamera.updateCameraVectors();
    mCamera.Position = glm::vec3(0.f, 15.f, 20.f);

    UpdateMeshSelect(0);
    UpdateMeshBoundings(mMeshSelected);
}

void GpbVertexViewer::UpdateMeshSelect(int select) {
    if (mMeshes.size() == 0)
        mMeshSelected = -1;
    else
        mMeshSelected = glm::clamp(select, 0, (int)mMeshes.size() - 1);
}

void GpbVertexViewer::UpdateMeshBoundings(int select) {
    if (select == -1)
        return;

    float radius_new = glm::tan(mCamera.Zoom/2) * mCamera.Position.z;
    float radius = mMeshInformations[0].radius;
    float scale = radius_new / radius;

    // ImGui::Combo("Fit Type", &_fit_current, names.data(), names.size());

    // mesh�� center�� ȭ�� �߽����� �̵� ��Ű�� ����, center�� �̿�
    mParent = glm::translate(glm::mat4(1.f), -mMeshInformations[select].center);
    mModel = glm::scale(glm::mat4(1.f), glm::vec3(scale));
}

void GpbVertexViewer::LoadMeshes(const std::string& filename) 
{
    assert(std::filesystem::exists(filename));

    ParseGpbXml xml;
    xml.parse(filename);
    
    std::vector<Mesh> meshes;
    std::vector<MeshInformation> meshInformations;
    for (auto& src : xml.meshes) {
        Mesh mesh;
        MeshFromGpbXml(mesh, src);
        meshes.push_back(mesh);
        MeshInformation infomation = {src.id, src.min, src.max, src.center, src.radius };
        meshInformations.push_back(infomation);
    }
    mMeshes = meshes;
    mMeshInformations = meshInformations;
}

bool GpbVertexViewer::UpdateVertexGrid(const std::string& filename)
{
    if (!std::filesystem::exists(filename)) {
        printf("file not exist\n");
        return false;
    }
    auto lastWriteTime = std::filesystem::last_write_time(filename);
    if (mFilename == filename && lastWriteTime <= mGridWriteTime) {
        return true;
    }
    mFilename = filename;
    mGridWriteTime = lastWriteTime;

    using json = nlohmann::json;
    std::ifstream in(filename);

    json j;
    try {
        j = json::parse(in, nullptr, true, true);
    }
    catch (std::exception& ex) {
        printf("%s\n", ex.what());
        return false;
    }
    GridType grid = j;

    std::swap(mGrid, grid);

    VerifyGridData();

    return true;
}

void GpbVertexViewer::VerifyGridData()
{
    if (mMeshSelected < 0)
        return;

    // check if valid vertex id
    Mesh& mesh = mMeshes[mMeshSelected];
    const std::vector<vec3>& positions = mesh.GetPosition();

    for (size_t i = 0; i < mGrid.size(); i++) {
        const GridColType& col = mGrid[i];
        for (size_t j = 0; j < col.size(); j++) {
            if (col[j] < 0)
                printf("[%d][%d] has nagative value %d\n", (int)i, (int)j, col[j]);
            if (positions.size() <= col[j])
                printf("[%d][%d] has out of range value %d\n", (int)i, (int)j, col[j]);
        }
    }

    // vertify grid data
    GridColType arr;
    for (auto& col : mGrid)
        arr.insert(arr.end(), std::begin(col), std::end(col));
    std::sort(arr.begin(), arr.end());
    GridColType duplicated;
    for (size_t i = 0; i+1 < arr.size(); i++) {
        if (arr[i] == arr[i+1])
            duplicated.push_back(arr[i]);
    }
    for (auto& v : duplicated) {
        printf("array has duplicated value %d \n", v);
    }
    float fillRate = (float)arr.size() / positions.size() * 100;
    if (positions.size() < arr.size())
        printf("grid length overflow, expected %d <= actual %d\n", (int)positions.size(), (int)arr.size());
    printf("fill rate : %3.1f\n", fillRate);
}

void GpbVertexViewer::RenderVertexGrid()
{
    glm::mat4 view = mCamera.GetViewMatrix();
    glm::mat4 projection = mCamera.GetProjectionMatrix();
    glm::mat4 mvp = projection * view * mModel * mParent;

    ImGuiIO& io = ImGui::GetIO();
    glm::vec2 viewport(io.DisplaySize.x, io.DisplaySize.y);
    glm::mat4 toScreen = GetViewportTransform(viewport);

    Mesh& mesh = mMeshes[mMeshSelected];
    const std::vector<vec3>& positions = mesh.GetPosition();

    for (size_t i = 0; i < mGrid.size(); i++) {
        float r = (float)i / mGrid.size();
        const GridColType& col = mGrid[i];
        for (size_t j = 0; j < col.size(); j++) {
            float g = (float)j / col.size();
            ImU32 color = ImGui::GetColorU32(ImVec4(r, g, 0.f, 1.f));
            if (positions.size() <= col[j] || col[j] < 0)
                continue;

            const vec3& p = positions[col[j]];
            const glm::vec3 v(p.x, p.y, p.z);

            glm::vec2 posN = GetScreenPos(mvp, toScreen, glm::vec4(v.x, v.y, v.z, 1.f));
            gContext.mDrawList->AddCircleFilled(ImVec2(posN.x, posN.y), 5, color);
        }
    }
}

void GpbVertexViewer::Update(float inDeltaTime) {
    mCameraControl.UpdateCamera(mCamera, inDeltaTime);

    if (ImGui::IsKeyPressed('1'))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed('2'))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed('3'))
        mCurrentGizmoOperation = ImGuizmo::SCALE;
}

void GpbVertexViewer::Render(float inAspectRatio) {
    mCamera.SetAspectRatio(inAspectRatio);

    glm::mat4 view = mCamera.GetViewMatrix();
    glm::mat4 projection = mCamera.GetProjectionMatrix();

    glm::mat4 viewProjection = projection * view;

    DrawGrid(viewProjection);

    // TODO: OSX���� ���� �߰��Ͽ��� �ӽ÷� ������, �̰ɸ��鶧 vao�� 8���̶� Ȯ���� ���⼭�� 8�� �صе�
    // glBindVertexArray(8);

    mShader->Bind();

    Uniform<glm::mat4>::Set(mShader->GetUniform("model"), mModel*mParent);
    Uniform<glm::mat4>::Set(mShader->GetUniform("view"), view);
    Uniform<glm::mat4>::Set(mShader->GetUniform("projection"), projection);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);

    for (unsigned int i = 0, size = (unsigned int)mMeshes.size(); i < size; ++i) {
        if (i != mMeshSelected)
            continue;

        // �ʿ��� �͸� index�� �־ bind
        int weights = -1;
        int influences = -1;
        int normals = -1;
        int texcoords = -1;

        mMeshes[i].Bind(mShader->GetAttribute("position"), normals, texcoords, weights, influences);
        mMeshes[i].Draw();
        mMeshes[i].UnBind(mShader->GetAttribute("position"), normals, texcoords, weights, influences);
    }
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);

    // mDisplayTexture->UnSet(0);
    mShader->UnBind();
}

void GpbVertexViewer::ImGui(nk_context* inContext) {
    ImGui::Begin("Control");
    ImGui::Text("Camera:");
    ImGui::InputFloat3("Pos", (float*)&mCamera.Position);
    ImGui::Separator();

    int meshCount = (int)mMeshInformations.size();
    std::vector<const char*> meshNames;
    for (auto& info : mMeshInformations)
        meshNames.push_back(info.id.c_str());
    if (ImGui::Combo("name", &mMeshSelected, meshNames.data(), meshCount)) {
        UpdateMeshBoundings(mMeshSelected);
        VerifyGridData();
    }

    ImGui::Separator();
    ImGui::Checkbox("Display Gizmo", &displayGizmo);
    ImGui::Checkbox("Show Vertex IDs", &showVertexIDs);
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
    ImGui::Separator();

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
    ImGui::End();

    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents((float*)&mModel, matrixTranslation, matrixRotation, matrixScale);
    ImGui::InputFloat3("Tr", matrixTranslation);
    ImGui::InputFloat3("Rt", matrixRotation);
    ImGui::InputFloat3("Sc", matrixScale);
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&mModel);

    if (displayGizmo) {
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        float rw = (float)ImGui::GetWindowWidth();
        float rh = (float)ImGui::GetWindowHeight();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::Manipulate(glm::value_ptr(mCamera.GetViewMatrix()),
            glm::value_ptr(mCamera.GetProjectionMatrix()),
            mCurrentGizmoOperation,
            mCurrentGizmoMode,
            glm::value_ptr(mModel),
            NULL,
            useSnap ? &snap[0] : NULL,
            boundSizing ? bounds : NULL,
            boundSizingSnap ? boundsSnap : NULL);
    }

    glm::mat4 view = mCamera.GetViewMatrix();
    glm::mat4 projection = mCamera.GetProjectionMatrix();
    glm::mat4 mvp = projection * view * mModel * mParent;

    ImGuiIO& io = ImGui::GetIO();
    glm::vec2 viewport(io.DisplaySize.x, io.DisplaySize.y);

    glm::mat4 toScreen = GetViewportTransform(viewport);

    if (showVertexIDs) {
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        const float screenRotateSize = 0.06f;
        float mRadiusSquareCenter = screenRotateSize * gContext.mHeight;

        BeginFrame();
        SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImU32 white = ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f));

        static int i = 0;
        if (i++ % 30 == 0)
            UpdateVertexGrid("Assets/parteyelahedown2.json");
        RenderVertexGrid();

        Mesh& mesh = mMeshes[mMeshSelected];
    #if 0
        const std::vector<vec3>& positions = mesh.GetPosition();
        for (size_t i = 0; i < positions.size(); i++) {
            vec3 v = positions[i];
            glm::vec2 posN = GetScreenPos(mvp, toScreen, glm::vec4(v.x, v.y, v.z, 1.f));
            gContext.mDrawList->AddText(ImVec2(posN.x, posN.y), white, std::to_string(i).c_str());
        }
    #else // dot ���� �˻�
        std::unordered_map<glm::vec3, std::vector<uint32_t>> map;
        const std::vector<vec3>& positions = mesh.GetPosition();
        for (size_t i = 0; i < positions.size(); i++) {
            const vec3& p = positions[i];
            const glm::vec3 position(p.x, p.y, p.z);
            auto it = map.find(position);
            if (it == map.end()) {
                auto ret = map.insert({position, {}});
                it = ret.first;
            }
            it->second.push_back((uint32_t)i);
        }

        std::vector<std::pair<glm::vec3, std::vector<uint32_t>>> list;
        for (auto& v : map) {
            std::sort(v.second.begin(), v.second.end());
            auto it = std::unique(v.second.begin(), v.second.end());
            list.push_back({v.first, std::vector<uint32_t>(v.second.begin(), it)});
        }
        for (size_t i = 0; i < list.size(); i++) {
            glm::vec3 v = list[i].first;
            std::stringstream ss;
            if (list[i].second.size() > 1) {
                ss << "{";
                for (auto id : list[i].second)
                    ss << id << ", ";
                ss << "}";
            } else {
                ss << list[i].second.front();
            }
            glm::vec2 posN = GetScreenPos(mvp, toScreen, glm::vec4(v.x, v.y, v.z, 1.f));
            gContext.mDrawList->AddText(ImVec2(posN.x, posN.y), white, ss.str().c_str());
        }
    #endif
    }

    static std::string s_AssetsDirectory = "Assets";

    ImGui::Begin("Content Browser");
    for (auto& p : std::filesystem::directory_iterator(s_AssetsDirectory)) {
        std::string path = p.path().string();
        auto p0 = p.path().filename().string();
        auto p1 = p.path().stem().string();
        ImGui::Text("Filename: %s", p0.c_str());
        ImGui::Text("Stem: %s", p1.c_str());
        if (p.is_directory()) {
            if (ImGui::Button(path.c_str())) {
            }
        }
    }

    ImGui::End();
}

void GpbVertexViewer::NanoGui(NVGcontext* inContext) {
    glm::mat4 view = mCamera.GetViewMatrix();
    glm::mat4 projection = mCamera.GetProjectionMatrix();
    glm::mat4 mvp = projection * view * mModel;

    ImGuiIO& io = ImGui::GetIO();
    glm::vec2 viewport(io.DisplaySize.x, io.DisplaySize.y);

    glm::mat4 toScreen = GetViewportTransform(viewport);

#if 0
    if (showVertexIDs) {
        nvgFontFace(inContext, "sans");
        nvgFontSize(inContext, fontSize);
        nvgFillColor(inContext, nvgRGBA(255, 255, 255, 255));
        for (Mesh& mesh : mMeshes) {
            std::vector<vec3> positions = mesh.GetPosition();
            for (size_t i = 0; i < positions.size(); i++) {
                vec3 v = positions[i];
                glm::vec2 posN = GetScreenPos(mvp, toScreen, glm::vec4(v.x, v.y, v.z, 1.f));
                nvgText(inContext, posN.x, posN.y, std::to_string(i).c_str(), NULL);
            }
        }
    }
#endif
}

void GpbVertexViewer::Shutdown() {
    delete mShader;
    delete mDisplayTexture;
    delete mVertexPositions;
    delete mVertexNormals;
    delete mVertexTexCoords;
    delete mIndexBuffer;
}