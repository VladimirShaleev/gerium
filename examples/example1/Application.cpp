#include "Application.hpp"
#include "services/GameService.hpp"
#include "services/InputService.hpp"
#include "services/PhysicsService.hpp"
#include "services/RenderService.hpp"
#include "services/SceneService.hpp"
#include "services/TimeService.hpp"

#include "components/Camera.hpp"
#include "components/Collider.hpp"
#include "components/Name.hpp"
#include "components/Node.hpp"
#include "components/Renderable.hpp"
#include "components/RigidBody.hpp"
#include "components/Static.hpp"
#include "components/Transform.hpp"
#include "components/Vehicle.hpp"
#include "components/Wheel.hpp"

#include "Model.hpp"

using namespace entt::literals;

Application::Application() {
    check(gerium_logger_create("example1", &_logger));
}

Application::~Application() {
    if (_application) {
        gerium_application_destroy(_application);
        _application = nullptr;
    }
    if (_logger) {
        gerium_logger_destroy(_logger);
        _logger = nullptr;
    }
}

void Application::run(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) noexcept {
    gerium_logger_set_level_by_tag("gerium", GERIUM_LOGGER_LEVEL_VERBOSE);

    try {
        check(gerium_application_create(title, width, height, &_application));
        gerium_application_set_background_wait(_application, true);

        gerium_application_set_frame_func(_application, frame, (gerium_data_t) this);
        gerium_application_set_state_func(_application, state, (gerium_data_t) this);

        auto result = gerium_application_run(_application);
        if (_error) {
            std::rethrow_exception(_error);
        }
        check(result);
    } catch (const std::exception& exc) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
        gerium_application_show_message(_application, "example1", exc.what());
    } catch (...) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
        gerium_application_show_message(_application, "example1", "unknown error");
    }
}

glm::mat4 calcMatrix(const glm::vec3& translate, const glm::quat& rotation, const glm::vec3& scale) {
    auto matT = glm::translate(glm::identity<glm::mat4>(), translate);
    auto matR = glm::mat4_cast(rotation);
    auto matS = glm::scale(glm::identity<glm::mat4>(), scale);
    return matT * matR * matS;
}

