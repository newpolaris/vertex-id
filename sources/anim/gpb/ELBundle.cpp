#define _CRT_SECURE_NO_WARNINGS
#include "ELBundle.h"

#include <memory>
#include "ELBase.h"
#include "ELFileSystem.h"
#include "ELStream.h"
#include "ELVertexFormat.h"
#include "ELMatrix.h"
#include "ELQuaternion.h"
#include "ELPredeclare.h"
#include "ELNode.h"

// Minimum version numbers supported
#define BUNDLE_VERSION_MAJOR_REQUIRED   1 
#define BUNDLE_VERSION_MINOR_REQUIRED   2

#define BUNDLE_TYPE_SCENE               1
#define BUNDLE_TYPE_NODE                2
#define BUNDLE_TYPE_ANIMATIONS          3
#define BUNDLE_TYPE_ANIMATION           4
#define BUNDLE_TYPE_ANIMATION_CHANNEL   5
#define BUNDLE_TYPE_MODEL               10
#define BUNDLE_TYPE_MATERIAL            16
#define BUNDLE_TYPE_EFFECT              18
#define BUNDLE_TYPE_CAMERA              32
#define BUNDLE_TYPE_LIGHT               33
#define BUNDLE_TYPE_MESH                34
#define BUNDLE_TYPE_MESHPART            35
#define BUNDLE_TYPE_MESHSKIN            36
#define BUNDLE_TYPE_FONT                128

// For sanity checking string reads
#define BUNDLE_MAX_STRING_LENGTH        5000

#define BUNDLE_VERSION_MAJOR_FONT_FORMAT  1
#define BUNDLE_VERSION_MINOR_FONT_FORMAT  5

using namespace el;

static std::string readString(std::unique_ptr<Stream>& stream)
{
    GP_ASSERT(stream);

    unsigned int length;
    if (stream->read(&length, 4, 1) != 1)
    {
        GP_ERROR("Failed to read the length of a string from a bundle.");
        return std::string();
    }

    // Sanity check to detect if string length is far too big.
    GP_ASSERT(length < BUNDLE_MAX_STRING_LENGTH);

    std::string str;
    if (length > 0)
    {
        str.resize(length);
        if (stream->read(&str[0], 1, length) != length)
        {
            GP_ERROR("Failed to read string from bundle.");
            return std::string();
        }
    }
    return str;
}

Bundle::Bundle()
{
}

Bundle::~Bundle() 
{
}

bool Bundle::create(const std::string& path) 
{
    GP_ASSERT(!path.empty());

    // Open the bundle.
    auto stream = std::unique_ptr<Stream>(FileSystem::open(path));
    if (!stream)
    {
        GP_WARN("Failed to open file '%s'.", path.c_str());
        return false;
    }

    // Read the GPB header info.
    char sig[9] = { 0, };
    
    // GPB, GPBx
    if (stream->read(sig, 1, 9) != 9) {
        GP_WARN("Invalid GPB header for bundle '%s'.", path.c_str());
        return false;
    }

    bool isGPBX = false;
    if (memcmp(sig, "\xABGPB\xBB\r\n\x1B\n", 9) == 0)
        isGPBX = true;

    // Read version.
    unsigned char version[2] = { 0, };
    if (stream->read(version, 1, 2) != 2)
    {
        GP_WARN("Failed to read GPB version for bundle '%s'.", path.c_str());
        return false;
    }
    auto a = version[0] << 16 | version[1];
    auto b = BUNDLE_VERSION_MAJOR_REQUIRED << 16 | BUNDLE_VERSION_MINOR_REQUIRED;
    // Check for the minimal 
    if (version[0] < BUNDLE_VERSION_MAJOR_REQUIRED || version[1] < BUNDLE_VERSION_MINOR_REQUIRED)
    {
        GP_WARN("Unsupported version (%d.%d) for bundle '%s' (expected %d.%d).", (int)version[0], (int)version[1], path.c_str(), BUNDLE_VERSION_MAJOR_REQUIRED, BUNDLE_VERSION_MINOR_REQUIRED);
        return false;
    }

    // Read ref table.
    unsigned int refCount = 0;
    if (stream->read(&refCount, 4, 1) != 1) {
        GP_WARN("Failed to read ref table for bundle '%s'.", path.c_str());
        return false;
    }

    // Read all refs.
    auto refs = std::vector<Reference>(refCount);
    for (unsigned int i = 0; i < refCount; ++i) {
        if ((refs[i].id = readString(stream)).empty() ||
            stream->read(&refs[i].type, 4, 1) != 1 ||
            stream->read(&refs[i].offset, 4, 1) != 1)
        {
            GP_WARN("Failed to read ref number %d for bundle '%s'.", i, path.c_str());
            return false;
        }
    }

    // Keep file open for faster reading later.
    _bGPBX = isGPBX;
    _version[0] = version[0];
    _version[1] = version[1];
    _references = refs;
    _stream = std::move(stream);
    _path = path;

    return true;
}

