#include "DeveloperUI.hpp"
#include "../events/AddModelEvent.hpp"
#include "../events/AddNodeNameEvent.hpp"
#include "../events/ChangeColliderEvent.hpp"
#include "../events/ChangeNodeNameEvent.hpp"
#include "../events/ChangeRigidBodyEvent.hpp"
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

std::tuple<BoundingBox, Shape, gerium_float32_t> DeveloperUI::getBBoxAndShapes(entt::entity entity) {
    BoundingBox bbox{};
    Shape shape{};
    gerium_float32_t mass{};
    if (auto renderable = _registry.try_get<Renderable>(entity)) {
        for (const auto& mesh : renderable->meshes) {
            if (!_models.contains(mesh.model)) {
                _models[mesh.model] = loadModel(_cluster, mesh.model);
            }
            const auto& nodes = _models[mesh.model].nodes;
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (mesh.node == i) {
                    bbox  = nodes[i].bbox.combine(bbox);
                    shape = nodes[i].colliderShape;
                    mass  = nodes[i].mass;
                    break;
                }
            }
            if (shape == Shape::ConvexHull || shape == Shape::Mesh) {
                break;
            }
        }
    }
    bbox = BoundingBox({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }).combine(bbox);
    return { bbox, shape, mass };
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

        if (ImGuizmo::Manipulate(
                &camera->view[0][0], &camera->projection[0][0], operation, ImGuizmo::MODE::LOCAL, &world[0][0])) {
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

    auto valid = _isValidName;

    const auto label = "Unique name";
    const auto width = calcItemWidth(std::array{ label });

    ImGui::SetNextItemWidth(width);
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

    const auto label1 = "Position";
    const auto label2 = "Scale";
    const auto label3 = "Rotation";
    const auto width  = calcItemWidth(std::array{ label1, label2, label3 });

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
    const auto label1 = "Shape";
    const auto label2 = "Half extent";
    const auto label3 = "Radius";
    const auto label4 = "Half height";
    const auto width  = calcItemWidth(std::array{ label1 });
    auto width2       = calcItemWidth(std::array{ label3 });

    auto newCollider    = collider;
    newCollider.changed = false;

    constexpr auto values       = magic_enum::enum_names<Shape>();
    const auto selected         = (int) magic_enum::enum_index(newCollider.shape).value();
    const auto [bbox, shape, _] = getBBoxAndShapes(entity);

    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo(label1, values[selected].data(), ImGuiComboFlags_HeightRegular)) {
        for (size_t i = 0; i < values.size(); ++i) {
            const auto isSelected = selected == i;
            const auto isEnable   = i < (size_t) Shape::ConvexHull || i == (size_t) shape;
            if (!isEnable) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Selectable(values[i].data(), isSelected)) {
                newCollider.changed = true;
                newCollider.shape   = magic_enum::enum_values<Shape>()[i];
                switch (newCollider.shape) {
                    case Shape::Box: {
                        newCollider.halfExtent = (bbox.max() - bbox.min()) * 0.5f;
                        break;
                    }
                    case Shape::Capsule: {
                        const auto radius                = bbox.getCentroid() - bbox.min();
                        newCollider.halfHeightOfCylinder = bbox.getWidth(Axis::Y) * 0.5f;
                        newCollider.radius               = glm::min(glm::min(radius.x, radius.y), radius.z);
                        break;
                    }
                    case Shape::Sphere: {
                        const auto radius  = bbox.getCentroid() - bbox.min();
                        newCollider.radius = glm::max(glm::max(radius.x, radius.y), radius.z);
                        break;
                    }
                    case Shape::ConvexHull:
                    case Shape::Mesh: {
                        break;
                    }
                }
            }
            if (!isEnable) {
                ImGui::EndDisabled();
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    switch (newCollider.shape) {
        case Shape::Box: {
            ImGui::SetNextItemWidth(calcItemWidth(std::array{ label2 }));
            if (ImGui::DragFloat3(label2, &newCollider.halfExtent.x, 0.001f, 0.01f, 10000.0f, "%.6f")) {
                newCollider.changed = true;
            }
            break;
        }
        case Shape::Capsule: {
            width2 = calcItemWidth(std::array{ label2, label3 });
            ImGui::SetNextItemWidth(width2);
            if (ImGui::DragFloat(label4, &newCollider.halfHeightOfCylinder, 0.001f, 0.01f, 10000.0f, "%.6f")) {
                newCollider.changed = true;
            }
        }
        case Shape::Sphere: {
            ImGui::SetNextItemWidth(width2);
            if (ImGui::DragFloat(label3, &newCollider.radius, 0.001f, 0.01f, 10000.0f, "%.6f")) {
                newCollider.changed = true;
            }
            break;
        }
        case Shape::ConvexHull:
        case Shape::Mesh: {
            ImGui::Text("Loaded from model");
            break;
        }
    }

    if (newCollider.changed) {
        _dispatcher.enqueue<ChangeColliderEvent>(entity, newCollider);
    }
}

void DeveloperUI::showComponent(entt::entity entity, RigidBody& rigidBody) {
    const auto label1 = "Mass";
    const auto label2 = "Linear damping";
    const auto label3 = "Angular damping";
    const auto label4 = "Kinematic";
    const auto width  = calcItemWidth(std::array{ label1, label2, label3, label4 });

    auto newRigidBody    = rigidBody;
    newRigidBody.changed = false;

    ImGui::SetNextItemWidth(width);
    newRigidBody.changed |= ImGui::DragFloat(label1, &newRigidBody.mass, 0.01f, 0.0f, 100000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    newRigidBody.changed |= ImGui::DragFloat(label2, &newRigidBody.linearDamping, 0.001f, 0.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    newRigidBody.changed |= ImGui::DragFloat(label3, &newRigidBody.angularDamping, 0.001f, 0.0f, 10000.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    newRigidBody.changed |= ImGui::Checkbox(label4, &newRigidBody.isKinematic);
    if (newRigidBody.changed) {
        _dispatcher.enqueue<ChangeRigidBodyEvent>(entity, newRigidBody);
    }
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

    std::array labels = { "Name",
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

    const auto width = calcItemWidth(labels);

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
        ImGui::Text("%s", name.c_str());
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

    auto width     = calcItemWidth(engineLabels);
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

    width             = calcItemWidth(constraintLabels);
    auto maxRollAngle = glm::degrees(vehicle.maxRollAngle);
    auto antiRollbar  = vehicle.antiRollbar;
    ImGui::Spacing();
    ImGui::Text("constraints");
    ImGui::Separator();
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(constraintLabels[0], &maxRollAngle, 0.001f, -360.0f, 360.0f, "%.6f");
    ImGui::SetNextItemWidth(width);
    ImGui::Checkbox(constraintLabels[1], &antiRollbar);

    width                          = calcItemWidth(wheelSettingsLabels);
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

    width               = calcItemWidth(transmissionLabels);
    auto inputWidth     = calcItemWidth(std::array{ "Remove" }) - ImGui::GetStyle().ItemSpacing.x * 2.0f;
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

    width                           = calcItemWidth(differentialLabels);
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

    width                             = calcItemWidth(driveLabels);
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

void DeveloperUI::addComponent(entt::entity entity, Static& isStatic) {
}

void DeveloperUI::addComponent(entt::entity entity, Collider& collider) {
    const auto [bbox, shape, _] = getBBoxAndShapes(entity);
    Collider newCollider;
    newCollider.halfExtent = (bbox.max() - bbox.min()) * 0.5f;
    newCollider.shape      = shape;
    newCollider.changed    = true;
    _dispatcher.enqueue<ChangeColliderEvent>(entity, newCollider);
}

void DeveloperUI::addComponent(entt::entity entity, RigidBody& rigidBody) {
    const auto [_1, _2, mass] = getBBoxAndShapes(entity);

    auto& rb = _registry.emplace<RigidBody>(entity);
    rb.mass  = mass;
}

void DeveloperUI::addComponent(entt::entity entity, Renderable& renderable) {
}

void DeveloperUI::addComponent(entt::entity entity, Vehicle& vehicle) {
    // auto& component       = _registry.emplace<Vehicle>(entity);
    // const auto& transform = _registry.get<Transform>(entity);
    // const auto invMatrix  = glm::inverse(transform.matrix);
    // for (const auto child : _registry.get<Node>(entity).childs) {
    //     const entt::hashed_string name        = _registry.get<Node>(child).name;
    //     std::optional<WheelPosition> wheelPos = std::nullopt;
    //     switch (name) {
    //         case "wheel_lf"_hs:
    //             wheelPos = WheelPosition::FrontLeft;
    //             break;
    //         case "wheel_rf"_hs:
    //             wheelPos = WheelPosition::FrontRight;
    //             break;
    //         case "wheel_lb"_hs:
    //             wheelPos = WheelPosition::BackLeft1;
    //             break;
    //         case "wheel_rb"_hs:
    //             wheelPos = WheelPosition::BackRight1;
    //             break;
    //         case "wheel_lb2"_hs:
    //             wheelPos = WheelPosition::BackLeft2;
    //             break;
    //         case "wheel_rb2"_hs:
    //             wheelPos = WheelPosition::BackRight2;
    //             break;
    //     }

    //     if (wheelPos) {
    //         const auto& childTransform = _registry.get<Transform>(child);

    //         const auto localMatrix = invMatrix * childTransform.matrix;
    //         glm::vec3 tanslation;
    //         glm::vec3 scale;
    //         glm::quat orientation;
    //         glm::vec3 skew;
    //         glm::vec4 perspective;
    //         glm::decompose(localMatrix, scale, orientation, tanslation, skew, perspective);

    //         auto& wheel    = _registry.emplace<Wheel>(child);
    //         wheel.parent   = entity;
    //         wheel.position = wheelPos.value();
    //         wheel.point    = tanslation * scale;
    //         component.wheels.push_back(child);
    //     }
    // }
    // updateBodies();
}

void DeveloperUI::addComponent(entt::entity entity, VehicleController& vehicleController) {
    _registry.emplace<VehicleController>(entity);
}

void DeveloperUI::deleteComponent(entt::entity entity, Name& name) {
    _dispatcher.trigger<ChangeNodeNameEvent>({ name.name, ""_hs });
}

void DeveloperUI::deleteComponent(entt::entity entity, Static& isStatic) {
    // _registry.erase<Static>(entity);
    // updateBodies();
}

void DeveloperUI::deleteComponent(entt::entity entity, Collider& collider) {
    _dispatcher.enqueue<ChangeColliderEvent>(entity, std::nullopt);
}

void DeveloperUI::deleteComponent(entt::entity entity, RigidBody& rigidBody) {
    _registry.erase<RigidBody>(entity);
}

void DeveloperUI::deleteComponent(entt::entity entity, Renderable& renderable) {
}

void DeveloperUI::deleteComponent(entt::entity entity, Vehicle& vehicle) {
    // for (auto wheel : vehicle.wheels) {
    //     _registry.erase<Wheel>(wheel);
    // }
    // _registry.erase<Vehicle>(entity);
    // updateBodies();
}

void DeveloperUI::deleteComponent(entt::entity entity, VehicleController& vehicleController) {
    _registry.erase<VehicleController>(entity);
}
