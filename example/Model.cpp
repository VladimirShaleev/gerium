#include "Model.hpp"

#include <limits>
#include <nlohmann/json.hpp>
#include <queue>
#include <string_view>

using namespace std::string_literals;

using json = nlohmann::json;

constexpr gerium_uint32_t INVALID_INT_VALUE    = std::numeric_limits<gerium_uint32_t>::max();
constexpr gerium_float32_t INVALID_FLOAT_VALUE = std::numeric_limits<gerium_float32_t>::max();

enum class ComponentType {
    Byte          = 5120,
    UnsignedByte  = 5121,
    Short         = 5122,
    UnsignedShort = 5123,
    UnsignedInt   = 5125,
    Float         = 5126
};

enum class Type {
    Scalar,
    Vec2,
    Vec3,
    Vec4,
    Mat2,
    Mat3,
    Mat4
};

enum class Target {
    ArrayBuffer        = 34962,
    ElementArrayBuffer = 3496
};

enum class Mode {
    Points        = 0,
    Lines         = 1,
    LineLoop      = 2,
    LineStrip     = 3,
    Triangles     = 4,
    TriangleStrip = 5,
    TriangleFan   = 6
};

enum class Filter {
    Nearest              = 9728,
    Linear               = 9729,
    NearestMipmapNearest = 9984,
    LinearMipmapNearest  = 9985,
    NearestMipmapLinear  = 9986,
    LinearMipmapLinear   = 9987
};

enum class Wrap {
    ClampToEdge    = 33071,
    MirroredRepeat = 33648,
    Repeat         = 10497
};

struct Accessor {
    gerium_uint32_t bufferView;
    gerium_uint32_t byteOffset;
    ComponentType componentType;
    gerium_uint32_t count;
    std::vector<float> max;
    std::vector<float> min;
    Type type;
};

struct BufferView {
    gerium_uint32_t buffer;
    gerium_uint32_t byteLength;
    gerium_uint32_t byteOffset;
    gerium_uint32_t byteStride;
    Target target;
    std::string name;
};

struct Buffer {
    gerium_uint32_t byteLength;
    std::string uri;
    std::string name;
};

struct Attribute {
    std::string key;
    gerium_uint32_t accessorIndex;
};

struct MeshPrimitive {
    std::vector<Attribute> attributes;
    gerium_uint32_t indices;
    gerium_uint32_t material;
    Mode mode;
};

struct MeshPrimitives {
    std::vector<MeshPrimitive> primitives;
    std::vector<gerium_float32_t> weights;
    std::string name;
};

struct Image {
    gerium_uint32_t bufferView;
    std::string mimeType;
    std::string uri;
};

struct Texture {
    gerium_uint32_t sampler;
    gerium_uint32_t source;
    std::string name;
};

struct Sampler {
    Filter magFilter;
    Filter minFilter;
    Wrap wrapS;
    Wrap wrapT;
};

struct Optional {
    bool has;
};

struct TextureInfo : Optional {
    gerium_uint32_t index;
    gerium_uint32_t texCoord;
};

struct MaterialNormalTextureInfo : TextureInfo {
    gerium_float32_t scale;
};

struct MaterialOcclusionTextureInfo : TextureInfo {
    gerium_uint32_t index;
    gerium_uint32_t texCoord;
    gerium_float32_t strength;
};

struct MaterialPBRMetallicRoughness : Optional {
    std::vector<gerium_float32_t> baseColorFactor;
    TextureInfo baseColorTexture;
    gerium_float32_t metallicFactor;
    TextureInfo metallicRoughnessTexture;
    gerium_float32_t roughnessFactor;
};

struct Material {
    gerium_float32_t alphaCutoff;
    std::string alphaMode;
    bool doubleSided;
    std::vector<gerium_float32_t> emissiveFactor;
    TextureInfo emissiveTexture;
    MaterialNormalTextureInfo normalTexture;
    MaterialOcclusionTextureInfo occlusionTexture;
    MaterialPBRMetallicRoughness pbrMetallicRoughness;
    std::string name;
};

