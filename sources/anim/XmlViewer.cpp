#include "XmlViewer.h"

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

std::vector<uint32_t> gpbXmlVertexId = {
    42, 67, 69, 72, 90, 91,
    65, 43, 44, 47, 62, 61,

    66, 68, 70, 71, 88, 89,
    39, 40, 45, 46, 60, 59,

    129, 75, 74, 73, 84, 87,
    38, 41, 49, 48, 55, 58,

    36, 76, 77, 78, 83, 85,
    128, 37, 50, 51, 53, 56,

    35, 81, 80, 79, 82, 86,
    126, 33, 34, 52, 54, 57,

    32, 29, 27, 28, 64, 63,
    123, 124, 125, 127, 93, 92,

    31, 26, 21, 20, 18, 19,
    122, 120, 121, 100, 95, 94,

    30, 25, 22, 17, 15, 14,
    118, 119, 106, 101, 97, 96,

    131, 24, 23, 16, 13, 12,
    117, 114, 107, 102, 99, 98,

    1, 3, 5, 6, 8, 10,
    115, 112, 108, 105, 104, 103,

    0, 2, 4, 7, 9, 11,
    116, 113, 111, 110, 109, 130
};

typedef std::vector<uint32_t> GridColType;

GridType upeyelashGrid {
    { 186, 211, 63, 61, 62, 193, 192, 190, 191, 149, 147, 148, 175, 174, 172, 173 },
    { 184, 208, 212, 32, 28, 30, 70, 69, 67, 68, 118, 114, 116, 156, 155, 153, 154 },
    { 182, 204, 207, 31, 29, 27, 54, 53, 51, 52, 117, 115, 113, 140, 139, 137, 138 }, 
    { 180, 199, 203, 22, 21, 20, 24, 23, 26, 25, 108, 107, 106, 110, 109, 112, 111 },
    { 177, 195, 200, 13, 10, 5, 0, 2, 6, 8, 96, 98, 91, 86, 88, 92, 94 },
    { 178, 196, 197, 11, 12, 4, 1, 3, 7, 9, 97, 99, 90, 87, 89, 93, 95 },
    { 179, 198, 201, 36, 37, 33, 34, 35, 389, 39, 122, 123, 119, 120, 121, 124, 125 },
    { 181, 202, 205, 49, 50, 47, 46, 48, 60, 59, 135, 136, 133, 132, 134, 146, 145 },
    { 183, 206, 209, 58, 57, 56, 55, 18, 15, 17, 144, 143, 142, 141, 104, 101, 103 },
    { 185, 210, 213, 74, 73, 72, 71, 19, 16, 14, 150, 159, 158, 157, 105, 102, 100 },
    { 187, 214, 217, }
};

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

void XmlViewer::Initialize() {
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

void XmlViewer::UpdateMeshSelect(int select) {
    if (mMeshes.size() == 0)
        mMeshSelected = -1;
    else
        mMeshSelected = glm::clamp(select, 0, (int)mMeshes.size() - 1);
}

void XmlViewer::UpdateMeshBoundings(int select) {
    if (select == -1)
        return;

    float radius_new = glm::tan(mCamera.Zoom/2) * mCamera.Position.z;
    float radius = mMeshInformations[0].radius;
    float scale = radius_new / radius;

    // ImGui::Combo("Fit Type", &_fit_current, names.data(), names.size());

    // mesh의 center를 화면 중심으로 이동 시키기 위해, center를 이용
    mParent = glm::translate(glm::mat4(1.f), -mMeshInformations[select].center);
    mModel = glm::scale(glm::mat4(1.f), glm::vec3(scale));
}

void XmlViewer::LoadMeshes(const std::string& filename) 
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

bool XmlViewer::UpdateVertexGrid(const std::string& filename)
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

void XmlViewer::VerifyGridData()
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
    for (auto v : duplicated) {
        printf("array has duplicated value %d \n", v);
    }
    float fillRate = (float)arr.size() / positions.size();
    if (positions.size() < arr.size())
        printf("grid length overflow, expected %d <= actual %d\n", (int)positions.size(), (int)arr.size());
    printf("fill rate : %3.1f%\n", fillRate*100.f);
}

void XmlViewer::RenderVertexGrid()
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

void XmlViewer::Update(float inDeltaTime) {
    mCameraControl.UpdateCamera(mCamera, inDeltaTime);

    if (ImGui::IsKeyPressed('1'))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed('2'))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed('3'))
        mCurrentGizmoOperation = ImGuizmo::SCALE;
}

void XmlViewer::Render(float inAspectRatio) {
    mCamera.SetAspectRatio(inAspectRatio);

    glm::mat4 view = mCamera.GetViewMatrix();
    glm::mat4 projection = mCamera.GetProjectionMatrix();

    glm::mat4 viewProjection = projection * view;

    DrawGrid(viewProjection);

    // TODO: !!
    glBindVertexArray(8);

    mShader->Bind();

    Uniform<glm::mat4>::Set(mShader->GetUniform("model"), mModel*mParent);
    Uniform<glm::mat4>::Set(mShader->GetUniform("view"), view);
    Uniform<glm::mat4>::Set(mShader->GetUniform("projection"), projection);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);

    for (unsigned int i = 0, size = (unsigned int)mMeshes.size(); i < size; ++i) {
        if (i != mMeshSelected)
            continue;

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

void XmlViewer::ImGui(nk_context* inContext) {
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
    #else // dot 으로 검사
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
            it->second.push_back(i);
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
}

void XmlViewer::NanoGui(NVGcontext* inContext) {
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

void XmlViewer::Shutdown() {
    delete mShader;
    delete mDisplayTexture;
    delete mVertexPositions;
    delete mVertexNormals;
    delete mVertexTexCoords;
    delete mIndexBuffer;
}