int Bundle::getVersion() const
{
    return getVersion(_version[0], _version[1]);
}

int Bundle::getVersion(unsigned char major, unsigned char minor) const
{
    return (int)major * 0xFF + minor;
}

bool Bundle::skipNode()
{
    const char* id = getIdFromOffset();
    GP_ASSERT(id);
    GP_ASSERT(_stream);

    // Skip the node's type.
    unsigned int nodeType;
    if (!read(&nodeType)) {
        GP_ERROR("Failed to skip node type for node '%s'.", id);
        return false;
    }

    // Skip over the node's transform and parent ID.
    if (_stream->seek(sizeof(float) * 16, SEEK_CUR) == false) {
        GP_ERROR("Failed to skip over node transform for node '%s'.", id);
        return false;
    }
    readString(_stream);

    // Skip over the node's children.
    unsigned int childrenCount;
    if (!read(&childrenCount)) {
        GP_ERROR("Failed to skip over node's children count for node '%s'.", id);
        return false;
    } else if (childrenCount > 0) {
        for (unsigned int i = 0; i < childrenCount; i++) {
            if (!skipNode())
                return false;
        }
    }

    // Skip over the node's camera, light, and model attachments.
    std::shared_ptr<Camera> camera(readCamera());
    std::shared_ptr<Light> light(readLight());
    ModelPtr model = readModel(id);

    return true;
}

ScenePtr Bundle::loadScene(const char* id)
{
    const Reference* ref = NULL;
    if (id) {
        ref = seekTo(id, BUNDLE_TYPE_SCENE);
        if (!ref) {
            GP_ERROR("Failed to load scene with id '%s' from bundle.", id);
            return NULL;
        }
    } else {
        ref = seekToFirstType(BUNDLE_TYPE_SCENE);
        if (!ref)
        {
            GP_ERROR("Failed to load scene from bundle; bundle contains no scene objects.");
            return NULL;
        }
    }

    auto scene = std::make_shared<Scene>();
    if (!scene->create(getIdFromOffset()))
        return nullptr;

    // Read the number of children.
    unsigned int childrenCount;
    if (!read(&childrenCount)) {
        GP_ERROR("Failed to read the scene's number of children.");
        return NULL;
    }
    if (childrenCount > 0) {
        // Read each child directly into the scene.
        for (unsigned int i = 0; i < childrenCount; i++) {
            NodePtr node = readNode(scene.get(), NULL);
            if (node) {
                scene->addNode(node);
            }
        }
    }

    // Read active camera.
    std::string xref = readString(_stream);
    if (xref.length() > 1 && xref[0] == '#') // TODO: Handle full xrefs
    {
        // Node* node = scene->findNode(xref.c_str() + 1, true);
        // GP_ASSERT(node);
        // Camera* camera = node->getCamera();
        // GP_ASSERT(camera);
        // scene->setActiveCamera(camera);
    }

    // Read ambient color.
    float red, blue, green;
    if (!read(&red)) {
        GP_ERROR("Failed to read red component of the scene's ambient color in bundle '%s'.", _path.c_str());
        return NULL;
    }
    if (!read(&green)) {
        GP_ERROR("Failed to read green component of the scene's ambient color in bundle '%s'.", _path.c_str());
        return NULL;
    }
    if (!read(&blue))
    {
        GP_ERROR("Failed to read blue component of the scene's ambient color in bundle '%s'.", _path.c_str());
        return NULL;
    }
    // scene->setAmbientColor(red, green, blue);

    // Parse animations.
    GP_ASSERT(_stream);
    for (unsigned int i = 0; i < _references.size(); ++i) {
        Reference* ref = &_references[i];
        if (ref->type == BUNDLE_TYPE_ANIMATIONS)
        {
            // Found a match.
            if (_stream->seek(ref->offset, SEEK_SET) == false)
            {
                GP_ERROR("Failed to seek to object '%s' in bundle '%s'.", ref->id.c_str(), _path.c_str());
                return NULL;
            }
            readAnimations(scene.get());
        }
    }

    resolveJointReferences(scene.get(), NULL);

    return scene;
}