struct Node {
    gerium_uint32_t mesh;
    std::vector<gerium_uint32_t> children;
    std::optional<glm::mat4> matrix;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 translation;
    std::string name;
};

struct Scene {
    std::vector<gerium_uint32_t> nodes;
};

struct glTF {
    gerium_uint32_t scene;
    std::vector<Accessor> accessors;
    std::vector<BufferView> bufferViews;
    std::vector<Buffer> buffers;
    std::vector<MeshPrimitives> meshes;
    std::vector<Image> images;
    std::vector<Texture> textures;
    std::vector<Sampler> samplers;
    std::vector<Material> materials;
    std::vector<Node> nodes;
    std::vector<Scene> scenes;
};

template <typename T>
static void readInt(json& jsonData, std::string_view key, T& value) {
    value = (T) jsonData.value(key, INVALID_INT_VALUE);
}

template <typename T>
static void readFloat(json& jsonData, std::string_view key, T& value) {
    value = (T) jsonData.value(key, INVALID_FLOAT_VALUE);
}

static void readString(json& jsonData, std::string_view key, std::string& value) {
    value = jsonData.value(key, "");
}

template <typename T>
static void readArray(json& jsonData, std::string_view key, std::vector<T>& array) {
    auto it = jsonData.find(key);
    if (it == jsonData.end()) {
        return;
    }
    array.resize(it->size());
    for (size_t i = 0; i < array.size(); ++i) {
        array[i] = (T) it->at(i);
    }
}

static void reatType(json& jsonData, std::string_view key, Type& type) {
    std::string value = jsonData.value(key, "");
    if (value == "SCALAR") {
        type = Type::Scalar;
    } else if (value == "VEC2") {
        type = Type::Vec2;
    } else if (value == "VEC3") {
        type = Type::Vec3;
    } else if (value == "VEC4") {
        type = Type::Vec4;
    } else if (value == "MAT2") {
        type = Type::Mat2;
    } else if (value == "MAT3") {
        type = Type::Mat3;
    } else if (value == "MAT4") {
        type = Type::Mat4;
    } else {
        assert(!"unknown glTF accessor type");
    }
}

static void readTextureInfo(json& jsonData, std::string_view key, TextureInfo& textureInfo) {
    textureInfo = {};

    auto it = jsonData.find(key);
    if (it == jsonData.end()) {
        return;
    }

    textureInfo.has = true;
    readInt(*it, "index", textureInfo.index);
    readInt(*it, "texCoord", textureInfo.texCoord);
}

static void readMaterialNormalTextureInfo(json& jsonData,
                                          std::string_view key,
                                          MaterialNormalTextureInfo& textureInfo) {
    readTextureInfo(jsonData, key, textureInfo);
    if (textureInfo.has) {
        readFloat(jsonData[key], "scale", textureInfo.scale);
    }
}

static void readMaterialOcclusionTextureInfo(json& jsonData,
                                             std::string_view key,
                                             MaterialOcclusionTextureInfo& textureInfo) {
    readTextureInfo(jsonData, key, textureInfo);
    if (textureInfo.has) {
        readFloat(jsonData[key], "strength", textureInfo.strength);
    }
}

static void readMaterialPBRMetallicRoughness(json& jsonData,
                                             std::string_view key,
                                             MaterialPBRMetallicRoughness& textureInfo) {
    textureInfo = {};

    auto it = jsonData.find(key);
    if (it == jsonData.end()) {
        return;
    }

    textureInfo.has = true;
    readArray(*it, "baseColorFactor", textureInfo.baseColorFactor);
    readTextureInfo(*it, "baseColorTexture", textureInfo.baseColorTexture);
    readFloat(*it, "metallicFactor", textureInfo.metallicFactor);
    readTextureInfo(*it, "metallicRoughnessTexture", textureInfo.metallicRoughnessTexture);
    readFloat(*it, "roughnessFactor", textureInfo.roughnessFactor);
}

