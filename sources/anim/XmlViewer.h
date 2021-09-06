#pragma once

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

class XmlViewer : public Application {
protected:
	Shader* mShader;
	Attribute<glm::vec3>* mVertexPositions;
	Attribute<glm::vec3>* mVertexNormals;
	Attribute<glm::vec2>* mVertexTexCoords;
	IndexBuffer* mIndexBuffer;
	Texture* mDisplayTexture;

    Camera m_Camera;
    CameraManipulate m_CameraControl;

	std::vector<Mesh> mMeshes;
	std::vector<MeshInformation> mMeshInformations;

    glm::mat4 mParent;
    glm::mat4 mModel;

public:
	void Initialize();
	void Update(float inDeltaTime);
	void Render(float inAspectRatio);
	void Shutdown();
    void ImGui(nk_context* inContext);
    void NanoGui(NVGcontext* inContext);

    void LoadMeshes(const std::string& filename);
};
