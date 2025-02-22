#include "Snapshot.hpp"
#include "components/Camera.hpp"
#include "components/Collider.hpp"
#include "components/Name.hpp"
#include "components/Node.hpp"
#include "components/Renderable.hpp"
#include "components/RigidBody.hpp"
#include "components/Settings.hpp"
#include "components/Static.hpp"
#include "components/Transform.hpp"
#include "components/Vehicle.hpp"
#include "components/VehicleController.hpp"
#include "components/Wheel.hpp"

namespace rfl::capnproto {

template <class T, class... Ps>
struct SchemaHolder2 {
    static SchemaHolder2<T, Ps...> make() noexcept {
        const auto internal_schema = parsing::schema::make<Reader, Writer, T, Processors<Ps...>>();
        const auto str             = to_string_representation(internal_schema);
        return SchemaHolder2<T, Ps...>{ Schema<T>::from_string(str) };
    }

    rfl::Result<Schema<T>> schema_;
};

template <class T, class... Ps>
static const SchemaHolder2<T, Ps...> schema_holder2 = SchemaHolder2<T, Ps...>::make();

template <class T, class... Ps>
Schema<T> to_schema2() {
    return schema_holder2<T, Ps...>.schema_.value();
}

template <class... Ps>
std::vector<uint8_t> write2(const auto& _obj, const auto& _schema) {
    using T          = std::remove_cvref_t<decltype(_obj)>;
    using U          = typename std::remove_cvref_t<decltype(_schema)>::Type;
    using ParentType = parsing::Parent<Writer>;
    static_assert(std::is_same<T, U>(), "The schema must be compatible with the type to write.");
    const auto root_name   = get_root_name<T, Ps...>();
    const auto root_schema = _schema.value().getNested(root_name.c_str());
    capnp::MallocMessageBuilder message_builder;
    auto root         = message_builder.initRoot<capnp::DynamicStruct>(root_schema.asStruct());
    const auto writer = Writer(&root);
    Parser<T, Processors<Ps...>>::write(writer, _obj, typename ParentType::Root{});
    kj::VectorOutputStream output_stream;
    capnp::writePackedMessage(output_stream, message_builder);
    auto arr_ptr = output_stream.getArray();
    return std::vector<uint8_t>(internal::ptr_cast<uint8_t*>(arr_ptr.begin()),
                                internal::ptr_cast<uint8_t*>(arr_ptr.end()));
}

template <class... Ps>
std::vector<uint8_t> write2(const auto& _obj) {
    using T           = std::remove_cvref_t<decltype(_obj)>;
    const auto schema = to_schema2<T, Ps...>();
    return write2<Ps...>(_obj, schema);
}

template <class T, class... Ps>
auto read2(const InputVarType& _obj) {
    const auto r = Reader();
    return Parser<T, Processors<Ps...>>::read(r, _obj);
}

template <class T, class... Ps>
Result<internal::wrap_in_rfl_array_t<T>> read2(const gerium_uint8_t* _bytes,
                                               const size_t _size,
                                               const Schema<T>& _schema) {
    const auto array_ptr   = kj::ArrayPtr<const kj::byte>(internal::ptr_cast<const kj::byte*>(_bytes), _size);
    auto input_stream      = kj::ArrayInputStream(array_ptr);
    auto message_reader    = capnp::PackedMessageReader(input_stream);
    const auto root_name   = get_root_name<std::remove_cv_t<T>, Ps...>();
    const auto root_schema = _schema.value().getNested(root_name.c_str());
    const auto input_var   = InputVarType{ message_reader.getRoot<capnp::DynamicStruct>(root_schema.asStruct()) };
    return read2<T, Ps...>(input_var);
}

template <class T, class... Ps>
auto read2(const gerium_uint8_t* _bytes, const size_t _size) {
    const auto schema = to_schema2<std::remove_cvref_t<T>, Ps...>();
    return read2<T, Ps...>(_bytes, _size, schema);
}

template <class T, class... Ps>
auto read2(const std::vector<gerium_uint8_t>& _bytes) {
    return read2<T, Ps...>(_bytes.data(), _bytes.size());
}

} // namespace rfl::capnproto

