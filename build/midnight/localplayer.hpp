#pragma once
#include "Globals.hpp"
#include "Entity.hpp"

class LocalPlayer {
public:
    uintptr_t pawn = 0;
    uintptr_t controller = 0;
    Vector3 position = {0,0,0};
    int team = 0;

    void Update() {
        pawn = mem.Read<uintptr_t>(CS2::ClientDll + Offsets::dwLocalPlayerPawn);
        if (!pawn) return;
        
        controller = mem.Read<uintptr_t>(CS2::ClientDll + Offsets::dwLocalPlayerController);
        position = mem.Read<Vector3>(pawn + Offsets::m_vOldOrigin);
        team = mem.Read<int>(pawn + Offsets::m_iTeamNum);
    }
};

inline LocalPlayer localPlayer;