static void loadAccessor(Accessor& accessor, json& jsonData) {
    readInt(jsonData, "bufferView", accessor.bufferView);
    readInt(jsonData, "byteOffset", accessor.byteOffset);
    readInt(jsonData, "componentType", accessor.componentType);
    readInt(jsonData, "count", accessor.count);
    readArray(jsonData, "max", accessor.max);
    readArray(jsonData, "min", accessor.min);
    reatType(jsonData, "type", accessor.type);
}

static void loadBufferView(BufferView& bufferView, json& jsonData) {
    readInt(jsonData, "buffer", bufferView.buffer);
    readInt(jsonData, "byteLength", bufferView.byteLength);
    readInt(jsonData, "byteOffset", bufferView.byteOffset);
    readInt(jsonData, "byteStride", bufferView.byteStride);
    readInt(jsonData, "target", bufferView.target);
    readString(jsonData, "name", bufferView.name);
}

static void loadBuffer(Buffer& buffer, json& jsonData) {
    readInt(jsonData, "byteLength", buffer.byteLength);
    readString(jsonData, "uri", buffer.uri);
    readString(jsonData, "name", buffer.name);
}

static void loadMeshPrimitive(MeshPrimitive& meshPrimitive, json& jsonData) {
    readInt(jsonData, "indices", meshPrimitive.indices);
    readInt(jsonData, "material", meshPrimitive.material);
    readInt(jsonData, "mode", meshPrimitive.mode);

    json attributes = jsonData["attributes"];
    meshPrimitive.attributes.resize(attributes.size());

    uint32_t index = 0;
    for (auto json_attribute : attributes.items()) {
        Attribute& attribute    = meshPrimitive.attributes[index];
        attribute.key           = json_attribute.key();
        attribute.accessorIndex = json_attribute.value();
        ++index;
    }
}

static void loadMesh(MeshPrimitives& mesh, json& jsonData) {
    json array = jsonData["primitives"];
    mesh.primitives.resize(array.size());
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        loadMeshPrimitive(mesh.primitives[i], array[i]);
    }
    readArray(jsonData, "weights", mesh.weights);
    readString(jsonData, "name", mesh.name);
}

static void loadImage(Image& image, json& jsonData) {
    readInt(jsonData, "bufferView", image.bufferView);
    readString(jsonData, "mimeType", image.mimeType);
    readString(jsonData, "uri", image.uri);
}

static void loadTexture(Texture& texture, json& jsonData) {
    readInt(jsonData, "sampler", texture.sampler);
    readInt(jsonData, "source", texture.source);
    readString(jsonData, "name", texture.name);
}

static void loadSampler(Sampler& sampler, json& jsonData) {
    readInt(jsonData, "magFilter", sampler.magFilter);
    readInt(jsonData, "minFilter", sampler.minFilter);
    readInt(jsonData, "wrapS", sampler.wrapS);
    readInt(jsonData, "wrapT", sampler.wrapT);
}

static void loadMaterial(Material& material, json& jsonData) {
    readFloat(jsonData, "alphaCutoff", material.alphaCutoff);
    readString(jsonData, "alphaMode", material.alphaMode);
    readInt(jsonData, "doubleSided", material.doubleSided);
    readArray(jsonData, "emissiveFactor", material.emissiveFactor);
    readTextureInfo(jsonData, "emissiveTexture", material.emissiveTexture);
    readMaterialNormalTextureInfo(jsonData, "normalTexture", material.normalTexture);
    readMaterialOcclusionTextureInfo(jsonData, "occlusionTexture", material.occlusionTexture);
    readMaterialPBRMetallicRoughness(jsonData, "pbrMetallicRoughness", material.pbrMetallicRoughness);
    readString(jsonData, "name", material.name);
}