namespace {

template <typename T>
struct Component {
    entt::entity entity;
    T data;
};

struct Snapshot {
    gerium_uint32_t head;
    std::vector<entt::entity> entities;
    std::vector<Component<Node>> nodes;
    std::vector<Component<Static>> statics;
    std::vector<Component<Transform>> transforms;
    std::vector<Component<RigidBody>> rigidBodies;
    std::vector<Component<Collider>> colliders;
    std::vector<Component<Wheel>> wheels;
    std::vector<Component<Vehicle>> vehicles;
    std::vector<Component<VehicleController>> vehicleControllers;
    std::vector<Component<Camera>> cameras;
    std::vector<Component<Renderable>> renderables;
};

struct ArchiveEntities final {
    ArchiveEntities(gerium_uint32_t& head, std::vector<entt::entity>& entities) noexcept :
        _head(head),
        _entities(entities) {
    }

    void operator()(auto&& value) {
        if (!_sizeSaved) {
            _sizeSaved = true;
            _entities.resize(size_t(value), entt::null);
        } else if (!_headSaved) {
            _headSaved = true;
            _head      = gerium_uint32_t(value);
        } else {
            _entities[_pushed++] = entt::entity{ value };
        }
    }

    gerium_uint32_t& _head;
    std::vector<entt::entity>& _entities;

    bool _sizeSaved{};
    bool _headSaved{};
    gerium_uint32_t _pushed{};
};

struct InputArchiveEntities final {
    InputArchiveEntities(gerium_uint32_t head, const std::vector<entt::entity>& entities) noexcept :
        _head(head),
        _entities(entities) {
    }

    void operator()(auto& value) {
        using Type = std::remove_cvref_t<decltype(value)>;
        if (!_sizeReaded) {
            _sizeReaded = true;
            value       = Type(_entities.size());
        } else if (!_headReaded) {
            _headReaded = true;
            value       = Type(_head);
        } else {
            value = Type(_entities[_pulled++]);
        }
    }

    gerium_uint32_t _head;
    const std::vector<entt::entity>& _entities;

    bool _sizeReaded{};
    bool _headReaded{};
    gerium_uint32_t _pulled{};
};

template <typename C>
struct ArchiveComponents final {
    using ComponentsType = std::vector<Component<C>>;

    ArchiveComponents(ComponentsType& components) noexcept : _components(components) {
    }

    template <typename T>
    void operator()(T&& value) {
        using Type = std::remove_cvref_t<T>;

        constexpr auto isInteger = std::is_same_v<Type, unsigned int> || std::is_same_v<Type, entt::entity>;

        if constexpr (isInteger) {
            if (!_sizeSaved) {
                _sizeSaved = true;
                _components.resize(size_t(value));
            } else {
                _components[_pushed++].entity = entt::entity{ value };
            }
        } else {
            _components[_pushed - 1].data = value;
        }
    }

    ComponentsType& _components;

    bool _sizeSaved{};
    gerium_uint32_t _pushed{};
};

template <typename C>
struct InputArchiveComponents final {
    using ComponentsType = std::vector<Component<C>>;

    InputArchiveComponents(const ComponentsType& components) noexcept : _components(components) {
    }

    template <typename T>
    void operator()(T& value) {
        using Type = std::remove_cvref_t<T>;

        constexpr auto isInteger = std::is_same_v<Type, unsigned int> || std::is_same_v<Type, entt::entity>;

        if constexpr (isInteger) {
            if (!_sizeReaded) {
                _sizeReaded = true;
                value       = Type(_components.size());
            } else {
                value = Type(_components[_pulled++].entity);
            }
        } else {
            value = _components[_pulled - 1].data;
        }
    }

    const ComponentsType& _components;

    bool _sizeReaded{};
    gerium_uint32_t _pulled{};
};

} // namespace

