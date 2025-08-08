#pragma once
#include <cmath>
#include <algorithm>
#include "Globals.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

struct Vector2 { float x, y; };

struct Vector3 { 
    float x, y, z;
    
    float Magnitude() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    
    Vector3 operator-(const Vector3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }
};

struct view_matrix_t {
    float matrix[16];
};

class Entity {
public:
    int Team = 0;
    int Health = 100;
    char Name[128] = {0};
    Vector3 Position = {0,0,0};

    void Update(uintptr_t pawn, uintptr_t controller) {
        uintptr_t nameAddr = mem.Read<uintptr_t>(controller + Offsets::m_sSanitizedPlayerName);
        mem.ReadArray(nameAddr, Name, sizeof(Name));
        
        Team = mem.Read<int>(pawn + Offsets::m_iTeamNum);
        Health = mem.Read<int>(pawn + Offsets::m_iHealth);
        Position = mem.Read<Vector3>(pawn + Offsets::m_vOldOrigin);
    }

    ImColor GetHealthColor() const {
        float healthPct = std::clamp(Health / 100.0f, 0.0f, 1.0f);
        return ImColor(
            2.0f * (1 - healthPct),
            2.0f * healthPct,
            0.0f
        );
    }
};

extern Entity entities[64];