static void loadNode(Node& node, json& jsonData) {
    std::vector<gerium_float32_t> matrix;
    std::vector<gerium_float32_t> rotation;
    std::vector<gerium_float32_t> scale;
    std::vector<gerium_float32_t> translation;

    readInt(jsonData, "mesh", node.mesh);
    readArray(jsonData, "children", node.children);
    readArray(jsonData, "matrix", matrix);
    readArray(jsonData, "rotation", rotation);
    readArray(jsonData, "scale", scale);
    readArray(jsonData, "translation", translation);
    readString(jsonData, "name", node.name);

    if (matrix.size()) {
        node.matrix = glm::make_mat4(matrix.data());
    }
    if (rotation.size()) {
        node.rotation = glm::quat(rotation[3], rotation[0], rotation[1], rotation[2]);
    } else {
        node.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
    if (scale.size()) {
        node.scale = glm::make_vec3(scale.data());
    } else {
        node.scale = glm::vec3(1.0f);
    }
    if (translation.size()) {
        node.translation = glm::make_vec3(translation.data());
    }
}

static void loadScene(Scene& scene, json& jsonData) {
    readArray(jsonData, "nodes", scene.nodes);
}

static void loadAccessors(glTF& gltf, json& jsonData) {
    json array = jsonData["accessors"];
    gltf.accessors.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadAccessor(gltf.accessors[i], array[i]);
    }
}

static void loadBufferViews(glTF& gltf, json& jsonData) {
    json array = jsonData["bufferViews"];
    gltf.bufferViews.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadBufferView(gltf.bufferViews[i], array[i]);
    }
}

static void loadBuffers(glTF& gltf, json& jsonData) {
    json array = jsonData["buffers"];
    gltf.buffers.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadBuffer(gltf.buffers[i], array[i]);
    }
}

static void loadMeshes(glTF& gltf, json& jsonData) {
    json array = jsonData["meshes"];
    gltf.meshes.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadMesh(gltf.meshes[i], array[i]);
    }
}

static void loadImages(glTF& gltf, json& jsonData) {
    json array = jsonData["images"];
    gltf.images.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadImage(gltf.images[i], array[i]);
    }
}

static void loadTextures(glTF& gltf, json& jsonData) {
    json array = jsonData["textures"];
    gltf.textures.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadTexture(gltf.textures[i], array[i]);
    }
}

static void loadSamplers(glTF& gltf, json& jsonData) {
    json array = jsonData["samplers"];
    gltf.samplers.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadSampler(gltf.samplers[i], array[i]);
    }
}

static void loadMaterials(glTF& gltf, json& jsonData) {
    json array = jsonData["materials"];
    gltf.materials.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadMaterial(gltf.materials[i], array[i]);
    }
}

static void loadNodes(glTF& gltf, json& jsonData) {
    json array = jsonData["nodes"];
    gltf.nodes.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadNode(gltf.nodes[i], array[i]);
    }
}

static void loadScenes(glTF& gltf, json& jsonData) {
    json array = jsonData["scenes"];
    gltf.scenes.resize(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        loadScene(gltf.scenes[i], array[i]);
    }
}

static void loadGlTF(glTF& gltf, const std::filesystem::path& path) {
    gerium_file_t file;
    check(gerium_file_open(path.string().c_str(), true, &file));

    auto data     = (char*) gerium_file_map(file);
    auto jsonData = json::parse(data, data + gerium_file_get_size(file));
    gerium_file_destroy(file);

    readInt(jsonData, "scene", gltf.scene);
    loadAccessors(gltf, jsonData);
    loadBufferViews(gltf, jsonData);
    loadBuffers(gltf, jsonData);
    loadMeshes(gltf, jsonData);
    loadImages(gltf, jsonData);
    loadTextures(gltf, jsonData);
    loadSamplers(gltf, jsonData);
    loadMaterials(gltf, jsonData);
    loadNodes(gltf, jsonData);
    loadScenes(gltf, jsonData);
}

