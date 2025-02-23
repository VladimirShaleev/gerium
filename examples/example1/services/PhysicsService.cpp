#include "PhysicsService.hpp"
#include "../components/Static.hpp"

using namespace entt::literals;

constexpr float mUpdateFrequency    = 60.0f;
constexpr float mRequestedDeltaTime = 1.0f / mUpdateFrequency;

void PhysicsService::createBodies(const Cluster& cluster) {
    auto view = entityRegistry().view<RigidBody, Collider, Transform>(entt::exclude<Wheel>);

    for (auto entity : view) {
        auto& rigidBody       = view.get<RigidBody>(entity);
        const auto& collider  = view.get<Collider>(entity);
        const auto& transform = view.get<Transform>(entity);
        const auto vehicle    = entityRegistry().try_get<Vehicle>(entity);
        const auto isDynamic  = !entityRegistry().any_of<::Static>(entity);

        glm::vec3 scale;
        glm::quat orientation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transform.matrix, scale, orientation, translation, skew, perspective);

        const auto position = JPH::Vec3(translation.x, translation.y, translation.z);
        const auto rotation = JPH::Quat(orientation.x, orientation.y, orientation.z, orientation.w);

        auto shape = getShape(collider, cluster);
        shape->ScaleShape(JPH::Vec3(transform.scale.x, transform.scale.y, transform.scale.z));

        JPH::BodyCreationSettings settings(shape,
                                           position,
                                           rotation,
                                           isDynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                                           isDynamic ? Dynamic : Static);

        settings.mOverrideMassProperties       = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMassPropertiesOverride.mMass = rigidBody.mass;
        settings.mLinearDamping                = rigidBody.linearDamping;
        settings.mAngularDamping               = rigidBody.angularDamping;

        auto body = _physicsSystem->GetBodyInterface().CreateBody(settings);

        auto bodyID      = body->GetID();
        rigidBody.bodyID = bodyID.GetIndexAndSequenceNumber();
        _physicsSystem->GetBodyInterface().AddBody(
            bodyID, isDynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);

        if (vehicle) {
            createVehicleConstraints(entity, *vehicle, rigidBody);
        }
    }

    _physicsSystem->OptimizeBroadPhase();
}

void PhysicsService::start() {
    JPH::RegisterDefaultAllocator();

    JPH::Factory::sInstance = new JPH::Factory();

    JPH::RegisterTypes();

    auto numThreads = std::thread::hardware_concurrency() - 1;

    _allocator                     = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
    _jobSystem                     = std::make_unique<JPH::JobSystemThreadPool>(512, 8, numThreads);
    _broadPhaseLayerInterface      = std::make_unique<BroadPhaseLayerInterface>(this);
    _objectVsBroadPhaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilter>(this);
    _objectLayerPairFilter         = std::make_unique<ObjectLayerPairFilter>(this);

    JPH::PhysicsSettings settings{};
    settings.mMaxInFlightBodyPairs                 = 16384;
    settings.mStepListenersBatchSize               = 8;
    settings.mStepListenerBatchesPerJob            = 1;
    settings.mBaumgarte                            = 0.2f;
    settings.mSpeculativeContactDistance           = 0.02f;
    settings.mPenetrationSlop                      = 0.02f;
    settings.mLinearCastThreshold                  = 0.75f;
    settings.mLinearCastMaxPenetration             = 0.25f;
    settings.mManifoldToleranceSq                  = 0.001f;
    settings.mMaxPenetrationDistance               = 0.2f;
    settings.mBodyPairCacheMaxDeltaPositionSq      = 0.000001f;
    settings.mBodyPairCacheCosMaxDeltaRotationDiv2 = 0.99984769515639123915701155881391f;
    settings.mContactNormalCosMaxDeltaRotation     = 0.99619469809174553229501040247389f;
    settings.mContactPointPreserveLambdaMaxDistSq  = 0.0001f;
    settings.mNumVelocitySteps                     = 10;
    settings.mNumPositionSteps                     = 2;
    settings.mMinVelocityForRestitution            = 1.0f;
    settings.mTimeBeforeSleep                      = 0.5f;
    settings.mPointVelocitySleepThreshold          = 0.03f;
    settings.mDeterministicSimulation              = true;
    settings.mConstraintWarmStart                  = true;
    settings.mUseBodyPairContactCache              = true;
    settings.mUseManifoldReduction                 = true;
    settings.mUseLargeIslandSplitter               = true;
    settings.mAllowSleeping                        = true;
    settings.mCheckActiveEdges                     = true;

    _physicsSystem = std::make_unique<JPH::PhysicsSystem>();
    _physicsSystem->Init(MAX_INSTANCES_PER_TECHNIQUE,
                         0,
                         MAX_INSTANCES_PER_TECHNIQUE,
                         MAX_INSTANCES_PER_TECHNIQUE,
                         *_broadPhaseLayerInterface.get(),
                         *_objectVsBroadPhaseLayerFilter.get(),
                         *_objectLayerPairFilter.get());
    _physicsSystem->SetPhysicsSettings(settings);
    _vehicleTester = new JPH::VehicleCollisionTesterCastCylinder(Dynamic);
}

