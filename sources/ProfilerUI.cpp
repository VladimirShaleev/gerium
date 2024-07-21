/*
 * For testing, the UI for the profiler is posted here.
 *
 * UI taken from this project:
 *   https://github.com/PacktPublishing/Mastering-Graphics-Programming-with-Vulkan
 */

#include "ProfilerUI.hpp"

namespace gerium {

static const uint32_t k_distinct_colors[] = {
    0xFF010067, 0xFF00FF00, 0xFFFF0000, 0xFF0000FF, 0xFFFEFF01, 0xFFFEA6FF, 0xFF66DBFF, 0xFF016400,
    0xFF670001, 0xFF3A0095, 0xFFB57D00, 0xFFF600FF, 0xFFE8EEFF, 0xFF004D77, 0xFF92FB90, 0xFFFF7600,
    0xFF00FFD5, 0xFF7E93FF, 0xFF6C826A, 0xFF9D02FF, 0xFF0089FE, 0xFF82477A, 0xFFD22D7E, 0xFF00A985,
    0xFF5600FF, 0xFF0024A4, 0xFF7EAE00, 0xFF3B3D68, 0xFFFFC6BD, 0xFF003426, 0xFF93D3BD, 0xFF17B900,
    0xFF8E009E, 0xFF441500, 0xFF9F8CC2, 0xFFA374FF, 0xFFFFD001, 0xFF544700, 0xFFFE6FE5, 0xFF318278,
    0xFFA14C0E, 0xFFCBD091, 0xFF7099BE, 0xFFE88A96, 0xFF0088BB, 0xFF2C0043, 0xFF74FFDE, 0xFFC6FF00,
    0xFF02E5FF, 0xFF000E62, 0xFF9C8F00, 0xFF52FF98, 0xFFB14475, 0xFFFF00B5, 0xFF78FF00, 0xFF416EFF,
    0xFF395F00, 0xFF82686B, 0xFF4EAD5F, 0xFF4057A7, 0xFFD2FFA5, 0xFF67B1FF, 0xFFFF9B00, 0xFFBE5EE8
};

static uint32_t get_distinct_color(uint32_t index) {
    return k_distinct_colors[index % 64];
}

void ProfilerUI::draw(Profiler* profiler, bool* show, uint32_t maxFrames) {
    if (timestamps.empty()) {
        timestamps.resize(maxFrames * 32);
        colors.resize(maxFrames * 32);
        perFrameActive.resize(maxFrames);

        maxDuration  = 2.0f;
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

            auto hashedName     = hash(timestamp.name);
            auto colorIndexIt   = nameToColor.find(hashedName);
            uint32_t colorIndex = 0;
            if (colorIndexIt == nameToColor.end()) {
                colorIndex = (uint32_t) nameToColor.size();
                nameToColor.insert({ hashedName, colorIndex });
            } else {
                colorIndex = colorIndexIt->second;
            }

            colors[32 * currentFrame + i] = get_distinct_color(colorIndex);
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

    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(500, 260));
    if (ImGui::Begin("GPU Profiler", show, ImGuiWindowFlags_NoScrollbar)) {
        ImGui::Text("GPU Memory Total %uMB", totalMemoryUsed);

        ImGui::Separator();

        // if (!device->profilerSupported()) {
        //     ImGui::BeginDisabled();
        // }

        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos     = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size    = ImGui::GetContentRegionAvail();
            float widget_height   = canvas_size.y - 100;

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
            sprintf(buf, "%3.4fms", maxDuration);
            draw_list->AddText({ cursor_pos.x, cursor_pos.y }, 0xff0000ff, buf);
            draw_list->AddLine(
                { cursor_pos.x + rect_width, cursor_pos.y }, { cursor_pos.x + graph_width, cursor_pos.y }, 0xff0000ff);

            sprintf(buf, "%3.4fms", maxDuration / 2.f);
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
                    draw_list->AddRectFilled({ frame_x, cursor_pos.y + widget_height - rect_height },
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
                    for (int s = 0; s < timestamp.depth; ++s) {
                        spaces[s] = '-';
                        spaces[s + 1] = ' ';
                    }

                    sprintf(buf, "%s%s %2.4f", spaces, timestamp.name, timestamp.elapsed);
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

        static int max_duration_index = 4;
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
