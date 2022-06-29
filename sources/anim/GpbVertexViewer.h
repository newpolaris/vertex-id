#pragma once

#include <filesystem>
#include <Application.h>
#include <glm/glm.hpp>
#include <string>
#include "sample/Attribute.h"
#include "sample/Mesh.h"
#include "camera.h"
#include "json.hpp"
#include "CameraManipulate.h"

class Shader;
class IndexBuffer;
class Texture;

struct MeshInformation {
    std::string id;

    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 center;
	float radius;
};

enum class EyelashLocationType {
    LeftUpper, LeftLower, RightUpper, RightLower, LeftCover, RightCover
};

// map TaskState values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM(EyelashLocationType, {
    {EyelashLocationType::LeftUpper, "LeftUpper"},
    {EyelashLocationType::LeftLower, "LeftLower"},
    {EyelashLocationType::RightUpper, "RightUpper"},
    {EyelashLocationType::RightLower, "RightLower"},
    {EyelashLocationType::LeftCover, "LeftCover"},
    {EyelashLocationType::RightCover, "RightCover"},
})

using MeshColType = std::vector<std::uint32_t>;
using MeshVertexGrid = std::vector<MeshColType>;

class MeshParam {
public:
    // ¼Ó´«½ç ºÎÂøµÉ À§Ä¡
    EyelashLocationType location;
    // mesh »óÀÇ vertex id¸¦ 2D ±×¸®µå»ó¿¡ ³ªÅ¸³¿
    MeshVertexGrid grid;
};

using MeshParamMap = std::map<std::string, MeshParam>;

class GpbVertexViewer : public Application {
protected:
	Shader* mShader;
	Attribute<glm::vec3>* mVertexPositions;
	Attribute<glm::vec3>* mVertexNormals;
	Attribute<glm::vec2>* mVertexTexCoords;
	IndexBuffer* mIndexBuffer;
	Texture* mDisplayTexture;

    size_t s_frameCounter = 0;

    float mRadiusScale = 0.5f;
    bool mbUpdateParamSelected = true;
    bool mbUpdateMeshCenter = true;
    int mMeshSelected = -1;
    int mGpbSelected = 0;

    Camera mCamera;
    CameraManipulate mCameraControl;

	std::vector<Mesh> mMeshes;
	std::vector<MeshInformation> mMeshInformations;

    glm::mat4 mParent;
    glm::mat4 mModel;

    int mMeshParamSelected = -1;
    std::vector<std::string> mMeshParamNames;
    MeshParamMap mMeshParam; 

    std::string mFilename;
    std::filesystem::file_time_type mMeshParamWriteTime;

public:
	void Initialize();
	void Update(float inDeltaTime);
	void Render(float inAspectRatio);
	void Shutdown();
    void ImGui(nk_context* inContext);
    void ImGuiContents(nk_context* inContext);
    void NanoGui(NVGcontext* inContext);

    bool LoadMeshes(const std::string& filename);
    bool LoadGpbXml(const std::string& filename);
    bool LoadGpb(const std::string& filename);

    bool UpdateVertexGrid();
    bool UpdateVertexGridJson(const std::string& filename);
    void RenderVertexGrid();

    void UpdateGpbSelect(int select);
    void UpdateMeshSelect(int select);
    void UpdateMeshBoundings(int select);

    void VerifyGridData();
};