void PhysicsService::stop() {
    if (_physicsSystem) {
        for (auto& [_, constraint] : _vehicleConstraints) {
            _physicsSystem->RemoveStepListener(constraint);
        }
    }

    _vehicleConstraints.clear();
    _vehicleTester                 = nullptr;
    _physicsSystem                 = nullptr;
    _objectLayerPairFilter         = nullptr;
    _objectVsBroadPhaseLayerFilter = nullptr;
    _broadPhaseLayerInterface      = nullptr;
    _jobSystem                     = nullptr;
    _allocator                     = nullptr;

    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsService::update(gerium_uint64_t /* elapsedMs */, gerium_float64_t elapsed) {
    float worldDeltaTime = 0.0f;

    static float mResidualDeltaTime = 0.0f;

    worldDeltaTime = (float) elapsed + mResidualDeltaTime;

    if (worldDeltaTime < mRequestedDeltaTime) {
        mResidualDeltaTime = worldDeltaTime;
        worldDeltaTime     = 0.0f;
    } else {
        mResidualDeltaTime = min(mRequestedDeltaTime, worldDeltaTime - mRequestedDeltaTime);
        worldDeltaTime     = mRequestedDeltaTime;
    }

    if (worldDeltaTime > 0.0f) {
        step();
    }
}

void PhysicsService::step() {
    auto view = entityRegistry().view<Vehicle>();
    for (auto entity : view) {
        auto& vehicle = view.get<Vehicle>(entity);
        driverInput(entity, vehicle);
    }

    _physicsSystem->Update(mRequestedDeltaTime, 1, _allocator.get(), _jobSystem.get());

    syncPhysicsToECS();
}

void PhysicsService::createVehicleConstraints(entt::entity entity, Vehicle& vehicle, RigidBody& rigidBody) {
    JPH::VehicleConstraintSettings constraint;
    constraint.mMaxPitchRollAngle = vehicle.maxRollAngle;
    constraint.mWheels.resize(vehicle.wheels.size());

    size_t frontWheels = 0;
    size_t rearWheels  = 0;

    for (const auto& wheelEntity : vehicle.wheels) {
        const auto& wheel = entityRegistry().get<Wheel>(wheelEntity);

        const auto index = int(wheel.position) / 2;
        if (index == 0) {
            ++frontWheels;
        } else {
            ++rearWheels;
        }

        const auto& wheelCollider = entityRegistry().get<Collider>(wheelEntity);

        constraint.mWheels[int(wheel.position)] =
            createWheelSettings(vehicle, wheel, wheelCollider.halfExtent.z, wheelCollider.halfExtent.x);
    }

    if (vehicle.antiRollbar) {
        constraint.mAntiRollBars.resize(vehicle.wheels.size() / 2);
        for (auto wheelEntity : vehicle.wheels) {
            const auto& wheel = entityRegistry().get<Wheel>(wheelEntity);
            const auto index  = int(wheel.position) / 2;
            const auto isLeft = int(wheel.position) % 2 == 0;

            if (isLeft) {
                constraint.mAntiRollBars[index].mLeftWheel = int(wheel.position);
            } else {
                constraint.mAntiRollBars[index].mRightWheel = int(wheel.position);
            }
        }
    }

    size_t differentials;
    switch (vehicle.wheelDrive) {
        case WheelDrive::Front:
            differentials = frontWheels / 2;
            break;
        case WheelDrive::Rear:
            differentials = rearWheels / 2;
            break;
        case WheelDrive::All:
            differentials = (frontWheels + rearWheels) / 2;
            break;
    }
    const auto engineTorqueRatio = 1.0f / differentials;

    auto controller        = new JPH::WheeledVehicleControllerSettings;
    constraint.mController = controller;
    controller->mDifferentials.resize(differentials);
    for (auto wheelEntity : vehicle.wheels) {
        const auto& wheel = entityRegistry().get<Wheel>(wheelEntity);
        auto index        = int(wheel.position) / 2;
        const auto isLeft = int(wheel.position) % 2 == 0;
        if ((index == 0 && vehicle.wheelDrive == WheelDrive::Front) ||
            (index != 0 && vehicle.wheelDrive == WheelDrive::Rear) || vehicle.wheelDrive == WheelDrive::All) {
            if (vehicle.wheelDrive == WheelDrive::Rear) {
                index = 0;
            }
            if (isLeft) {
                controller->mDifferentials[index].mLeftWheel = int(wheel.position);
            } else {
                controller->mDifferentials[index].mRightWheel = int(wheel.position);
            }
            controller->mDifferentials[index].mEngineTorqueRatio = engineTorqueRatio;
        }
    }

    auto body = _physicsSystem->GetBodyLockInterface().TryGetBody(JPH::BodyID(rigidBody.bodyID));

    _vehicleConstraints[entity] = new JPH::VehicleConstraint(*body, constraint);
    auto& vehicleConstraint     = _vehicleConstraints[entity];
    vehicleConstraint->SetVehicleCollisionTester(_vehicleTester);
    updateVehicleSettings(vehicle, vehicleConstraint);

    static_cast<JPH::WheeledVehicleController*>(vehicleConstraint->GetController())
        ->SetTireMaxImpulseCallback([](uint,
                                       float& outLongitudinalImpulse,
                                       float& outLateralImpulse,
                                       float inSuspensionImpulse,
                                       float inLongitudinalFriction,
                                       float inLateralFriction,
                                       float,
                                       float,
                                       float) {
        outLongitudinalImpulse = 10.0f * inLongitudinalFriction * inSuspensionImpulse;
        outLateralImpulse      = inLateralFriction * inSuspensionImpulse;
    });

    _physicsSystem->AddConstraint(vehicleConstraint);
    _physicsSystem->AddStepListener(vehicleConstraint);
}

entt::hashed_string PhysicsService::stateName() const noexcept {
    return "physics_service"_hs;
}

std::vector<gerium_uint8_t> PhysicsService::saveState() {
    struct Stream : JPH::StreamOut {
        void WriteBytes(const void* inData, size_t inNumBytes) override {
            auto offset = data.size();
            data.resize(offset + inNumBytes);
            memcpy(data.data() + offset, inData, inNumBytes);
        }

        bool IsFailed() const override {
            return false;
        }

        std::vector<gerium_uint8_t> data;
    };

    Stream stream;
    JPH::Ref<JPH::PhysicsScene> scene = new JPH::PhysicsScene();
    scene->FromPhysicsSystem(_physicsSystem.get());
    scene->SaveBinaryState(stream, true, true);
    return std::move(stream.data);
}

void PhysicsService::restoreState(const std::vector<gerium_uint8_t>& data) {
    stop();
    start();

    struct Stream : JPH::StreamIn {
        explicit Stream(const std::vector<gerium_uint8_t>& d) noexcept : data(d) {
        }

        void ReadBytes(void* outData, size_t inNumBytes) override {
            auto count = std::min(data.size() - offset, inNumBytes);
            if (count) {
                memcpy(outData, data.data() + offset, count);
                offset += count;
            }
        }

        bool IsEOF() const override {
            return data.size() - offset == 0;
        }

        bool IsFailed() const override {
            return false;
        }

        const std::vector<gerium_uint8_t>& data;
        size_t offset{};
    };

    Stream stream(data);
    auto result = JPH::PhysicsScene::sRestoreFromBinaryState(stream);
    if (result.HasError()) {
        throw std::runtime_error(result.GetError().c_str());
    }
    auto scene = result.Get();

    scene->CreateBodies(_physicsSystem.get());

    auto view = entityRegistry().view<Vehicle, RigidBody>(entt::exclude<::Static, Wheel>);

    for (auto entity : view) {
        auto& vehicle   = view.get<Vehicle>(entity);
        auto& rigidBody = view.get<RigidBody>(entity);
        createVehicleConstraints(entity, vehicle, rigidBody);
    }
}

void PhysicsService::driverInput(entt::entity entity, Vehicle& vehicle) {
    auto& constraint = _vehicleConstraints[entity];

    auto body = constraint->GetVehicleBody();

    auto brake = 0.0f;
    if (vehicle.previousForward * vehicle.forward < 0.0f) {
        auto velocity = (body->GetRotation().Conjugated() * body->GetLinearVelocity()).GetZ();
        if ((vehicle.forward > 0.0f && velocity < -0.1f) || (vehicle.forward < 0.0f && velocity > 0.1f)) {
            vehicle.forward = 0.0f;
            brake           = 1.0f;
        } else {
            vehicle.previousForward = vehicle.forward;
        }
    }

    auto handBrakeValue = 0.0f;
    if (vehicle.handBrakePressed) {
        vehicle.forward = 0.0f;
        handBrakeValue  = 1.0f;
    }

    if (vehicle.forward != 0.0f || vehicle.right != 0.0f || brake != 0.0f || handBrakeValue != 0.0f) {
        _physicsSystem->GetBodyInterface().ActivateBody(constraint->GetVehicleBody()->GetID());
    }

    auto controller = static_cast<JPH::WheeledVehicleController*>(constraint->GetController());

    updateVehicleSettings(vehicle, constraint);
    controller->SetDriverInput(vehicle.forward, -vehicle.right, brake, handBrakeValue);

    vehicle.forward          = 0.0f;
    vehicle.right            = 0.0f;
    vehicle.handBrakePressed = false;
}

void PhysicsService::updateVehicleSettings(const Vehicle& vehicle, JPH::Ref<JPH::VehicleConstraint>& constraint) {
    auto controller    = static_cast<JPH::WheeledVehicleController*>(constraint->GetController());
    auto& engine       = controller->GetEngine();
    auto& transmission = controller->GetTransmission();

    engine.mMaxTorque            = vehicle.maxTorque;
    engine.mMinRPM               = vehicle.minRPM;
    engine.mMaxRPM               = vehicle.maxRPM;
    transmission.mClutchStrength = vehicle.clutchStrength;
    transmission.mSwitchTime     = vehicle.switchTime;

    if (transmission.mGearRatios.size() != vehicle.gearRatios.size()) {
        transmission.mGearRatios.resize(vehicle.gearRatios.size());
    }

    for (size_t i = 0; i < vehicle.gearRatios.size(); ++i) {
        transmission.mGearRatios[i] = vehicle.gearRatios[i];
    }

    controller->SetDifferentialLimitedSlipRatio(vehicle.limitedSlipRatio);
    for (auto& differential : controller->GetDifferentials()) {
        differential.mLimitedSlipRatio = vehicle.limitedSlipRatio;
    }
}

void PhysicsService::syncPhysicsToECS() {
    auto view = entityRegistry().view<RigidBody, Transform>(entt::exclude<::Static, Wheel>);

    for (auto entity : view) {
        auto& rigidBody = view.get<RigidBody>(entity);
        auto& transform = view.get<Transform>(entity);
        auto vehicle    = entityRegistry().try_get<Vehicle>(entity);

        JPH::BodyID bodyId(rigidBody.bodyID);
        const auto mat = _physicsSystem->GetBodyInterface().GetWorldTransform(bodyId);

        transform.prevMatrix = transform.matrix;
        memcpy((void*) &transform.matrix, (void*) &mat, sizeof(glm::mat4));

        if (vehicle) {
            for (auto wheelEntity : vehicle->wheels) {
                auto& wheel          = entityRegistry().get<Wheel>(wheelEntity);
                auto& wheelTransform = entityRegistry().get<Transform>(wheelEntity);
                auto wheelMat        = _vehicleConstraints[entity]->GetWheelWorldTransform(
                    (JPH::uint) wheel.position, JPH::Vec3::sAxisX(), JPH::Vec3::sAxisY());
                wheelTransform.prevMatrix = wheelTransform.matrix;
                memcpy((void*) &wheelTransform.matrix, (void*) &wheelMat, sizeof(glm::mat4));
            }
        }
    }
}

JPH::WheelSettings* PhysicsService::createWheelSettings(const Vehicle& vehicle,
                                                        const Wheel& wheel,
                                                        gerium_float32_t radius,
                                                        gerium_float32_t width) {
    const auto isFront = wheel.position == WheelPosition::FrontLeft || wheel.position == WheelPosition::FrontRight;

    const auto isRight = wheel.position == WheelPosition::FrontRight || wheel.position == WheelPosition::BackRight1 ||
                         wheel.position == WheelPosition::BackRight2;

    const JPH::Vec3 flipX(-1, 1, 1);

    auto getSuspensionDirection = [&vehicle, isFront, isRight, &flipX]() {
        const auto suspensionSidewaysAngle =
            isFront ? vehicle.frontSuspensionSidewaysAngle : vehicle.rearSuspensionSidewaysAngle;
        const auto suspensionForwardAngle =
            isFront ? vehicle.frontSuspensionForwardAngle : vehicle.rearSuspensionForwardAngle;
        const auto result =
            JPH::Vec3(JPH::Tan(suspensionSidewaysAngle), -1, JPH::Tan(suspensionForwardAngle)).Normalized();
        return isRight ? flipX * result : result;
    };

    auto getSteeringAxis = [&vehicle, isFront]() {
        const auto kingPinAngle = isFront ? vehicle.frontKingPinAngle : vehicle.rearKingPinAngle;
        const auto casterAngle  = isFront ? vehicle.frontCasterAngle : vehicle.rearCasterAngle;
        return JPH::Vec3(-JPH::Tan(kingPinAngle), 1, -JPH::Tan(casterAngle)).Normalized();
    };

    auto getWheelUp = [&vehicle, isFront]() {
        const auto camber = isFront ? vehicle.frontCamber : vehicle.rearCamber;
        return JPH::Vec3(JPH::Sin(camber), JPH::Cos(camber), 0);
    };

    auto getWheelForward = [&vehicle, isFront, isRight, &flipX]() {
        const auto toe    = isFront ? vehicle.frontToe : vehicle.rearToe;
        const auto result = JPH::Vec3(-JPH::Sin(toe), 0, JPH::Cos(toe));
        return isRight ? flipX * result : result;
    };

    auto getSuspensionMinLength = [&vehicle, isFront]() {
        return isFront ? vehicle.frontSuspensionMinLength : vehicle.rearSuspensionMinLength;
    };

    auto getSuspensionMaxLength = [&vehicle, isFront]() {
        return isFront ? vehicle.frontSuspensionMaxLength : vehicle.rearSuspensionMaxLength;
    };

    auto getSuspensionFrequency = [&vehicle, isFront]() {
        return isFront ? vehicle.frontSuspensionFrequency : vehicle.rearSuspensionFrequency;
    };

    auto getSuspensionDamping = [&vehicle, isFront]() {
        return isFront ? vehicle.frontSuspensionDamping : vehicle.rearSuspensionDamping;
    };

    auto getMaxSteerAngle = [&vehicle, isFront]() {
        return isFront ? vehicle.maxSteeringAngle : 0.0f;
    };

    auto getMaxHandBrakeTorque = [&vehicle, isFront]() {
        if (vehicle.handBrake == WheelDrive::All) {
            return vehicle.maxHandBrakeTorque;
        }
        if (vehicle.handBrake == WheelDrive::Front) {
            return isFront ? vehicle.maxHandBrakeTorque : 0.0f;
        }
        return isFront ? 0.0f : vehicle.maxHandBrakeTorque;
    };

    const auto wPosition = JPH::Vec3(wheel.point.x, wheel.point.y + getSuspensionMinLength(), wheel.point.z);

    auto settings                          = new JPH::WheelSettingsWV;
    settings->mSuspensionDirection         = getSuspensionDirection();
    settings->mAngularDamping              = vehicle.angularDamping;
    settings->mSteeringAxis                = getSteeringAxis();
    settings->mWheelUp                     = getWheelUp();
    settings->mWheelForward                = getWheelForward();
    settings->mSuspensionMinLength         = getSuspensionMinLength();
    settings->mSuspensionMaxLength         = getSuspensionMaxLength();
    settings->mSuspensionSpring.mFrequency = getSuspensionFrequency();
    settings->mSuspensionSpring.mDamping   = getSuspensionDamping();
    settings->mMaxSteerAngle               = getMaxSteerAngle();
    settings->mMaxHandBrakeTorque          = getMaxHandBrakeTorque();
    settings->mPosition                    = wPosition;
    settings->mRadius                      = radius;
    settings->mWidth                       = width;
    return settings;
}

JPH::Ref<JPH::Shape> PhysicsService::getShape(const Collider& collider, const Cluster& cluster) {
    JPH::Ref<JPH::Shape> shape{};
    switch (collider.shape) {
        case Shape::Box: {
            const auto& size = collider.halfExtent;
            JPH::BoxShapeSettings shape(JPH::Vec3(size.x, size.y, size.z));
            shape.SetEmbedded();
            return shape.Create().Get();
        }
        case Shape::Sphere: {
            JPH::SphereShapeSettings shape(collider.radius);
            shape.SetEmbedded();
            return shape.Create().Get();
        }
        case Shape::Capsule: {
            JPH::CapsuleShapeSettings shape(collider.halfHeightOfCylinder, collider.radius);
            shape.SetEmbedded();
            return shape.Create().Get();
        }
        case Shape::ConvexHull: {
            const auto convexHulls = cluster.convexHullColliders[collider.index];
            JPH::Array<JPH::Vec3> vertices;
            JPH::StaticCompoundShapeSettings shape;
            for (const auto& convexHull : convexHulls.convexHulls) {
                vertices.resize(convexHull.vertices.size());
                for (size_t i = 0; i < convexHull.vertices.size(); ++i) {
                    const auto& vertex = convexHull.vertices[i];
                    vertices[i].SetX(vertex.x);
                    vertices[i].SetY(vertex.y);
                    vertices[i].SetZ(vertex.z);
                }
                JPH::ConvexHullShapeSettings convexShape(vertices);
                convexShape.SetEmbedded();
                shape.AddShape(JPH::Vec3(0.0f, 0.0f, 0.0f), JPH::Quat::sIdentity(), convexShape.Create().Get());
            }
            shape.SetEmbedded();
            return shape.Create().Get();
        }
        case Shape::Mesh: {
            const auto& meshCollider = cluster.meshColliders[collider.index];
            JPH::VertexList vertices;
            JPH::IndexedTriangleList indices;
            vertices.resize(meshCollider.vertices.size());
            indices.resize(meshCollider.indices.size() / 3);
            for (int i = 0; i < meshCollider.vertices.size(); ++i) {
                vertices[i].x = meshCollider.vertices[i].x;
                vertices[i].y = meshCollider.vertices[i].y;
                vertices[i].z = meshCollider.vertices[i].z;
            }
            for (int i = 0; i < meshCollider.indices.size() / 3; ++i) {
                indices[i].mIdx[0] = meshCollider.indices[i * 3 + 2];
                indices[i].mIdx[1] = meshCollider.indices[i * 3 + 1];
                indices[i].mIdx[2] = meshCollider.indices[i * 3 + 0];
            }
            JPH::MeshShapeSettings shape(vertices, indices);
            shape.SetEmbedded();
            return shape.Create().Get();
        }
        default:
            assert(!"unreachable code");
            return {};
    }
}

PhysicsService::BroadPhaseLayerInterface::BroadPhaseLayerInterface(PhysicsService* service) noexcept :
    _service(service) {
}

JPH::uint PhysicsService::BroadPhaseLayerInterface::GetNumBroadPhaseLayers() const {
    return magic_enum::enum_count<ObjectLayers>();
}

JPH::BroadPhaseLayer PhysicsService::BroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const {
    switch (inLayer) {
        case ObjectLayers::Static:
            return JPH::BroadPhaseLayer(0);
        case ObjectLayers::Dynamic:
            return JPH::BroadPhaseLayer(1);
        default:
            return JPH::BroadPhaseLayer(0);
    }
}

PhysicsService::ObjectVsBroadPhaseLayerFilter::ObjectVsBroadPhaseLayerFilter(PhysicsService* service) noexcept :
    _service(service) {
}

bool PhysicsService::ObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1,
                                                                  JPH::BroadPhaseLayer inLayer2) const {
    switch (inLayer1) {
        case Static:
            return inLayer2 == JPH::BroadPhaseLayer(Dynamic);
        case Dynamic:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
    }
}

PhysicsService::ObjectLayerPairFilter::ObjectLayerPairFilter(PhysicsService* service) noexcept : _service(service) {
}

bool PhysicsService::ObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const {
    switch (inLayer1) {
        case Static:
            return inLayer2 == Dynamic;
        case Dynamic:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
    }
}
