#ifndef DEVELOP_DEVELOP_UI_HPP
#define DEVELOP_DEVELOP_UI_HPP

#include "../Common.hpp"
#include "../components/Camera.hpp"
#include "../components/Collider.hpp"
#include "../components/Name.hpp"
#include "../components/Node.hpp"
#include "../components/Renderable.hpp"
#include "../components/RigidBody.hpp"
#include "../components/Static.hpp"
#include "../components/Transform.hpp"
#include "../components/Vehicle.hpp"
#include "../components/VehicleController.hpp"

template <typename>
struct DevelopName;

template <>
struct DevelopName<Name> {
    static constexpr char name[] = "name";
};

template <>
struct DevelopName<Transform> {
    static constexpr char name[] = "transform";
};

template <>
struct DevelopName<Static> {
    static constexpr char name[] = "static";
};

template <>
struct DevelopName<Collider> {
    static constexpr char name[] = "collider";
};

template <>
struct DevelopName<RigidBody> {
    static constexpr char name[] = "rigid body";
};

template <>
struct DevelopName<Camera> {
    static constexpr char name[] = "camera";
};

template <>
struct DevelopName<Renderable> {
    static constexpr char name[] = "renderable";
};

template <>
struct DevelopName<Vehicle> {
    static constexpr char name[] = "vehicle";
};

template <>
struct DevelopName<VehicleController> {
    static constexpr char name[] = "vehicle controller";
};

class DevelopUI final {
public:
    DevelopUI(entt::registry& registry, entt::dispatcher& dispatcher) noexcept;

    void show(gerium_command_buffer_t commandBuffer);

private:
    enum Action {
        None,
        AddModel,
        DeleteNode
    };

    template <typename T>
    void showComponent(entt::entity entity) {
        if (std::is_same_v<T, Name> && _registry.get<Node>(entity).parent == entt::null) {
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
            auto& name = DevelopName<T>::name;
            if (ImGui::BeginChild(name,
                                  ImVec2(0, 0),
                                  ImGuiChildFlags_FrameStyle | ImGuiChildFlags_AutoResizeY,
                                  ImGuiWindowFlags_NoScrollbar)) {
                gerium_float32_t sizeX = 16.0f;

                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(name).x - sizeX) * 0.5f);
                ImGui::Text("%s", name);

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
                }

                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + sizeX, p.y + sizeX), background);
                p += { 1.5f, 1.5f };
                sizeX -= 3.5f;
                draw_list->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + sizeX, p.y + sizeX), color);
                draw_list->AddLine(ImVec2(p.x + sizeX - 0.5f, p.y), ImVec2(p.x, p.y + sizeX - 0.5f), color);

                showComponent(entity, *component);
            }
            ImGui::EndChild();
        }
    }

    template <typename... Types>
    void showComponents(entt::entity entity) {
        (showComponent<Types>(entity), ...);
    }

    void showProfiler(gerium_command_buffer_t commandBuffer);
    void showSettings();
    void showSceneGraph();
    void showComponent(entt::entity entity, Name& name);
    void showComponent(entt::entity entity, Transform& transform);
    void showComponent(entt::entity entity, Static& isStatic);
    void showComponent(entt::entity entity, Collider& collider);
    void showComponent(entt::entity entity, RigidBody& rigidBody);
    void showComponent(entt::entity entity, Camera& camera);
    void showComponent(entt::entity entity, Renderable& renderable);
    void showComponent(entt::entity entity, Vehicle& vehicle);
    void showComponent(entt::entity entity, VehicleController& vehicleController);

    entt::registry& _registry;
    entt::dispatcher& _dispatcher;
    Action _action{ None };
    entt::entity _selected{ entt::null };
    int _addModelIndex{ 0 };
    bool _addModelNameValid{ true };
    char _addModelName[256]{};
    bool _isValidName{ true };
};

#endif
