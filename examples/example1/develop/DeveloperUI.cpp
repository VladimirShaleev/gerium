#include "DeveloperUI.hpp"
#include "../components/Settings.hpp"
#include "../events/AddModelEvent.hpp"
#include "../events/AddNodeNameEvent.hpp"
#include "../events/ChangeNodeNameEvent.hpp"
#include "../events/DeleteNodeEvent.hpp"
#include "../events/TransformNodeEvent.hpp"

using namespace entt::literals;

#define ComponentTypes Name, Transform, Static, Collider, RigidBody, Camera, Renderable, Vehicle, VehicleController

DeveloperUI::DeveloperUI(entt::registry& registry, entt::dispatcher& dispatcher) noexcept :
    _registry(registry),
    _dispatcher(dispatcher) {
}

void DeveloperUI::show(gerium_command_buffer_t commandBuffer) {
    showProfiler(commandBuffer);
    showSettings();
    showSceneGraph();
}

void DeveloperUI::showProfiler(gerium_command_buffer_t commandBuffer) {
    auto& settings = _registry.ctx().get<Settings>();
    if (settings.showProfiler) {
        gerium_bool_t show = settings.showProfiler ? 1 : 0;
        gerium_command_buffer_draw_profiler(commandBuffer, &show);
        settings.showProfiler = !!show;
    }
}

void DeveloperUI::showSettings() {
    if (ImGui::Begin("Settings")) {
        auto& settings = _registry.ctx().get<Settings>();
        ImGui::Checkbox("Show Profiler", &settings.showProfiler);
        ImGui::Checkbox("Transform affects child nodes", &settings.transformChilds);
        if (ImGui::Button("Save state")) {
            settings.state = Settings::Save;
        }
        if (ImGui::Button("Load state")) {
            settings.state = Settings::Load;
        }
    }
    ImGui::End();
}

