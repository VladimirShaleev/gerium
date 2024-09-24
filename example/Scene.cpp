#include "Scene.hpp"

#include <queue>
#include <stack>

void Scene::create(ResourceManager* resourceManger, bool bindlessEnabled) {
    _resourceManger  = resourceManger;
    _bindlessEnabled = bindlessEnabled;

    auto renderer = _resourceManger->renderer();

    gerium_texture_info_t info{};
    info.width            = 1;
    info.height           = 1;
    info.depth            = 1;
    info.mipmaps          = 1;
    info.format           = GERIUM_FORMAT_R8G8B8A8_UNORM;
    info.type             = GERIUM_TEXTURE_TYPE_2D;
    info.name             = "empty_texture";
    gerium_uint32_t white = 0xFF000000;
    _emptyTexture         = _resourceManger->createTexture(info, (gerium_cdata_t) &white);
    gerium_renderer_texture_sampler(_resourceManger->renderer(),
                                    _emptyTexture,
                                    GERIUM_FILTER_NEAREST,
                                    GERIUM_FILTER_NEAREST,
                                    GERIUM_FILTER_NEAREST,
                                    GERIUM_ADDRESS_MODE_REPEAT,
                                    GERIUM_ADDRESS_MODE_REPEAT,
                                    GERIUM_ADDRESS_MODE_REPEAT);

    if (_bindlessEnabled) {
        _bindlessTextures = _resourceManger->createDescriptorSet("", true);
        for (int i = 0; i < 1000; ++i) {
            gerium_renderer_bind_texture(renderer, _bindlessTextures, BINDLESS_BINDING, i, _emptyTexture);
        }
    }

    const auto meshDataSize = _bindlessEnabled ? sizeof(MeshDataBindless) : sizeof(MeshData);

    _meshDatas = _resourceManger->createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "mesh_data", nullptr, meshDataSize * kMaxMeshDatas);

    for (auto& set : _textureSets) {
        set = _resourceManger->createDescriptorSet("");
    }

    gerium_uint32_t width           = 1000;
    gerium_uint32_t height          = 800;
    gerium_uint32_t tileXCount      = width / TILE_SIZE;
    gerium_uint32_t tileYCount      = height / TILE_SIZE;
    gerium_uint32_t tilesEntryCount = tileXCount * tileYCount * NUM_WORDS;
    gerium_uint32_t bufferSize      = tilesEntryCount * sizeof(gerium_uint32_t);

    _lights = _resourceManger->createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "light_data", nullptr, sizeof(PointLight) * MAX_LIGHTS);
    _lightIndices = _resourceManger->createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "light_indices", nullptr, sizeof(gerium_uint32_t) * MAX_LIGHTS);
    _lightDataLUT = _resourceManger->createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "light_lut", nullptr, sizeof(gerium_uint32_t) * LIGHT_Z_BINS);
    _lightTiles = _resourceManger->createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "light_tiles", nullptr, bufferSize);
    _lightSet = _resourceManger->createDescriptorSet("");
}

SceneNode* Scene::root() {
    if (!_root) {
        _root = allocateNode();
    }
    return _root;
}

SceneNode* Scene::addNode(SceneNode* parent) {
    assert(parent);
    auto node = allocateNode();
    if (parent) {
        node->_parent = parent;
        parent->_childrens.push_back(node);
    }
    return node;
}

void Scene::update() {
    auto rootMat = glm::identity<glm::mat4>();
    std::queue<std::tuple<glm::mat4*, bool, SceneNode*>> nodes;
    nodes.push({ &rootMat, false, root() });

    bool transformUpdated = false;
    std::vector<Light*> lights;
    lights.resize(10);

    while (!nodes.empty()) {
        auto& [parentMat, parentUpdated, node] = nodes.front();
        nodes.pop();

        auto mat     = parentMat;
        auto updated = parentUpdated;

        if (auto transform = getComponentNode<Transform>(node)) {
            if (transform->updated || updated) {
                transform->worldMatrix = *mat * transform->localMatrix;
                transform->updated     = false;
                updated                = true;
                transformUpdated       = true;
            }
            mat = &transform->worldMatrix;
        }

        if (auto model = getComponentNode<Model>(node)) {
            if (model->updateMatrices(*mat, updated)) {
                transformUpdated = true;
            }
            model->updateMaterials();
        }

        gerium_uint16_t lightCount = 10;
        getComponents<Light>(lightCount, lights.data());

        for (gerium_uint16_t i = 0; i < lightCount; ++i) {
            if (lights[i]->updateBbox()) { // *mat, updated)) {
                transformUpdated = true;
            }
        }

        _registry.update(node->_entity, (gerium_data_t) this);

        for (auto& child : node->childrens()) {
            nodes.push({ mat, updated, child });
        }
    }

    if (transformUpdated) {
        if (_bvh) {
            delete _bvh;
        }
        _bvh = BVHNode::build(*this);
    }
}

