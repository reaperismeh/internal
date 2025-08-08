#pragma once
#include "entity.hpp"
#include "game.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

namespace ESP {
    inline bool enabled = true;
    inline bool show_names = true;
    inline bool show_health = true;
    inline bool show_distance = false;

    bool WorldToScreen(const Vector3& world, Vector2& screen, const view_matrix_t& matrix) {
        // Simple W2S implementation using the matrix array
        float x = world.x * matrix.matrix[0] + world.y * matrix.matrix[1] + world.z * matrix.matrix[2] + matrix.matrix[3];
        float y = world.x * matrix.matrix[4] + world.y * matrix.matrix[5] + world.z * matrix.matrix[6] + matrix.matrix[7];
        float w = world.x * matrix.matrix[12] + world.y * matrix.matrix[13] + world.z * matrix.matrix[14] + matrix.matrix[15];

        if (w < 0.001f)
            return false;

        float inv_w = 1.0f / w;
        x *= inv_w;
        y *= inv_w;

        // Convert to screen coordinates
        screen.x = (x + 1.0f) * 0.5f * SCREEN_WIDTH;
        screen.y = (1.0f - y) * 0.5f * SCREEN_HEIGHT;

        return true;
    }

    void RenderPlayer(const Entity& ent) {
        if (!enabled) return;

        Vector2 screenPos;
        if (!WorldToScreen(ent.Position, screenPos, game.viewMatrix))
            return;
            
        auto drawList = ImGui::GetBackgroundDrawList();
        
        if (show_names) {
            drawList->AddText(
                ImVec2(screenPos.x, screenPos.y), 
                ent.GetHealthColor(), 
                ent.Name
            );
        }
        
        if (show_health) {
            char healthText[16];
            snprintf(healthText, sizeof(healthText), "HP: %d", ent.Health);
            drawList->AddText(
                ImVec2(screenPos.x, screenPos.y + 15), 
                IM_COL32(255, 255, 255, 255), 
                healthText
            );
        }
    }

    void Render() {
        if (!CS2::ClientDll || !enabled) return;
        
        for (int i = 0; i < 64; i++) {
            if (entities[i].Health > 0) {
                RenderPlayer(entities[i]);
            }
        }
    }
}
