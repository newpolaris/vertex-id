#ifndef _H_CHAPTER10SAMPLE02_
#define _H_CHAPTER10SAMPLE02_

#include "Application.h"
#include <vector>
#include "Pose.h"
#include "Clip.h"
#include "Skeleton.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "DebugDraw.h"
#include <vector>

struct AnimationInstance {
	Pose mAnimatedPose;
	std::vector <mat4> mPosePalette;
	unsigned int mClip;
	float mPlayback;
	Transform mModel;

	inline AnimationInstance() : mClip(0), mPlayback(0.0f) { }
};

class Chapter10Sample02 : public Application {
protected:
	Texture* mDiffuseTexture;
	Shader* mStaticShader;
	Shader* mSkinnedShader;
	std::vector<Mesh> mCPUMeshes;
	std::vector<Mesh> mGPUMeshes;
	Skeleton mSkeleton;
	std::vector<Clip> mClips;

	AnimationInstance mGPUAnimInfo;
	AnimationInstance mCPUAnimInfo;
public:
	void Initialize();
	void Update(float deltaTime);
	void Render(float inAspectRatio);
	void Shutdown();
	void ImGui(nk_context* inContext);
};

#endif