bool Bundle::read(unsigned int* ptr)
{
    return _stream->read(ptr, sizeof(unsigned int), 1) == 1;
}

bool Bundle::read(unsigned char* ptr)
{
    return _stream->read(ptr, sizeof(unsigned char), 1) == 1;
}

bool Bundle::read(float* ptr)
{
    return _stream->read(ptr, sizeof(float), 1) == 1;
}

bool Bundle::readMatrix(float* m)
{
    return _stream->read(m, sizeof(float), 16) == 16;
}

const Bundle::Reference* Bundle::seekTo(const char* id, unsigned int type)
{
    const Reference* ref = find(id);
    if (ref == NULL)
    {
        GP_ERROR("No object with name '%s' in bundle '%s'.", id, _path.c_str());
        return NULL;
    }

    if (ref->type != type)
    {
        GP_ERROR("Object '%s' in bundle '%s' has type %d (expected type %d).", id, _path.c_str(), (int)ref->type, (int)type);
        return NULL;
    }

    // Seek to the offset of this object.
    GP_ASSERT(_stream);
    if (_stream->seek(ref->offset, SEEK_SET) == false)
    {
        GP_ERROR("Failed to seek to object '%s' in bundle '%s'.", id, _path.c_str());
        return NULL;
    }

    return ref;
}

Bundle::Reference* Bundle::seekToFirstType(unsigned int type)
{
    GP_ASSERT(_stream);

    for (unsigned int i = 0; i < _references.size(); ++i)
    {
        Reference& ref = _references[i];
        if (ref.type == type)
        {
            // Found a match.
            if (_stream->seek(ref.offset, SEEK_SET) == false)
            {
                GP_ERROR("Failed to seek to object '%s' in bundle '%s'.", ref.id.c_str(), _path.c_str());
                return NULL;
            }
            return &ref;
        }
    }
    return NULL;
}

const Bundle::Reference* Bundle::find(const char* id) const
{
    GP_ASSERT(id);

    // Search the ref table for the given id (case-sensitive).
    for (unsigned int i = 0; i < _references.size(); ++i)
    {
        if (_references[i].id == id)
        {
            // Found a match
            return &_references[i];
        }
    }

    return NULL;
}

const char* Bundle::getIdFromOffset() const
{
    GP_ASSERT(_stream);
    return getIdFromOffset((unsigned int) _stream->position());
}

const char* Bundle::getIdFromOffset(unsigned int offset) const
{
    // Search the ref table for the given offset.
    if (offset > 0)
    {
        for (unsigned int i = 0; i < _references.size(); ++i)
        {
            if (_references[i].offset == offset && _references[i].id.length() > 0)
            {
                return _references[i].id.c_str();
            }
        }
    }
    return NULL;
}

Camera* Bundle::readCamera() 
{
    unsigned char cameraType;
    if (!read(&cameraType))
    {
        GP_ERROR("Failed to load camera type in bundle '%s'.", _path.c_str());
        return NULL;
    }

    // Check if there isn't a camera to load.
    if (cameraType == 0)
    {
        return NULL;
    }

    float aspectRatio;
    if (!read(&aspectRatio))
    {
        GP_ERROR("Failed to load camera aspect ratio in bundle '%s'.", _path.c_str());
        return NULL;
    }

    float nearPlane;
    if (!read(&nearPlane))
    {
        GP_ERROR("Failed to load camera near plane in bundle '%s'.", _path.c_str());
        return NULL;
    }

    float farPlane;
    if (!read(&farPlane))
    {
        GP_ERROR("Failed to load camera far plane in bundle '%s'.", _path.c_str());
        return NULL;
    }

    Camera* camera = NULL;
    if (cameraType == Camera::PERSPECTIVE)
    {
        float fieldOfView;
        if (!read(&fieldOfView))
        {
            GP_ERROR("Failed to load camera field of view in bundle '%s'.", _path.c_str());
            return NULL;
        }

        // camera = Camera::createPerspective(fieldOfView, aspectRatio, nearPlane, farPlane);
    }
    else if (cameraType == Camera::ORTHOGRAPHIC)
    {
        float zoomX;
        if (!read(&zoomX))
        {
            GP_ERROR("Failed to load camera zoomX in bundle '%s'.", _path.c_str());
            return NULL;
        }

        float zoomY;
        if (!read(&zoomY))
        {
            GP_ERROR("Failed to load camera zoomY in bundle '%s'.", _path.c_str());
            return NULL;
        }

        // camera = Camera::createOrthographic(zoomX, zoomY, aspectRatio, nearPlane, farPlane);
    }
    else
    {
        GP_ERROR("Unsupported camera type (%d) in bundle '%s'.", cameraType, _path.c_str());
        return NULL;
    }

    return nullptr;
}

