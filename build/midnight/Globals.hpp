#pragma once
#include "Memory.hpp"

#define SCREEN_WIDTH GetSystemMetrics(SM_CXSCREEN)
#define SCREEN_HEIGHT GetSystemMetrics(SM_CYSCREEN)
#define GAME_METRIC_UNITS 40.0f

namespace CS2 {
    inline DWORD ProcID = 0;
    inline uintptr_t ClientDll = 0;
    inline uintptr_t Engine2Dll = 0;
    
    inline bool Initialize() {
        ProcID = mem.GetProcessID(L"cs2.exe");
        if (ProcID) {
            ClientDll = mem.GetModuleBase(L"client.dll");
            Engine2Dll = mem.GetModuleBase(L"engine2.dll");
            return ClientDll && Engine2Dll;
        }
        return false;
    }
}

namespace Offsets {
    inline constexpr uintptr_t dwEntityList = 0x1CBE620;
    inline constexpr uintptr_t dwLocalPlayerPawn = 0x1AF4B80;
    inline constexpr uintptr_t dwLocalPlayerController = 0x1D10240;
    inline constexpr uintptr_t m_sSanitizedPlayerName = 0x850;
    inline constexpr uintptr_t m_iTeamNum = 0x3EB;
    inline constexpr uintptr_t m_iHealth = 0x34C;
    inline constexpr uintptr_t m_vOldOrigin = 0x15B0;
    inline constexpr uintptr_t dwViewMatrix = 0x1D21A00;
}