// This header is used (include) in both shaders code and C++ code to avoid duplicate declarations.

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "defines.h"

// SceneData contains camera and scene-related data used by shaders.
struct SHADER_ALIGN SceneData {
    mat4  view;               // View matrix (camera transformation)
    mat4  viewProjection;     // View-projection matrix (camera + projection)
    mat4  prevViewProjection; // Previous frame's view-projection matrix (for motion vectors)
    mat4  invViewProjection;  // Inverse of the view-projection matrix
    vec4  viewPosition;       // Camera position in world space
    vec4  eye;                // Eye direction or camera-specific data
    vec4  frustum;            // Frustum planes for culling or clipping
    vec2  p00p11;             // Projection matrix terms
    vec2  farNear;            // Far and near clipping planes
    vec2  invResolution;      // Inverse of the screen resolution (1.0 / width, 1.0 / height)
    ivec2 resolution;         // Screen resolution (width, height)
    float lodTarget;          // Target level of detail (LOD) for mesh rendering
};

// DrawData contains data related to drawing commands.
struct SHADER_ALIGN DrawData {
    uint drawCount; // Number of draw calls to issue
};

// MeshInstance contains per-instance data for rendering a mesh.
struct SHADER_ALIGN MeshInstance {
    mat4  world;        // World transformation matrix for the instance
    mat4  prevWorld;    // Previous frame's world matrix (for motion vectors)
    mat4  normalMatrix; // Matrix for transforming normals (usually the inverse transpose of the world matrix)
    float scale;        // Scale factor for the instance
    uint  mesh;         // Index of the mesh to render
    uint  technique;    // Rendering technique to use for this instance
    uint  material;     // Index of the material to use for this instance
};

// MeshLod contains level of detail (LOD) data for a mesh.
struct MeshLod {
    uint  primitiveOffset; // Offset to the first primitive in the LOD
    uint  primitiveCount;  // Number of primitives in the LOD
    float lodError;        // Error metric for this LOD (used for LOD selection)
};

// VertexNonCompressed contains uncompressed vertex data.
struct VertexNonCompressed {
    float px, py, pz; // Vertex position (x, y, z)
    float nx, ny, nz; // Vertex normal (x, y, z)
    float tx, ty, tz; // Vertex tangent (x, y, z)
    float ts;         // Tangent sign (used for handedness)
    float tu, tv;     // Texture coordinates (u, v)
};

// MeshNonCompressed contains uncompressed mesh data.
struct MeshNonCompressed {
    float   center[3];    // Bounding sphere center (x, y, z)
    float   radius;       // Bounding sphere radius
    float   bboxMin[3];   // Minimum bounds of the axis-aligned bounding box (AABB)
    float   bboxMax[3];   // Maximum bounds of the axis-aligned bounding box (AABB)
    uint    vertexOffset; // Offset to the first vertex in the vertex buffer
    uint    vertexCount;  // Number of vertices in the mesh
    uint    lodCount;     // Number of LODs available for this mesh
    MeshLod lods[8];      // Array of LODs (up to 8 LODs supported)
};

// MaterialNonCompressed contains uncompressed material data.
struct MaterialNonCompressed {
    float baseColorFactor[4];       // Base color (RGBA)
    float emissiveFactor[3];        // Emissive color (RGB)
    float metallicFactor;           // Metallic factor (0 = dielectric, 1 = metallic)
    float roughnessFactor;          // Roughness factor (0 = smooth, 1 = rough)
    float occlusionStrength;        // Strength of ambient occlusion
    float alphaCutoff;              // Alpha cutoff for transparency (used for alpha testing)
    uint  baseColorTexture;         // Index of the base color texture
    uint  metallicRoughnessTexture; // Index of the metallic-roughness texture
    uint  normalTexture;            // Index of the normal map texture
    uint  occlusionTexture;         // Index of the occlusion texture
    uint  emissiveTexture;          // Index of the emissive texture
};

// If USE_COMPRESSED_TYPES is not defined, use uncompressed types.
#ifndef USE_COMPRESSED_TYPES

#define Vertex   VertexNonCompressed
#define Mesh     MeshNonCompressed
#define Material MaterialNonCompressed

#else

// Vertex contains compressed vertex data for reduced memory usage.
struct Vertex {
    float16_t px, py, pz; // Compressed vertex position (x, y, z)
    uint8_t   nx, ny, nz; // Compressed vertex normal (x, y, z)
    uint8_t   tx, ty, tz; // Compressed vertex tangent (x, y, z)
    int8_t    ts;         // Compressed tangent sign (used for handedness)
    float16_t tu, tv;     // Compressed texture coordinates (u, v)
};

// Mesh contains compressed mesh data for reduced memory usage.
struct Mesh {
    float16_t center[3];    // Compressed bounding sphere center (x, y, z)
    float16_t radius;       // Compressed bounding sphere radius
    float16_t bboxMin[3];   // Compressed minimum bounds of the AABB
    float16_t bboxMax[3];   // Compressed maximum bounds of the AABB
    uint      vertexOffset; // Offset to the first vertex in the vertex buffer
    uint      vertexCount;  // Number of vertices in the mesh
    uint8_t   lodCount;     // Number of LODs available for this mesh
    MeshLod   lods[8];      // Array of LODs (up to 8 LODs supported)
};

// Material contains compressed material data for reduced memory usage.
struct Material {
    float16_t baseColorFactor[4];       // Compressed base color (RGBA)
    float16_t emissiveFactor[3];        // Compressed emissive color (RGB)
    float16_t metallicFactor;           // Compressed metallic factor
    float16_t roughnessFactor;          // Compressed roughness factor
    float16_t occlusionStrength;        // Compressed occlusion strength
    float16_t alphaCutoff;              // Compressed alpha cutoff
    uint16_t  baseColorTexture;         // Index of the base color texture
    uint16_t  metallicRoughnessTexture; // Index of the metallic-roughness texture
    uint16_t  normalTexture;            // Index of the normal map texture
    uint16_t  occlusionTexture;         // Index of the occlusion texture
    uint16_t  emissiveTexture;          // Index of the emissive texture
};

#endif

// Structure specifying an indirect drawing command
struct IndirectDraw {
    uint vertexCount;   // Is the number of vertices to draw
    uint instanceCount; // Is the number of instances to draw
    uint firstVertex;   // Is the index of the first vertex to draw
    uint firstInstance; // Is the instance ID of the first instance to draw
    uint lodIndex;      // Is level of detail number
};

#endif