static gerium_sint32_t attributeAccessorIndex(const Attribute* attributes,
                                              gerium_uint32_t attributeCount,
                                              std::string_view attributeName) {
    for (gerium_uint32_t index = 0; index < attributeCount; ++index) {
        const auto& attribute = attributes[index];
        if (attribute.key == attributeName) {
            return attribute.accessorIndex;
        }
    }

    return -1;
}

static void getMeshVertexBuffer(glTF& gltf,
                                std::vector<DataBuffer>& buffers,
                                gerium_sint32_t accessorIndex,
                                gerium_buffer_h& out,
                                gerium_uint32_t& offset) {
    if (accessorIndex != -1) {
        auto& bufferAccessor = gltf.accessors[accessorIndex];
        auto& bufferView     = gltf.bufferViews[bufferAccessor.bufferView];
        auto buffer          = buffers[bufferAccessor.bufferView];

        out    = buffer.buffer();
        offset = bufferAccessor.byteOffset == INVALID_INT_VALUE ? 0 : bufferAccessor.byteOffset;
    } else {
        out    = { std::numeric_limits<gerium_uint16_t>::max() };
        offset = 0;
    }
}

static void fillPbrMaterial(Material& material, PBRMaterial& pbrMaterial) {
    if (material.alphaMode == "MASK") {
        pbrMaterial.flags |= DrawFlags::AlphaMask;
    } else if (material.alphaMode == "BLEND") {
        pbrMaterial.flags |= DrawFlags::Transparent;
    }

    pbrMaterial.flags |= material.doubleSided ? DrawFlags::DoubleSided : DrawFlags::None;
    pbrMaterial.alphaCutoff = material.alphaCutoff != INVALID_FLOAT_VALUE ? material.alphaCutoff : 1.f;

    if (material.pbrMetallicRoughness.has) {
        if (material.pbrMetallicRoughness.baseColorFactor.size() != 0) {
            memcpy(glm::value_ptr(pbrMaterial.baseColorFactor),
                   material.pbrMetallicRoughness.baseColorFactor.data(),
                   sizeof(glm::vec4));
        } else {
            pbrMaterial.baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
        }

        pbrMaterial.metallicRoughnessOcclusionFactor.x =
            material.pbrMetallicRoughness.roughnessFactor != INVALID_FLOAT_VALUE
                ? material.pbrMetallicRoughness.roughnessFactor
                : 1.0f;
        pbrMaterial.metallicRoughnessOcclusionFactor.y =
            material.pbrMetallicRoughness.metallicFactor != INVALID_FLOAT_VALUE
                ? material.pbrMetallicRoughness.metallicFactor
                : 1.0f;

        // pbrMaterial.diffuseTextureIndex = getMaterialTexture(&material.pbr_metallic_roughness.base_color_texture);
        // pbrMaterial.roughnessTextureIndex =
        //     getMaterialTexture(&material.pbr_metallic_roughness.metallic_roughness_texture);
    }

    // pbrMaterial.occlusionTextureIndex =
    //     getMaterialTexture((material.occlusion_texture.has) ? material.occlusion_texture.index : -1);
    // pbrMaterial.normalTextureIndex =
    //     getMaterialTexture((material.normal_texture.has) ? material.normal_texture.index : -1);

    // if (material.occlusion_texture.has) {
    //     if (material.occlusion_texture.strength != vision_flow::graphic::gltf::INVALID_FLOAT_VALUE) {
    //         pbrMaterial.metallicRoughnessOcclusionFactor.z = material.occlusion_texture.strength;
    //     } else {
    //         pbrMaterial.metallicRoughnessOcclusionFactor.z = 1.0f;
    //     }
    // }
}

void Hierarchy::resize(gerium_uint32_t numNodes) {
    auto oldSize = nodesHierarchy.size();

    nodesHierarchy.resize(numNodes);
    localMatrices.resize(numNodes);
    worldMatrices.resize(numNodes);

    updatedNodes.resize(numNodes);

    memset(nodesHierarchy.data() + oldSize, 0, (numNodes - oldSize) * sizeof(NodeHierarchy));
    for (uint32_t i = 0; i < numNodes; ++i) {
        nodesHierarchy[i].parent = -1;
    }
}

