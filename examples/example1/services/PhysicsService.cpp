#include "PhysicsService.hpp"
#include "../components/Collider.hpp"
#include "../components/Constraint.hpp"
#include "../components/LocalTransform.hpp"
#include "../components/Node.hpp"
#include "../components/RigidBody.hpp"
#include "../components/Static.hpp"
#include "../components/Vehicle.hpp"
#include "../components/Wheel.hpp"
#include "../components/WorldTransform.hpp"
#include "InputService.hpp"

using namespace entt::literals;

constexpr float mUpdateFrequency    = 60.0f;
constexpr float mRequestedDeltaTime = 1.0f / mUpdateFrequency;

// std::vector<JPH::HingeConstraint*> h;

void PhysicsService::createBodies(const Cluster& cluster) {
    /*std::set<entt::entity> parentConstraints;
    for (auto [_, constraint] : entityRegistry().view<::Constraint>().each()) {
        parentConstraints.insert(constraint.parent);
    }

    auto view = entityRegistry().view<RigidBody, Collider, WorldTransform>();

    for (auto entity : view) {
        auto& rigidBody      = view.get<RigidBody>(entity);
        auto& collider       = view.get<Collider>(entity);
        auto& worldTransform = view.get<WorldTransform>(entity);
        auto isDynamic       = !entityRegistry().any_of<::Static>(entity);
        auto isConstraint    = entityRegistry().any_of<::Constraint>(entity);

        if (!isConstraint && parentConstraints.contains(entity)) {
            isConstraint = true;
        }

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 position;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(worldTransform.matrix, scale, rotation, position, skew, perspective);

        auto pos  = JPH::Vec3(position.x, position.y, position.z);
        auto quat = JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w);
        auto size = collider.size;

        JPH::Ref<JPH::Shape> shape{};
        switch (collider.shape) {
            case Collider::Shape::Box: {
                JPH::BoxShapeSettings box(JPH::Vec3(size.x, size.y, size.z));
                box.SetEmbedded();
                auto shapeResult = box.Create();
                shape            = shapeResult.Get();
                break;
            }
            case Collider::Shape::Sphere: {
                JPH::SphereShapeSettings sphere(size.x);
                sphere.SetEmbedded();
                auto shapeResult = sphere.Create();
                shape            = shapeResult.Get();
                break;
            }
            case Collider::Shape::Capsule: {
                JPH::CapsuleShapeSettings capsule(size.x, size.y);
                capsule.SetEmbedded();
                auto shapeResult = capsule.Create();
                shape            = shapeResult.Get();
                break;
            }
        }
        shape->ScaleShape(JPH::Vec3(scale.x, scale.y, scale.z));

        JPH::BodyCreationSettings settings(shape,
                                           pos,
                                           quat,
                                           isDynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                                           isDynamic ? (isConstraint ? Constraint : Dynamic) : Static);

        auto inertia = rigidBody.mass * 0.2f;

        settings.mOverrideMassProperties          = JPH::EOverrideMassProperties::MassAndInertiaProvided;
        settings.mMassPropertiesOverride.mMass    = rigidBody.mass;
        settings.mMassPropertiesOverride.mInertia = JPH::Mat44::sScale(JPH::Vec3(inertia, inertia, inertia));

        rigidBody.body = _physicsSystem->GetBodyInterface().CreateBody(settings)->GetID();
        _physicsSystem->GetBodyInterface().AddBody(
            rigidBody.body, isDynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);

        // _physicsSystem->GetBodyInterface().SetLinearVelocity(rigidBody.body, JPH::Vec3(0.0f, -0.0f, 0.0f));
    }

    auto view2 = entityRegistry().view<RigidBody, ::Constraint>();

    for (auto entity : view2) {
        const auto& rigidBody  = view2.get<RigidBody>(entity);
        const auto& constraint = view2.get<::Constraint>(entity);

        const JPH::Vec3 point(constraint.point.x, constraint.point.y, constraint.point.z);
        const JPH::Vec3 axis(constraint.axis.x, constraint.axis.y, constraint.axis.z);

        auto parentID = entityRegistry().get<RigidBody>(constraint.parent).body;

        JPH::HingeConstraintSettings settings;
        settings.mPoint1 = settings.mPoint2 = point;
        settings.mHingeAxis1 = settings.mHingeAxis2 = axis;

        settings.mMotorSettings                            = JPH::MotorSettings();
        settings.mMotorSettings.mSpringSettings.mFrequency = 2.0f;
        settings.mMotorSettings.mSpringSettings.mDamping   = 1.0f;
        settings.mMotorSettings.mMinTorqueLimit            = -1000.0f;
        settings.mMotorSettings.mMaxTorqueLimit            = 1000.0f;

        // settings.mLimitsMin = -0.5f * 0.5f * JPH::JPH_PI;
        // settings.mLimitsMax = 0.5f * 0.5f * JPH::JPH_PI;

        auto parent = _physicsSystem->GetBodyLockInterface().TryGetBody(parentID);
        auto child  = _physicsSystem->GetBodyLockInterface().TryGetBody(rigidBody.body);

        auto hinge = (JPH::HingeConstraint*) settings.Create(*parent, *child);
        hinge->SetMotorState(JPH::EMotorState::Velocity);
        hinge->SetTargetAngularVelocity(-2.0f);
        h.push_back(hinge);
        _physicsSystem->AddConstraint(hinge);
    }*/

    ////////////////

    _tester = new JPH::VehicleCollisionTesterCastCylinder(Dynamic);

    auto view = entityRegistry().view<RigidBody, Collider, WorldTransform>(entt::exclude<Wheel>);

    for (auto entity : view) {
        auto& rigidBody            = view.get<RigidBody>(entity);
        const auto& collider       = view.get<Collider>(entity);
        const auto& worldTransform = view.get<WorldTransform>(entity);
        const auto vehicle         = entityRegistry().try_get<Vehicle>(entity);
        const auto isDynamic       = !entityRegistry().any_of<::Static>(entity);

        glm::vec3 scale;
        glm::quat orientation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(worldTransform.matrix, scale, orientation, translation, skew, perspective);

        const auto position = JPH::Vec3(translation.x, translation.y, translation.z);
        const auto rotation = JPH::Quat(orientation.x, orientation.y, orientation.z, orientation.w);

        auto shape = getShape(collider, cluster);
        shape->ScaleShape(JPH::Vec3(worldTransform.scale.x, worldTransform.scale.y, worldTransform.scale.z));

        JPH::BodyCreationSettings settings(shape,
                                           position,
                                           rotation,
                                           isDynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                                           isDynamic ? Dynamic : Static);

        settings.mOverrideMassProperties       = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMassPropertiesOverride.mMass = rigidBody.mass;

        settings.mLinearDamping  = 0.2f;
        settings.mAngularDamping = 0.2f;

        auto b = _physicsSystem->GetBodyInterface().CreateBody(settings);
        // b->SetFriction(1.0f); TODO
        rigidBody.body = b->GetID();
        _physicsSystem->GetBodyInterface().AddBody(
            rigidBody.body, isDynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);

        if (vehicle) {
            JPH::VehicleConstraintSettings constraint;
            constraint.mMaxPitchRollAngle = vehicle->maxRollAngle;

            const auto sFrontSuspensionSidewaysAngle = 0.0f;
            const auto sFrontSuspensionForwardAngle  = 0.0f;
            const auto sFrontKingPinAngle            = 0.0f;
            const auto sFrontCasterAngle             = 0.0f;
            const auto sFrontCamber                  = 0.0f;
            const auto sFrontToe                     = 0.0f;
            const auto sRearSuspensionSidewaysAngle  = 0.0f;
            const auto sRearSuspensionForwardAngle   = 0.0f;
            const auto sRearKingPinAngle             = 0.0f;
            const auto sRearCasterAngle              = 0.0f;
            const auto sRearCamber                   = 0.0f;
            const auto sRearToe                      = 0.0f;
            const auto sFrontSuspensionMinLength     = 0.3f;
            const auto sFrontSuspensionMaxLength     = 0.5f;
            const auto sFrontSuspensionFrequency     = 1.5f;
            const auto sFrontSuspensionDamping       = 0.5f;
            const auto sRearSuspensionMinLength      = 0.3f;
            const auto sRearSuspensionMaxLength      = 0.5f;
            const auto sRearSuspensionFrequency      = 1.5f;
            const auto sRearSuspensionDamping        = 0.5f;
            const auto sMaxSteeringAngle             = JPH::DegreesToRadians(30.0f);
            const auto sFourWheelDrive               = false;
            const auto sAntiRollbar                  = true;

            auto frontSuspensionDir =
                JPH::Vec3(JPH::Tan(sFrontSuspensionSidewaysAngle), -1, JPH::Tan(sFrontSuspensionForwardAngle))
                    .Normalized();
            auto frontSteeringAxis =
                JPH::Vec3(-JPH::Tan(sFrontKingPinAngle), 1, -JPH::Tan(sFrontCasterAngle)).Normalized();
            auto frontWheelUp      = JPH::Vec3(JPH::Sin(sFrontCamber), JPH::Cos(sFrontCamber), 0);
            auto frontWheelForward = JPH::Vec3(-JPH::Sin(sFrontToe), 0, JPH::Cos(sFrontToe));
            auto rearSuspensionDir =
                JPH::Vec3(JPH::Tan(sRearSuspensionSidewaysAngle), -1, JPH::Tan(sRearSuspensionForwardAngle))
                    .Normalized();
            auto rearSteeringAxis =
                JPH::Vec3(-JPH::Tan(sRearKingPinAngle), 1, -JPH::Tan(sRearCasterAngle)).Normalized();
            auto rearWheelUp      = JPH::Vec3(JPH::Sin(sRearCamber), JPH::Cos(sRearCamber), 0);
            auto rearWheelForward = JPH::Vec3(-JPH::Sin(sRearToe), 0, JPH::Cos(sRearToe));
            JPH::Vec3 flipX(-1, 1, 1);

            if (sAntiRollbar) {
                constraint.mAntiRollBars.resize(2);
                // constraint.mAntiRollBars[0].mLeftWheel  = 0;
                // constraint.mAntiRollBars[0].mRightWheel = 1;
                // constraint.mAntiRollBars[1].mLeftWheel  = 2;
                // constraint.mAntiRollBars[1].mRightWheel = 3;
            }

            for (const auto& wheel : vehicle->wheels) {
                auto localMatrix =
                    glm::inverse(worldTransform.matrix) * entityRegistry().get<WorldTransform>(wheel).matrix;

                auto mm = localMatrix[3].xyz();

                auto lp = entityRegistry().get<LocalTransform>(wheel).position;

                // glm::vec3 scale;
                // glm::quat quat;
                // glm::vec3 tr;
                // glm::vec3 skew;
                // glm::vec4 perspective;
                // glm::decompose(localMatrix, scale, quat, tr, skew, perspective);

                auto m3 = glm::mat3(localMatrix);

                auto up      = glm::normalize(m3 * glm::vec3(0.0f, 1.0f, 0.0f));
                auto forward = glm::normalize(m3 * glm::vec3(0.0f, 0.0f, 1.0f));
                auto right   = glm::normalize(m3 * glm::vec3(1.0f, 0.0f, 0.0f));

                JPH::Vec3 suspensionDir, steeringAxis, wheelUp, wheelForward;

                float suspensionMinLength, suspensionMaxLength, suspensionFrequency, suspensionDamping,
                    maxSteeringAngle;
                float maxHandBrakeTorque = 4000.0f;
                switch (entityRegistry().get<Wheel>(wheel).position) {
                    case Wheel::FrontLeft:
                        suspensionDir       = frontSuspensionDir;
                        steeringAxis        = frontSteeringAxis;
                        wheelUp             = frontWheelUp;
                        wheelForward        = frontWheelForward;
                        suspensionMinLength = sFrontSuspensionMinLength;
                        suspensionMaxLength = sFrontSuspensionMaxLength;
                        suspensionFrequency = sFrontSuspensionFrequency;
                        suspensionDamping   = sFrontSuspensionDamping;
                        maxSteeringAngle    = sMaxSteeringAngle;
                        maxHandBrakeTorque  = 0.0f;

                        entityRegistry().get<Wheel>(wheel).id  = 0;
                        constraint.mAntiRollBars[0].mLeftWheel = 0;
                        break;
                    case Wheel::FrontRight:
                        suspensionDir       = flipX * frontSuspensionDir;
                        steeringAxis        = frontSteeringAxis;
                        wheelUp             = frontWheelUp;
                        wheelForward        = flipX * frontWheelForward;
                        suspensionMinLength = sFrontSuspensionMinLength;
                        suspensionMaxLength = sFrontSuspensionMaxLength;
                        suspensionFrequency = sFrontSuspensionFrequency;
                        suspensionDamping   = sFrontSuspensionDamping;
                        maxSteeringAngle    = sMaxSteeringAngle;
                        maxHandBrakeTorque  = 0.0f;

                        entityRegistry().get<Wheel>(wheel).id   = 1;
                        constraint.mAntiRollBars[0].mRightWheel = 1;
                        break;
                    case Wheel::BackLeft1:
                        suspensionDir       = rearSuspensionDir;
                        steeringAxis        = rearSteeringAxis;
                        wheelUp             = rearWheelUp;
                        wheelForward        = rearWheelForward;
                        suspensionMinLength = sRearSuspensionMinLength;
                        suspensionMaxLength = sRearSuspensionMaxLength;
                        suspensionFrequency = sRearSuspensionFrequency;
                        suspensionDamping   = sRearSuspensionDamping;
                        maxSteeringAngle    = 0.0f;

                        entityRegistry().get<Wheel>(wheel).id  = 2;
                        constraint.mAntiRollBars[1].mLeftWheel = 2;
                        break;
                    case Wheel::BackRight1:
                        suspensionDir       = flipX * rearSuspensionDir;
                        steeringAxis        = rearSteeringAxis;
                        wheelUp             = rearWheelUp;
                        wheelForward        = flipX * rearWheelForward;
                        suspensionMinLength = sRearSuspensionMinLength;
                        suspensionMaxLength = sRearSuspensionMaxLength;
                        suspensionFrequency = sRearSuspensionFrequency;
                        suspensionDamping   = sRearSuspensionDamping;
                        maxSteeringAngle    = 0.0f;

                        entityRegistry().get<Wheel>(wheel).id   = 3;
                        constraint.mAntiRollBars[1].mRightWheel = 3;
                        break;
                    case Wheel::BackLeft2:
                        suspensionDir       = rearSuspensionDir;
                        steeringAxis        = rearSteeringAxis;
                        wheelUp             = rearWheelUp;
                        wheelForward        = rearWheelForward;
                        suspensionMinLength = sRearSuspensionMinLength;
                        suspensionMaxLength = sRearSuspensionMaxLength;
                        suspensionFrequency = sRearSuspensionFrequency;
                        suspensionDamping   = sRearSuspensionDamping;
                        maxSteeringAngle    = 0.0f;
                        break;
                    case Wheel::BackRight2:
                        suspensionDir       = flipX * rearSuspensionDir;
                        steeringAxis        = rearSteeringAxis;
                        wheelUp             = rearWheelUp;
                        wheelForward        = flipX * rearWheelForward;
                        suspensionMinLength = sRearSuspensionMinLength;
                        suspensionMaxLength = sRearSuspensionMaxLength;
                        suspensionFrequency = sRearSuspensionFrequency;
                        suspensionDamping   = sRearSuspensionDamping;
                        maxSteeringAngle    = 0.0f;
                        break;
                }

                const auto& wheelCollider = entityRegistry().get<Collider>(wheel);

                auto w            = new JPH::WheelSettingsWV;
                const auto& point = entityRegistry().get<Wheel>(wheel).point;
                w->mPosition = JPH::Vec3(mm.x, mm.y + 0.3f, mm.z); // JPH::Vec3(point.x - 2, point.y + 8, point.z + 8);
                w->mSuspensionDirection = suspensionDir;

                w->mAngularDamping = 0.8f;

                w->mSteeringAxis                = steeringAxis;
                w->mWheelUp                     = wheelUp;
                w->mWheelForward                = wheelForward;
                w->mSuspensionMinLength         = suspensionMinLength;
                w->mSuspensionMaxLength         = suspensionMaxLength;
                w->mSuspensionSpring.mFrequency = suspensionFrequency;
                w->mSuspensionSpring.mDamping   = suspensionDamping;
                w->mMaxSteerAngle               = maxSteeringAngle;
                w->mMaxHandBrakeTorque          = maxHandBrakeTorque;
                w->mRadius                      = wheelCollider.data.radius;
                w->mWidth                       = wheelCollider.data.radius / 2;

                // w->mLongitudinalFriction;
                // w->mLateralFriction;

                constraint.mWheels.push_back(w);
            }

            auto controller        = new JPH::WheeledVehicleControllerSettings;
            constraint.mController = controller;
            controller->mDifferentials.resize(sFourWheelDrive ? 2 : 1);
            controller->mDifferentials[0].mLeftWheel  = 0;
            controller->mDifferentials[0].mRightWheel = 1;
            if (sFourWheelDrive) {
                controller->mDifferentials[1].mLeftWheel         = 2;
                controller->mDifferentials[1].mRightWheel        = 3;
                controller->mDifferentials[0].mEngineTorqueRatio = controller->mDifferentials[1].mEngineTorqueRatio =
                    0.5f;
            }

            _car               = _physicsSystem->GetBodyLockInterface().TryGetBody(rigidBody.body);
            _vehicleConstraint = new JPH::VehicleConstraint(*_car, constraint);

            // for (auto& w : _vehicleConstraint->GetWheels()) {
            // }

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
            // mPhysicsSystem->RemoveStepListener(mVehicleConstraint);
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

    // auto&& physicsTransforms = entityRegistry().storage<WorldTransform>("physics_transforms"_hs);

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

    // OK
    // if (forward == 0.0f) {
    //     JPH::Vec3 currentVelocity = _car->GetLinearVelocity();
    //     _car->SetLinearVelocity(currentVelocity * 0.95f);
    // }

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
    auto view = entityRegistry().view<RigidBody, WorldTransform>(entt::exclude<::Static, Wheel>);

    for (auto entity : view) {
        auto& rigidBody      = view.get<RigidBody>(entity);
        auto& worldTransform = view.get<WorldTransform>(entity);
        auto vehicle         = entityRegistry().try_get<Vehicle>(entity);

        auto position = _physicsSystem->GetBodyInterface().GetPosition(rigidBody.body);
        auto rrrr     = _physicsSystem->GetBodyInterface().GetRotation(rigidBody.body);

        auto newPos = glm::vec3(position.GetX(), position.GetY(), position.GetZ());
        auto newRot = glm::quat(rrrr.GetW(), rrrr.GetX(), rrrr.GetY(), rrrr.GetZ());

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 p;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(worldTransform.matrix, scale, rotation, p, skew, perspective);

        auto matS = glm::scale(glm::identity<glm::mat4>(), scale);
        auto matT = glm::translate(glm::identity<glm::mat4>(), newPos);
        auto matR = glm::mat4_cast(newRot);
        auto mat  = matT * matR * matS;

        worldTransform.prevMatrix = worldTransform.matrix;
        worldTransform.matrix     = mat;

        if (vehicle) {
            for (auto wheel : vehicle->wheels) {
                auto& w = entityRegistry().get<Wheel>(wheel);
                auto& t = entityRegistry().get<WorldTransform>(wheel);
                // const auto settings = _vehicleConstraint->GetWheels()[w.id]->GetSettings();
                auto wheelTransform =
                    _vehicleConstraint->GetWheelWorldTransform(w.id, JPH::Vec3::sAxisX(), JPH::Vec3::sAxisY());
                t.prevMatrix = t.matrix;
                memcpy((void*) &t.matrix, (void*) &wheelTransform, sizeof(glm::mat4));
            }
        }
    }
}

void PhysicsService::updatePhysicsTransforms(entt::storage<WorldTransform>& storage) {
    auto view = entityRegistry().view<RigidBody, WorldTransform>();

    for (auto entity : view) {
        auto& rigidBody      = view.get<RigidBody>(entity);
        auto& worldTransform = view.get<WorldTransform>(entity);

        if (!rigidBody.isKinematic) {
            auto physicsMatrix = worldTransform.matrix; // getPhysicsTransform();

            if (worldTransform.matrix != physicsMatrix) {
                worldTransform.matrix = physicsMatrix;
                storage.push(entity);
            }
        }
    }
}

void PhysicsService::updateLocalTransforms(entt::storage<WorldTransform>& storage) {
    auto view = entityRegistry().view<LocalTransform>() | entt::basic_view{ storage };

    for (auto entity : view) {
        const auto node = entityRegistry().try_get<Node>(entity);

        const auto parentInverseWorldMatrix =
            node && node->parent != entt::null ? glm::inverse(entityRegistry().get<WorldTransform>(node->parent).matrix)
                                               : glm::identity<glm::mat4>();

        const auto& worldMatrix = view.get<WorldTransform>(entity).matrix;
        auto localMatrix        = parentInverseWorldMatrix * worldMatrix;

        auto& localTransform = view.get<LocalTransform>(entity);
        assert(!localTransform.isDirty && "The physics service can only work with calculated world transforms.");

        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(
            localMatrix, localTransform.scale, localTransform.rotation, localTransform.position, skew, perspective);
        localTransform.isDirty = true;
    }
}

glm::mat4 PhysicsService::getPhysicsTransform() {
    return {};
}

JPH::Ref<JPH::Shape> PhysicsService::getShape(const Collider& collider, const Cluster& cluster) {
    JPH::Ref<JPH::Shape> shape{};
    switch (collider.shape) {
        case Collider::Shape::Box: {
            const auto& size = collider.data.size;
            JPH::BoxShapeSettings shape(JPH::Vec3(size.x, size.y, size.z));
            shape.SetEmbedded();
            return shape.Create().Get();
        }
        case Collider::Shape::Sphere: {
            JPH::SphereShapeSettings shape(collider.data.radius);
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
            const auto& meshCollider = cluster.meshColliders[collider.data.colliderIndex];
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
