#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "ELNode.h"
#include "ELPredeclare.h"
#include "ELVertexFormat.h"

namespace el {

class Bundle 
{
private:

    class Reference
    {
    public:
        std::string id;
        unsigned int type;
        unsigned int offset;

        /**
         * Constructor.
         */
        Reference() = default;

        /**
         * Destructor.
         */
        ~Reference() = default;
    };

public:

    Bundle();

    bool create(const std::string& path);

    /**
     * Destructor.
     */
    ~Bundle();

    /**
     * Hidden copy assignment operator.
     */
    Bundle& operator=(const Bundle&);

    /**
     * Finds a reference by ID.
     */
    const Reference* find(const char* id) const;

    /**
     * Seeks the file pointer to the object with the given ID and type
     * and returns the relevant Reference.
     *
     * @param id The ID string to search for.
     * @param type The object type.
     * 
     * @return The reference object or NULL if there was an error.
     */
    const Reference* seekTo(const char* id, unsigned int type);

    /**
     * Seeks the file pointer to the first object that matches the given type.
     * 
     * @param type The object type.
     * 
     * @return The reference object or NULL if there was an error.
     */
    Reference* seekToFirstType(unsigned int type);

    /**
     * Internal method to load a node.
     *
     * Only one of node or scene should be passed as non-NULL (or neither).
     */
    Node* loadNode(const char* id, Scene* sceneContext, Node* nodeContext);

    /**
     * Internal method for SceneLoader to load a node into a scene.
     */
    Node* loadNode(const char* id, Scene* sceneContext);

    /**
     * Loads a mesh with the specified ID from the bundle.
     *
     * @param id The ID of the mesh to load.
     * @param nodeId The id of the mesh's model's parent node.
     * 
     * @return The loaded mesh, or NULL if the mesh could not be loaded.
     */
    MeshDataPtr loadMesh(const char* id, const std::string& nodeId);

    ScenePtr loadScene(const char* id = NULL);

    int getVersion() const;
    int getVersion(unsigned char major, unsigned char minor) const;

    bool skipNode();

    /**
     * Returns the ID of the object at the current file position.
     * Returns NULL if not found.
     * 
     * @return The ID string or NULL if not found.
     */
    const char* getIdFromOffset() const;

    /**
     * Returns the ID of the object at the given file offset by searching through the reference table.
     * Returns NULL if not found.
     *
     * @param offset The file offset.
     * 
     * @return The ID string or NULL if not found.
     */
    const char* getIdFromOffset(unsigned int offset) const;

    /**
     * Reads an unsigned int from the current file position.
     *
     * @param ptr A pointer to load the value into.
     * 
     * @return True if successful, false if an error occurred.
     */
    bool read(unsigned int* ptr);

    /**
     * Reads an unsigned char from the current file position.
     * 
     * @param ptr A pointer to load the value into.
     * 
     * @return True if successful, false if an error occurred.
     */
    bool read(unsigned char* ptr);

    /**
     * Reads a float from the current file position.
     * 
     * @param ptr A pointer to load the value into.
     * 
     * @return True if successful, false if an error occurred.
     */
    bool read(float* ptr);

    /**
     * Reads an array of values and the array length from the current file position.
     * 
     * @param length A pointer to where the length of the array will be copied to.
     * @param ptr A pointer to the array where the data will be copied to.
     * 
     * @return True if successful, false if an error occurred.
     */
    template <class T>
    bool readArray(unsigned int* length, T** ptr);

    /**
     * Reads an array of values and the array length from the current file position.
     * 
     * @param length A pointer to where the length of the array will be copied to.
     * @param values A pointer to the vector to copy the values to. The vector will be resized if it is smaller than length.
     * 
     * @return True if successful, false if an error occurred.
     */
    template <class T>
    bool readArray(unsigned int* length, std::vector<T>* values);

    /**
     * Reads an array of values and the array length from the current file position.
     * 
     * @param length A pointer to where the length of the array will be copied to.
     * @param values A pointer to the vector to copy the values to. The vector will be resized if it is smaller than length.
     * @param readSize The size that reads will be performed at, size must be the same as or smaller then the sizeof(T)
     * 
     * @return True if successful, false if an error occurred.
     */
    template <class T>
    bool readArray(unsigned int* length, std::vector<T>* values, unsigned int readSize);
    