void Hierarchy::setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept {
    for (uint32_t i = 0; i < updatedNodes.size(); ++i) { // TODO
        updatedNodes[i] = true;
    }
    nodesHierarchy[nodeIndex].parent = parentIndex;
    nodesHierarchy[nodeIndex].level  = level;
}

void Hierarchy::setLocalMatrix(gerium_sint32_t nodeIndex, const glm::mat4& localMatrix) noexcept {
    for (uint32_t i = 0; i < updatedNodes.size(); ++i) { // TODO
        updatedNodes[i] = true;
    }
    localMatrices[nodeIndex] = localMatrix;
}

void Hierarchy::updateMatrices() noexcept {
    gerium_uint32_t maxLevel = 0;
    for (const auto& node : nodesHierarchy) {
        maxLevel = std::max(maxLevel, (gerium_uint32_t) node.level);
    }
    gerium_uint32_t currentLevel = 0;
    gerium_uint32_t nodesVisited = 0;

    while (currentLevel <= maxLevel) {
        for (gerium_uint32_t i = 0; i < nodesHierarchy.size(); ++i) {
            if (nodesHierarchy[i].level != currentLevel) {
                continue;
            }

            if (!updatedNodes[i]) {
                continue;
            }

            updatedNodes[i] = false;

            if (nodesHierarchy[i].parent < 0) {
                worldMatrices[i] = localMatrices[i];
            } else {
                const auto& parentMatrix = worldMatrices[nodesHierarchy[i].parent];
                worldMatrices[i]         = parentMatrix * localMatrices[i];
            }

            ++nodesVisited;
        }

        ++currentLevel;
    }
}

