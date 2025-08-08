#pragma once
#include "Globals.hpp"

class GameState {
public:
    view_matrix_t viewMatrix;
    uintptr_t entityList = 0;

    void Update() {
        entityList = mem.Read<uintptr_t>(CS2::ClientDll + Offsets::dwEntityList);
        viewMatrix = mem.Read<view_matrix_t>(CS2::ClientDll + Offsets::dwViewMatrix);
    }
};

inline GameState game;