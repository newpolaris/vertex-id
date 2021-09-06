#ifndef _H_CHAPTER06_SAMPLE01_
#define _H_CHAPTER06_SAMPLE01_

#include <Application.h>
#include "sample/vec3.h"
#include "sample/vec2.h"
#include "sample/Attribute.h"

class Shader;
class IndexBuffer;
class Texture;

class Chapter06Sample01 : public Application {
protected:
	Shader* mShader;
	Attribute<vec3>* mVertexPositions;
	Attribute<vec3>* mVertexNormals;
	Attribute<vec2>* mVertexTexCoords;
	IndexBuffer* mIndexBuffer;
	Texture* mDisplayTexture;
	float mRotation;
public:
	void Initialize();
	void Update(float inDeltaTime);
	void Render(float inAspectRatio);
	void Shutdown();
};

#endif // _H_CHAPTER06_SAMPLE01_