std::shared_ptr<Model> loadGlTF(gerium_renderer_t renderer, const std::filesystem::path& path) {
    glTF gltf{};
    loadGlTF(gltf, path);

    auto model      = std::make_shared<Model>();
    model->renderer = renderer;

    std::vector<gerium_file_t> bufferFiles;
    std::vector<gerium_uint8_t*> buffers;
    bufferFiles.resize(gltf.buffers.size());
    buffers.resize(gltf.buffers.size());
    auto i = 0;
    for (const auto& buffer : gltf.buffers) {
        const auto fullPath = path.parent_path() / buffer.uri;
        check(gerium_file_open(fullPath.string().c_str(), true, &bufferFiles[i]));
        buffers[i] = (gerium_uint8_t*) gerium_file_map(bufferFiles[i]);
        ++i;
    }

    model->buffers.resize(gltf.bufferViews.size());
    i = 0;
    for (const auto& bufferView : gltf.bufferViews) {
        auto offset     = bufferView.byteOffset == INVALID_INT_VALUE ? 0 : bufferView.byteOffset;
        auto bufferData = buffers[bufferView.buffer] + offset;
        auto name       = "buffer_"s + (bufferView.name.length() ? bufferView.name : std::to_string(i));

        gerium_buffer_h buffer;
        check(gerium_renderer_create_buffer(renderer,
                                            GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_INDEX_BIT,
                                            false,
                                            name.c_str(),
                                            (gerium_cdata_t) bufferData,
                                            bufferView.byteLength,
                                            &buffer));

        model->buffers[i++].setBuffer(renderer, buffer);
    }

    for (auto& bufferFile : bufferFiles) {
        gerium_file_destroy(bufferFile);
    }

    auto& root    = gltf.scenes[gltf.scene];
    auto numNodes = root.nodes.size();

    std::queue<gerium_uint32_t> nodesToVisit;
    for (const auto& node : root.nodes) {
        nodesToVisit.push(node);
    }

    while (!nodesToVisit.empty()) {
        auto nodeIndex = nodesToVisit.front();
        nodesToVisit.pop();

        auto& node = gltf.nodes[nodeIndex];
        for (const auto& child : node.children) {
            nodesToVisit.push(child);
        }
        numNodes += node.children.size();
    }

    model->hierarchy.resize(numNodes);

    for (const auto& node : root.nodes) {
        nodesToVisit.push(node);
    }

    while (!nodesToVisit.empty()) {
        auto nodeIndex = nodesToVisit.front();
        nodesToVisit.pop();

        auto& node = gltf.nodes[nodeIndex];

        if (node.matrix) {
            model->hierarchy.setLocalMatrix(nodeIndex, node.matrix.value());
        } else {
            auto matS = glm::scale(glm::identity<glm::mat4>(), node.scale);
            auto matT = glm::translate(glm::identity<glm::mat4>(), node.translation);
            auto matR = glm::mat4_cast(node.rotation);
            auto mat  = matS * matR * matT;
            model->hierarchy.setLocalMatrix(nodeIndex, mat);
        }

        if (node.children.size()) {
            const auto& nodeHierarchy = model->hierarchy.nodesHierarchy[nodeIndex];
            for (const auto& childIndex : node.children) {
                auto& childHierarchy = model->hierarchy.nodesHierarchy[childIndex];
                model->hierarchy.setHierarchy(childIndex, nodeIndex, nodeHierarchy.level + 1);
                nodesToVisit.push(childIndex);
            }
        }

        if (node.mesh == INVALID_INT_VALUE) {
            continue;
        }

        auto& gltfMesh = gltf.meshes[node.mesh];

        for (const auto& primitive : gltfMesh.primitives) {
            model->meshes.push_back({});
            auto& mesh = model->meshes.back();

            mesh.nodeIndex = nodeIndex;

            const auto positionAccessorIndex =
                attributeAccessorIndex(primitive.attributes.data(), primitive.attributes.size(), "POSITION");
            const auto tangentAccessorIndex =
                attributeAccessorIndex(primitive.attributes.data(), primitive.attributes.size(), "TANGENT");
            const auto normalAccessorIndex =
                attributeAccessorIndex(primitive.attributes.data(), primitive.attributes.size(), "NORMAL");
            const auto texcoordAccessorIndex =
                attributeAccessorIndex(primitive.attributes.data(), primitive.attributes.size(), "TEXCOORD_0");

            getMeshVertexBuffer(gltf, model->buffers, positionAccessorIndex, mesh.positions, mesh.positionOffset);
            getMeshVertexBuffer(gltf, model->buffers, tangentAccessorIndex, mesh.tangents, mesh.tangentsOffset);
            getMeshVertexBuffer(gltf, model->buffers, normalAccessorIndex, mesh.normals, mesh.normalOffset);
            getMeshVertexBuffer(gltf, model->buffers, texcoordAccessorIndex, mesh.texcoords, mesh.texcoordsOffset);

            auto& indicesAccessor = gltf.accessors[primitive.indices];
            mesh.indexType = indicesAccessor.componentType == ComponentType::UnsignedShort ? GERIUM_INDEX_TYPE_UINT16
                                                                                           : GERIUM_INDEX_TYPE_UINT32;

            auto& indicesBufferView = gltf.bufferViews[indicesAccessor.bufferView];
            auto indicesBuffer      = model->buffers[indicesAccessor.bufferView];
            mesh.indices            = indicesBuffer.buffer();
            mesh.indexOffset        = indicesAccessor.byteOffset == INVALID_INT_VALUE ? 0 : indicesAccessor.byteOffset;
            mesh.primitiveCount     = indicesAccessor.count;

            auto& material = gltf.materials[primitive.material];
            fillPbrMaterial(material, mesh.material);

            // check(gerium_renderer_create_buffer(renderer,
            //                                     GERIUM_BUFFER_USAGE_UNIFORM_BIT,
            //                                     true,
            //                                     "mesh_data",
            //                                     nullptr,
            //                                     sizeof(MeshData),
            //                                     &mesh.material.data));

            // check(gerium_renderer_create_descriptor_set(renderer, &mesh.material.descriptorSet));
        }
    }

    return model;
}
