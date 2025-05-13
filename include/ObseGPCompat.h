#pragma once

// Include our Windows wrapper first
#include "WindowsWrapper.h"

// Standard includes
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>

namespace ObseGPCompat
{
    // Version information
    constexpr const char *VERSION = "0.2.0";

    // Logging levels
    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error
    };

    // Forward declarations
    class PathTranslator;
    class APIHookManager;
    class VirtualFileSystem;
    class ConfigurationManager;

    // Global variables - simplified to focus only on GamePass
    extern std::filesystem::path g_GamePassInstallPath;
    extern std::filesystem::path g_ObsePath; // Path where OBSE64 is installed (same dir as game exe)
    extern std::filesystem::path g_CompatLayerPath;

    // Global components
    extern std::unique_ptr<PathTranslator> g_PathTranslator;
    extern std::unique_ptr<APIHookManager> g_APIHookManager;
    extern std::unique_ptr<VirtualFileSystem> g_VirtualFileSystem;
    extern std::unique_ptr<ConfigurationManager> g_ConfigurationManager;

    // Core functions
    bool Initialize();
    void Shutdown();
    void Log(LogLevel level, const char *format, ...);

    // Helper function
    std::filesystem::path GetLocalAppDataPath();
}