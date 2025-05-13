#include "ProxyLauncher.h"
#include <Windows.h>
#include <winreg.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <iostream>

// For detecting Windows Store apps
#include <wrl.h>
#include <wrl/client.h>
#include <wrl/event.h>
#include <wrl/wrappers/corewrappers.h>
#include <roapi.h>
#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <shobjidl.h>

namespace ObseGPCompat {

    ProxyLauncher::ProxyLauncher() 
        : m_ProcessInfo({0}) {
        // Constructor
    }

    ProxyLauncher::~ProxyLauncher() {
        // Clean up process handles if needed
        if (m_ProcessInfo.hProcess) {
            CloseHandle(m_ProcessInfo.hProcess);
        }
        if (m_ProcessInfo.hThread) {
            CloseHandle(m_ProcessInfo.hThread);
        }
    }

    bool ProxyLauncher::Initialize() {
        Log(LogLevel::Info, "Initializing ProxyLauncher");
        
        // Find Game Pass installation
        if (!FindGamePassInstallation()) {
            Log(LogLevel::Error, "Failed to find Game Pass installation");
            return false;
        }
        
        // Find OBSE64 installation
        if (!FindObse64Installation()) {
            Log(LogLevel::Error, "Failed to find OBSE64 installation");
            return false;
        }
        
        Log(LogLevel::Info, "ProxyLauncher initialized successfully");
        return true;
    }
    
    bool ProxyLauncher::FindGamePassInstallation() {
    // First check if set in configuration
    if (!g_GamePassInstallPath.empty() && std::filesystem::exists(g_GamePassInstallPath)) {
        Log(LogLevel::Info, "Using Game Pass installation from configuration: %s", 
            g_GamePassInstallPath.string().c_str());
        return true;
    }
    
    // Get available drive letters
    char drives[MAX_PATH];
    if (GetLogicalDriveStringsA(MAX_PATH, drives)) {
        char* drive = drives;
        while (*drive) {
            std::string driveLetter = drive;
            
            // Check ModifiableWindowsApps location
            std::filesystem::path modifiablePath = driveLetter + "Program Files\\ModifiableWindowsApps\\The Elder Scrolls IV- Oblivion Remastered";
            if (std::filesystem::exists(modifiablePath)) {
                Log(LogLevel::Info, "Found Game Pass installation at: %s", modifiablePath.string().c_str());
                g_GamePassInstallPath = modifiablePath;
                return true;
            }
            
            // Check XboxGames location
            std::filesystem::path xboxGamesPath = driveLetter + "XboxGames\\The Elder Scrolls IV- Oblivion Remastered";
            if (std::filesystem::exists(xboxGamesPath)) {
                Log(LogLevel::Info, "Found Game Pass installation at: %s", xboxGamesPath.string().c_str());
                g_GamePassInstallPath = xboxGamesPath;
                return true;
            }
            
            // Move to next drive letter
            drive += strlen(drive) + 1;
        }
    }
    
    // Not found in common locations, try registry detection
    // (Registry detection code here)
    
    Log(LogLevel::Warning, "Game Pass installation not found automatically. Please set it manually in the configuration.");
    return false;
}
    
    bool ProxyLauncher::FindObse64Installation() {
        // First try Steam installation location
        std::filesystem::path steamPath = "C:\\Program Files\\Steam\\steamapps\\common\\Oblivion Remastered";
        
        if (std::filesystem::exists(steamPath)) {
            Log(LogLevel::Info, "Found Steam installation at: %s", steamPath.string().c_str());
            g_SteamInstallPath = steamPath;
            
            // Check for OBSE64 loader
            std::filesystem::path obseLoaderPath = steamPath / "OblivionRemastered" / "Binaries" / "Win64" / "obse64_loader.exe";
            if (std::filesystem::exists(obseLoaderPath)) {
                Log(LogLevel::Info, "Found OBSE64 loader at: %s", obseLoaderPath.string().c_str());
                return true;
            }
        }
        
        // Not found in common location, check if we have it in configuration
        if (!g_SteamInstallPath.empty() && std::filesystem::exists(g_SteamInstallPath)) {
            Log(LogLevel::Info, "Using Steam installation from configuration: %s", 
                g_SteamInstallPath.string().c_str());
            
            // Check for OBSE64 loader
            std::filesystem::path obseLoaderPath = g_SteamInstallPath / "OblivionRemastered" / "Binaries" / "Win64" / "obse64_loader.exe";
            if (std::filesystem::exists(obseLoaderPath)) {
                Log(LogLevel::Info, "Found OBSE64 loader at: %s", obseLoaderPath.string().c_str());
                return true;
            }
        }
        
        Log(LogLevel::Warning, "OBSE64 installation not found");
        return false;
    }
    
    bool ProxyLauncher::IsObse64Installed() {
        std::filesystem::path obseLoaderPath = g_SteamInstallPath / "OblivionRemastered" / "Binaries" / "Win64" / "obse64_loader.exe";
        return std::filesystem::exists(obseLoaderPath);
    }
    
    bool ProxyLauncher::IsGamePassVersionInstalled() {
        return !g_GamePassInstallPath.empty() && std::filesystem::exists(g_GamePassInstallPath);
    }
    
    std::filesystem::path ProxyLauncher::GetGamePassExecutablePath() {
        return g_GamePassInstallPath / "Content" / "OblivionRemastered" / "Binaries" / "Win64" / "OblivionRemastered-Win64-Shipping.exe";
    }
    
    std::filesystem::path ProxyLauncher::GetObse64LoaderPath() {
        return g_SteamInstallPath / "OblivionRemastered" / "Binaries" / "Win64" / "obse64_loader.exe";
    }