Light* Bundle::readLight() 
{
    unsigned char type;
    if (!read(&type))
    {
        GP_ERROR("Failed to load light type in bundle '%s'.", _path.c_str());
        return NULL;
    }

    // Check if there isn't a light to load.
    if (type == 0)
    {
        return NULL;
    }

    // Read color.
    float red, blue, green;
    if (!read(&red) || !read(&blue) || !read(&green))
    {
        GP_ERROR("Failed to load light color in bundle '%s'.", _path.c_str());
        return NULL;
    }
    glm::vec3 color(red, blue, green);

    Light* light = NULL;
    if (type == Light::DIRECTIONAL)
    {
        // light = Light::createDirectional(color);
    }
    else if (type == Light::POINT)
    {
        float range;
        if (!read(&range))
        {
            GP_ERROR("Failed to load point light range in bundle '%s'.", _path.c_str());
            return NULL;
        }
        // light = Light::createPoint(color, range);
    }
    else if (type == Light::SPOT)
    {
        float range, innerAngle, outerAngle;
        if (!read(&range))
        {
            GP_ERROR("Failed to load spot light range in bundle '%s'.", _path.c_str());
            return NULL;
        }
        if (!read(&innerAngle))
        {
            GP_ERROR("Failed to load spot light inner angle in bundle '%s'.", _path.c_str());
            return NULL;
        }
        if (!read(&outerAngle))
        {
            GP_ERROR("Failed to load spot light outer angle in bundle '%s'.", _path.c_str());
            return NULL;
        }
        // light = Light::createSpot(color, range, innerAngle, outerAngle);
    }
    else
    {
        GP_ERROR("Unsupported light type (%d) in bundle '%s'.", type, _path.c_str());
        return NULL;
    }
    return light;

    return nullptr;
}

ModelPtr Bundle::readModel(const std::string& nodeId) 
{
    std::string xref = readString(_stream);
    if (xref.length() <= 1 || xref[0] != '#') // TODO: Handle full xrefs
        return nullptr;

    MeshDataPtr meshData = loadMesh(xref.c_str() + 1, nodeId);
    if (!meshData)
        return nullptr;
    
    auto model = std::make_shared<Model>();
    if (!model)
        return nullptr;

    model->setMeshData(std::move(meshData));

    if (getVersion() >= getVersion(9, 1)) {
        // Read VertexAnimationCache
        unsigned char hasVertexAnimCache = 0;
        if (!read(&hasVertexAnimCache)) {
            GP_ERROR("Failed to load whether model with mesh");
            return NULL;
        }
    }

    // Read skin.
    unsigned char hasSkin = 0;
    if (!read(&hasSkin)) {
        GP_ERROR("Failed to load whether model with mesh '%s' has a mesh skin in bundle '%s'.", xref.c_str() + 1, _path.c_str());
        return NULL;
    }
    if (hasSkin) {
        MeshSkin* skin = readMeshSkin();
        if (skin) {
            // model->setSkin(skin);
        }
    }
    // Read material.
    unsigned int materialCount;
    if (!read(&materialCount)) {
        GP_ERROR("Failed to load material count for model with mesh '%s' in bundle '%s'.", xref.c_str() + 1, _path.c_str());
        return NULL;
    }
    if (materialCount > 0) {
        for (unsigned int i = 0; i < materialCount; ++i) {
            std::string materialName = readString(_stream);
    #if 0
            std::string materialPath = getMaterialPath();
            if (materialPath.length() > 0)
            {
                materialPath.append("#");
                materialPath.append(materialName);
                Material* material = Material::create(materialPath.c_str());
                if (material)
                {
                    int partIndex = model->getMesh()->getPartCount() > 0 ? i : -1;
                    model->setMaterial(material, partIndex);
                    SAFE_RELEASE(material);
                }
            }
    #endif
        }
    }
    return model;
}

