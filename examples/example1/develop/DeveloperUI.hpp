#ifndef DEVELOP_DEVELOPER_UI_HPP
#define DEVELOP_DEVELOPER_UI_HPP

#include "../Common.hpp"
#include "../Model.hpp"
#include "../components/Camera.hpp"
#include "../components/Collider.hpp"
#include "../components/Name.hpp"
#include "../components/Node.hpp"
#include "../components/Renderable.hpp"
#include "../components/RigidBody.hpp"
#include "../components/Settings.hpp"
#include "../components/Static.hpp"
#include "../components/Transform.hpp"
#include "../components/Vehicle.hpp"
#include "../components/VehicleController.hpp"
#include "../components/Wheel.hpp"

template <typename>
struct ComponentInfo;

template <>
struct ComponentInfo<Name> {
    static constexpr char name[]    = "name";
    static constexpr bool deletable = true;
    static constexpr bool addable   = true;
};

template <>
struct ComponentInfo<Transform> {
    static constexpr char name[]    = "transform";
    static constexpr bool deletable = false;
    static constexpr bool addable   = false;
};

template <>
struct ComponentInfo<Static> {
    static constexpr char name[]    = "static";
    static constexpr bool deletable = true;
    static constexpr bool addable   = true;
};

template <>
struct ComponentInfo<Collider> {
    static constexpr char name[]    = "collider";
    static constexpr bool deletable = true;
    static constexpr bool addable   = true;
};

template <>
struct ComponentInfo<RigidBody> {
    static constexpr char name[]    = "rigid body";
    static constexpr bool deletable = true;
    static constexpr bool addable   = true;
};

template <>
struct ComponentInfo<Camera> {
    static constexpr char name[]    = "camera";
    static constexpr bool deletable = true;
    static constexpr bool addable   = true;
};

template <>
struct ComponentInfo<Renderable> {
    static constexpr char name[]    = "renderable";
    static constexpr bool deletable = true;
    static constexpr bool addable   = false;
};

template <>
struct ComponentInfo<Vehicle> {
    static constexpr char name[]    = "vehicle";
    static constexpr bool deletable = true;
    static constexpr bool addable   = true;
};

template <>
struct ComponentInfo<VehicleController> {
    static constexpr char name[]    = "vehicle controller";
    static constexpr bool deletable = true;
    static constexpr bool addable   = true;
};

class DeveloperUI final {
public:
    DeveloperUI(entt::registry& registry, entt::dispatcher& dispatcher) noexcept;

    void show(gerium_command_buffer_t commandBuffer);

private:
    enum Action {
        None,
        AddModel,
        DeleteNode,
        AddComponent
    };