    /**
     * Reads 16 floats from the current file position.
     *
     * @param m A pointer to float array of size 16.
     * 
     * @return True if successful, false if an error occurred.
     */
    bool readMatrix(float* m);

    /**
     * Reads an xref string from the current file position.
     * 
     * @param id The string to load the ID string into.
     * 
     * @return True if successful, false if an error occurred.
     */
    bool readXref(std::string& id);

    /**
     * Recursively reads nodes from the current file position.
     * This method will load cameras, lights and models in the nodes.
     * 
     * @return A pointer to new node or NULL if there was an error.
     */
    NodePtr readNode(Scene* sceneContext, Node* nodeContext);

    /**
     * Reads a camera from the current file position.
     *
     * @return A pointer to a new camera or NULL if there was an error.
     */
    Camera* readCamera();

    /**
     * Reads a light from the current file position.
     *
     * @return A pointer to a new light or NULL if there was an error.
     */
    Light* readLight();

    /**
     * Reads a model from the current file position.
     * 
     * @return A pointer to a new model or NULL if there was an error.
     */
    ModelPtr readModel(const std::string& nodeId);

    /**
     * Reads mesh data from the current file position.
     */
    MeshDataPtr readMeshData();

    /**
     * Reads mesh data for the specified URL.
     *
     * The specified URL should be formatted as 'bundle#id', where
     * 'bundle' is the bundle file containing the mesh and 'id' is the ID
     * of the mesh to read data for.
     *
     * @param url The URL to read mesh data from.
     *
     * @return The mesh rigid body data.
     */
    static MeshData* readMeshData(const char* url);

    bool readMeshBlendShape(MeshData* meshData);

    /**
     * Reads a mesh skin from the current file position.
     *
     * @return A pointer to a new mesh skin or NULL if there was an error.
     */
    MeshSkin* readMeshSkin();

    /**
     * Reads an animation from the current file position.
     * 
     * @param scene The scene to load the animations into.
     */
    void readAnimation(Scene* scene);

    /**
     * Reads an "animations" object from the current file position and all of the animations contained in it.
     * 
     * @param scene The scene to load the animations into.
     */
    void readAnimations(Scene* scene);

    /**
     * Reads an animation channel at the current file position into the given animation.
     * 
     * @param scene The scene that the animation is in.
     * @param animation The animation to the load channel into.
     * @param animationId The ID of the animation that this channel is loaded into.
     * 
     * @return The animation that the channel was loaded into.
     */
    Animation* readAnimationChannel(Scene* scene, Animation* animation, const char* animationId);

    /**
     * Reads the animation channel data at the current file position into the given animation
     * (with the given animation target and target attribute).
     * 
     * Note: this is used by Bundle::loadNode(const char*, Scene*) and Bundle::readAnimationChannel(Scene*, Animation*, const char*).
     * 
     * @param animation The animation to the load channel into.
     * @param id The ID of the animation that this channel is loaded into.
     * @param target The animation target.
     * @param targetAttribute The target attribute being animated.
     * 
     * @return The animation that the channel was loaded into.
     */
    Animation* readAnimationChannelData(Animation* animation, const char* id, AnimationTarget* target, unsigned int targetAttribute);

    /**
     * Sets the transformation matrix.
     *
     * @param values A pointer to array of 16 floats.
     * @param transform The transform to set the values in.
     */
    void setTransform(const float* values, Transform* transform);

    /**
     * Resolves joint references for all pending mesh skins.
     */
    void resolveJointReferences(Scene* sceneContext, Node* nodeContext);

    bool _bGPBX = false;
    unsigned char _version[2];
    std::string _path;
    std::string _materialPath;
    std::vector<Reference> _references;
    std::unique_ptr<Stream> _stream;

    ModelPtr _model;

    // std::vector<MeshSkinData*> _meshSkins;
    std::map<std::string, Node*>* _trackedNodes;
};

} // namespace el {
