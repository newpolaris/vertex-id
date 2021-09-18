#pragma once

#include <filesystem>
#include <Application.h>
#include <glm/glm.hpp>
#include "sample/Attribute.h"
#include "sample/Mesh.h"
#include "camera.h"
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

typedef std::vector<uint32_t> GridColType;
typedef std::vector<GridColType> GridType;

class XmlViewer : public Application {
protected:
	Shader* mShader;
	Attribute<glm::vec3>* mVertexPositions;
	Attribute<glm::vec3>* mVertexNormals;
	Attribute<glm::vec2>* mVertexTexCoords;
	IndexBuffer* mIndexBuffer;
	Texture* mDisplayTexture;

    int mMeshSelected = -1;

    Camera mCamera;
    CameraManipulate mCameraControl;

	std::vector<Mesh> mMeshes;
	std::vector<MeshInformation> mMeshInformations;

    glm::mat4 mParent;
    glm::mat4 mModel;

    GridType mGrid; 
    std::string mFilename;
    std::filesystem::file_time_type mGridWriteTime;

public:
	void Initialize();
	void Update(float inDeltaTime);
	void Render(float inAspectRatio);
	void Shutdown();
    void ImGui(nk_context* inContext);
    void NanoGui(NVGcontext* inContext);

    void LoadMeshes(const std::string& filename);

    bool UpdateVertexGrid(const std::string& filename);
    void RenderVertexGrid();

    void UpdateMeshSelect(int select);
    void UpdateMeshBoundings(int select);

    void VerifyGridData();
};