    bool ProxyLauncher::Launch() {
        Log(LogLevel::Info, "Launching game with compatibility layer");
        
        // Get Game Pass executable path
        std::filesystem::path exePath = GetGamePassExecutablePath();
        
        if (!std::filesystem::exists(exePath)) {
            Log(LogLevel::Error, "Game executable not found at: %s", exePath.string().c_str());
            return false;
        }
        
        // Launch the process suspended
        if (!LaunchProcessSuspended(exePath, "")) {
            Log(LogLevel::Error, "Failed to launch game process");
            return false;
        }
        
        // Inject our compatibility layer DLL
        if (!InjectCompatibilityLayer(m_ProcessInfo.hProcess)) {
            Log(LogLevel::Error, "Failed to inject compatibility layer");
            // Terminate the process since injection failed
            TerminateProcess(m_ProcessInfo.hProcess, 1);
            return false;
        }
        
        // Resume the process
        if (!ResumeProcess()) {
            Log(LogLevel::Error, "Failed to resume game process");
            return false;
        }
        
        Log(LogLevel::Info, "Game launched successfully with compatibility layer");
        return true;
    }
    
    bool ProxyLauncher::LaunchProcessSuspended(const std::filesystem::path& executablePath, const std::string& commandLine) {
        Log(LogLevel::Info, "Launching process suspended: %s", executablePath.string().c_str());
        
        // Prepare command line
        std::string cmdLine = executablePath.string() + " " + commandLine;
        
        // Create process with suspended flag
        STARTUPINFOA startupInfo = { sizeof(STARTUPINFOA) };
        
        if (!CreateProcessA(
            NULL,                               // Application name (NULL means use command line)
            const_cast<LPSTR>(cmdLine.c_str()), // Command line
            NULL,                               // Process security attributes
            NULL,                               // Thread security attributes
            FALSE,                              // Inherit handles
            CREATE_SUSPENDED,                   // Creation flags - create suspended
            NULL,                               // Environment
            executablePath.parent_path().string().c_str(), // Current directory
            &startupInfo,                       // Startup info
            &m_ProcessInfo                      // Process information
        )) {
            DWORD error = GetLastError();
            Log(LogLevel::Error, "CreateProcess failed with error: %d", error);
            return false;
        }
        
        Log(LogLevel::Info, "Process created with PID: %d", m_ProcessInfo.dwProcessId);
        return true;
    }
    
    bool ProxyLauncher::ResumeProcess() {
        if (m_ProcessInfo.hThread) {
            Log(LogLevel::Info, "Resuming process");
            
            // Resume the main thread
            DWORD result = ResumeThread(m_ProcessInfo.hThread);
            if (result == -1) {
                DWORD error = GetLastError();
                Log(LogLevel::Error, "ResumeThread failed with error: %d", error);
                return false;
            }
            
            Log(LogLevel::Info, "Process resumed successfully");
            return true;
        }
        
        Log(LogLevel::Error, "Invalid thread handle");
        return false;
    }
    
    bool ProxyLauncher::InjectCompatibilityLayer(HANDLE processHandle) {
        Log(LogLevel::Info, "Injecting compatibility layer DLL");
        
        // Get path to our DLL
        std::filesystem::path dllPath = g_CompatLayerPath / "OBSE64GP.dll";
        
        if (!std::filesystem::exists(dllPath)) {
            Log(LogLevel::Error, "Compatibility layer DLL not found at: %s", dllPath.string().c_str());
            return false;
        }
        
        // Allocate memory in remote process for DLL path
        size_t pathSize = dllPath.string().length() + 1;
        LPVOID remotePath = VirtualAllocEx(
            processHandle, 
            NULL, 
            pathSize, 
            MEM_COMMIT, 
            PAGE_READWRITE
        );
        
        if (!remotePath) {
            DWORD error = GetLastError();
            Log(LogLevel::Error, "VirtualAllocEx failed with error: %d", error);
            return false;
        }
        
        // Write DLL path to remote process
        if (!WriteProcessMemory(
            processHandle,
            remotePath,
            dllPath.string().c_str(),
            pathSize,
            NULL
        )) {
            DWORD error = GetLastError();
            Log(LogLevel::Error, "WriteProcessMemory failed with error: %d", error);
            VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
            return false;
        }
        
        // Get address of LoadLibraryA
        FARPROC loadLibraryAddr = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
        if (!loadLibraryAddr) {
            DWORD error = GetLastError();
            Log(LogLevel::Error, "GetProcAddress failed with error: %d", error);
            VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
            return false;
        }
        
        // Create remote thread to load our DLL
        HANDLE remoteThread = CreateRemoteThread(
            processHandle,
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)loadLibraryAddr,
            remotePath,
            0,
            NULL
        );
        
        if (!remoteThread) {
            DWORD error = GetLastError();
            Log(LogLevel::Error, "CreateRemoteThread failed with error: %d", error);
            VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
            return false;
        }
        
        // Wait for thread to complete
        WaitForSingleObject(remoteThread, INFINITE);
        
        // Check if DLL was loaded successfully
        DWORD exitCode;
        GetExitCodeThread(remoteThread, &exitCode);
        
        // Clean up
        CloseHandle(remoteThread);
        VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
        
        if (!exitCode) {
            Log(LogLevel::Error, "DLL injection failed - LoadLibraryA returned 0");
            return false;
        }
        
        Log(LogLevel::Info, "Compatibility layer DLL injected successfully");
        return true;
    }

} // namespace ObseGPCompat