void Bundle::setTransform(const float* values, Transform* transform)
{
    GP_ASSERT(transform);

    // Load array into transform.
    Matrix matrix(values);
    Vector3 scale, translation;
    Quaternion rotation;
    matrix.decompose(&scale, &rotation, &translation);
    // transform->setScale(scale);
    // transform->setTranslation(translation);
    // transform->setRotation(rotation);
}

void Bundle::resolveJointReferences(Scene* sceneContext, Node* nodeContext)
{
}

NodePtr Bundle::readNode(Scene* sceneContext, Node* nodeContext)
{
    const char* id = getIdFromOffset();
    GP_ASSERT(id);
    GP_ASSERT(_stream);

    // Read node type.
    unsigned int nodeType;
    if (!read(&nodeType))
    {
        GP_ERROR("Failed to read node type for node '%s'.", id);
        return NULL;
    }

    std::shared_ptr<Node> node;
    switch (nodeType)
    {
    case Node::NODE:
        node = std::shared_ptr<Node>(Node::create(id));
        break;
    case Node::JOINT:
        node = std::shared_ptr<Node>(Joint::create(id));
        break;
    default:
        return NULL;
    }

    // Read transform.
    float transform[16] = { 0, };
    if (_stream->read(transform, sizeof(float), 16) != 16)
    {
        GP_ERROR("Failed to read transform for node '%s'.", id);
        return NULL;
    }
    setTransform(transform, node.get());

    // Skip the parent ID.
    readString(_stream);

    // Read children.
    unsigned int childrenCount;
    if (!read(&childrenCount)) {
        GP_ERROR("Failed to read children count for node '%s'.", id);
        return NULL;
    }
    if (childrenCount > 0) {
        // Read each child.
        for (unsigned int i = 0; i < childrenCount; i++) {
            // Search the passed in loading contexts (scene/node) first to see
            // if we've already loaded this child node during this load session.
            NodePtr child = NULL;
            id = getIdFromOffset();
            GP_ASSERT(id);

            if (sceneContext) {
                child = sceneContext->findNode(id, true);
            }
            if (child == NULL && nodeContext) {
                child = nodeContext->findNode(id, true);
            }

            // If the child was already loaded, skip it, otherwise read it
            if (child) {
                skipNode();
            } else {
                child = readNode(sceneContext, nodeContext);
            }
            if (child) {
                node->addChild(child);
            }
        }
    }

    // Read camera.
    // TODO:
    std::shared_ptr<Camera> camera(readCamera());
    if (camera)
        ; // node->setCamera(camera);

    // Read light.
    // TODO:
    std::shared_ptr<Light> light(readLight());
    if (light)
        ; // node->setLight(light);

    // Read model.
    ModelPtr model = readModel(node->getId());
    if (model)
        node->setDrawable(model);

    return node;
}

MeshDataPtr Bundle::loadMesh(const char* id, const std::string& nodeId)
{
    GP_ASSERT(_stream);
    GP_ASSERT(id);

    // Save the file position.
    long position = _stream->position();
    if (position == -1L)
    {
        GP_ERROR("Failed to save the current file position before loading mesh '%s'.", id);
        return NULL;
    }

    // Seek to the specified mesh.
    const Reference* ref = seekTo(id, BUNDLE_TYPE_MESH);
    if (ref == NULL)
    {
        GP_ERROR("Failed to locate ref for mesh '%s'.", id);
        return NULL;
    }

    // Read mesh data.
    MeshDataPtr meshData(readMeshData());
    if (meshData == NULL) {
        GP_ERROR("Failed to load mesh data for mesh '%s'.", id);
        return NULL;
    }

    // Create mesh.
#if 0
    Mesh* mesh = Mesh::createMesh(meshData->vertexFormat, meshData->vertexCount, false);
    if (mesh == NULL)
    {
        GP_ERROR("Failed to create mesh '%s'.", id);
        SAFE_DELETE_ARRAY(meshData);
        return NULL;
    }
    return mesh;
#endif

    // Restore file pointer.
    if (_stream->seek(position, SEEK_SET) == false)
    {
        GP_ERROR("Failed to restore file pointer after loading mesh '%s'.", id);
        return NULL;
    }

    return meshData;
}