void Scene::culling() {
    auto camera = getActiveCamera();

    _visibleMeshes.clear();
    _visibleLights.clear();

    std::function<void(const BVHNode*)> cullingMesh;
    cullingMesh = [this, camera, &cullingMesh](const BVHNode* node) {
        if (camera->test(node->bbox()) == Intersection::None) {
            return;
        }
        if (node->leaf()) {
            for (auto& obj : node->objects()) {
                if (obj.type == BVHNode::MeshType) {
                    if (camera->test(obj.mesh->worldBoundingBox()) != Intersection::None) {
                        obj.mesh->visible(true);
                        _visibleMeshes.push_back(obj.mesh);
                    } else {
                        obj.mesh->visible(false);
                    }
                } else if (obj.type == BVHNode::LightType) {
                    // _visibleLights.push_back(obj.light);
                }
            }
        }
        if (node->left()) {
            cullingMesh(node->left());
        }
        if (node->right()) {
            cullingMesh(node->right());
        }
    };

    ///
    _visibleLights.clear();
    _visibleLights.resize(MAX_LIGHTS);
    gerium_uint16_t ll = MAX_LIGHTS;
    getComponents<Light>(ll, _visibleLights.data());

    cullingMesh(_bvh);

    _instances.clear();
    _instancesLinear.clear();
    _techniques.clear();

    auto renderer    = _resourceManger->renderer();
    auto countMeshes = 0;

    for (const auto mesh : _visibleMeshes) {
        if (_instances.size() == kMaxDraws) {
            break;
        }
        auto& meshInstance = _instances[mesh->hash(_bindlessEnabled)];
        if (!meshInstance.mesh) {
            gerium_technique_h technique = mesh->getMaterial().getTechnique();
            _techniques.insert(technique.index);
            meshInstance.mesh = mesh;
            if (!_bindlessEnabled) {
                meshInstance.textureSet = _textureSets[_instances.size() - 1];
                gerium_renderer_bind_texture(renderer, meshInstance.textureSet, 0, 0, mesh->getMaterial().getDiffuse());
                gerium_renderer_bind_texture(renderer, meshInstance.textureSet, 1, 0, mesh->getMaterial().getNormal());
                gerium_renderer_bind_texture(
                    renderer, meshInstance.textureSet, 2, 0, mesh->getMaterial().getRoughness());
            } else {
                gerium_texture_h diffuse   = mesh->getMaterial().getDiffuse();
                gerium_texture_h normal    = mesh->getMaterial().getNormal();
                gerium_texture_h roughness = mesh->getMaterial().getRoughness();
                gerium_renderer_bind_texture(renderer, _bindlessTextures, BINDLESS_BINDING, diffuse.index, diffuse);
                gerium_renderer_bind_texture(renderer, _bindlessTextures, BINDLESS_BINDING, normal.index, normal);
                gerium_renderer_bind_texture(renderer, _bindlessTextures, BINDLESS_BINDING, roughness.index, roughness);
            }
        }
        if (meshInstance.meshDatas.size() > MAX_INSTANCES) {
            continue;
        }
        if (countMeshes > kMaxMeshDatas) {
            break;
        }
        meshInstance.meshDatas.push_back(&mesh->getMaterial().meshData());
        ++countMeshes;
    }

    auto mapPtr = gerium_renderer_map_buffer(renderer, _meshDatas, 0, 0);
    MeshDataBindless* bindlessPtr{};
    MeshData* ptr{};
    if (_bindlessEnabled) {
        bindlessPtr = (MeshDataBindless*) mapPtr;
    } else {
        ptr = (MeshData*) mapPtr;
    }
    gerium_uint16_t offset = 0;
    for (auto& [_, instance] : _instances) {
        instance.first = offset;
        instance.count = (gerium_uint16_t) instance.meshDatas.size();
        offset += instance.count;

        for (auto meshData : instance.meshDatas) {
            if (_bindlessEnabled) {
                *((MeshData*) bindlessPtr) = *meshData;
                bindlessPtr->textures.x    = ((gerium_texture_h) instance.mesh->getMaterial().getDiffuse()).index;
                bindlessPtr->textures.y    = ((gerium_texture_h) instance.mesh->getMaterial().getNormal()).index;
                bindlessPtr->textures.z    = ((gerium_texture_h) instance.mesh->getMaterial().getRoughness()).index;
                bindlessPtr->textures.w    = UndefinedHandle;
                ++bindlessPtr;
            } else {
                *ptr = *meshData;
                ++ptr;
            }
        }
    }
    gerium_renderer_unmap_buffer(renderer, _meshDatas);

    _instancesLinear.resize(_instances.size());

    int i = 0;
    for (auto& [_, instance] : _instances) {
        _instancesLinear[i++] = &instance;
    }

    if (_techniques.size() > 1) {
        std::sort(_instancesLinear.begin(), _instancesLinear.end(), [](const auto i1, const auto i2) {
            gerium_technique_h t1 = i1->mesh->getMaterial().getTechnique();
            gerium_technique_h t2 = i2->mesh->getMaterial().getTechnique();
            return t1.index < t2.index;
        });
    }

    std::vector<SortedLight> sortedLights;
    sortedLights.resize(_visibleLights.size());

    const auto& worldToCamera = camera->view();
    const auto zFar           = camera->farPlane();
    const auto zNear          = camera->nearPlane();
    for (gerium_uint32_t i = 0; i < _visibleLights.size(); ++i) {
        const auto& light = _visibleLights[i]->pointLight();

        const auto p = light.position;

        const auto projectedP    = worldToCamera * p;
        const auto projectedPMin = projectedP + glm::vec4(0.0f, 0.0f, -light.attenuation, 0.0f);
        const auto projectedPMax = projectedP + glm::vec4(0.0f, 0.0f, light.attenuation, 0.0f);

        auto& sortedLight         = sortedLights[i];
        sortedLight.index         = i;
        sortedLight.projectedZ    = ((projectedP.z - zNear) / (zFar - zNear));
        sortedLight.projectedZMin = ((projectedPMin.z - zNear) / (zFar - zNear));
        sortedLight.projectedZMax = ((projectedPMax.z - zNear) / (zFar - zNear));
    }

    std::sort(sortedLights.begin(), sortedLights.end(), [](const auto& l1, const auto& l2) {
        return l1.projectedZ < l2.projectedZ;
    });

    auto gpuLights = (PointLight*) gerium_renderer_map_buffer(renderer, _lights, 0, 0);
    for (const auto light : _visibleLights) {
        *gpuLights = light->pointLight();
        ++gpuLights;
    }
    gerium_renderer_unmap_buffer(renderer, _lights);

    const auto binSize = BIN_WIDTH;

    for (gerium_float32_t bin = 0; bin < LIGHT_Z_BINS; ++bin) {
        gerium_uint32_t minLightId = MAX_LIGHTS + 1;
        gerium_uint32_t maxLightId = 0;

        gerium_float32_t binMin = binSize * bin;
        gerium_float32_t binMax = binMin + binSize;

        for (gerium_uint32_t i = 0; i < sortedLights.size(); ++i) {
            const auto& light = sortedLights[i];

            if ((light.projectedZ >= binMin && light.projectedZ <= binMax) ||
                (light.projectedZMin >= binMin && light.projectedZMin <= binMax) ||
                (light.projectedZMax >= binMin && light.projectedZMax <= binMax)) {
                if (i < minLightId) {
                    minLightId = i;
                }

                if (i > maxLightId) {
                    maxLightId = i;
                }
            }
        }

        _lightsLUT[bin] = minLightId | (maxLightId << 16);
    }

    auto gpuLightIndices = (gerium_uint32_t*) gerium_renderer_map_buffer(renderer, _lightIndices, 0, 0);
    for (int i = 0; i < sortedLights.size(); ++i) {
        gpuLightIndices[i] = sortedLights[i].index;
    }
    gerium_renderer_unmap_buffer(renderer, _lightIndices);

    auto gpuLightLUT = (gerium_uint32_t*) gerium_renderer_map_buffer(renderer, _lightDataLUT, 0, 0);
    memcpy(gpuLightLUT, _lightsLUT.data(), _lightsLUT.size() * sizeof(gerium_uint32_t));
    gerium_renderer_unmap_buffer(renderer, _lightDataLUT);

    const gerium_uint32_t width  = camera->sceneData().resolution.x;
    const gerium_uint32_t height = camera->sceneData().resolution.y;

    const auto tileXCount      = width / TILE_SIZE;
    const auto tileYCount      = height / TILE_SIZE;
    const auto tilesEntryCount = tileXCount * tileYCount * NUM_WORDS;

    std::vector<gerium_uint32_t> lightTilesBits;
    lightTilesBits.resize(tilesEntryCount);

    gerium_float32_t tileSizeInv = 1.0f / TILE_SIZE;
    gerium_uint32_t tileStride   = tileXCount * NUM_WORDS;

    for (gerium_uint32_t i = 0; i < sortedLights.size(); ++i) {
        const gerium_float32_t lightIndex = sortedLights[i].index;
        const auto& light                 = _visibleLights[lightIndex]->pointLight();

        const auto pos    = light.position;
        const auto radius = light.attenuation;

        const auto viewSpacePos = worldToCamera * pos;
        bool cameraVisible      = -viewSpacePos.z - radius < zNear;

        if (!cameraVisible) {
            continue;
        }

        glm::vec2 aabbMin{ FLT_MAX, FLT_MAX };
        glm::vec2 aabbMax{ -FLT_MAX, -FLT_MAX };

        for (gerium_uint32_t c = 0; c < 8; ++c) {
            glm::vec3 corner{ (c % 2) ? 1.f : -1.f, (c & 2) ? 1.f : -1.f, (c & 4) ? 1.f : -1.f };
            corner *= radius;
            corner += pos.xyz();

            glm::vec4 cornerVs = worldToCamera * glm::vec4(corner, 1.0f);
            cornerVs.z = glm::max(zNear, cornerVs.z);

            glm::vec4 cornerNdc = camera->projection() * cornerVs;
            cornerNdc           = cornerNdc / cornerNdc.w;

            aabbMin.x = glm::min(aabbMin.x, cornerNdc.x);
            aabbMin.y = glm::min(aabbMin.y, cornerNdc.y);

            aabbMax.x = glm::max(aabbMax.x, cornerNdc.x);
            aabbMax.y = glm::max(aabbMax.y, cornerNdc.y);
        }

        glm::vec4 aabb{};
        aabb.x = aabbMin.x;
        aabb.y = -1 * aabbMax.y;
        aabb.z = aabbMax.x;
        aabb.w = -1 * aabbMin.y;

        const auto positionLen  = glm::length(viewSpacePos.xyz());
        const bool cameraInside = (positionLen - radius) < zNear;

        if (cameraInside) {
            aabb = { -1, -1, 1, 1 };
        }

        glm::vec4 aabbScreen{ (aabb.x * 0.5f + 0.5f) * (width - 1),
                              (aabb.y * 0.5f + 0.5f) * (height - 1),
                              (aabb.z * 0.5f + 0.5f) * (width - 1),
                              (aabb.w * 0.5f + 0.5f) * (height - 1) };

        const auto fwidth  = aabbScreen.z - aabbScreen.x;
        const auto fheight = aabbScreen.w - aabbScreen.y;

        if (fwidth < 0.0001f || fheight < 0.0001f) {
            continue;
        }

        float minX = aabbScreen.x;
        float minY = aabbScreen.y;

        float maxX = minX + fwidth;
        float maxY = minY + fheight;

        if (minX > width || minY > height) {
            continue;
        }

        if (maxX < 0.0f || maxY < 0.0f) {
            continue;
        }

        minX = std::max(minX, 0.0f);
        minY = std::max(minY, 0.0f);

        maxX = std::min(maxX, (float) width);
        maxY = std::min(maxY, (float) height);

        gerium_uint32_t firstTileX = (gerium_uint32_t) (minX * tileSizeInv);
        gerium_uint32_t lastTileX  = std::min(tileXCount, (gerium_uint32_t) (maxX * tileSizeInv));

        gerium_uint32_t firstTileY = (gerium_uint32_t) (minY * tileSizeInv);
        gerium_uint32_t lastTileY  = std::min(tileYCount, (gerium_uint32_t) (maxY * tileSizeInv));

        gerium_uint32_t wordIndex = i / 32;
        gerium_uint32_t bitIndex  = i % 32;

        for (gerium_uint32_t y = firstTileY; y < lastTileY; ++y) {
            for (gerium_uint32_t x = firstTileX; x < lastTileX; ++x) {
                gerium_uint32_t array_index = y * tileStride + x;

                lightTilesBits[array_index + wordIndex] |= 1 << bitIndex;
            }
        }
    }

    auto gpuLightTiles = (gerium_uint32_t*) gerium_renderer_map_buffer(renderer, _lightTiles, 0, 0);
    memcpy(gpuLightTiles, lightTilesBits.data(), lightTilesBits.size() * sizeof(gerium_uint32_t));
    gerium_renderer_unmap_buffer(renderer, _lightTiles);

    gerium_renderer_bind_buffer(renderer, _lightSet, 0, _lightDataLUT);
    gerium_renderer_bind_buffer(renderer, _lightSet, 1, _lights);
    gerium_renderer_bind_buffer(renderer, _lightSet, 2, _lightTiles);
    gerium_renderer_bind_buffer(renderer, _lightSet, 3, _lightIndices);
}

void Scene::clear() {
    _instances.clear();
    for (auto& set : _textureSets) {
        set = nullptr;
    }
    _lightSet         = nullptr;
    _lightTiles       = nullptr;
    _lightDataLUT     = nullptr;
    _lightIndices     = nullptr;
    _lights           = nullptr;
    _meshDatas        = nullptr;
    _emptyTexture     = nullptr;
    _bindlessTextures = nullptr;
    _registry.clear();
    _nodes.clear();
    _root = nullptr;
}

SceneNode* Scene::allocateNode() {
    _nodes.push_back(std::make_shared<SceneNode>());
    return _nodes.back().get();
}
