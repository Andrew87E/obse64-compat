#include "APIHookManager.h"
#include "PathTranslator.h"
#include <detours.h>
#pragma comment(lib, "detours.lib")

// Windows API functions we need to hook
#include <Windows.h>
#include <ShlObj.h>  // For SHGetFolderPath and related functions
#include <Shlwapi.h> // For path functions

namespace ObseGPCompat {

    // Original API function pointers
    static HANDLE(WINAPI* OriginalCreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE) = CreateFileW;
    static HANDLE(WINAPI* OriginalCreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE) = CreateFileA;
    static BOOL(WINAPI* OriginalReadFile)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED) = ReadFile;
    static BOOL(WINAPI* OriginalWriteFile)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED) = WriteFile;
    static BOOL(WINAPI* OriginalCloseHandle)(HANDLE) = CloseHandle;
    static BOOL(WINAPI* OriginalGetFileAttributesExW)(LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID) = GetFileAttributesExW;
    static DWORD(WINAPI* OriginalGetFileAttributesW)(LPCWSTR) = GetFileAttributesW;
    static DWORD(WINAPI* OriginalGetFileAttributesA)(LPCSTR) = GetFileAttributesA;
    static BOOL(WINAPI* OriginalCreateDirectoryW)(LPCWSTR, LPSECURITY_ATTRIBUTES) = CreateDirectoryW;
    static BOOL(WINAPI* OriginalCreateDirectoryA)(LPCSTR, LPSECURITY_ATTRIBUTES) = CreateDirectoryA;
    static BOOL(WINAPI* OriginalDeleteFileW)(LPCWSTR) = DeleteFileW;
    static BOOL(WINAPI* OriginalDeleteFileA)(LPCSTR) = DeleteFileA;
    static BOOL(WINAPI* OriginalMoveFileW)(LPCWSTR, LPCWSTR) = MoveFileW;
    static BOOL(WINAPI* OriginalMoveFileA)(LPCSTR, LPCSTR) = MoveFileA;
    static BOOL(WINAPI* OriginalCopyFileW)(LPCWSTR, LPCWSTR, BOOL) = CopyFileW;
    static BOOL(WINAPI* OriginalCopyFileA)(LPCSTR, LPCSTR, BOOL) = CopyFileA;
    static HMODULE(WINAPI* OriginalLoadLibraryA)(LPCSTR) = LoadLibraryA;
    static HMODULE(WINAPI* OriginalLoadLibraryW)(LPCWSTR) = LoadLibraryW;
    static HMODULE(WINAPI* OriginalLoadLibraryExA)(LPCSTR, HANDLE, DWORD) = LoadLibraryExA;
    static HMODULE(WINAPI* OriginalLoadLibraryExW)(LPCWSTR, HANDLE, DWORD) = LoadLibraryExW;

    // API hook implementations
    HANDLE WINAPI HookedCreateFileW(
        LPCWSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile
    ) {
        // Convert wide string to narrow for logging
        char narrowPath[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, narrowPath, MAX_PATH, NULL, NULL);
        
        // Only handle paths related to Oblivion
        if (strstr(narrowPath, "Oblivion") != nullptr) {
            Log(LogLevel::Debug, "CreateFileW called for: %s", narrowPath);
            
            // Convert path from Steam to Game Pass if necessary
            std::filesystem::path filePath(lpFileName);
            if (g_PathTranslator->IsSteamPath(filePath)) {
                std::filesystem::path gamePassPath = g_PathTranslator->SteamToGamePass(filePath);
                
                // Convert back to wide string
                wchar_t wideGamePassPath[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, gamePassPath.string().c_str(), -1, wideGamePassPath, MAX_PATH);
                
                Log(LogLevel::Debug, "Redirecting CreateFileW to: %s", gamePassPath.string().c_str());
                
                // Call original function with translated path
                return OriginalCreateFileW(
                    wideGamePassPath,
                    dwDesiredAccess,
                    dwShareMode,
                    lpSecurityAttributes,
                    dwCreationDisposition,
                    dwFlagsAndAttributes,
                    hTemplateFile
                );
            }
        }
        
        // Pass through to original function for unmodified paths
        return OriginalCreateFileW(
            lpFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile
        );
    }

    HANDLE WINAPI HookedCreateFileA(
        LPCSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile
    ) {
        // Only handle paths related to Oblivion
        if (strstr(lpFileName, "Oblivion") != nullptr) {
            Log(LogLevel::Debug, "CreateFileA called for: %s", lpFileName);
            
            // Convert path from Steam to Game Pass if necessary
            std::filesystem::path filePath(lpFileName);
            if (g_PathTranslator->IsSteamPath(filePath)) {
                std::filesystem::path gamePassPath = g_PathTranslator->SteamToGamePass(filePath);
                
                Log(LogLevel::Debug, "Redirecting CreateFileA to: %s", gamePassPath.string().c_str());
                
                // Call original function with translated path
                return OriginalCreateFileA(
                    gamePassPath.string().c_str(),
                    dwDesiredAccess,
                    dwShareMode,
                    lpSecurityAttributes,
                    dwCreationDisposition,
                    dwFlagsAndAttributes,
                    hTemplateFile
                );
            }
        }
        
        // Pass through to original function for unmodified paths
        return OriginalCreateFileA(
            lpFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile
        );
    }

    HMODULE WINAPI HookedLoadLibraryA(LPCSTR lpLibFileName) {
        // Only handle paths related to OBSE
        if (strstr(lpLibFileName, "obse") != nullptr || strstr(lpLibFileName, "OBSE") != nullptr) {
            Log(LogLevel::Debug, "LoadLibraryA called for: %s", lpLibFileName);
            
            // Convert path from Steam to Game Pass if necessary
            std::filesystem::path libPath(lpLibFileName);
            if (g_PathTranslator->IsSteamPath(libPath)) {
                std::filesystem::path gamePassPath = g_PathTranslator->SteamToGamePass(libPath);
                
                Log(LogLevel::Debug, "Redirecting LoadLibraryA to: %s", gamePassPath.string().c_str());
                
                // Call original function with translated path
                return OriginalLoadLibraryA(gamePassPath.string().c_str());
            }
        }
        
        // Pass through to original function for unmodified paths
        return OriginalLoadLibraryA(lpLibFileName);
    }

    HMODULE WINAPI HookedLoadLibraryW(LPCWSTR lpLibFileName) {
        // Convert wide string to narrow for logging and checking
        char narrowPath[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, lpLibFileName, -1, narrowPath, MAX_PATH, NULL, NULL);
        
        // Only handle paths related to OBSE
        if (strstr(narrowPath, "obse") != nullptr || strstr(narrowPath, "OBSE") != nullptr) {
            Log(LogLevel::Debug, "LoadLibraryW called for: %s", narrowPath);
            
            // Convert path from Steam to Game Pass if necessary
            std::filesystem::path libPath(lpLibFileName);
            if (g_PathTranslator->IsSteamPath(libPath)) {
                std::filesystem::path gamePassPath = g_PathTranslator->SteamToGamePass(libPath);
                
                // Convert back to wide string
                wchar_t wideGamePassPath[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, gamePassPath.string().c_str(), -1, wideGamePassPath, MAX_PATH);
                
                Log(LogLevel::Debug, "Redirecting LoadLibraryW to: %s", gamePassPath.string().c_str());
                
                // Call original function with translated path
                return OriginalLoadLibraryW(wideGamePassPath);
            }
        }
        
        // Pass through to original function for unmodified paths
        return OriginalLoadLibraryW(lpLibFileName);
    }

    APIHookManager::APIHookManager() {
        // Constructor
    }

    APIHookManager::~APIHookManager() {
        // Make sure all hooks are removed
        Shutdown();
    }

    bool APIHookManager::Initialize() {
        Log(LogLevel::Info, "Initializing APIHookManager");
        
        // Start transaction for Microsoft Detours
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        
        // Attach hooks for file operations
        DetourAttach(&(PVOID&)OriginalCreateFileW, HookedCreateFileW);
        DetourAttach(&(PVOID&)OriginalCreateFileA, HookedCreateFileA);
        DetourAttach(&(PVOID&)OriginalLoadLibraryA, HookedLoadLibraryA);
        DetourAttach(&(PVOID&)OriginalLoadLibraryW, HookedLoadLibraryW);
        
        // Add hook info to our list
        m_Hooks.push_back({ (void*)OriginalCreateFileW, (void*)HookedCreateFileW, "kernel32.dll", "CreateFileW" });
        m_Hooks.push_back({ (void*)OriginalCreateFileA, (void*)HookedCreateFileA, "kernel32.dll", "CreateFileA" });
        m_Hooks.push_back({ (void*)OriginalLoadLibraryA, (void*)HookedLoadLibraryA, "kernel32.dll", "LoadLibraryA" });
        m_Hooks.push_back({ (void*)OriginalLoadLibraryW, (void*)HookedLoadLibraryW, "kernel32.dll", "LoadLibraryW" });
        
        // Commit transaction
        LONG result = DetourTransactionCommit();
        if (result != NO_ERROR) {
            Log(LogLevel::Error, "Failed to install API hooks: %ld", result);
            return false;
        }
        
        Log(LogLevel::Info, "API hooks installed successfully");
        return true;
    }

    void APIHookManager::Shutdown() {
        Log(LogLevel::Info, "Removing API hooks");
        
        // Start transaction for Microsoft Detours
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        
        // Detach all hooks
        for (const auto& hookInfo : m_Hooks) {
            DetourDetach((PVOID*)&hookInfo.original, hookInfo.hooked);
        }
        
        // Commit transaction
        LONG result = DetourTransactionCommit();
        if (result != NO_ERROR) {
            Log(LogLevel::Error, "Failed to remove API hooks: %ld", result);
        } else {
            Log(LogLevel::Info, "API hooks removed successfully");
        }
        
        // Clear hook list
        m_Hooks.clear();
    }

    bool APIHookManager::AddHook(const char* moduleName, const char* functionName, void* hookFunction, void** originalFunction) {
        Log(LogLevel::Info, "Adding hook for %s::%s", moduleName, functionName);
        
        // Get module handle
        HMODULE moduleHandle = GetModuleHandleA(moduleName);
        if (!moduleHandle) {
            // Try loading the module if it's not already loaded
            moduleHandle = LoadLibraryA(moduleName);
            if (!moduleHandle) {
                Log(LogLevel::Error, "Failed to get module handle for %s", moduleName);
                return false;
            }
        }
        
        // Get function address
        void* functionAddress = GetProcAddress(moduleHandle, functionName);
        if (!functionAddress) {
            Log(LogLevel::Error, "Failed to get address for %s::%s", moduleName, functionName);
            return false;
        }
        
        // Store original function pointer
        *originalFunction = functionAddress;
        
        // Start transaction for Microsoft Detours
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        
        // Attach hook
        DetourAttach(originalFunction, hookFunction);
        
        // Commit transaction
        LONG result = DetourTransactionCommit();
        if (result != NO_ERROR) {
            Log(LogLevel::Error, "Failed to install API hook for %s::%s: %ld", moduleName, functionName, result);
            return false;
        }
        
        // Add hook info to our list
        m_Hooks.push_back({ *originalFunction, hookFunction, moduleName, functionName });
        
        Log(LogLevel::Info, "Hook added successfully for %s::%s", moduleName, functionName);
        return true;
    }

    bool APIHookManager::RemoveHook(void* hookFunction) {
        Log(LogLevel::Info, "Removing specific hook");
        
        // Find hook in our list
        auto it = std::find_if(m_Hooks.begin(), m_Hooks.end(), 
            [hookFunction](const HookInfo& hookInfo) { return hookInfo.hooked == hookFunction; });
        
        if (it == m_Hooks.end()) {
            Log(LogLevel::Error, "Hook not found");
            return false;
        }
        
        // Start transaction for Microsoft Detours
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        
        // Detach hook
        DetourDetach(&it->original, it->hooked);
        
        // Commit transaction
        LONG result = DetourTransactionCommit();
        if (result != NO_ERROR) {
            Log(LogLevel::Error, "Failed to remove API hook: %ld", result);
            return false;
        }
        
        // Remove from our list
        m_Hooks.erase(it);
        
        Log(LogLevel::Info, "Hook removed successfully");
        return true;
    }

} // namespace ObseGPCompat