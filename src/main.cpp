#include "ObseGPCompat.h"
#include "PathTranslator.h"
#include "APIHookManager.h"
#include "VirtualFileSystem.h"
#include "ConfigurationManager.h"
#include "ProxyLauncher.h"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <shlobj.h>
#include <filesystem>
#include <stdarg.h>

namespace ObseGPCompat
{
    // Global variables
    std::filesystem::path g_GamePassInstallPath;
    std::filesystem::path g_ObsePath;
    std::filesystem::path g_CompatLayerPath;

    // Global components
    std::unique_ptr<PathTranslator> g_PathTranslator;
    std::unique_ptr<APIHookManager> g_APIHookManager;
    std::unique_ptr<VirtualFileSystem> g_VirtualFileSystem;
    std::unique_ptr<ConfigurationManager> g_ConfigurationManager;

    // Log file handle
    static std::ofstream g_LogFile;

    // Helper function to get Local AppData path
    std::filesystem::path GetLocalAppDataPath()
    {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
        {
            return std::filesystem::path(path);
        }
        return std::filesystem::path();
    }

    // Initialize the compatibility layer
    bool Initialize()
    {
        // Create log directory
        std::filesystem::path logPath = GetLocalAppDataPath() / "OBSE64GP" / "Logs";
        std::filesystem::create_directories(logPath);

        // Open log file
        g_LogFile.open((logPath / "compat_layer.log").string());
        if (!g_LogFile.is_open())
        {
            std::cerr << "Failed to open log file" << std::endl;
            return false;
        }

        Log(LogLevel::Info, "OBSE64GP Compatibility Layer v%s - Initializing", VERSION);

        // Initialize component instances
        g_ConfigurationManager = std::make_unique<ConfigurationManager>();
        if (!g_ConfigurationManager->Initialize())
        {
            Log(LogLevel::Error, "Failed to initialize ConfigurationManager");
            return false;
        }

        // Load paths from configuration
        g_GamePassInstallPath = g_ConfigurationManager->GetString("Paths", "GamePassInstall", "");

        // Set CompatLayerPath to current executable directory
        g_CompatLayerPath = std::filesystem::current_path();

        // Initialize other components
        g_PathTranslator = std::make_unique<PathTranslator>();
        if (!g_PathTranslator->Initialize())
        {
            Log(LogLevel::Error, "Failed to initialize PathTranslator");
            return false;
        }

        g_VirtualFileSystem = std::make_unique<VirtualFileSystem>();
        if (!g_VirtualFileSystem->Initialize())
        {
            Log(LogLevel::Error, "Failed to initialize VirtualFileSystem");
            return false;
        }

        g_APIHookManager = std::make_unique<APIHookManager>();
        if (!g_APIHookManager->Initialize())
        {
            Log(LogLevel::Error, "Failed to initialize APIHookManager");
            return false;
        }

        Log(LogLevel::Info, "Compatibility layer initialized successfully");
        return true;
    }

    // Shutdown the compatibility layer
    void Shutdown()
    {
        Log(LogLevel::Info, "Shutting down compatibility layer");

        // Shutdown components in reverse order
        g_APIHookManager.reset();
        g_VirtualFileSystem.reset();
        g_PathTranslator.reset();
        g_ConfigurationManager.reset();

        // Close log file
        g_LogFile.close();
    }

    // Log a message
    void Log(LogLevel level, const char *format, ...)
    {
        static const char *levelStrings[] = {
            "DEBUG",
            "INFO",
            "WARNING",
            "ERROR"};

        // Format time
        SYSTEMTIME st;
        GetLocalTime(&st);
        char timeStr[20];
        sprintf_s(timeStr, sizeof(timeStr), "%02d:%02d:%02d.%03d",
                  st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

        // Format message
        char buffer[4096];
        va_list args;
        va_start(args, format);
        vsprintf_s(buffer, sizeof(buffer), format, args);
        va_end(args);

        // Write to log file
        if (g_LogFile.is_open())
        {
            g_LogFile << timeStr << " [" << levelStrings[static_cast<int>(level)] << "] " << buffer << std::endl;
            g_LogFile.flush();
        }

        // Also write to console if available
        printf("%s [%s] %s\n", timeStr, levelStrings[static_cast<int>(level)], buffer);
    }

} // namespace ObseGPCompat

// Main entry point
int main(int argc, char *argv[])
{
    // Initialize compatibility layer
    if (!ObseGPCompat::Initialize())
    {
        std::cerr << "Failed to initialize compatibility layer" << std::endl;
        return 1;
    }

    // Create launcher
    ObseGPCompat::ProxyLauncher launcher;
    if (!launcher.Initialize())
    {
        ObseGPCompat::Log(ObseGPCompat::LogLevel::Error, "Failed to initialize launcher");
        ObseGPCompat::Shutdown();
        return 1;
    }

    // Check if OBSE64 is installed
    if (!launcher.IsObse64Installed())
    {
        ObseGPCompat::Log(ObseGPCompat::LogLevel::Error, "OBSE64 is not installed or could not be found");
        ObseGPCompat::Shutdown();
        return 1;
    }

    // Check if Game Pass version is installed
    if (!launcher.IsGamePassVersionInstalled())
    {
        ObseGPCompat::Log(ObseGPCompat::LogLevel::Error, "Game Pass version is not installed or could not be found");
        ObseGPCompat::Shutdown();
        return 1;
    }

    // Launch the game
    if (!launcher.Launch())
    {
        ObseGPCompat::Log(ObseGPCompat::LogLevel::Error, "Failed to launch the game");
        ObseGPCompat::Shutdown();
        return 1;
    }

    ObseGPCompat::Log(ObseGPCompat::LogLevel::Info, "Game launched successfully");
    ObseGPCompat::Shutdown();
    return 0;
}