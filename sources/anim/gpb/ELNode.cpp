#include "ELNode.h"

#include <memory>
#include "ELBase.h"
#include "ELFileSystem.h"
#include "ELStream.h"
#include "ELVertexFormat.h"
#include "ELMatrix.h"
#include "ELQuaternion.h"
#include "ELPredeclare.h"
#include "ELNode.h"
#include "ELBundle.h"

namespace el {

BoundingBox::BoundingBox()
{
}

BoundingSphere::BoundingSphere()
{
}

Node* Node::create(const char* id)
{
    return new Node(id);
}

const char* Node::getTypeName() const
{
    return nullptr;
}

const std::string& Node::getId() const
{
    return _id;
}

void Node::setId(const std::string& id)
{
    _id = id;
}

Node::Type Node::getType() const
{
    return Node::Type();
}

void Node::addChild(const NodePtr& child)
{
}

NodePtr Node::findNode(const std::string& id, bool recursive, bool exactMatch) const
{
    return NULL;
}

void Node::setDrawable(const ModelPtr& model)
{
    _model = model;
}

ModelPtr Node::getDrawable() const
{
    return _model;
}

Node::Node(const char* id)
{
    if (id)
    {
        _id = id;
    }
}


Joint* Joint::create(const char* id)
{
    return nullptr;
}

bool Scene::create(const char* id)
{
    setId(id);
    return true;
}

const char* Scene::getId() const
{
    return _id.c_str();
}

void Scene::setId(const char* id)
{
    _id = id ? id : "";
}

void Scene::addNode(const NodePtr& node)
{
    _nodes.push_back(node);
}

NodePtr Scene::findNode(const std::string& id, bool recursive, bool exactMatch) const
{
    // Search immediate children first.
    for (auto& child : _nodes) {
        // Does this child's ID match?
        if ((exactMatch && child->_id == id) || (!exactMatch && child->_id.find(id) == 0)) {
            return child;
        }
    }

    // Recurse.
    if (recursive) {
        for (auto& child : _nodes) {
            NodePtr match = child->findNode(id, true, exactMatch);
            if (match) {
                return match;
            }
        }
    }
    return NULL;
}

BlendShape::BlendShape(const std::string& name) :
    name(name)
{
}

MeshPartData::MeshPartData() :
        primitiveType(Mesh::TRIANGLES), indexFormat(Mesh::INDEX32), indexCount(0), indexData(NULL)
{
}

MeshPartData::~MeshPartData()
{
    SAFE_DELETE_ARRAY(indexData);
}

MeshData::MeshData(const VertexFormat& vertexFormat) : 
    vertexFormat(vertexFormat),
    vertexCount(0),
    vertexData(NULL),
    primitiveType(Mesh::TRIANGLES)
{
}

MeshData::~MeshData()
{
    SAFE_DELETE_ARRAY(vertexData);
    parts.clear();
}

Model::Model()
{
}

void Model::setMeshData(MeshDataPtr&& meshData)
{
    _meshData = std::move(meshData);
}

const MeshDataPtr& Model::getMeshData() const
{
    return _meshData;
}

ScenePtr loadScene(const std::string& filePath)
{
    auto bundle = std::make_unique<Bundle>();
    if (!bundle->create(filePath))
        return nullptr;
    return bundle->loadScene();
}

} // el

