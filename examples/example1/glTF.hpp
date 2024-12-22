#ifndef GLTF_HPP
#define GLTF_HPP

#include "ResourceManager.hpp"

namespace gltf {

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

gerium_sint32_t attributeAccessorIndex(const Attribute* attributes,
                                       gerium_uint32_t attributeCount,
                                       std::string_view attributeName);

void getMeshVertexBuffer(glTF& gltf,
                         const std::vector<::Buffer>& buffers,
                         gerium_sint32_t accessorIndex,
                         ::Buffer& out,
                         gerium_uint32_t& offset,
                         glm::vec3* min = nullptr,
                         glm::vec3* max = nullptr);

void loadGlTF(glTF& gltf, const std::filesystem::path& path);

} // namespace gltf

#endif
