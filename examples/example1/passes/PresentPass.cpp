#include "PresentPass.hpp"
#include "../Application.hpp"
#include "../components/Name.hpp"
#include "../components/Node.hpp"
#include "../components/Settings.hpp"
#include "../events/AddModelEvent.hpp"
#include "../events/DeleteNodeEvent.hpp"

using namespace entt::literals;

void PresentPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto technique = resourceManager().loadTechnique(TECH_POSTPROCESS_ID);

    auto ds = resourceManager().createDescriptorSet("");
    gerium_renderer_bind_resource(renderer, ds, 0, "base", false);

    gerium_command_buffer_bind_technique(commandBuffer, technique);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, 0);
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);
    gerium_command_buffer_draw_profiler(commandBuffer, nullptr);

    if (ImGui::Begin("Settings")) {
        auto& settings = entityRegistry().ctx().get<Settings>();
        if (ImGui::Button("Save state")) {
            settings.state = Settings::Save;
        }
        if (ImGui::Button("Load state")) {
            settings.state = Settings::Load;
        }
    }
    ImGui::End();

    developSceneGraph();
}

void PresentPass::developSceneGraph() {
    static entt::entity selected     = entt::null;
    static entt::entity addModelItem = entt::null;
    static entt::entity deleteItem   = entt::null;
    static int addModelIndex         = 0;
    static bool addModelNameValid    = true;
    static char addModelName[256]{};

    if (ImGui::Begin("Scene Graph")) {
        if (ImGui::BeginTable("mygrid", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Hierarhy");
            ImGui::TableSetupColumn("Properties");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            entt::entity root = entt::null;
            auto& registry    = entityRegistry();
            auto view         = registry.view<Node, Name>();
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
                const auto& node    = entityRegistry().get<Node>(entity);
                const auto name     = entityRegistry().try_get<Name>(entity);
                const auto isUnique = name != nullptr;
                const auto isRoot   = entity == root;
                const auto nodeName = name ? (isRoot ? "root" : name->name.data()) : node.name.data();

                ImGuiTreeNodeFlags flags = isRoot ? ImGuiTreeNodeFlags_DefaultOpen : 0;
                if (node.childs.empty()) {
                    flags |= ImGuiTreeNodeFlags_Leaf;
                }
                auto pushedStyles = 1;
                ImGui::PushID((int) entity);
                ImGui::PushStyleColor(ImGuiCol_Text, isUnique ? uniqueColor : color);
                if (selected == entity) {
                    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                    pushedStyles += 2;
                }
                auto isOpen = ImGui::TreeNodeEx(nodeName, flags);
                if (ImGui::IsItemClicked()) {
                    selected = entity;
                }
                if (ImGui::BeginPopupContextItem()) {
                    selected = entity;
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    if (!isRoot && ImGui::MenuItem("Up")) {
                        auto parent        = entityRegistry().get<Node>(entity).parent;
                        auto& parentChilds = entityRegistry().get<Node>(parent).childs;
                        auto it            = std::find(parentChilds.begin(), parentChilds.end(), entity);
                        if (it != parentChilds.begin()) {
                            std::swap(*it, *(it - 1));
                        }
                    }
                    if (!isRoot && ImGui::MenuItem("Down")) {
                        auto parent        = entityRegistry().get<Node>(entity).parent;
                        auto& parentChilds = entityRegistry().get<Node>(parent).childs;
                        auto it            = std::find(parentChilds.rbegin(), parentChilds.rend(), entity);
                        if (it != parentChilds.rbegin()) {
                            std::swap(*it, *(it - 1));
                        }
                    }
                    if (ImGui::MenuItem("Add model")) {
                        addModelItem      = entity;
                        addModelIndex     = 0;
                        addModelNameValid = false;
                        memset(addModelName, 0, sizeof(addModelName));
                    }
                    if (!isRoot && ImGui::MenuItem("Delete")) {
                        deleteItem = entity;
                    }
                    ImGui::PopStyleColor();
                    ImGui::EndPopup();
                }
                if (!isRoot && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("TREE_NODE", &entity, sizeof(entity));
                    ImGui::Text("Moving %s", nodeName);
                    ImGui::EndDragDropSource();
                }
                if (!isRoot && ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TREE_NODE")) {
                        IM_ASSERT(payload->DataSize == sizeof(entt::entity));
                        auto source = *(entt::entity*) payload->Data;
                        auto dest   = entity;

                        std::queue<entt::entity> visit;
                        for (auto child : entityRegistry().get<Node>(source).childs) {
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
                            for (auto child : entityRegistry().get<Node>(item).childs) {
                                visit.push(child);
                            }
                        }
                        if (correct) {
                            auto prevParent        = entityRegistry().get<Node>(source).parent;
                            auto& prevParentChilds = entityRegistry().get<Node>(prevParent).childs;
                            prevParentChilds.erase(std::find(prevParentChilds.begin(), prevParentChilds.end(), source));

                            entityRegistry().get<Node>(source).parent = dest;
                            entityRegistry().get<Node>(dest).childs.push_back(source);
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

            // ImGui::BeginChild("actions");
            // ImGui::Columns(2, "mygrid", false);
            // if (ImGui::Button("UP", ImVec2(-1, 0))) {
            // }
            // ImGui::NextColumn();
            // if (ImGui::Button("DOWN", ImVec2(-1, 0))) {
            // }
            // ImGui::Columns(1);
            // ImGui::EndChild();

            ImGui::EndTable();
        }

        if (addModelItem != entt::null) {
            ImGui::OpenPopup("Add Model to Node");
            if (ImGui::BeginPopupModal("Add Model to Node", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                auto validText = 0;
                if (!addModelNameValid) {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
                    ++validText;
                }
                if (ImGui::InputText("Unique name", addModelName, sizeof(addModelName))) {
                    std::string str = addModelName;
                    strcpy(addModelName, trim(str).c_str());
                    addModelNameValid = true;
                    if (strlen(addModelName) == 0) {
                        addModelNameValid = false;
                    } else {
                        entt::hashed_string hashedName = addModelName;
                        if (hashedName == "root"_hs) {
                            addModelNameValid = false;
                        } else {
                            for (const auto [_, name] : entityRegistry().view<Name>().each()) {
                                if (name.name == hashedName) {
                                    addModelNameValid = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                ImGui::PopStyleColor(validText);
                ImGui::ListBox("models", &addModelIndex, [](auto _, auto index) {
                    return modelIds[index].data();
                }, nullptr, std::size(modelIds));
                if (!addModelNameValid) {
                    ImGui::BeginDisabled();
                }
                if (ImGui::Button("Add", ImVec2(120, 0))) {
                    auto& dispatcher = renderService().application().dispatcher();
                    dispatcher.enqueue<AddModelEvent>(addModelItem, addModelName, modelIds[addModelIndex]);
                    addModelItem = entt::null;
                    ImGui::CloseCurrentPopup();
                }
                if (!addModelNameValid) {
                    ImGui::EndDisabled();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    addModelItem = entt::null;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        if (deleteItem != entt::null) {
            ImGui::OpenPopup("Delete Node");
            if (ImGui::BeginPopupModal("Delete Node", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Do you want to delete\nthe selected node?");
                if (ImGui::Button("Yes", ImVec2(120, 0))) {
                    auto& dispatcher = renderService().application().dispatcher();
                    dispatcher.enqueue<DeleteNodeEvent>(deleteItem);
                    deleteItem = entt::null;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    deleteItem = entt::null;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }
}