    template <typename T>
    void showComponent(entt::entity entity) {
        if (std::is_same_v<T, Name> && _registry.get<Node>(entity).parent == entt::null) {
            return;
        }

        if (std::is_same_v<T, Transform> && _registry.any_of<Wheel>(entity)) {
            return;
        }

        T* component = nullptr;
        if constexpr (std::is_same_v<T, Static> || std::is_same_v<T, VehicleController>) {
            static T instance;
            if (_registry.any_of<T>(entity)) {
                component = &instance;
            }
        } else {
            component = _registry.try_get<T>(entity);
        }
        if (component) {
            auto& name = ComponentInfo<T>::name;
            if (ImGui::BeginChild(name,
                                  ImVec2(0, 0),
                                  ImGuiChildFlags_FrameStyle | ImGuiChildFlags_AutoResizeY,
                                  ImGuiWindowFlags_NoScrollbar)) {
                gerium_float32_t sizeX = 16.0f;

                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(name).x - sizeX) * 0.5f);
                ImGui::Text("%s", name);

                auto deleted = false;
                if constexpr (ComponentInfo<T>::deletable) {
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - sizeX - ImGui::GetStyle().FramePadding.y);
                    ImVec2 p         = ImGui::GetCursorScreenPos();
                    ImU32 background = 0;
                    ImU32 color      = ImGui::GetColorU32(ImGuiCol_Text);

                    ImGui::InvisibleButton("close", ImVec2(sizeX, sizeX));
                    if (ImGui::IsItemHovered()) {
                        background = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
                    }
                    if (ImGui::IsItemActive()) {
                        background = ImGui::GetColorU32(ImGuiCol_ButtonActive);
                    }
                    if (ImGui::IsItemClicked()) {
                        deleteComponent(entity, *component);
                    }

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + sizeX, p.y + sizeX), background);
                    p += { 1.5f, 1.5f };
                    sizeX -= 3.5f;
                    draw_list->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + sizeX, p.y + sizeX), color);
                    draw_list->AddLine(ImVec2(p.x + sizeX - 0.5f, p.y), ImVec2(p.x, p.y + sizeX - 0.5f), color);
                }
                if (!deleted) {
                    showComponent(entity, *component);
                }
            }
            ImGui::EndChild();
            if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) || _selected == entt::null) {
                _registry.ctx().get<Settings>().transforming = false;
            }
        }
    }

    template <typename... Types>
    std::set<const char*> missingComponents(entt::entity entity) {
        std::set<const char*> components;
        std::set<const char*> hasComponents;
        (components.insert(ComponentInfo<Types>::addable ? ComponentInfo<Types>::name : nullptr), ...);
        (components.erase(_registry.any_of<Types>(entity) ? ComponentInfo<Types>::name : nullptr), ...);
        return components;
    }

    template <typename... Types>
    void showComponents(entt::entity entity) {
        (showComponent<Types>(entity), ...);
    }

    template <typename T>
    void addComponent(entt::entity entity, const char* component) {
        if constexpr (ComponentInfo<T>::addable) {
            if (ComponentInfo<T>::name == component) {
                T data{};
                addComponent(entity, data);
            }
        }
    }

    template <typename... Types>
    void addComponents(entt::entity entity, const char* component) {
        (addComponent<Types>(entity, component), ...);
    }

    template <typename A>
    static float calcItemWidth(const A& labels) noexcept {
        auto width = 0.0f;
        for (auto label : labels) {
            width = std::max(width, ImGui::CalcTextSize(label).x);
        }
        const auto availableWidth = ImGui::GetContentRegionAvail().x;
        return availableWidth - (width + ImGui::GetStyle().FramePadding.x * 2);
    }

    std::tuple<BoundingBox, Shape, gerium_float32_t> getBBoxAndShapes(entt::entity entity);

    void showProfiler(gerium_command_buffer_t commandBuffer);
    void showSettings();
    void showSceneGraph();

    void showComponent(entt::entity entity, Name& name);
    void showComponent(entt::entity entity, Transform& transform);
    void showComponent(entt::entity entity, Static& isStatic);
    void showComponent(entt::entity entity, Collider& collider);
    void showComponent(entt::entity entity, RigidBody& rigidBody);
    void showComponent(entt::entity entity, Renderable& renderable);
    void showComponent(entt::entity entity, Vehicle& vehicle);
    void showComponent(entt::entity entity, VehicleController& vehicleController);

    void addComponent(entt::entity entity, Name& name);
    void addComponent(entt::entity entity, Static& isStatic);
    void addComponent(entt::entity entity, Collider& collider);
    void addComponent(entt::entity entity, RigidBody& rigidBody);
    void addComponent(entt::entity entity, Vehicle& vehicle);
    void addComponent(entt::entity entity, VehicleController& vehicleController);

    void deleteComponent(entt::entity entity, Name& name);
    void deleteComponent(entt::entity entity, Static& isStatic);
    void deleteComponent(entt::entity entity, Collider& collider);
    void deleteComponent(entt::entity entity, RigidBody& rigidBody);
    void deleteComponent(entt::entity entity, Renderable& renderable);
    void deleteComponent(entt::entity entity, Vehicle& vehicle);
    void deleteComponent(entt::entity entity, VehicleController& vehicleController);

    entt::registry& _registry;
    entt::dispatcher& _dispatcher;
    Action _action{ None };
    entt::entity _selected{ entt::null };
    int _index{ 0 };
    bool _addModelNameValid{ true };
    char _addModelName[256]{};
    bool _isValidName{ true };
    std::set<const char*> _missingComponents{};
    Cluster _cluster{};
    std::map<entt::hashed_string, Model> _models{};
    std::vector<hashed_string_owner> _poolTextures{};
    std::vector<const char*> _textures{};
};

#endif