MeshDataPtr Bundle::readMeshData()
{
    // Read vertex format/elements.
    unsigned int vertexElementCount;
    if (_stream->read(&vertexElementCount, 4, 1) != 1)
    {
        GP_ERROR("Failed to load vertex element count.");
        return NULL;
    }
    if (vertexElementCount < 1)
    {
        GP_ERROR("Failed to load mesh data; invalid vertex element count (must be greater than 0).");
        return NULL;
    }

    VertexFormat::Element* vertexElements = new VertexFormat::Element[vertexElementCount];
    for (unsigned int i = 0; i < vertexElementCount; ++i)
    {
        unsigned int vUsage, vSize;
        if (_stream->read(&vUsage, 4, 1) != 1)
        {
            GP_ERROR("Failed to load vertex usage.");
            SAFE_DELETE_ARRAY(vertexElements);
            return NULL;
        }
        if (_stream->read(&vSize, 4, 1) != 1)
        {
            GP_ERROR("Failed to load vertex size.");
            SAFE_DELETE_ARRAY(vertexElements);
            return NULL;
        }

        vertexElements[i].usage = (VertexFormat::Usage)vUsage;
        vertexElements[i].size = vSize;
    }

    auto meshData = std::make_unique<MeshData>(VertexFormat(vertexElements, vertexElementCount));
    SAFE_DELETE_ARRAY(vertexElements);

    // Read vertex data.
    unsigned int vertexByteCount;
    if (_stream->read(&vertexByteCount, 4, 1) != 1) {
        GP_ERROR("Failed to load vertex byte count.");
        return NULL;
    }
    if (vertexByteCount == 0) {
        GP_ERROR("Failed to load mesh data; invalid vertex byte count of 0.");
        return NULL;
    }

    GP_ASSERT(meshData->vertexFormat.getVertexSize());
    meshData->vertexCount = vertexByteCount / meshData->vertexFormat.getVertexSize();
    meshData->vertexData = new unsigned char[vertexByteCount];
    if (_stream->read(meshData->vertexData, 1, vertexByteCount) != vertexByteCount) {
        GP_ERROR("Failed to load vertex data.");
        return NULL;
    }

    // Read mesh bounds (bounding box and bounding sphere).
    if (_stream->read(&meshData->boundingBox.min.x, 4, 3) != 3 || _stream->read(&meshData->boundingBox.max.x, 4, 3) != 3) {
        GP_ERROR("Failed to load mesh bounding box.");
        return NULL;
    }
    if (_stream->read(&meshData->boundingSphere.center.x, 4, 3) != 3 || _stream->read(&meshData->boundingSphere.radius, 4, 1) != 1) {
        GP_ERROR("Failed to load mesh bounding sphere.");
        return NULL;
    }

    // Read mesh parts.
    unsigned int meshPartCount;
    if (_stream->read(&meshPartCount, 4, 1) != 1) {
        GP_ERROR("Failed to load mesh part count.");
        return NULL;
    }
    for (unsigned int i = 0; i < meshPartCount; ++i)
    {
        // Read primitive type, index format and index count.
        unsigned int pType, iFormat, iByteCount;
        if (_stream->read(&pType, 4, 1) != 1)
        {
            GP_ERROR("Failed to load primitive type for mesh part with index %d.", i);
            return NULL;
        }
        if (_stream->read(&iFormat, 4, 1) != 1)
        {
            GP_ERROR("Failed to load index format for mesh part with index %d.", i);
            return NULL;
        }
        if (_stream->read(&iByteCount, 4, 1) != 1)
        {
            GP_ERROR("Failed to load index byte count for mesh part with index %d.", i);
            return NULL;
        }

        auto partData = std::make_unique<MeshPartData>();
        partData->primitiveType = (Mesh::PrimitiveType)pType;
        partData->indexFormat = (Mesh::IndexFormat)iFormat;

        unsigned int indexSize = 0;
        switch (partData->indexFormat)
        {
        case Mesh::INDEX8:
            indexSize = 1;
            break;
        case Mesh::INDEX16:
            indexSize = 2;
            break;
        case Mesh::INDEX32:
            indexSize = 4;
            break;
        default:
            GP_ERROR("Unsupported index format for mesh part with index %d.", i);
            return NULL;
        }

        GP_ASSERT(indexSize);
        partData->indexCount = iByteCount / indexSize;

        partData->indexData = new unsigned char[iByteCount];
        if (_stream->read(partData->indexData, 1, iByteCount) != iByteCount) {
            GP_ERROR("Failed to read index data for mesh part with index %d.", i);
            return NULL;
        }
        meshData->parts.push_back(std::move(partData));
    }

    if (!readMeshBlendShape(meshData.get())) {
    }

    if (0) {
        std::string name = "eyelashdump.dat";
        FILE* file = fopen(name.c_str(), "ab");
        fseek(file, 0, SEEK_END);
        uint32_t vsize = meshData->vertexFormat.getVertexSize();
        uint32_t vcount = meshData->vertexCount;
        unsigned char* vertexData = meshData->vertexData;

        fwrite(&vsize, sizeof(uint32_t), 1, file);
        fwrite(&vcount, sizeof(uint32_t), 1, file);
        fwrite(vertexData, vsize*vcount, 1, file);

        uint32_t partCount = meshData->parts.size();
        fwrite(&partCount, sizeof(uint32_t), 1, file);
        for (auto& part : meshData->parts) {
            assert(part->indexFormat == Mesh::INDEX16);
            uint32_t isize = 2;
            uint32_t icount = part->indexCount; 
            unsigned char* indexData = part->indexData;
            fwrite(&isize, sizeof(uint32_t), 1, file);
            fwrite(&icount, sizeof(uint32_t), 1, file);
            fwrite(indexData, isize* icount, 1, file);
        }
        fclose(file);
    }

    return meshData;
}

