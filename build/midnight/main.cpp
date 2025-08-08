#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "MinHook/MinHook.h"
#include "Memory.hpp"
#include "LocalPlayer.hpp"
#include "ESP.hpp"
#include "Game.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Function pointers
typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(__stdcall* ResizeBuffers_t)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

static Present_t oPresent = nullptr;
static ResizeBuffers_t oResizeBuffers = nullptr;
static IDXGISwapChain* pSwapChain = nullptr;
static ID3D11Device* pDevice = nullptr;
static ID3D11DeviceContext* pContext = nullptr;
static HWND hWindow = nullptr;
static bool g_bInit = false;
static bool g_bInitSuccess = false;

// Hook function
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_bInit) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&pDevice)))) {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            hWindow = sd.OutputWindow;

            // Initialize ImGui
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            ImGui_ImplWin32_Init(hWindow);
            ImGui_ImplDX11_Init(pDevice, pContext);

            // Initialize game components
            g_bInitSuccess = CS2::Initialize();
            g_bInit = true;
        }
    }

    if (!g_bInitSuccess) {
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    // Start new ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Update game state and render ESP
    localPlayer.Update();
    game.Update();
    ESP::Render();

    // ImGui Menu
    static bool show_menu = true;
    
    // Simple menu toggle with INSERT key
    if (GetAsyncKeyState(VK_INSERT) & 1) {
        show_menu = !show_menu;
    }

    if (show_menu) {
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Midnight Menu", &show_menu);
        
        ImGui::Text("Midnight - CS2 External");
        ImGui::Separator();
        
        if (ImGui::CollapsingHeader("ESP")) {
            ImGui::Checkbox("Enable ESP", &ESP::enabled);
            ImGui::Checkbox("Show Names", &ESP::show_names);
            ImGui::Checkbox("Show Health", &ESP::show_health);
            ImGui::Checkbox("Show Distance", &ESP::show_distance);
        }
        
        if (ImGui::CollapsingHeader("Misc")) {
            ImGui::Text("Game Status: %s", CS2::ClientDll ? "Connected" : "Disconnected");
            ImGui::Text("Local Player: %s", localPlayer.pawn ? "Valid" : "Invalid");
        }
        
        ImGui::End();
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

// Function to find the correct swap chain
BOOL FindSwapChainAndHook() {
    HWND hwnd = FindWindowA("Valve001", NULL);
    if (!hwnd) {
        hwnd = GetForegroundWindow();
    }

    if (!hwnd) {
        return FALSE;
    }

    // Create a temporary device and swap chain to get the real one
    D3D_FEATURE_LEVEL featureLevel;
    DXGI_SWAP_CHAIN_DESC sd = { 0 };
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.Width = 800;
    sd.BufferDesc.Height = 600;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sd,
        &swapChain,
        &device,
        &featureLevel,
        &context
    );

    if (FAILED(hr)) {
        if (device) device->Release();
        if (context) context->Release();
        if (swapChain) swapChain->Release();
        return FALSE;
    }

    // Get the vtable
    void** pVTable = *reinterpret_cast<void***>(swapChain);
    
    // Hook Present
    if (MH_CreateHook(pVTable[8], &hkPresent, reinterpret_cast<void**>(&oPresent)) != MH_OK) {
        device->Release();
        context->Release();
        swapChain->Release();
        return FALSE;
    }

    device->Release();
    context->Release();
    swapChain->Release();
    return TRUE;
}

// Cleanup function
void Cleanup() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if (g_bInit) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    if (pContext) pContext->Release();
    if (pDevice) pDevice->Release();
    if (pSwapChain) pSwapChain->Release();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(nullptr, 0, [](LPVOID) -> DWORD {
            Sleep(5000); // Wait for game to initialize
            
            if (MH_Initialize() != MH_OK) {
                return FALSE;
            }

            if (!FindSwapChainAndHook()) {
                MH_Uninitialize();
                return FALSE;
            }

            if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
                MH_Uninitialize();
                return FALSE;
            }

            return TRUE;
        }, nullptr, 0, nullptr);
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
        Cleanup();
    }
    return TRUE;
}
