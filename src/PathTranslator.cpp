#include "PathTranslator.h"
#include "ObseGPCompat.h"
#include <shlobj.h>
#include <Windows.h>

namespace ObseGPCompat
{

    PathTranslator::PathTranslator()
    {
        // Constructor
    }

    PathTranslator::~PathTranslator()
    {
        // Destructor
    }

    bool PathTranslator::Initialize()
    {
        Log(LogLevel::Info, "Initializing PathTranslator");

        // Ensure paths are set
        if (g_GamePassInstallPath.empty())
        {
            Log(LogLevel::Error, "Game Pass installation path is not set");
            return false;
        }

        if (g_ObsePath.empty())
        {
            Log(LogLevel::Error, "OBSE installation path is not set");
            return false;
        }

        // Build path mappings
        BuildPathMappings();

        Log(LogLevel::Info, "PathTranslator initialized successfully");
        return true;
    }

    void PathTranslator::BuildPathMappings()
    {
        Log(LogLevel::Info, "Building path mappings");

        // Clear existing mappings
        m_ObseToGamePaths.clear();
        m_GameToObsePaths.clear();

        // Game Pass path structure:
        // C:\XboxGames\The Elder Scrolls IV- Oblivion Remastered\Content\OblivionRemastered\Binaries\WinGDK
        // or
        // C:\Program Files\ModifiableWindowsApps\The Elder Scrolls IV- Oblivion Remastered\Content\OblivionRemastered\Binaries\WinGDK
        std::string gamePassBase = g_GamePassInstallPath.string();
        std::string obsePathStr = g_ObsePath.string();
        std::filesystem::path obsePath = g_ObsePath;

        // Main executable directory
        m_ObseToGamePaths[obsePathStr] =
            gamePassBase + "\\Content\\OblivionRemastered\\Binaries\\WinGDK";
        m_GameToObsePaths[gamePassBase + "\\Content\\OblivionRemastered\\Binaries\\WinGDK"] =
            obsePathStr;

        // Content directory
        m_ObseToGamePaths[obsePathStr + "\\Content"] =
            gamePassBase + "\\Content\\OblivionRemastered\\Content";
        m_GameToObsePaths[gamePassBase + "\\Content\\OblivionRemastered\\Content"] =
            obsePathStr + "\\Content";

        // Data directory
        m_ObseToGamePaths[obsePathStr + "\\Data"] =
            gamePassBase + "\\Content\\OblivionRemastered\\Content\\Dev\\ObvData\\data";
        m_GameToObsePaths[gamePassBase + "\\Content\\OblivionRemastered\\Content\\Dev\\ObvData\\data"] =
            obsePathStr + "\\Data";

        // OBSE64 directory for plugins
        std::filesystem::path obsePluginsPath = obsePath / "OBSE" / "Plugins";
        std::filesystem::path gamePluginsPath = g_GamePassInstallPath / "Content" / "OblivionRemastered" / "Binaries" / "Win64" / "OBSE" / "Plugins";

        // Create the plugins directory if it doesn't exist
        std::filesystem::create_directories(gamePluginsPath);

        m_ObseToGamePaths[obsePluginsPath.string()] = gamePluginsPath.string();
        m_GameToObsePaths[gamePluginsPath.string()] = obsePluginsPath.string();

        // OBSE64 logs directory
        char localAppData[MAX_PATH];
        SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData);

        std::filesystem::path obseLogsPath = g_ObsePath / "OBSE" / "Logs";
        std::filesystem::path gameLogsPath = std::filesystem::path(localAppData) / "My Games" / "Oblivion Remastered GP" / "OBSE" / "Logs";

        // Create the logs directory
        std::filesystem::create_directories(gameLogsPath);

        m_ObseToGamePaths[obseLogsPath.string()] = gameLogsPath.string();
        m_GameToObsePaths[gameLogsPath.string()] = obseLogsPath.string();

        // Log the mappings
        Log(LogLevel::Debug, "Path mappings created:");
        for (const auto &mapping : m_ObseToGamePaths)
        {
            Log(LogLevel::Debug, "  OBSE -> Game Pass: '%s' -> '%s'",
                mapping.first.c_str(), mapping.second.c_str());
        }
    }

    std::filesystem::path PathTranslator::TranslateObsePath(const std::filesystem::path &path)
    {
        std::string pathStr = path.string();

        // Check each prefix mapping
        for (const auto &mapping : m_ObseToGamePaths)
        {
            if (pathStr.find(mapping.first) == 0)
            {
                // Replace prefix
                std::string result = mapping.second + pathStr.substr(mapping.first.length());
                Log(LogLevel::Debug, "Translated OBSE path '%s' to Game Pass path '%s'",
                    pathStr.c_str(), result.c_str());
                return std::filesystem::path(result);
            }
        }

        // No mapping found, return original
        return path;
    }

    bool PathTranslator::IsObsePath(const std::filesystem::path &path)
    {
        std::string pathStr = path.string();

        // Check if the path starts with any OBSE prefix
        for (const auto &mapping : m_ObseToGamePaths)
        {
            if (pathStr.find(mapping.first) == 0)
            {
                return true;
            }
        }

        return false;
    }

    bool PathTranslator::IsGamePath(const std::filesystem::path &path)
    {
        std::string pathStr = path.string();

        // Check if the path starts with any Game Pass prefix
        for (const auto &mapping : m_GameToObsePaths)
        {
            if (pathStr.find(mapping.first) == 0)
            {
                return true;
            }
        }

        return false;
    }

} // namespace ObseGPCompat