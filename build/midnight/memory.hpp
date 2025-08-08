#pragma once
#include <Windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <algorithm>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "kernel32.lib")

class Memory {
private:
    HANDLE ProcessHandle = nullptr;
    DWORD ProcessID = 0;

    bool iequals(const std::wstring& a, const std::wstring& b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(),
            [](wchar_t a, wchar_t b) { return towlower(a) == towlower(b); });
    }

public:
    ~Memory() {
        if (ProcessHandle) {
            CloseHandle(ProcessHandle);
        }
    }

    bool Attach(DWORD pid) {
        ProcessID = pid;
        ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        return ProcessHandle != nullptr;
    }

    template <typename T>
    T Read(uintptr_t address) {
        T buffer{};
        if (!ProcessHandle) return buffer;
        
        SIZE_T bytesRead;
        ReadProcessMemory(ProcessHandle, reinterpret_cast<LPCVOID>(address), 
                        &buffer, sizeof(T), &bytesRead);
        return buffer;
    }

    bool ReadArray(uintptr_t address, void* buffer, size_t size) {
        if (!ProcessHandle) return false;
        
        SIZE_T bytesRead;
        return ReadProcessMemory(ProcessHandle, reinterpret_cast<LPCVOID>(address),
                              buffer, size, &bytesRead);
    }

    template <typename T>
    bool Write(uintptr_t address, const T& value) {
        if (!ProcessHandle) return false;
        
        SIZE_T bytesWritten;
        return WriteProcessMemory(ProcessHandle, reinterpret_cast<LPVOID>(address),
                                &value, sizeof(T), &bytesWritten);
    }

    DWORD GetProcessID(const std::wstring& processName) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return 0;
        
        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (iequals(pe32.szExeFile, processName)) {
                    CloseHandle(hSnapshot);
                    return pe32.th32ProcessID;
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        
        CloseHandle(hSnapshot);
        return 0;
    }

    uintptr_t GetModuleBase(const std::wstring& moduleName) {
        if (!ProcessHandle) return 0;
        
        HMODULE hMods[1024];
        DWORD cbNeeded;
        
        if (EnumProcessModules(ProcessHandle, hMods, sizeof(hMods), &cbNeeded)) {
            for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                wchar_t szModName[MAX_PATH];
                if (GetModuleFileNameExW(ProcessHandle, hMods[i], szModName, MAX_PATH)) {
                    if (iequals(szModName, moduleName)) {
                        return reinterpret_cast<uintptr_t>(hMods[i]);
                    }
                }
            }
        }
        return 0;
    }
};

inline Memory mem;