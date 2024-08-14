#include "glTF.hpp"

using namespace std::string_literals;

using json = nlohmann::json;

namespace gltf {

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

gerium_sint32_t attributeAccessorIndex(const Attribute* attributes,
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

void getMeshVertexBuffer(glTF& gltf,
                         const std::vector<gerium_buffer_h>& buffers,
                         gerium_sint32_t accessorIndex,
                         gerium_buffer_h& out,
                         gerium_uint32_t& offset) {
    if (accessorIndex != -1) {
        auto& bufferAccessor = gltf.accessors[accessorIndex];
        auto& bufferView     = gltf.bufferViews[bufferAccessor.bufferView];

        out    = buffers[bufferAccessor.bufferView];
        offset = bufferAccessor.byteOffset == INVALID_INT_VALUE ? 0 : bufferAccessor.byteOffset;
    } else {
        out    = { std::numeric_limits<gerium_uint16_t>::max() };
        offset = 0;
    }
}

void loadGlTF(glTF& gltf, const std::filesystem::path& path) {
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

} // namespace gltf
