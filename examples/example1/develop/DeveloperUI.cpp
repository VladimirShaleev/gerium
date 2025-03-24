#include "DeveloperUI.hpp"
#include "../events/AddModelEvent.hpp"
#include "../events/AddNodeNameEvent.hpp"
#include "../events/ChangeNodeNameEvent.hpp"
#include "../events/DeleteNodeEvent.hpp"
#include "../events/MoveNodeEvent.hpp"

using namespace std::string_literals;
using namespace entt::literals;

#define ComponentTypes Name, Transform, Static, Collider, RigidBody, Renderable, Vehicle, VehicleController

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
        // ImGui::Checkbox("Snap to grid", &settings.snapToGrid);

        constexpr auto values = magic_enum::enum_names<Settings::Transfrom>();

        auto selected = (int) magic_enum::enum_index(settings.transform).value();

        if (ImGui::BeginCombo("Transform", values[selected].data(), ImGuiComboFlags_HeightRegular)) {
            for (size_t i = 0; i < values.size(); ++i) {
                auto isSelected = selected == i;
                if (ImGui::Selectable(values[i].data(), isSelected)) {
                    settings.transform = magic_enum::enum_values<Settings::Transfrom>()[i];
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

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
    auto& settings = _registry.ctx().get<Settings>();

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

    if (ImGui::Begin("Scene Graph")) {
        if (ImGui::BeginTable("mygrid", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Hierarhy");
            ImGui::TableSetupColumn("Properties");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

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
    }
    ImGui::End();

    if (_selected != entt::null && !_registry.any_of<Wheel>(_selected)) {
        Camera* camera = nullptr;
        for (auto [_, c] : _registry.view<Camera>().each()) {
            if (c.active) {
                camera = &c;
                break;
            }
        }
        assert(camera);

        glm::mat4 identity = glm::identity<glm::mat4>();

        auto world = _registry.get<Transform>(_selected).matrix;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::BeginFrame();

        ImGuiIO& io               = ImGui::GetIO();
        float viewManipulateRight = io.DisplaySize.x;
        float viewManipulateTop   = 0;

        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::DrawGrid(&camera->view[0][0], &camera->projection[0][0], &identity[0][0], 100.0f);

        ImGuizmo::OPERATION operation;
        switch (settings.transform) {
            case Settings::Translate:
                operation = ImGuizmo::OPERATION::TRANSLATE;
                break;
            case Settings::Rotate:
                operation = ImGuizmo::OPERATION::ROTATE;
                break;
            case Settings::Scale:
                operation = ImGuizmo::OPERATION::SCALE;
                break;
        }

        static float snap[3] = { 10.0f, 10.0f, 10.0f };
        if (ImGuizmo::Manipulate(&camera->view[0][0],
                                 &camera->projection[0][0],
                                 operation,
                                 ImGuizmo::MODE::LOCAL,
                                 &world[0][0],
                                 settings.snapToGrid ? snap : nullptr)) {
            glm::vec3 tanslation;
            glm::vec3 scale;
            glm::quat orientation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(world, scale, orientation, tanslation, skew, perspective);

            settings.transforming = true;
            _dispatcher.enqueue<MoveNodeEvent>(_selected, tanslation, orientation, scale, settings.transformChilds);
        }
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
        auto& settings        = _registry.ctx().get<Settings>();
        settings.transforming = true;
        _dispatcher.enqueue<MoveNodeEvent>(
            entity, tanslation, glm::quat(glm::radians(euler)), scale, settings.transformChilds);
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
            ImGui::DragFloat3(label2, &collider.halfExtent.x, 0.001f, -10000.0f, 10000.0f, "%.6f");
            break;
        }
        case Shape::Capsule: {
            auto widthHalfHeight = availableWidth - (labelSize4 + ImGui::GetStyle().FramePadding.x * 2);
            widthRadius = widthHalfHeight = std::min(widthRadius, widthHalfHeight);
            ImGui::SetNextItemWidth(widthHalfHeight);
            static float halfHeight{};
            ImGui::DragFloat(label4, &collider.halfHeightOfCylinder, 0.001f, -10000.0f, 10000.0f, "%.6f");
        }
        case Shape::Sphere: {
            ImGui::SetNextItemWidth(widthRadius);
            ImGui::DragFloat(label3, &collider.radius, 0.001f, -10000.0f, 10000.0f, "%.6f");
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

    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(label1, &rigidBody.mass, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(label2, &rigidBody.linearDamping, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(label3, &rigidBody.angularDamping, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::Checkbox(label4, &rigidBody.isKinematic);
}

void DeveloperUI::showComponent(entt::entity entity, Renderable& renderable) {
    std::vector<hashed_string_owner> strPool;
    std::vector<const char*> textures;
    for (const auto& texId : texIds) {
        const auto path = std::filesystem::path(texId.data()).make_preferred();
        strPool.push_back(path.string());
    }
    textures.reserve(strPool.size() + 1);
    textures.push_back("None");
    for (const auto& str : strPool) {
        textures.push_back(str.data());
    }

    constexpr std::array labels = { "Name",
                                    "Base Color",
                                    "Metallic Roughness",
                                    "Normal",
                                    "Occlusion",
                                    "Emissive",
                                    "Base Color Factor",
                                    "Emissive Factor",
                                    "Metallic Factor",
                                    "Roughness Factor",
                                    "Occlusion Strength",
                                    "Alpha Cutoff",
                                    "Alpha Mask",
                                    "Double Sided",
                                    "Transparent" };

    float availableWidth = ImGui::GetContentRegionAvail().x;

    float width = 0;
    for (size_t i = 0; i < labels.size(); ++i) {
        width = std::max(width, ImGui::CalcTextSize(labels[i]).x);
    }
    width = availableWidth - (width + ImGui::GetStyle().FramePadding.x * 2);

    auto selectTexture = [width, &strPool, &textures](int id, const char* label, hashed_string_owner& texture) {
        ImGui::PushID(id);

        auto findTexture = std::find(strPool.begin(), strPool.end(), texture);
        auto selected    = findTexture != strPool.end() ? std::distance(strPool.begin(), findTexture) + 1 : 0;

        ImGui::SetNextItemWidth(width);
        if (ImGui::BeginCombo(label, textures[selected], ImGuiComboFlags_HeightLarge)) {
            for (size_t i = 0; i < textures.size(); ++i) {
                auto isSelected = selected == i;
                if (ImGui::Selectable(textures[i], isSelected)) {
                    texture = i != 0 ? strPool[i - 1] : hashed_string_owner{ "" };
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::PopID();
    };

    for (size_t i = 0; i < renderable.meshes.size(); ++i) {
        int sId        = (int) i * 20;
        auto& material = renderable.meshes[i].material;

        const auto name = "material "s + std::to_string(i);
        ImGui::Text(name.c_str());
        ImGui::Separator();

        auto index =
            std::distance(std::begin(techIds), std::find(std::begin(techIds), std::end(techIds), material.name));

        ImGui::PushID(sId + 0);
        ImGui::SetNextItemWidth(width);
        if (ImGui::BeginCombo(labels[0], techIds[index].data(), ImGuiComboFlags_HeightRegular)) {
            for (size_t i = 0; i < std::size(techIds); ++i) {
                auto isSelected = index == i;
                if (ImGui::Selectable(techIds[i].data(), isSelected)) {
                    ////
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();

        selectTexture(sId + 1, labels[1], material.baseColorTexture);
        selectTexture(sId + 2, labels[2], material.metallicRoughnessTexture);
        selectTexture(sId + 3, labels[3], material.normalTexture);
        selectTexture(sId + 4, labels[4], material.occlusionTexture);
        selectTexture(sId + 5, labels[5], material.emissiveTexture);

        ImGui::PushID(sId + 6);
        ImGui::SetNextItemWidth(width);
        ImGui::ColorEdit4(labels[6], &material.baseColorFactor.x);
        ImGui::PopID();
        ImGui::PushID(sId + 7);
        ImGui::SetNextItemWidth(width);
        ImGui::ColorEdit3(labels[7], &material.emissiveFactor.x);
        ImGui::PopID();
        ImGui::PushID(sId + 8);
        ImGui::SetNextItemWidth(width);
        ImGui::DragFloat(labels[8], &material.metallicFactor, 0.001f, 0.0f, 1.0f, "%.3f");
        ImGui::PopID();
        ImGui::PushID(sId + 9);
        ImGui::SetNextItemWidth(width);
        ImGui::DragFloat(labels[9], &material.roughnessFactor, 0.001f, 0.0f, 1.0f, "%.3f");
        ImGui::PopID();
        ImGui::PushID(sId + 10);
        ImGui::SetNextItemWidth(width);
        ImGui::DragFloat(labels[10], &material.occlusionStrength, 0.001f, 0.0f, 1.0f, "%.3f");
        ImGui::PopID();
        ImGui::PushID(sId + 11);
        ImGui::SetNextItemWidth(width);
        ImGui::DragFloat(labels[11], &material.alphaCutoff, 0.001f, 0.0f, 1.0f, "%.3f");
        ImGui::PopID();

        auto alphaMask   = (material.flags & MaterialFlags::AlphaMask) == MaterialFlags::AlphaMask;
        auto doubleSided = (material.flags & MaterialFlags::DoubleSided) == MaterialFlags::DoubleSided;
        auto transparent = (material.flags & MaterialFlags::Transparent) == MaterialFlags::Transparent;
        ImGui::PushID(sId + 12);
        ImGui::SetNextItemWidth(width);
        ImGui::Checkbox(labels[12], &alphaMask);
        ImGui::PopID();
        ImGui::PushID(sId + 13);
        ImGui::SetNextItemWidth(width);
        ImGui::Checkbox(labels[13], &doubleSided);
        ImGui::PopID();
        ImGui::PushID(sId + 14);
        ImGui::SetNextItemWidth(width);
        ImGui::Checkbox(labels[14], &transparent);
        ImGui::PopID();

        if (alphaMask) {
            material.flags |= MaterialFlags::AlphaMask;
        } else {
            material.flags &= ~MaterialFlags::AlphaMask;
        }

        if (doubleSided) {
            material.flags |= MaterialFlags::DoubleSided;
        } else {
            material.flags &= ~MaterialFlags::DoubleSided;
        }

        if (transparent) {
            material.flags |= MaterialFlags::Transparent;
        } else {
            material.flags &= ~MaterialFlags::Transparent;
        }
    }
}

void DeveloperUI::showComponent(entt::entity entity, Vehicle& vehicle) {
    constexpr std::array engineLabels        = { "Max Torque", "Min RPM", "Max RPM" };
    constexpr std::array constraintLabels    = { "Max Roll Angle", "Anti Rollbar" };
    constexpr std::array wheelSettingsLabels = {
        "Angular Damping", "Max Steering Angle", "Max Hand Brake Torque", "Hand Brake"
    };
    constexpr std::array transmissionLabels = { "Clutch Strength", "Switch Time", "Gear Ratios" };
    constexpr std::array differentialLabels = { "Limited Slip Ratio", "Wheel Drive" };
    constexpr std::array driveLabels        = {
        "Camber", "Caster Angle",       "King Pin Angle",       "Suspension Forward Angle", "Suspension Sideways Angle",
        "Toe",    "Suspension Damping", "Suspension Frequency", "Suspension Max Length",    "Suspension Min Length"
    };

    float availableWidth = ImGui::GetContentRegionAvail().x;

    float width = 0;
    for (size_t i = 0; i < engineLabels.size(); ++i) {
        width = std::max(width, ImGui::CalcTextSize(engineLabels[i]).x);
    }
    width          = availableWidth - (width + ImGui::GetStyle().FramePadding.x * 2);
    auto maxTorque = vehicle.maxTorque;
    auto minRPM    = vehicle.minRPM;
    auto maxRPM    = vehicle.maxRPM;
    ImGui::Text("engine");
    ImGui::Separator();
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(engineLabels[0], &maxTorque, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(engineLabels[1], &minRPM, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(engineLabels[2], &maxRPM, 0.001f, -10000.0f, 10000.0f, "%.6f");

    width = 0;
    for (size_t i = 0; i < constraintLabels.size(); ++i) {
        width = std::max(width, ImGui::CalcTextSize(constraintLabels[i]).x);
    }
    width             = availableWidth - (width + ImGui::GetStyle().FramePadding.x * 2);
    auto maxRollAngle = glm::degrees(vehicle.maxRollAngle);
    auto antiRollbar  = vehicle.antiRollbar;
    ImGui::Spacing();
    ImGui::Text("constraints");
    ImGui::Separator();
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(constraintLabels[0], &maxRollAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::Checkbox(constraintLabels[1], &antiRollbar);

    width = 0;
    for (size_t i = 0; i < wheelSettingsLabels.size(); ++i) {
        width = std::max(width, ImGui::CalcTextSize(wheelSettingsLabels[i]).x);
    }
    width                          = availableWidth - (width + ImGui::GetStyle().FramePadding.x * 2);
    auto angularDamping            = glm::degrees(vehicle.angularDamping);
    auto maxSteeringAngle          = glm::degrees(vehicle.maxSteeringAngle);
    auto maxHandBrakeTorque        = vehicle.maxHandBrakeTorque;
    constexpr auto handBrakeValues = magic_enum::enum_names<WheelDrive>();
    auto handBrake                 = (int) magic_enum::enum_index(vehicle.handBrake).value();
    ImGui::Spacing();
    ImGui::Text("wheel settings");
    ImGui::Separator();
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(wheelSettingsLabels[0], &angularDamping, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(wheelSettingsLabels[1], &maxSteeringAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(wheelSettingsLabels[2], &maxHandBrakeTorque, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo(wheelSettingsLabels[3], handBrakeValues[handBrake].data(), ImGuiComboFlags_HeightRegular)) {
        for (size_t i = 0; i < handBrakeValues.size(); ++i) {
            auto isSelected = handBrake == i;
            if (ImGui::Selectable(handBrakeValues[i].data(), isSelected)) {
                ////
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    width = 0;
    for (size_t i = 0; i < transmissionLabels.size(); ++i) {
        width = std::max(width, ImGui::CalcTextSize(transmissionLabels[i]).x);
    }
    width               = availableWidth - (width + ImGui::GetStyle().FramePadding.x * 2);
    float inputWidth    = availableWidth - (ImGui::CalcTextSize("Remove").x + ImGui::GetStyle().ItemSpacing.x +
                                         ImGui::GetStyle().FramePadding.x * 2);
    auto clutchStrength = vehicle.clutchStrength;
    auto switchTime     = vehicle.switchTime;
    auto gearRatios     = vehicle.gearRatios;
    ImGui::Spacing();
    ImGui::Text("transmission");
    ImGui::Separator();
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(transmissionLabels[0], &clutchStrength, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(transmissionLabels[1], &switchTime, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::Text("%s", transmissionLabels[2]);
    for (size_t i = 0; i < gearRatios.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        ImGui::PushItemWidth(inputWidth);
        ImGui::InputFloat("##input", &gearRatios[i]);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            gearRatios.erase(gearRatios.begin() + i);
            --i;
        }
        ImGui::PopID();
    }
    if (ImGui::Button("Add")) {
        gearRatios.push_back(0.0f);
    }

    width = 0;
    for (size_t i = 0; i < differentialLabels.size(); ++i) {
        width = std::max(width, ImGui::CalcTextSize(differentialLabels[i]).x);
    }
    width                           = availableWidth - (width + ImGui::GetStyle().FramePadding.x * 2);
    auto limitedSlipRatio           = vehicle.limitedSlipRatio;
    constexpr auto wheelDriveValues = magic_enum::enum_names<WheelDrive>();
    auto wheelDrive                 = (int) magic_enum::enum_index(vehicle.wheelDrive).value();
    ImGui::Spacing();
    ImGui::Text("differential");
    ImGui::Separator();
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(differentialLabels[0], &limitedSlipRatio, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo(differentialLabels[1], wheelDriveValues[wheelDrive].data(), ImGuiComboFlags_HeightRegular)) {
        for (size_t i = 0; i < wheelDriveValues.size(); ++i) {
            auto isSelected = wheelDrive == i;
            if (ImGui::Selectable(wheelDriveValues[i].data(), isSelected)) {
                ////
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    width = 0;
    for (size_t i = 0; i < driveLabels.size(); ++i) {
        width = std::max(width, ImGui::CalcTextSize(driveLabels[i]).x);
    }
    width                             = availableWidth - (width + ImGui::GetStyle().FramePadding.x * 2);
    auto frontCamber                  = glm::degrees(vehicle.frontCamber);
    auto frontCasterAngle             = glm::degrees(vehicle.frontCasterAngle);
    auto frontKingPinAngle            = glm::degrees(vehicle.frontKingPinAngle);
    auto frontSuspensionForwardAngle  = glm::degrees(vehicle.frontSuspensionForwardAngle);
    auto frontSuspensionSidewaysAngle = glm::degrees(vehicle.frontSuspensionSidewaysAngle);
    auto frontToe                     = glm::degrees(vehicle.frontToe);
    auto frontSuspensionDamping       = vehicle.frontSuspensionDamping;
    auto frontSuspensionFrequency     = vehicle.frontSuspensionFrequency;
    auto frontSuspensionMaxLength     = vehicle.frontSuspensionMaxLength;
    auto frontSuspensionMinLength     = vehicle.frontSuspensionMinLength;
    ImGui::Spacing();
    ImGui::Text("front");
    ImGui::Separator();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(0);
    ImGui::DragFloat(driveLabels[0], &frontCamber, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(1);
    ImGui::DragFloat(driveLabels[1], &frontCasterAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(2);
    ImGui::DragFloat(driveLabels[2], &frontKingPinAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(3);
    ImGui::DragFloat(driveLabels[3], &frontSuspensionForwardAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(4);
    ImGui::DragFloat(driveLabels[4], &frontSuspensionSidewaysAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(5);
    ImGui::DragFloat(driveLabels[5], &frontToe, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(6);
    ImGui::DragFloat(driveLabels[6], &frontSuspensionDamping, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(7);
    ImGui::DragFloat(driveLabels[7], &frontSuspensionFrequency, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(8);
    ImGui::DragFloat(driveLabels[8], &frontSuspensionMaxLength, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(9);
    ImGui::DragFloat(driveLabels[9], &frontSuspensionMinLength, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::PopID();

    auto rearCamber                  = glm::degrees(vehicle.rearCamber);
    auto rearCasterAngle             = glm::degrees(vehicle.rearCasterAngle);
    auto rearKingPinAngle            = glm::degrees(vehicle.rearKingPinAngle);
    auto rearSuspensionForwardAngle  = glm::degrees(vehicle.rearSuspensionForwardAngle);
    auto rearSuspensionSidewaysAngle = glm::degrees(vehicle.rearSuspensionSidewaysAngle);
    auto rearToe                     = glm::degrees(vehicle.rearToe);
    auto rearSuspensionDamping       = vehicle.rearSuspensionDamping;
    auto rearSuspensionFrequency     = vehicle.rearSuspensionFrequency;
    auto rearSuspensionMaxLength     = vehicle.rearSuspensionMaxLength;
    auto rearSuspensionMinLength     = vehicle.rearSuspensionMinLength;
    ImGui::Spacing();
    ImGui::Text("rear");
    ImGui::Separator();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(10);
    ImGui::DragFloat(driveLabels[0], &rearCamber, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(11);
    ImGui::DragFloat(driveLabels[1], &rearCasterAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(12);
    ImGui::DragFloat(driveLabels[2], &rearKingPinAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(13);
    ImGui::DragFloat(driveLabels[3], &rearSuspensionForwardAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(14);
    ImGui::DragFloat(driveLabels[4], &rearSuspensionSidewaysAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(15);
    ImGui::DragFloat(driveLabels[5], &rearToe, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(16);
    ImGui::DragFloat(driveLabels[6], &rearSuspensionDamping, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(17);
    ImGui::DragFloat(driveLabels[7], &rearSuspensionFrequency, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(18);
    ImGui::DragFloat(driveLabels[8], &rearSuspensionMaxLength, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::PopID();
    ImGui::SetNextItemWidth(width);
    ImGui::PushID(19);
    ImGui::DragFloat(driveLabels[9], &rearSuspensionMinLength, 0.001f, -10000.0f, 10000.0f, "%.6f");
    ImGui::PopID();
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

void DeveloperUI::deleteComponent(entt::entity entity, Renderable& renderable) {
}

void DeveloperUI::deleteComponent(entt::entity entity, Vehicle& vehicle) {
}

void DeveloperUI::deleteComponent(entt::entity entity, VehicleController& vehicleController) {
}
