#ifndef COMMON_HPP
#define COMMON_HPP

// gerium API
#include <gerium/gerium.h>

// Dear ImGui: Bloat-free Graphical User interface
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// Standard C++ libraries
#include <cmath>
#include <filesystem>
#include <limits>
#include <memory>
#include <queue>
#include <span>
#include <string_view>

// Yaml parser
#include <yaml-cpp/yaml.h>

// Open-Asset-Importer-Library
#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// Mesh optimization library that makes
// meshes smaller and faster to render
#include <meshoptimizer.h>

// C++ mathematics library for graphics software
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_SWIZZLE
#include <glm/ext.hpp>

// Very fast, high quality, platform-independent
// hashing algorithm
#include <rapidhash.h>

// Static reflection for enums
#define MAGIC_ENUM_RANGE_MAX 255
#include <magic_enum/magic_enum.hpp>

// A C++20 library for fast serialization,
// deserialization and validation using reflection
#include <rfl.hpp>
#include <rfl/capnproto.hpp>
#include <rfl/json.hpp>

// Internal dependencies for code maintenance
#include "utils/Constants.hpp"
#include "utils/Finally.hpp"
#include "utils/Functions.hpp"
#include "utils/NonCopyable.hpp"
#include "utils/NonMovable.hpp"
#include "utils/Primitives.hpp"
#include "utils/ReflectCppConverters.hpp"
#include "utils/YamlConverters.hpp"

// Defining Constants to Identify Assets
#include "Resources.hpp"

// Declaring shared structures between
// C++ code and shaders code
#include "shaders/common/types.h"

#endif
