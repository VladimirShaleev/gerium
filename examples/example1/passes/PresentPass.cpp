#include "PresentPass.hpp"
#include "../Application.hpp"
#include "../components/Name.hpp"
#include "../components/Node.hpp"
#include "../components/Settings.hpp"
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

        static int currentItem = 0;
        ImGui::ListBox("models", &currentItem, [](auto _, auto index) {
            return modelIds[index].data();
        }, nullptr, std::size(modelIds));

        if (ImGui::Button("Add model")) {
            renderService().application().addModel(modelIds[currentItem]);
        }
    }
    ImGui::End();

    developSceneGraph();
}

void PresentPass::developSceneGraph() {
    static entt::entity selected   = entt::null;
    static entt::entity deleteItem = entt::null;

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
                if (!isRoot && ImGui::BeginPopupContextItem()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    auto parent        = entityRegistry().get<Node>(entity).parent;
                    auto& parentChilds = entityRegistry().get<Node>(parent).childs;
                    if (ImGui::MenuItem("Up")) {
                        auto it = std::find(parentChilds.begin(), parentChilds.end(), entity);
                        if (it != parentChilds.begin()) {
                            std::swap(*it, *(it - 1));
                        }
                    }
                    if (ImGui::MenuItem("Down")) {
                        auto it = std::find(parentChilds.rbegin(), parentChilds.rend(), entity);
                        if (it != parentChilds.rbegin()) {
                            std::swap(*it, *(it - 1));
                        }
                    }
                    if (ImGui::MenuItem("Delete")) {
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

        if (deleteItem != entt::null) {
            ImGui::OpenPopup("Delete Node");
            if (ImGui::BeginPopupModal("Delete Node", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Do you want to delete\nthe selected node?");
                if (ImGui::Button("Yes", ImVec2(120, 0))) {
                    // auto parent  = entityRegistry().get<Node>(deleteItem).parent;
                    // auto& childs = entityRegistry().get<Node>(parent).childs;
                    // childs.erase(std::find(childs.begin(), childs.end(), deleteItem));
                    // // entityRegistry().destroy(deleteItem);
                    // entityRegistry().erase<Node>(deleteItem);
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
