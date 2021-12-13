#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "ELPredeclare.h"
#include "ELVertexFormat.h"

namespace el {

class BoundingSphere
{
public:

    /**
     * The center point.
     */
    glm::vec3 center;

    /**
     * The sphere radius.
     */
    float radius;

    /**
     *  Constructs a new bounding sphere initialized to all zeros.
     */
    BoundingSphere();
};

class BoundingBox
{
public:

    /**
     * The minimum point.
     */
    glm::vec3 min;

    /**
     * The maximum point.
     */
    glm::vec3 max;

    /**
     * Constructs an empty bounding box at the origin.
     */
    BoundingBox();
};

class Mesh
{
    friend class Model;
    friend class Bundle;

#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_2_BYTES 0x1407
#define GL_3_BYTES 0x1408
#define GL_4_BYTES 0x1409
#define GL_DOUBLE 0x140A

#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_QUADS 0x0007
#define GL_QUAD_STRIP 0x0008
#define GL_POLYGON 0x0009

public:

    /**
     * Defines supported index formats.
     */
    enum IndexFormat
    {
        INDEX8 = GL_UNSIGNED_BYTE,
        INDEX16 = GL_UNSIGNED_SHORT,
        INDEX32 = GL_UNSIGNED_INT
    };

    /**
     * Defines supported primitive types.
     */
    enum PrimitiveType
    {
        TRIANGLES = GL_TRIANGLES,
        TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
        LINES = GL_LINES,
        LINE_STRIP = GL_LINE_STRIP,
        POINTS = GL_POINTS
    };
};

struct BlendShape
{
public:

    BlendShape(const std::string& name);
    
    bool hasNormals = false;
    std::string name;
    std::vector<char> deltaIndices;
    std::vector<char> deltas;
};

struct MeshPartData
{
    MeshPartData();
    ~MeshPartData();

    Mesh::PrimitiveType primitiveType;
    Mesh::IndexFormat indexFormat;
    unsigned int indexCount;
    unsigned char* indexData;
};

struct MeshData
{
    MeshData(const VertexFormat& vertexFormat);
    ~MeshData();

    VertexFormat vertexFormat;
    unsigned int vertexCount;
    unsigned char* vertexData;
    BoundingBox boundingBox;
    BoundingSphere boundingSphere;
    Mesh::PrimitiveType primitiveType;
    std::vector<MeshPartDataPtr> parts;
    std::map<std::string, BlendShapePtr> blendShapes;
};

class Transform
{
};

class Node : public Transform
{
    friend class Scene;
    friend class SceneLoader;
    friend class Bundle;
    friend class MeshSkin;
    friend class Light;

public:

    /**
     * Defines the types of nodes.
     */
    enum Type
    {
        NODE = 1,
        JOINT
    };

    /**
     * Creates a new node with the specified ID.
     *
     * @param id The ID for the new node.
     * @script{create}
     */
    static Node* create(const char* id = NULL);

    /**
     * Extends ScriptTarget::getTypeName() to return the type name of this class.
     *
     * @return The type name of this class: "Node"
     * @see ScriptTarget::getTypeName()
     */
    const char* getTypeName() const;

    /**
     * Gets the identifier for the node.
     *
     * @return The node identifier.
     */
    const std::string& getId() const;

    /**
     * Sets the identifier for the node.
     *
     * @param id The identifier to set for the node.
     */
    void setId(const std::string& id);

    /**
     * Returns the type of the node.
     */
    virtual Node::Type getType() const;

    /**
     * Adds a child node.
     *
     * @param child The child to add.
     */
    virtual void addChild(const NodePtr& child);

    NodePtr findNode(const std::string& id, bool recursive = true, bool exactMatch = true) const;

    void setDrawable(const ModelPtr& model);
    ModelPtr getDrawable() const;

protected:

    Node(const char* id);

    /** The nodes id. */
    std::string _id;
    ModelPtr _model;
};

class Scene
{
public:

    bool create(const char* id = NULL);

    const char* getId() const;
    void setId(const char* id);
    void addNode(const NodePtr& node);
    NodePtr findNode(const std::string& id, bool recursive, bool exactMatch = true) const;

    std::string _id;

    std::vector<NodePtr> _nodes;
};

class Joint : public Node
{
public:
    static Joint* create(const char* id = NULL);
};

class MeshSkin
{
};

class Camera
{
    friend class Node;
public:

    enum Type
    {
        PERSPECTIVE = 1,
        ORTHOGRAPHIC = 2
    };

};

class Animation
{
};

class AnimationTarget
{
};

class Light
{
    friend class Node;

public:

    enum Type
    {
        DIRECTIONAL = 1,
        POINT = 2,
        SPOT = 3
    };
};

class Model
{
public:

    Model();
    
    const MeshDataPtr& getMeshData() const;
    void setMeshData(MeshDataPtr&& meshData);

    MeshDataPtr _meshData;
};

ScenePtr loadScene(const std::string& filePath);

} // namespace el 