bool Bundle::readMeshBlendShape(MeshData* meshData)
{
    if (!_bGPBX)
        return false;

    unsigned int blendShapeCount = 0;
    if (_stream->read(&blendShapeCount, 4, 1) != 1) {
        GP_ERROR("Failed to load mesh blend shape count");
        return false;
    }

    for (unsigned int i = 0; i < blendShapeCount; ++i) {
        unsigned int nameLength = 0;
        if (_stream->read(&nameLength, 4, 1) != 1 || nameLength == 0) {
            GP_ERROR("Failed to load blend shape name length");
            return false;
        }

        std::vector<char> nameTemp(nameLength + 1);
        if (_stream->read(nameTemp.data(), sizeof(unsigned char), nameLength) != nameLength) {
            GP_ERROR("Failed to load blend shape name");
            return false;
        }
        std::string name(nameTemp.data());
        auto blendShape = std::make_unique<BlendShape>(name);
        unsigned int vertexSize = 0;
        if (_stream->read(&vertexSize, 4, 1) != 1) {
            GP_ERROR("Failed to load vertexsize");
            return false;
        }

        std::vector<char> indices(vertexSize*4);
        if (_stream->read(indices.data(), 4, vertexSize) != vertexSize) {
            GP_ERROR("Failed to load indices");
            return false;
        }
        unsigned int hasNormals = 0;
        if (_stream->read(&hasNormals, 4, 1) != 1) {
            GP_ERROR("Failed to load hasNormals");
            return false;
        }
        int readsize = vertexSize;
        if (hasNormals) {
            readsize *= 2;
        }

        std::vector<char> deltas(readsize*4*3);
        if (_stream->read(deltas.data(), 4, readsize*3) != readsize*3) {
            GP_ERROR("Failed to load vertex delta data");
            return false;
        }
        blendShape->hasNormals = hasNormals;
        blendShape->deltaIndices = std::move(indices);
        blendShape->deltas = std::move(deltas);
        meshData->blendShapes.insert({std::string(name.data()), std::move(blendShape)});
    }
    return true;
}

MeshData* Bundle::readMeshData(const char* url)
{
    return nullptr;
}

MeshSkin* Bundle::readMeshSkin()
{
    return nullptr;
}

void Bundle::readAnimations(Scene* scene)
{
}