std::vector<gerium_uint8_t> makeSnapshot(const entt::registry& registry, SnapshotFormat format) {
    Snapshot snapshotData;
    auto archiveEntities           = ArchiveEntities(snapshotData.head, snapshotData.entities);
    auto archiveNodes              = ArchiveComponents(snapshotData.nodes);
    auto archiveStatics            = ArchiveComponents(snapshotData.statics);
    auto archiveTransforms         = ArchiveComponents(snapshotData.transforms);
    auto archiveRigidBodies        = ArchiveComponents(snapshotData.rigidBodies);
    auto archiveColliders          = ArchiveComponents(snapshotData.colliders);
    auto archiveWheels             = ArchiveComponents(snapshotData.wheels);
    auto archiveVehicles           = ArchiveComponents(snapshotData.vehicles);
    auto archiveVehicleControllers = ArchiveComponents(snapshotData.vehicleControllers);
    auto archiveCameras            = ArchiveComponents(snapshotData.cameras);
    auto archiveRenderables        = ArchiveComponents(snapshotData.renderables);

    entt::snapshot{ registry }
        .get<entt::entity>(archiveEntities)
        .get<Node>(archiveNodes)
        .get<Static>(archiveStatics)
        .get<Transform>(archiveTransforms)
        .get<RigidBody>(archiveRigidBodies)
        .get<Collider>(archiveColliders)
        .get<Wheel>(archiveWheels)
        .get<Vehicle>(archiveVehicles)
        .get<VehicleController>(archiveVehicleControllers)
        .get<Camera>(archiveCameras)
        .get<Renderable>(archiveRenderables);

    switch (format) {
        case SnapshotFormat::Json: {
            const auto json = rfl::json::write(snapshotData, rfl::json::pretty);
            auto it         = (const uint8_t*) json.data();
            return std::vector<gerium_uint8_t>(it, it + json.size());
        }
        case SnapshotFormat::Capnproto: {
            return rfl::capnproto::write2(snapshotData);
        }
        default: {
            assert(!"unreachable code");
            return {};
        }
    }
}

bool loadSnapshot(entt::registry& registry, SnapshotFormat format, const std::vector<gerium_uint8_t>& data) {
    rfl::Result<Snapshot> snapshotData = Snapshot{};
    switch (format) {
        case SnapshotFormat::Json:
            snapshotData = rfl::json::read<Snapshot>(std::string((char*) data.data(), data.size()));
            break;
        case SnapshotFormat::Capnproto:
            snapshotData = rfl::capnproto::read2<Snapshot>(data);
            break;
        default:
            return false;
    }
    if (!snapshotData) {
        return false;
    }

    const auto& result = snapshotData.value();

    auto archiveEntities           = InputArchiveEntities(result.head, result.entities);
    auto archiveNodes              = InputArchiveComponents(result.nodes);
    auto archiveStatics            = InputArchiveComponents(result.statics);
    auto archiveTransforms         = InputArchiveComponents(result.transforms);
    auto archiveRigidBodies        = InputArchiveComponents(result.rigidBodies);
    auto archiveColliders          = InputArchiveComponents(result.colliders);
    auto archiveWheels             = InputArchiveComponents(result.wheels);
    auto archiveVehicles           = InputArchiveComponents(result.vehicles);
    auto archiveVehicleControllers = InputArchiveComponents(result.vehicleControllers);
    auto archiveCameras            = InputArchiveComponents(result.cameras);
    auto archiveRenderables        = InputArchiveComponents(result.renderables);

    registry.clear();
    entt::snapshot_loader{ registry }
        .get<entt::entity>(archiveEntities)
        .get<Node>(archiveNodes)
        .get<Static>(archiveStatics)
        .get<Transform>(archiveTransforms)
        .get<RigidBody>(archiveRigidBodies)
        .get<Collider>(archiveColliders)
        .get<Wheel>(archiveWheels)
        .get<Vehicle>(archiveVehicles)
        .get<VehicleController>(archiveVehicleControllers)
        .get<Camera>(archiveCameras)
        .get<Renderable>(archiveRenderables)
        .orphans();

    return true;
}