entt::entity addModel(entt::registry& registry,
                      entt::entity parent,
                      const Model& model,
                      const glm::vec3& position = { 0.0f, 0.0f, 0.0f },
                      const glm::quat& rotation = { 1.0f, 0.0f, 0.0f, 0.0f },
                      const glm::vec3& scale    = { 1.0f, 1.0f, 1.0f }) {
    auto root = registry.create();
    registry.emplace<Node>(root);
    registry.get_or_emplace<Node>(parent).childs.push_back(root);

    auto& transform      = registry.emplace<Transform>(root);
    transform.matrix     = calcMatrix(position, rotation, scale);
    transform.prevMatrix = transform.matrix;

    struct Hierarchy {
        gerium_sint32_t nodeIndex;
        gerium_sint32_t childLevel;
        entt::entity parent;
        glm::mat4 parentMatrix;
        glm::vec3 parentScale;
    };

    std::queue<Hierarchy> nodesToVisit;

    for (gerium_sint32_t i = 0; i < (gerium_sint32_t) model.nodes.size(); ++i) {
        if (model.nodes[i].parent < 0) {
            nodesToVisit.emplace(i, model.nodes[i].level + 1, root, transform.matrix, scale);
        }
    }

    auto isStatic = true;

    entt::entity vehicle = entt::null;

    while (!nodesToVisit.empty()) {
        const auto [nodeIndex, childLevel, parent, parentMatrix, parentScale] = nodesToVisit.front();
        nodesToVisit.pop();

        const auto& modelNode = model.nodes[nodeIndex];

        const glm::vec3& localTranslate = modelNode.position;
        const glm::quat& localRotation  = modelNode.rotation;
        const glm::vec3& localScale     = modelNode.scale;

        auto node = registry.create();

        registry.emplace<Node>(node).parent = parent;
        registry.get_or_emplace<Node>(parent).childs.push_back(node);

        auto& transform      = registry.emplace<Transform>(node);
        transform.matrix     = parentMatrix * calcMatrix(localTranslate, localRotation, localScale);
        transform.prevMatrix = transform.matrix;
        transform.scale      = parentScale * localScale;

        if (modelNode.name == "vehicle"_hs) {
            isStatic = false;
            vehicle  = node;
            registry.emplace<Vehicle>(node);
        } else if (vehicle != entt::null) {
            std::optional<Wheel::Position> wheelPos = std::nullopt;
            switch (modelNode.name) {
                case "wheel_lf"_hs:
                    wheelPos = Wheel::FrontLeft;
                    break;
                case "wheel_rf"_hs:
                    wheelPos = Wheel::FrontRight;
                    break;
                case "wheel_lb"_hs:
                    wheelPos = Wheel::BackLeft1;
                    break;
                case "wheel_rb"_hs:
                    wheelPos = Wheel::BackRight1;
                    break;
                case "wheel_lb2"_hs:
                    wheelPos = Wheel::BackLeft2;
                    break;
                case "wheel_rb2"_hs:
                    wheelPos = Wheel::BackRight2;
                    break;
            }

            if (wheelPos) {
                auto& wheel    = registry.emplace<Wheel>(node);
                wheel.parent   = vehicle;
                wheel.point    = transform.matrix[3].xyz();
                wheel.position = wheelPos.value();
                registry.get<Vehicle>(vehicle).wheels.push_back(node);
            }
        }

        if (isStatic) {
            registry.emplace_or_replace<Static>(node);
        }

        for (const auto& mesh : model.meshes) {
            if (mesh.nodeIndex == nodeIndex) {
                auto& collider = registry.get_or_emplace<Collider>(node);
                auto& body     = registry.get_or_emplace<RigidBody>(node);
                body.mass      = 6000.0f;

                if (auto colliderIndex = modelNode.colliderIndex; colliderIndex >= 0) {
                    collider.shape = Collider::Shape::Mesh;
                    collider.index = colliderIndex;
                } else {
                    collider.shape = Collider::Shape::Box;
                    collider.size  = (modelNode.bbox.max() - modelNode.bbox.min()) * 0.5f;
                }

                auto& renderable = registry.get_or_emplace<Renderable>(node);
                renderable.meshes.push_back({});
                auto& meshData = renderable.meshes.back();

                meshData.model = model.name;
                meshData.mesh  = mesh.meshIndex;

                if (!model.materials.empty()) {
                    auto& mat = model.materials[mesh.materialIndex];

                    meshData.material.name                     = TECH_PBR_ID; // mat.name;
                    meshData.material.baseColorTexture         = mat.baseColorTexture;
                    meshData.material.metallicRoughnessTexture = mat.metallicRoughnessTexture;
                    meshData.material.normalTexture            = mat.normalTexture;
                    meshData.material.occlusionTexture         = mat.occlusionTexture;
                    meshData.material.emissiveTexture          = mat.emissiveTexture;
                    meshData.material.baseColorFactor          = mat.baseColorFactor;
                    meshData.material.emissiveFactor           = mat.emissiveFactor;
                    meshData.material.metallicFactor           = mat.metallicFactor;
                    meshData.material.roughnessFactor          = mat.roughnessFactor;
                    meshData.material.occlusionStrength        = mat.occlusionStrength;
                    meshData.material.alphaCutoff              = mat.alphaCutoff;
                    meshData.material.flags                    = (MaterialFlags) mat.flags;
                }
            }
        }

        for (gerium_sint32_t i = 0; i < (gerium_sint32_t) model.nodes.size(); ++i) {
            if (model.nodes[i].level == childLevel && model.nodes[i].parent == nodeIndex) {
                nodesToVisit.emplace(i, model.nodes[i].level + 1, node, transform.matrix, transform.scale);
            }
        }
    }

    return root;
}

