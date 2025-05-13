#pragma once

#include <windows.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <filesystem>
#include <shlobj.h>   // For SHGetFolderPath and related functions
#include <shlwapi.h>  // For path manipulation functions

namespace ObseGPCompat {

    // Version information
    constexpr const char* VERSION = "0.1.0";
    constexpr const char* COMPAT_LAYER_NAME = "OBSE64GP";
    
    // Path constants
    extern std::filesystem::path g_GamePassInstallPath;
    extern std::filesystem::path g_SteamInstallPath;
    extern std::filesystem::path g_CompatLayerPath;
    
    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Logging
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };
    
    void Log(LogLevel level, const char* format, ...);

    // Helper function to get Local AppData path
    std::filesystem::path GetLocalAppDataPath();

    // Forward declarations of main components
    class PathTranslator;
    class APIHookManager;
    class VirtualFileSystem;
    class ConfigurationManager;
    class ProxyLauncher;
    
    // Global access to components
    extern std::unique_ptr<PathTranslator> g_PathTranslator;
    extern std::unique_ptr<APIHookManager> g_APIHookManager;
    extern std::unique_ptr<VirtualFileSystem> g_VirtualFileSystem;
    extern std::unique_ptr<ConfigurationManager> g_ConfigurationManager;

} // namespace ObseGPCompat