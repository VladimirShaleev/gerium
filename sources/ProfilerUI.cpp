/*
 * For testing, the UI for the profiler is posted here.
 *
 * UI taken from this project:
 *   https://github.com/PacktPublishing/Mastering-Graphics-Programming-with-Vulkan
 */

#include "ProfilerUI.hpp"

namespace gerium {

void ProfilerUI::draw(Profiler* profiler, bool* show, uint32_t maxFrames) {
    if (timestamps.empty()) {
        timestamps.resize(maxFrames * 32);
        colors.resize(maxFrames * 32);
        perFrameActive.resize(maxFrames);

        maxDuration  = 8.0f;
        currentFrame = 0;
        minTime = maxTime = averageTime = 0.f;

        paused = false;
    }

    if (!paused && prevPaused) {
        memset(perFrameActive.data(), 0, perFrameActive.size() * sizeof(perFrameActive[0]));
        memset(timestamps.data(), 0, timestamps.size() * sizeof(timestamps[0]));
        prevPaused = false;
    }

    if (!paused) {
        perFrameActive[currentFrame] = 32;
        gerium_profiler_get_gpu_timestamps(profiler, &perFrameActive[currentFrame], &timestamps[currentFrame * 32]);
        totalMemoryUsed = gerium_profiler_get_gpu_total_memory_used(profiler) / (1024 * 1024);

        for (uint32_t i = 0; i < perFrameActive[currentFrame]; ++i) {
            auto& timestamp = timestamps[32 * currentFrame + i];

            colors[32 * currentFrame + i] = nameColor(timestamp.name);
        }

        if (currentFrame == 0) {
            maxTime     = -FLT_MAX;
            minTime     = FLT_MAX;
            averageTime = 0.f;
        }

        currentFrame = (currentFrame + 1) % maxFrames;

    } else {
        prevPaused = true;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(500, 320));
    if (ImGui::Begin("GPU Profiler", show)) {
        ImGui::Text("GPU Memory Total %uMB", totalMemoryUsed);

        ImGui::Separator();

        // if (!device->profilerSupported()) {
        //     ImGui::BeginDisabled();
        // }

        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos     = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size    = ImGui::GetContentRegionAvail();
            float widget_height   = canvas_size.y - 120;

            float legend_width  = 270;
            float graph_width   = fabsf(canvas_size.x - legend_width);
            uint32_t rect_width = std::ceil(graph_width / maxFrames);
            int32_t rect_x      = std::ceil(graph_width - rect_width);

            double new_average = 0;

            ImGuiIO& io = ImGui::GetIO();

            static char buf[128];

            const ImVec2 mouse_pos = io.MousePos;

            int32_t selected_frame = -1;

            // if (device->profilerSupported()) {
            //  Draw time reference lines
            snprintf(buf, 128, "%3.4fms", maxDuration);
            draw_list->AddText({ cursor_pos.x, cursor_pos.y }, 0xff0000ff, buf);
            draw_list->AddLine(
                { cursor_pos.x + rect_width, cursor_pos.y }, { cursor_pos.x + graph_width, cursor_pos.y }, 0xff0000ff);

            snprintf(buf, 128, "%3.4fms", maxDuration / 2.f);
            draw_list->AddText({ cursor_pos.x, cursor_pos.y + widget_height / 2.f }, 0xff00ffff, buf);
            draw_list->AddLine({ cursor_pos.x + rect_width, cursor_pos.y + widget_height / 2.f },
                               { cursor_pos.x + graph_width, cursor_pos.y + widget_height / 2.f },
                               0xff00ffff);
            //} else {
            //    draw_list->AddText(
            //        {cursor_pos.x, cursor_pos.y + widget_height / 2.f - 11}, 0xff00ffff, "Not supported");
            //}

            // Draw Graph
            for (uint32_t i = 0; i < maxFrames; ++i) {
                int32_t frame_pos = int32_t(currentFrame) - 1 - int32_t(i);
                if (frame_pos < 0) {
                    frame_pos = maxFrames + frame_pos;
                }
                int32_t frame_index = (frame_pos) % maxFrames;

                float frame_x          = cursor_pos.x + rect_x;
                auto* frame_timestamps = &timestamps[frame_index * 32];
                auto* frame_colors     = &colors[frame_index * 32];
                float frame_time       = (float) frame_timestamps[0].elapsed;
                // Clamp values to not destroy the frame data
                frame_time = std::clamp(frame_time, 0.00001f, 1000.f);
                // Update timings
                new_average += frame_time;
                minTime = std::min(minTime, frame_time);
                maxTime = std::max(maxTime, frame_time);

                float rect_height = frame_time / maxDuration * widget_height;
                // drawList->AddRectFilled( { frame_x, cursor_pos.y + rect_height }, { frame_x + rect_width,
                // cursor_pos.y }, 0xffffffff );

                for (uint32_t j = 0; j < perFrameActive[frame_index]; ++j) {
                    const auto& timestamp = frame_timestamps[j];
                    const auto& color     = frame_colors[j];

                    /*if ( timestamp.depth != 1 ) {
                        continue;
                    }*/

                    rect_height = (float) timestamp.elapsed / maxDuration * widget_height;
                    draw_list->AddRectFilled({ frame_x, std::max(cursor_pos.y + widget_height - rect_height, cursor_pos.y) },
                                             { frame_x + rect_width, cursor_pos.y + widget_height },
                                             color);
                }

                if (mouse_pos.x >= frame_x && mouse_pos.x < frame_x + rect_width && mouse_pos.y >= cursor_pos.y &&
                    mouse_pos.y < cursor_pos.y + widget_height) {
                    // if (device->profilerSupported()) {
                    draw_list->AddRectFilled(
                        { frame_x, cursor_pos.y + widget_height }, { frame_x + rect_width, cursor_pos.y }, 0x0fffffff);

                    if (perFrameActive[frame_index]) {
                        ImGui::SetTooltip("frame %u\nelapsed %fms", frame_timestamps[0].frame, frame_time);
                    }
                    //}

                    selected_frame = frame_index;
                }

                draw_list->AddLine({ frame_x, cursor_pos.y + widget_height }, { frame_x, cursor_pos.y }, 0x0fffffff);

                // Debug only
                /*static char buf[ 32 ];
                sprintf( buf, "%u", frame_index );
                draw_list->AddText( { frame_x, cursor_pos.y + widget_height - 64 }, 0xffffffff, buf );

                sprintf( buf, "%u", frame_timestamps[0].frame_index );
                drawList->AddText( { frame_x, cursor_pos.y + widget_height - 32 }, 0xffffffff, buf );*/

                rect_x -= rect_width;
            }

            averageTime = (float) new_average / maxFrames;

            // Draw legend
            ImGui::SetCursorPosX(cursor_pos.x + graph_width);
            // Default to last frame if nothing is selected.

            int32_t frame_pos = int32_t(currentFrame) - 1;
            if (frame_pos < 0) {
                frame_pos = maxFrames + frame_pos;
            }
            int32_t frame_index = (frame_pos) % maxFrames;
            selected_frame      = selected_frame == -1 ? frame_index : selected_frame;
            if (selected_frame >= 0) {
                auto* frame_timestamps = &timestamps[selected_frame * 32];
                auto* frame_colors     = &colors[selected_frame * 32];

                float x = cursor_pos.x + graph_width;
                float y = cursor_pos.y;

                for (uint32_t j = 0; j < perFrameActive[selected_frame]; ++j) {
                    const auto& timestamp = frame_timestamps[j];
                    const auto& color     = frame_colors[j];

                    draw_list->AddRectFilled({ x, y + 4 }, { x + 8, y + 12 }, color);

                    char spaces[33]{};
                    auto depth = timestamp.depth;
                    if (strcmp(timestamp.name, "imgui") == 0) {
                        depth = 1;
                    }
                    for (int s = 0; s < depth; ++s) {
                        spaces[s] = '-';
                        spaces[s + 1] = ' ';
                    }

                    snprintf(buf, 128, "%s%s: %2.4f", spaces, timestamp.name, timestamp.elapsed);
                    draw_list->AddText({ x + 12, y }, 0xffffffff, buf);

                    y += 16;
                }
            }

            ImGui::Dummy({ canvas_size.x, widget_height });
        }

        ImGui::SetNextItemWidth(140.f);
        ImGui::LabelText("", "Max %3.4fms", maxTime);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(140.f);
        ImGui::LabelText("", "Min %3.4fms", minTime);
        ImGui::SameLine();
        ImGui::LabelText("", "Ave %3.4fms", averageTime);

        ImGui::Separator();
        ImGui::Checkbox("Pause", &paused);

        static const char* items[]         = { "33ms", "16ms", "8ms", "4ms", "2ms" };
        static const float max_durations[] = { 33.f, 16.f, 8.f, 4.f, 2.f };

        static int max_duration_index = 2;
        if (ImGui::Combo("Graph Max", &max_duration_index, items, IM_ARRAYSIZE(items))) {
            maxDuration = max_durations[max_duration_index];
        }

        // if (!device->profilerSupported()) {
        //     ImGui::EndDisabled();
        // }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace gerium

using namespace gerium;