struct Archive {
    template <typename Arg>
    Archive& operator()(Arg&& arg) {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg)>, Camera>) {
            auto result       = rfl::capnproto::write(arg);
            const auto schema = rfl::capnproto::to_schema<std::remove_cvref_t<decltype(arg)>>();
            auto read         = rfl::capnproto::read<std::remove_cvref_t<decltype(arg)>>(result).value();
            auto i            = 0;
        }
        return *this;
    }
};

void Application::initialize() {
    Cluster cluster{};
    auto model1 = loadModel(cluster, "model1");
    auto model2 = loadModel(cluster, "model2");
    auto model3 = loadModel(cluster, "truck");
    auto model4 = loadModel(cluster, MODEL_FLOOR_ID);

    _serviceManager.create(this);
    _serviceManager.addService<TimeService>();
    _serviceManager.addService<InputService>();
    _serviceManager.addService<GameService>();
    _serviceManager.addService<PhysicsService>();
    _serviceManager.addService<SceneService>();
    _serviceManager.addService<RenderService>();

    auto root = _entityRegistry.create();
    addModel(_entityRegistry, root, model3, glm::vec3(2.0f, 0.0f, -8.0f));
    addModel(_entityRegistry, root, model4, glm::vec3(2.0f, -8.0f, -8.0f));
    addModel(_entityRegistry, root, model4, glm::vec3(2.0f, -10.0f, -20.0f));

    _serviceManager.getService<RenderService>()->createCluster(cluster);
    _serviceManager.getService<RenderService>()->createStaticInstances();
    _serviceManager.getService<PhysicsService>()->createBodies(cluster);
    cluster = {};

    auto& camera = _entityRegistry.emplace<Camera>(_entityRegistry.create());

    camera.active        = true;
    camera.movementSpeed = 2.0f;
    camera.rotationSpeed = 0.001f;
    camera.position      = { -3.0f, 0.0f, 0.0f };
    camera.yaw           = 0.0f;
    camera.pitch         = 0.0f;
    camera.nearPlane     = 0.01f;
    camera.farPlane      = 1000.0f;
    camera.fov           = glm::radians(60.0f);

    Archive archive;
    entt::snapshot{ _entityRegistry }.get<Camera>(archive);
}

void Application::uninitialize() {
    _serviceManager.destroy();
    _entityRegistry.clear();
}

void Application::frame(gerium_uint64_t elapsedMs) {
    _serviceManager.update(elapsedMs);
}

void Application::state(gerium_application_state_t state) {
    const auto stateStr = magic_enum::enum_name(state);
    gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_DEBUG, stateStr.data());

    gerium_uint16_t width, height;
    gerium_application_get_size(_application, &width, &height);

    switch (state) {
        case GERIUM_APPLICATION_STATE_INITIALIZE:
            initialize();
            break;
        case GERIUM_APPLICATION_STATE_UNINITIALIZE:
            uninitialize();
            break;
        case GERIUM_APPLICATION_STATE_GOT_FOCUS:
        case GERIUM_APPLICATION_STATE_LOST_FOCUS:
        case GERIUM_APPLICATION_STATE_VISIBLE:
        case GERIUM_APPLICATION_STATE_INVISIBLE:
        case GERIUM_APPLICATION_STATE_NORMAL:
        case GERIUM_APPLICATION_STATE_MINIMIZE:
        case GERIUM_APPLICATION_STATE_MAXIMIZE:
        case GERIUM_APPLICATION_STATE_FULLSCREEN:
        case GERIUM_APPLICATION_STATE_RESIZED:
            break;
        default:
            break;
    }
}

gerium_bool_t Application::frame(gerium_application_t /* application */,
                                 gerium_data_t data,
                                 gerium_uint64_t elapsedMs) {
    auto app = (Application*) data;
    try {
        app->frame(elapsedMs);
    } catch (...) {
        app->uninitialize();
        app->_error = std::current_exception();
        return false;
    }
    return true;
}

gerium_bool_t Application::state(gerium_application_t /* application */,
                                 gerium_data_t data,
                                 gerium_application_state_t state) {
    auto app = (Application*) data;
    try {
        app->state(state);
    } catch (...) {
        app->uninitialize();
        app->_error = std::current_exception();
        return false;
    }
    return true;
}
