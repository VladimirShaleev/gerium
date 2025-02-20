#include "PhysicsService.hpp"
#include "../components/Collider.hpp"
#include "../components/Node.hpp"
#include "../components/RigidBody.hpp"
#include "../components/Static.hpp"
#include "../components/Transform.hpp"
#include "../components/Vehicle.hpp"
#include "../components/Wheel.hpp"
#include "InputService.hpp"

using namespace entt::literals;

constexpr float mUpdateFrequency    = 60.0f;
constexpr float mRequestedDeltaTime = 1.0f / mUpdateFrequency;

void PhysicsService::createBodies(const Cluster& cluster) {
    _tester = new JPH::VehicleCollisionTesterCastCylinder(Dynamic);

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

        rigidBody.body = body->GetID();
        _physicsSystem->GetBodyInterface().AddBody(
            rigidBody.body, isDynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);

        if (vehicle) {
            JPH::VehicleConstraintSettings constraint;
            constraint.mMaxPitchRollAngle = vehicle->maxRollAngle;
            constraint.mWheels.resize(vehicle->wheels.size());

            size_t frontWheels = 0;
            size_t rearWheels  = 0;

            for (const auto& wheelEntity : vehicle->wheels) {
                const auto& wheel = entityRegistry().get<Wheel>(wheelEntity);

                const auto index = int(wheel.position) / 2;
                if (index == 0) {
                    ++frontWheels;
                } else {
                    ++rearWheels;
                }

                auto localMatrix = glm::inverse(transform.matrix) * entityRegistry().get<Transform>(wheelEntity).matrix;
                auto wheelPosition = localMatrix[3].xyz();

                const auto& wheelCollider = entityRegistry().get<Collider>(wheelEntity);

                constraint.mWheels[int(wheel.position)] =
                    createWheelSettings(*vehicle, wheel, wheelPosition, wheelCollider.size.z, wheelCollider.size.x);
            }

            if (vehicle->antiRollbar) {
                constraint.mAntiRollBars.resize(vehicle->wheels.size() / 2);
                for (auto wheelEntity : vehicle->wheels) {
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
            switch (vehicle->wheelDrive) {
                case Vehicle::FrontWheelDrive:
                    differentials = frontWheels / 2;
                    break;
                case Vehicle::RearWheelDrive:
                    differentials = rearWheels / 2;
                    break;
                case Vehicle::AllWheelDrive:
                    differentials = (frontWheels + rearWheels) / 2;
                    break;
            }
            const auto engineTorqueRatio = 1.0f / differentials;

            auto controller        = new JPH::WheeledVehicleControllerSettings;
            constraint.mController = controller;
            controller->mDifferentials.resize(differentials);
            for (auto wheelEntity : vehicle->wheels) {
                const auto& wheel = entityRegistry().get<Wheel>(wheelEntity);
                auto index        = int(wheel.position) / 2;
                const auto isLeft = int(wheel.position) % 2 == 0;
                if ((index == 0 && vehicle->wheelDrive == Vehicle::FrontWheelDrive) ||
                    (index != 0 && vehicle->wheelDrive == Vehicle::RearWheelDrive) ||
                    vehicle->wheelDrive == Vehicle::AllWheelDrive) {
                    if (vehicle->wheelDrive == Vehicle::RearWheelDrive) {
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

            _car               = _physicsSystem->GetBodyLockInterface().TryGetBody(rigidBody.body);
            _vehicleConstraint = new JPH::VehicleConstraint(*_car, constraint);

            static_cast<JPH::WheeledVehicleController*>(_vehicleConstraint->GetController())
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

            _physicsSystem->AddConstraint(_vehicleConstraint);
            _physicsSystem->AddStepListener(_vehicleConstraint);
        }
    }

    _physicsSystem->OptimizeBroadPhase();
}

void PhysicsService::ApplyThrottle(entt::entity entity, float throttle) {
    // auto bodyId = entityRegistry().get<RigidBody>(entity).body;

    // _physicsSystem->GetBodyInterface().AddTorque(bodyId, JPH::Vec3(0.0f, throttle, 0.0f));
    // for (auto hh : h) {
    //     hh->SetTargetAngularVelocity(-5.0f);
    // }
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
    // _physicsSystem->SetContactListener();
}

void PhysicsService::stop() {
    // _tester

    //////////////

    _physicsSystem                 = nullptr;
    _objectLayerPairFilter         = nullptr;
    _objectVsBroadPhaseLayerFilter = nullptr;
    _broadPhaseLayerInterface      = nullptr;
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

    // syncPhysicsToECS();

    // auto&& physicsTransforms = entityRegistry().storage<Transform>("physics_transforms"_hs);

    // updatePhysicsTransforms(physicsTransforms);
    // updateLocalTransforms(physicsTransforms);

    // physicsTransforms.clear();
}

float forward         = 0.0f;
float previousForward = 1.0f;
float right           = 0.0f;
float brake           = 0.0f;
float handBrake       = 0.0f;

void PhysicsService::step() {
    auto is = manager().getService<InputService>();

    forward = 0.0f;
    if (is->isPressScancode(GERIUM_SCANCODE_NUMPAD_8)) {
        forward = 1.0f;
    } else if (is->isPressScancode(GERIUM_SCANCODE_NUMPAD_2)) {
        forward = -1.0f;
    }

    brake = 0.0f;
    if (previousForward * forward < 0.0f) {
        // Get vehicle velocity in local space to the body of the vehicle
        float velocity = (_car->GetRotation().Conjugated() * _car->GetLinearVelocity()).GetZ();
        if ((forward > 0.0f && velocity < -0.1f) || (forward < 0.0f && velocity > 0.1f)) {
            // Brake while we've not stopped yet
            forward = 0.0f;
            brake   = 1.0f;
        } else {
            // When we've come to a stop, accept the new direction
            previousForward = forward;
        }
    }

    // Hand brake will cancel gas pedal
    handBrake = 0.0f;
    if (is->isPressScancode(GERIUM_SCANCODE_NUMPAD_0)) {
        forward   = 0.0f;
        handBrake = 1.0f;
    }

    // Steering
    right = 0.0f;
    if (is->isPressScancode(GERIUM_SCANCODE_NUMPAD_4)) {
        right = -1.0f;
    } else if (is->isPressScancode(GERIUM_SCANCODE_NUMPAD_6)) {
        right = 1.0f;
    }

    if (right != 0.0f || forward != 0.0f || brake != 0.0f || handBrake != 0.0f) {
        _physicsSystem->GetBodyInterface().ActivateBody(_car->GetID());
    }

    auto controller = static_cast<JPH::WheeledVehicleController*>(_vehicleConstraint->GetController());

    const auto sLimitedSlipDifferentials = true;
    const auto sMaxEngineTorque          = 500.0;
    const auto sClutchStrength           = 10.0;
    const auto limitedSlipRatio          = sLimitedSlipDifferentials ? 1.4f : FLT_MAX;

    controller->GetEngine().mMaxTorque            = sMaxEngineTorque;
    controller->GetEngine().mMinRPM               = 1000.0f;
    controller->GetEngine().mMaxRPM               = 6000.0f;
    controller->GetTransmission().mClutchStrength = 10.0f; // sClutchStrength;
    controller->GetTransmission().mGearRatios     = { 2.66f, 1.78f, 1.3f, 1.0f, 0.74f };
    controller->GetTransmission().mSwitchTime     = 0.5f;

    controller->SetDifferentialLimitedSlipRatio(limitedSlipRatio);
    for (auto& d : controller->GetDifferentials()) {
        d.mLimitedSlipRatio = limitedSlipRatio;
    }

    controller->SetDriverInput(forward, -right, brake, handBrake);

    _vehicleConstraint->SetVehicleCollisionTester(_tester);

    syncPhysicsToECS();

    _physicsSystem->Update(mRequestedDeltaTime, 1, _allocator.get(), _jobSystem.get());
}

void PhysicsService::syncPhysicsToECS() {
    auto view = entityRegistry().view<RigidBody, Transform>(entt::exclude<::Static, Wheel>);

    for (auto entity : view) {
        auto& rigidBody = view.get<RigidBody>(entity);
        auto& transform = view.get<Transform>(entity);
        auto vehicle    = entityRegistry().try_get<Vehicle>(entity);

        auto position = _physicsSystem->GetBodyInterface().GetPosition(rigidBody.body);
        auto rrrr     = _physicsSystem->GetBodyInterface().GetRotation(rigidBody.body);

        auto newPos = glm::vec3(position.GetX(), position.GetY(), position.GetZ());
        auto newRot = glm::quat(rrrr.GetW(), rrrr.GetX(), rrrr.GetY(), rrrr.GetZ());

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 p;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transform.matrix, scale, rotation, p, skew, perspective);

        auto matS = glm::scale(glm::identity<glm::mat4>(), scale);
        auto matT = glm::translate(glm::identity<glm::mat4>(), newPos);
        auto matR = glm::mat4_cast(newRot);
        auto mat  = matT * matR * matS;

        transform.prevMatrix = transform.matrix;
        transform.matrix     = mat;

        if (vehicle) {
            for (auto wheel : vehicle->wheels) {
                auto& w = entityRegistry().get<Wheel>(wheel);
                auto& t = entityRegistry().get<Transform>(wheel);
                // const auto settings = _vehicleConstraint->GetWheels()[w.id]->GetSettings();
                auto wheelTransform = _vehicleConstraint->GetWheelWorldTransform(
                    (JPH::uint) w.position, JPH::Vec3::sAxisX(), JPH::Vec3::sAxisY());
                t.prevMatrix = t.matrix;
                memcpy((void*) &t.matrix, (void*) &wheelTransform, sizeof(glm::mat4));
            }
        }
    }
}

void PhysicsService::updatePhysicsTransforms(entt::storage<Transform>& storage) {
    // auto view = entityRegistry().view<RigidBody, Transform>();

    // for (auto entity : view) {
    //     auto& rigidBody = view.get<RigidBody>(entity);
    //     auto& transform = view.get<Transform>(entity);

    //     if (!rigidBody.isKinematic) {
    //         auto physicsMatrix = transform.matrix; // getPhysicsTransform();

    //         if (transform.matrix != physicsMatrix) {
    //             transform.matrix = physicsMatrix;
    //             storage.push(entity);
    //         }
    //     }
    // }
}

void PhysicsService::updateLocalTransforms(entt::storage<Transform>& storage) {
    // auto view = entityRegistry().view<LocalTransform>() | entt::basic_view{ storage };

    // for (auto entity : view) {
    //     const auto node = entityRegistry().try_get<Node>(entity);

    //     const auto parentInverseWorldMatrix =
    //         node && node->parent != entt::null ?
    //         glm::inverse(entityRegistry().get<Transform>(node->parent).matrix)
    //                                            : glm::identity<glm::mat4>();

    //     const auto& worldMatrix = view.get<Transform>(entity).matrix;
    //     auto localMatrix        = parentInverseWorldMatrix * worldMatrix;

    //     auto& localTransform = view.get<LocalTransform>(entity);
    //     assert(!localTransform.isDirty && "The physics service can only work with calculated world transforms.");

    //     glm::vec3 skew;
    //     glm::vec4 perspective;
    //     glm::decompose(
    //         localMatrix, localTransform.scale, localTransform.rotation, localTransform.position, skew, perspective);
    //     localTransform.isDirty = true;
    // }
}

glm::mat4 PhysicsService::getPhysicsTransform() {
    return {};
}

JPH::WheelSettings* PhysicsService::createWheelSettings(const Vehicle& vehicle,
                                                        const Wheel& wheel,
                                                        const glm::vec3& position,
                                                        gerium_float32_t radius,
                                                        gerium_float32_t width) {
    const auto isFront = wheel.position == Wheel::FrontLeft || wheel.position == Wheel::FrontRight;

    const auto isRight = wheel.position == Wheel::FrontRight || wheel.position == Wheel::BackRight1 ||
                         wheel.position == Wheel::BackRight2;

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
        if (vehicle.handBrake == Vehicle::AllWheelDrive) {
            return vehicle.maxHandBrakeTorque;
        }
        if (vehicle.handBrake == Vehicle::FrontWheelDrive) {
            return isFront ? vehicle.maxHandBrakeTorque : 0.0f;
        }
        return isFront ? 0.0f : vehicle.maxHandBrakeTorque;
    };

    const auto wPosition = JPH::Vec3(position.x, position.y + getSuspensionMinLength(), position.z);

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
        case Collider::Shape::Box: {
            const auto& size = collider.size;
            JPH::BoxShapeSettings shape(JPH::Vec3(size.x, size.y, size.z));
            shape.SetEmbedded();
            return shape.Create().Get();
        }
        case Collider::Shape::Sphere: {
            JPH::SphereShapeSettings shape(collider.radius);
            shape.SetEmbedded();
            return shape.Create().Get();
        }
        case Collider::Shape::Capsule: {
            // JPH::CapsuleShapeSettings shape(size.x, size.y);
            // shape.SetEmbedded();
            // return shape.Create().Get();
            assert(!"unreachable code");
            return {};
        }
        case Collider::Shape::Mesh: {
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
        case ObjectLayers::Constraint:
            return JPH::BroadPhaseLayer(2);
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
            return true; // return inLayer2 == JPH::BroadPhaseLayer(Dynamic) || inLayer2 ==
                         // JPH::BroadPhaseLayer(Constraint);
        case Dynamic:
            return true;
        case Constraint:
            return true; // return inLayer2 != JPH::BroadPhaseLayer(Constraint);
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
            return true; // return inLayer2 == Dynamic || inLayer2 == Constraint;
        case Dynamic:
            return true;
        case Constraint:
            return true; // return inLayer2 != Constraint;
        default:
            JPH_ASSERT(false);
            return false;
    }
}
