#pragma once

#include <memory>

namespace el {

class Node;
class Transform;
class Animation;
class AnimationTarget;
class Scene;
class Stream;
class Mesh;
class Light;
class Model;
class Camera;
class MeshSkin;
struct MeshData;
struct BlendShape;

typedef std::unique_ptr<class Stream> StreamPtr;
typedef std::unique_ptr<struct MeshData> MeshDataPtr;
typedef std::unique_ptr<struct MeshPartData> MeshPartDataPtr;
typedef std::unique_ptr<struct BlendShape> BlendShapePtr;
typedef std::shared_ptr<class Model> ModelPtr;
typedef std::shared_ptr<class Node> NodePtr;
typedef std::shared_ptr<class Scene> ScenePtr;

} // namespace el {
