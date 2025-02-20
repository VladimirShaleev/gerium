#include "Application.hpp"
#include "services/GameService.hpp"
#include "services/InputService.hpp"
#include "services/PhysicsService.hpp"
#include "services/RenderService.hpp"
#include "services/SceneService.hpp"
#include "services/TimeService.hpp"

#include "components/Camera.hpp"
#include "components/Collider.hpp"
#include "components/Constraint.hpp"
#include "components/LocalTransform.hpp"
#include "components/Name.hpp"
#include "components/Node.hpp"
#include "components/Renderable.hpp"
#include "components/RigidBody.hpp"
#include "components/Static.hpp"
#include "components/Vehicle.hpp"
#include "components/Wheel.hpp"
#include "components/WorldTransform.hpp"

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

static entt::entity vechicle = {};
static entt::entity wheelLf  = {};
static entt::entity wheelLb  = {};
static entt::entity wheelRf  = {};
static entt::entity wheelRb  = {};

void addModel(entt::registry& registry,
              entt::entity parent,
              const Cluster& cluster,
              const Model& model,
              const glm::vec3 position,
              bool isStatic) {
    auto root = registry.create();
    registry.emplace<Node>(root);
    auto& transform = registry.emplace<WorldTransform>(root);

    transform.matrix     = glm::translate(glm::identity<glm::mat4>(), position);
    transform.prevMatrix = transform.matrix;

    struct Hierarchy {
        gerium_sint32_t nodeIndex;
        gerium_sint32_t childLevel;
        entt::entity parent;
        glm::mat4 parentMatrix;
    };

    std::map<std::string, Collider> meshColliders;

    std::queue<Hierarchy> nodesToVisit;

    for (gerium_sint32_t i = 0; i < (gerium_sint32_t) model.nodes.size(); ++i) {
        if (model.nodes[i].parent < 0) {
            nodesToVisit.emplace(i, model.nodes[i].level + 1, root, transform.matrix);
        }
    }

    while (!nodesToVisit.empty()) {
        const auto [nodeIndex, childLevel, parent, parentMatrix] = nodesToVisit.front();
        nodesToVisit.pop();

        const glm::vec3& scale     = model.nodes[nodeIndex].scale;
        const glm::vec3& translate = model.nodes[nodeIndex].position;
        const glm::quat& rotation  = model.nodes[nodeIndex].rotation;

        auto matS = glm::scale(glm::identity<glm::mat4>(), scale);
        auto matT = glm::translate(glm::identity<glm::mat4>(), translate);
        auto matR = glm::mat4_cast(rotation);
        auto mat  = matT * matR * matS;

        auto worldMatrix = parentMatrix * mat;

        auto node            = registry.create();
        auto& transform      = registry.emplace<WorldTransform>(node);
        transform.matrix     = worldMatrix;
        transform.prevMatrix = transform.matrix;
        transform.scale      = scale;

        auto& lt    = registry.emplace<LocalTransform>(node);
        lt.isDirty  = false;
        lt.position = translate;
        lt.rotation = rotation;
        lt.scale    = scale;

        registry.emplace<Node>(node).parent = parent;

        for (const auto& mesh : model.meshes) {
            if (mesh.nodeIndex == nodeIndex) {
                auto& renderable = registry.get_or_emplace<Renderable>(node);
                if (isStatic) {
                    registry.emplace_or_replace<Static>(node);
                }
                renderable.meshes.push_back({});
                auto& meshData = renderable.meshes.back();

                meshData.model = model.name;
                meshData.mesh  = mesh.meshIndex;

                // if (name == "vehicle") {
                //     meshData.mesh = 10;
                // }

                if (!model.materials.empty()) {
                    auto& mat = model.materials[mesh.materialIndex];

                    static int ii = 0;

                    meshData.material.name                     = ii++ % 2 ? TECH_OTHER_ID : TECH_PBR_ID; // mat.name;
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

                    if (ii == 1) { // vechicle
                        const auto& bbox   = model.nodes[nodeIndex].bbox;
                        auto& body         = registry.emplace<RigidBody>(node);
                        body.mass          = 6000.0f;
                        auto& collider     = registry.emplace<Collider>(node);
                        auto colliderIndex = model.nodes[nodeIndex].colliderIndex;
                        if (colliderIndex >= 0) {
                            collider.shape              = Collider::Shape::Mesh;
                            collider.data.colliderIndex = colliderIndex;
                        } else {
                            collider.shape     = Collider::Shape::Box;
                            collider.data.size = (bbox.max() - bbox.min()) * 0.5f;
                        }
                        vechicle = node;
                        registry.emplace<Vehicle>(node);
                    }
                    if (ii == 3) { // lf
                        const auto& bbox     = model.nodes[nodeIndex].bbox;
                        auto size            = bbox.max() - bbox.min();
                        auto& body           = registry.emplace<RigidBody>(node);
                        body.mass            = 120.0f;
                        auto& collider       = registry.emplace<Collider>(node);
                        collider.shape       = Collider::Shape::Sphere;
                        collider.data.radius = glm::max(glm::max(size.x, size.y), size.z) * 0.5f;
                        wheelLf              = node;
                        auto& wheel          = registry.emplace<Wheel>(node);
                        wheel.point          = worldMatrix[3].xyz();
                        wheel.position       = Wheel::FrontLeft;
                    }
                    if (ii == 2) { // lb
                        const auto& bbox     = model.nodes[nodeIndex].bbox;
                        auto size            = bbox.max() - bbox.min();
                        auto& body           = registry.emplace<RigidBody>(node);
                        body.mass            = 200.0f;
                        auto& collider       = registry.emplace<Collider>(node);
                        collider.shape       = Collider::Shape::Sphere;
                        collider.data.radius = glm::max(glm::max(size.x, size.y), size.z) * 0.5f;
                        wheelLb              = node;
                        auto& wheel          = registry.emplace<Wheel>(node);
                        wheel.point          = worldMatrix[3].xyz();
                        wheel.position       = Wheel::BackLeft1;
                    }
                    if (ii == 5) { // rf
                        const auto& bbox     = model.nodes[nodeIndex].bbox;
                        auto size            = bbox.max() - bbox.min();
                        auto& body           = registry.emplace<RigidBody>(node);
                        body.mass            = 120.0f;
                        auto& collider       = registry.emplace<Collider>(node);
                        collider.shape       = Collider::Shape::Sphere;
                        collider.data.radius = glm::max(glm::max(size.x, size.y), size.z) * 0.5f;
                        wheelRf              = node;
                        auto& wheel          = registry.emplace<Wheel>(node);
                        wheel.point          = worldMatrix[3].xyz();
                        wheel.position       = Wheel::FrontRight;
                    }
                    if (ii == 4) { // rb
                        const auto& bbox     = model.nodes[nodeIndex].bbox;
                        auto size            = bbox.max() - bbox.min();
                        auto& body           = registry.emplace<RigidBody>(node);
                        body.mass            = 200.0f;
                        auto& collider       = registry.emplace<Collider>(node);
                        collider.shape       = Collider::Shape::Sphere;
                        collider.data.radius = glm::max(glm::max(size.x, size.y), size.z) * 0.5f;
                        wheelRb              = node;
                        auto& wheel          = registry.emplace<Wheel>(node);
                        wheel.point          = worldMatrix[3].xyz();
                        wheel.position       = Wheel::BackRight1;
                    }
                    if (ii == 6) { // floor
                        const auto& bbox = model.nodes[nodeIndex].bbox;
                        auto& body       = registry.emplace<RigidBody>(node);
                        // body.mass        = 100000.0f;
                        auto& collider     = registry.emplace<Collider>(node);
                        collider.shape     = Collider::Shape::Box;
                        collider.data.size = (bbox.max() - bbox.min()) * 0.5f;
                    }
                    if (ii == 7) { // floor
                        const auto& bbox = model.nodes[nodeIndex].bbox;
                        auto& body       = registry.emplace<RigidBody>(node);
                        // body.mass        = 100000.0f;
                        auto& collider     = registry.emplace<Collider>(node);
                        collider.shape     = Collider::Shape::Box;
                        collider.data.size = (bbox.max() - bbox.min()) * 0.5f;
                    }
                }
            }
        }

        auto& childs = registry.get_or_emplace<Node>(parent);
        childs.childs.push_back(node);

        for (gerium_sint32_t i = 0; i < (gerium_sint32_t) model.nodes.size(); ++i) {
            if (model.nodes[i].level == childLevel && model.nodes[i].parent == nodeIndex) {
                nodesToVisit.emplace(i, model.nodes[i].level + 1, node, worldMatrix);
            }
        }
    }

    registry.get_or_emplace<Node>(parent).childs.push_back(root);
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

    _serviceManager.getService<RenderService>()->createCluster(cluster);

    auto root = _entityRegistry.create();

    // addModel(_entityRegistry, root, model3, glm::vec3(2.0f, 0.0f, 0.0f), true);
    // addModel(_entityRegistry, root, model3, glm::vec3(-2.0f, 0.0f, 0.0f), true);
    addModel(_entityRegistry, root, cluster, model3, glm::vec3(2.0f, 0.0f, -8.0f), false);
    addModel(_entityRegistry, root, cluster, model4, glm::vec3(2.0f, -8.0f, -8.0f), true);
    addModel(_entityRegistry, root, cluster, model4, glm::vec3(2.0f, -10.0f, -20.0f), true);
    // addModel(_entityRegistry, root, model1, glm::vec3(0.0f, 0.0f, 0.0f));
    // addModel(_entityRegistry, root, model2, glm::vec3(0.0f, 0.0f, 1.0f));
    // addModel(_entityRegistry, root, model1, glm::vec3(0.0f, 0.0f, 4.0f));

    auto axis = glm::vec3(1.0f, 0.0f, 0.0f);

    auto& wlf  = _entityRegistry.emplace<Constraint>(wheelLf);
    wlf.parent = vechicle;
    wlf.point  = _entityRegistry.get<WorldTransform>(wheelLf).matrix[3].xyz();
    wlf.axis   = axis;

    auto& wlb  = _entityRegistry.emplace<Constraint>(wheelLb);
    wlb.parent = vechicle;
    wlb.point  = _entityRegistry.get<WorldTransform>(wheelLb).matrix[3].xyz();
    wlb.axis   = axis;

    auto& wrf  = _entityRegistry.emplace<Constraint>(wheelRf);
    wrf.parent = vechicle;
    wrf.point  = _entityRegistry.get<WorldTransform>(wheelRf).matrix[3].xyz();
    wrf.axis   = axis;

    auto& wrb  = _entityRegistry.emplace<Constraint>(wheelRb);
    wrb.parent = vechicle;
    wrb.point  = _entityRegistry.get<WorldTransform>(wheelRb).matrix[3].xyz();
    wrb.axis   = axis;

    auto& vv = _entityRegistry.get<Vehicle>(vechicle);
    vv.wheels.push_back(wheelLf);
    vv.wheels.push_back(wheelRf);
    vv.wheels.push_back(wheelLb);
    vv.wheels.push_back(wheelRb);
    wlf.parent = vechicle;
    wrf.parent = vechicle;
    wlb.parent = vechicle;
    wrb.parent = vechicle;

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

    // auto truck = _entityRegistry.create();

    // _entityRegistry.emplace<Name>(truck).name = "truck"_hs;
    // _entityRegistry.emplace<LocalTransform>(truck);
    // _entityRegistry.emplace<WorldTransform>(truck);
    // auto wheels = _entityRegistry.emplace<Children>(truck);

    // for (int i = 0; i < 4; ++i) {
    //     auto wheel = _entityRegistry.create();

    //     auto name = "wheel_" + std::to_string(i);

    //     _entityRegistry.emplace<Name>(wheel).name = entt::hashed_string{ name.c_str(), name.length() };
    //     _entityRegistry.emplace<LocalTransform>(wheel);
    //     _entityRegistry.emplace<WorldTransform>(wheel);
    //     _entityRegistry.emplace<RigidBody>(wheel);
    //     _entityRegistry.emplace<Collider>(wheel);
    //     _entityRegistry.emplace<Parent>(wheel).parent = truck;

    //     wheels.childs.push_back(wheel);
    // }
}

void Application::uninitialize() {
    _serviceManager.destroy();
    _entityRegistry.clear();
}

void Application::frame(gerium_uint64_t elapsedMs) {
    // auto& transform = _entityRegistry.get<WorldTransform>(m3);

    // transform.prevMatrix = transform.matrix;
    // transform.matrix[3][2] -= elapsedMs * 0.001f * 1.0f;

    // _eventManager.dispatch();
    _serviceManager.update(elapsedMs);

    _serviceManager.getService<PhysicsService>()->ApplyThrottle(wheelRb, 100.0f);
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
            // _eventManager.send<WindowStateEvent>(state, width, height);
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