void DeveloperUI::showSceneGraph() {
    if (ImGui::Begin("Scene Graph")) {
        if (ImGui::BeginTable("mygrid", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Hierarhy");
            ImGui::TableSetupColumn("Properties");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            entt::entity root = entt::null;
            auto view         = _registry.view<Node, Name>();
            for (auto entity : view) {
                auto& node = view.get<Node>(entity);
                auto& name = view.get<Name>(entity);
                if (name.name == ""_hs) {
                    root = entity;
                    break;
                }
            }

            if (root == entt::null) {
                return;
            }

            // if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
            //     selected = entt::null;
            // }

            auto uniqueColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
            auto color       = uniqueColor;
            color.w          = 0.5f;
            std::function<void(entt::entity)> drawNode;
            drawNode = [this, root, &drawNode, uniqueColor, color](entt::entity entity) {
                const auto& node    = _registry.get<Node>(entity);
                const auto name     = _registry.try_get<Name>(entity);
                const auto isUnique = name != nullptr;
                const auto isRoot   = entity == root;
                auto nodeName       = name ? (isRoot ? "root" : name->name.data()) : node.name.data();
                if (!nodeName) {
                    nodeName = "node";
                }

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
                if (node.childs.empty()) {
                    flags |= ImGuiTreeNodeFlags_Leaf;
                }
                auto pushedStyles = 1;
                ImGui::PushID((int) entity);
                ImGui::PushStyleColor(ImGuiCol_Text, isUnique ? uniqueColor : color);
                if (_selected == entity) {
                    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                    pushedStyles += 2;
                }
                auto isOpen = ImGui::TreeNodeEx(nodeName, flags);
                if (ImGui::IsItemClicked()) {
                    _selected = entity;
                }
                if (ImGui::BeginPopupContextItem()) {
                    _selected = entity;
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    if (!isRoot && ImGui::MenuItem("Up")) {
                        auto parent        = _registry.get<Node>(entity).parent;
                        auto& parentChilds = _registry.get<Node>(parent).childs;
                        auto it            = std::find(parentChilds.begin(), parentChilds.end(), entity);
                        if (it != parentChilds.begin()) {
                            std::swap(*it, *(it - 1));
                        }
                    }
                    if (!isRoot && ImGui::MenuItem("Down")) {
                        auto parent        = _registry.get<Node>(entity).parent;
                        auto& parentChilds = _registry.get<Node>(parent).childs;
                        auto it            = std::find(parentChilds.rbegin(), parentChilds.rend(), entity);
                        if (it != parentChilds.rbegin()) {
                            std::swap(*it, *(it - 1));
                        }
                    }
                    if (ImGui::MenuItem("Add model")) {
                        _action            = AddModel;
                        _selected          = entity;
                        _index             = 0;
                        _addModelNameValid = false;
                        memset(_addModelName, 0, sizeof(_addModelName));
                    }
                    if (!isRoot && ImGui::MenuItem("Delete")) {
                        _action   = DeleteNode;
                        _selected = entity;
                    }
                    ImGui::PopStyleColor();
                    ImGui::EndPopup();
                }
                if (!isRoot && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("TREE_NODE", &entity, sizeof(entity));
                    ImGui::Text("Moving %s", nodeName);
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TREE_NODE")) {
                        IM_ASSERT(payload->DataSize == sizeof(entt::entity));
                        auto source = *(entt::entity*) payload->Data;
                        auto dest   = entity;

                        std::queue<entt::entity> visit;
                        for (auto child : _registry.get<Node>(source).childs) {
                            visit.push(child);
                        }
                        auto correct = true;
                        while (!visit.empty()) {
                            auto item = visit.front();
                            visit.pop();
                            if (item == dest) {
                                correct = false;
                                break;
                            }
                            for (auto child : _registry.get<Node>(item).childs) {
                                visit.push(child);
                            }
                        }
                        if (correct) {
                            auto prevParent        = _registry.get<Node>(source).parent;
                            auto& prevParentChilds = _registry.get<Node>(prevParent).childs;
                            prevParentChilds.erase(std::find(prevParentChilds.begin(), prevParentChilds.end(), source));

                            _registry.get<Node>(source).parent = dest;
                            _registry.get<Node>(dest).childs.push_back(source);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                if (isOpen) {
                    for (const auto& child : node.childs) {
                        drawNode(child);
                    }
                    ImGui::TreePop();
                }
                ImGui::PopStyleColor(pushedStyles);
                ImGui::PopID();
            };
            drawNode(root);

            ImGui::TableSetColumnIndex(1);

            if (_selected != entt::null) {
                const auto missing = missingComponents<ComponentTypes>(_selected);
                if (!missing.empty() && ImGui::Button("Add component", ImVec2(-1, 0))) {
                    _index             = 0;
                    _action            = AddComponent;
                    _missingComponents = std::move(missing);
                }

                showComponents<ComponentTypes>(_selected);
            }

            ImGui::EndTable();
        }

        if (_action == AddModel) {
            ImGui::OpenPopup("Add Model to Node");
            if (ImGui::BeginPopupModal("Add Model to Node", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                auto validText = 0;
                if (!_addModelNameValid) {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
                    ++validText;
                }
                if (ImGui::InputText("Unique name", _addModelName, sizeof(_addModelName))) {
                    std::string str = _addModelName;
                    strcpy(_addModelName, trim(str).c_str());
                    _addModelNameValid = true;
                    if (strlen(_addModelName) == 0) {
                        _addModelNameValid = false;
                    } else {
                        entt::hashed_string hashedName = entt::hashed_string(_addModelName, strlen(_addModelName));
                        if (hashedName == "root"_hs) {
                            _addModelNameValid = false;
                        } else {
                            for (const auto [_, name] : _registry.view<Name>().each()) {
                                if (name.name == hashedName) {
                                    _addModelNameValid = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                ImGui::PopStyleColor(validText);
                ImGui::ListBox("Models", &_index, [](auto _, auto index) {
                    return modelIds[index].data();
                }, nullptr, std::size(modelIds));
                if (!_addModelNameValid) {
                    ImGui::BeginDisabled();
                }
                if (ImGui::Button("Add", ImVec2(120, 0))) {
                    auto name = hashed_string_owner(_addModelName, strlen(_addModelName));
                    _dispatcher.enqueue<AddModelEvent>(_selected, name, modelIds[_index]);
                    _action   = None;
                    _selected = entt::null;
                    ImGui::CloseCurrentPopup();
                }
                if (!_addModelNameValid) {
                    ImGui::EndDisabled();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    _action   = None;
                    _selected = entt::null;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        if (_action == DeleteNode) {
            ImGui::OpenPopup("Delete Node");
            if (ImGui::BeginPopupModal("Delete Node", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Do you want to delete\nthe selected node?");
                if (ImGui::Button("Yes", ImVec2(120, 0))) {
                    _dispatcher.enqueue<DeleteNodeEvent>(_selected);
                    _action   = None;
                    _selected = entt::null;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    _action   = None;
                    _selected = entt::null;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        if (_action == AddComponent) {
            ImGui::OpenPopup("Add component");
            if (ImGui::BeginPopupModal("Add component", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                std::vector<const char*> components(_missingComponents.begin(), _missingComponents.end());
                ImGui::ListBox("Components", &_index, components.data(), (int) components.size());
                if (ImGui::Button("Add", ImVec2(120, 0))) {
                    addComponents<ComponentTypes>(_selected, components[_index]);
                    _action = None;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    _action = None;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }
}

void DeveloperUI::showComponent(entt::entity entity, Name& name) {
    auto isValidName = [this, entity](const char* buffer) {
        if (strlen(buffer) == 0) {
            return false;
        } else {
            entt::hashed_string hashedName = entt::hashed_string(buffer, strlen(buffer));
            if (hashedName == "root"_hs) {
                return false;
            } else {
                for (const auto [current, name] : _registry.view<Name>().each()) {
                    if (entity != current && name.name == hashedName) {
                        return false;
                    }
                }
            }
        }
        return true;
    };

    const auto& src = name.name.string();
    char buffer[256]{};
    strncpy(buffer, src.c_str(), std::size(buffer));

    if (_isValidName) {
        _isValidName = isValidName(buffer);
    }

    auto valid       = _isValidName;
    auto label       = "Unique name";
    ImVec2 labelSize = ImGui::CalcTextSize(label);

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - (labelSize.x + ImGui::GetStyle().FramePadding.x * 2));
    if (!valid) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
    }
    ImGui::InputText(label, buffer, std::size(buffer));
    std::string str = buffer;
    strcpy(buffer, trim(str).c_str());
    _isValidName = isValidName(buffer);
    if (_isValidName) {
        auto newName = hashed_string_owner(buffer, strlen(buffer));
        if (name.name != newName) {
            _dispatcher.trigger<ChangeNodeNameEvent>({ name.name, newName });
        }
    }

    if (!valid) {
        ImGui::PopStyleColor();
    }
}

void DeveloperUI::showComponent(entt::entity entity, Transform& transform) {
    float availableWidth = ImGui::GetContentRegionAvail().x;

    auto label1     = "Position";
    auto label2     = "Scale";
    auto label3     = "Rotation";
    auto labelSize1 = ImGui::CalcTextSize(label1).x;
    auto labelSize2 = ImGui::CalcTextSize(label2).x;
    auto labelSize3 = ImGui::CalcTextSize(label3).x;
    auto width      = availableWidth -
                 (std::max(std::max(labelSize1, labelSize2), labelSize3) + ImGui::GetStyle().FramePadding.x * 2);

    glm::vec3 tanslation;
    glm::vec3 scale;
    glm::quat orientation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(transform.matrix, scale, orientation, tanslation, skew, perspective);
    glm::vec3 euler = glm::degrees(glm::eulerAngles(orientation));
    auto hasChanges = false;
    
    ImGui::SetNextItemWidth(width);
    if (ImGui::DragFloat3(label1, &tanslation.x, 0.001f, -10000.0f, 10000.0f, "%.6f")) {
        hasChanges = true;
    }
    
    ImGui::SetNextItemWidth(width);
    if (ImGui::DragFloat3(label3, &euler.x, 0.01f, -360.0f, 360.0f, "%.6f")) {
        hasChanges = true;
    }
    
    ImGui::SetNextItemWidth(width);
    if (ImGui::DragFloat3(label2, &scale.x, 0.001f, -10000.0f, 10000.0f, "%.6f")) {
        hasChanges = true;
    }
    
    if (hasChanges) {
        auto& settings = _registry.ctx().get<Settings>();
        _dispatcher.enqueue<TransformNodeEvent>(entity, tanslation, glm::quat(glm::radians(euler)), scale, settings.transformChilds);
    }
}

void DeveloperUI::showComponent(entt::entity entity, Static& isStatic) {
}

void DeveloperUI::showComponent(entt::entity entity, Collider& collider) {
    float availableWidth = ImGui::GetContentRegionAvail().x;

    auto label1      = "Shape";
    auto label2      = "Half extent";
    auto label3      = "Radius";
    auto label4      = "Half height";
    auto labelSize1  = ImGui::CalcTextSize(label1).x;
    auto labelSize2  = ImGui::CalcTextSize(label2).x;
    auto labelSize3  = ImGui::CalcTextSize(label3).x;
    auto labelSize4  = ImGui::CalcTextSize(label4).x;
    auto width       = availableWidth - (labelSize1 + ImGui::GetStyle().FramePadding.x * 2);
    auto widthRadius = availableWidth - (labelSize3 + ImGui::GetStyle().FramePadding.x * 2);

    constexpr auto values = magic_enum::enum_names<Shape>();

    auto selected = (int) magic_enum::enum_index(collider.shape).value();

    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo(label1, values[selected].data(), ImGuiComboFlags_HeightRegular)) {
        for (size_t i = 0; i < values.size(); ++i) {
            auto isSelected = selected == i;
            if (ImGui::Selectable(values[i].data(), isSelected)) {
                collider.shape = magic_enum::enum_values<Shape>()[i];
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    switch (collider.shape) {
        case Shape::Box: {
            auto widthBox = availableWidth - (labelSize2 + ImGui::GetStyle().FramePadding.x * 2);
            ImGui::SetNextItemWidth(widthBox);
            static float halfExtent[3]{};
            ImGui::DragFloat3(label2, halfExtent, 0.001f, -10000.0f, 10000.0f, "%.6f");
            break;
        }
        case Shape::Capsule: {
            auto widthHalfHeight = availableWidth - (labelSize4 + ImGui::GetStyle().FramePadding.x * 2);
            widthRadius = widthHalfHeight = std::min(widthRadius, widthHalfHeight);
            ImGui::SetNextItemWidth(widthHalfHeight);
            static float halfHeight{};
            ImGui::DragFloat(label4, &halfHeight, 0.001f, -10000.0f, 10000.0f, "%.6f");
        }
        case Shape::Sphere: {
            ImGui::SetNextItemWidth(widthRadius);
            static float radius{};
            ImGui::DragFloat(label3, &radius, 0.001f, -10000.0f, 10000.0f, "%.6f");
            break;
        }
        case Shape::ConvexHull:
        case Shape::Mesh: {
            ImGui::Text("Loaded from model");
            break;
        }
    }
}

void DeveloperUI::showComponent(entt::entity entity, RigidBody& rigidBody) {
    float availableWidth = ImGui::GetContentRegionAvail().x;

    auto label1     = "Mass";
    auto label2     = "Linear damping";
    auto label3     = "Angular damping";
    auto label4     = "Kinematic";
    auto labelSize1 = ImGui::CalcTextSize(label1).x;
    auto labelSize2 = ImGui::CalcTextSize(label2).x;
    auto labelSize3 = ImGui::CalcTextSize(label3).x;
    auto labelSize4 = ImGui::CalcTextSize(label4).x;
    auto width      = availableWidth - (std::max(std::max(std::max(labelSize1, labelSize2), labelSize3), labelSize4) +
                                   ImGui::GetStyle().FramePadding.x * 2);

    static float mass{};
    static float linearDamping{};
    static float angularDamping{};
    static bool isKinematic{};

    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(label1, &mass, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(label2, &linearDamping, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(label3, &angularDamping, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::Checkbox(label4, &isKinematic);
}

void DeveloperUI::showComponent(entt::entity entity, Camera& camera) {
}

void DeveloperUI::showComponent(entt::entity entity, Renderable& renderable) {
}

void DeveloperUI::showComponent(entt::entity entity, Vehicle& vehicle) {
}

void DeveloperUI::showComponent(entt::entity entity, VehicleController& vehicleController) {
}

void DeveloperUI::addComponent(entt::entity entity, Name&) {
    std::random_device rd;
    auto seedData = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seedData), std::end(seedData), std::ref(rd));
    std::seed_seq seq(std::begin(seedData), std::end(seedData));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{ generator };

    const uuids::uuid id = gen();
    std::string uuidStr  = uuids::to_string(id);

    _dispatcher.trigger<AddNodeNameEvent>({
        entity, hashed_string_owner{ uuidStr.data(), uuidStr.length() }
    });
}

void DeveloperUI::addComponent(entt::entity entity, Collider& collider) {
}

void DeveloperUI::addComponent(entt::entity entity, RigidBody& rigidBody) {
}

void DeveloperUI::addComponent(entt::entity entity, Camera& camera) {
}

void DeveloperUI::addComponent(entt::entity entity, Renderable& renderable) {
}

void DeveloperUI::addComponent(entt::entity entity, Vehicle& vehicle) {
}

void DeveloperUI::deleteComponent(entt::entity entity, Name& name) {
    _dispatcher.trigger<ChangeNodeNameEvent>({ name.name, ""_hs });
}

void DeveloperUI::deleteComponent(entt::entity entity, Static& isStatic) {
}

void DeveloperUI::deleteComponent(entt::entity entity, Collider& collider) {
}

void DeveloperUI::deleteComponent(entt::entity entity, RigidBody& rigidBody) {
}

void DeveloperUI::deleteComponent(entt::entity entity, Camera& camera) {
}

void DeveloperUI::deleteComponent(entt::entity entity, Renderable& renderable) {
}

void DeveloperUI::deleteComponent(entt::entity entity, Vehicle& vehicle) {
}

void DeveloperUI::deleteComponent(entt::entity entity, VehicleController& vehicleController) {
}
