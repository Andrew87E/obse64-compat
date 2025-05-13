#include "PathTranslator.h"
#include <regex>
#include <shlobj.h>
#include <Windows.h>

namespace ObseGPCompat {

    PathTranslator::PathTranslator() {
        // Constructor
    }

    PathTranslator::~PathTranslator() {
        // Destructor
    }

    bool PathTranslator::Initialize() {
        Log(LogLevel::Info, "Initializing PathTranslator");
        
        // Ensure paths are set
        if (g_GamePassInstallPath.empty()) {
            Log(LogLevel::Error, "Game Pass installation path is not set");
            return false;
        }
        
        if (g_SteamInstallPath.empty()) {
            Log(LogLevel::Error, "Steam installation path is not set");
            return false;
        }
        
        // Build path mappings
        BuildPathMappings();
        
        Log(LogLevel::Info, "PathTranslator initialized successfully");
        return true;
    }
    
    void PathTranslator::BuildPathMappings() {
        Log(LogLevel::Info, "Building path mappings");
        
        // Clear existing mappings
        m_SteamToGamePassPrefixes.clear();
        m_GamePassToSteamPrefixes.clear();
        
        // Typical Steam path structure:
        // C:\Program Files\Steam\steamapps\common\Oblivion Remastered\OblivionRemastered\Binaries\Win64
        std::string steamBase = g_SteamInstallPath.string();
        
        // Typical Game Pass path structure:
        // C:\XboxGames\The Elder Scrolls IV- Oblivion Remastered\Content\OblivionRemastered\Binaries\Win64
        // or 
        // C:\Program Files\ModifiableWindowsApps\The Elder Scrolls IV- Oblivion Remastered\Content\OblivionRemastered\Binaries\Win64
        std::string gamePassBase = g_GamePassInstallPath.string();
        
        // Main executable directory
        m_SteamToGamePassPrefixes[steamBase + "\\OblivionRemastered\\Binaries\\Win64"] = 
            gamePassBase + "\\OblivionRemastered\\Binaries\\Win64";
        m_GamePassToSteamPrefixes[gamePassBase + "\\OblivionRemastered\\Binaries\\Win64"] = 
            steamBase + "\\OblivionRemastered\\Binaries\\Win64";
            
        // Content directory
        m_SteamToGamePassPrefixes[steamBase + "\\OblivionRemastered\\Content"] = 
            gamePassBase + "\\OblivionRemastered\\Content";
        m_GamePassToSteamPrefixes[gamePassBase + "\\OblivionRemastered\\Content"] = 
            steamBase + "\\OblivionRemastered\\Content";
            
        // Data directory
        m_SteamToGamePassPrefixes[steamBase + "\\OblivionRemastered\\Content\\Dev\\ObvData\\data"] = 
            gamePassBase + "\\OblivionRemastered\\Content\\Dev\\ObvData\\data";
        m_GamePassToSteamPrefixes[gamePassBase + "\\OblivionRemastered\\Content\\Dev\\ObvData\\data"] = 
            steamBase + "\\OblivionRemastered\\Content\\Dev\\ObvData\\data";
            
        // Save directory (different location for Game Pass)
        char localAppData[MAX_PATH];
        SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData);
        
        std::string steamSavePath = std::string(localAppData) + "\\My Games\\Oblivion Remastered\\Saved\\SaveGames";
        std::string gamePassSavePath = std::string(localAppData) + 
            "\\Packages\\BethesdaSoftworks.ProjectAltar_3275kfvn8vcwc\\SystemAppData\\wgs";
            
        m_SteamToGamePassPrefixes[steamSavePath] = gamePassSavePath;
        m_GamePassToSteamPrefixes[gamePassSavePath] = steamSavePath;
        
        // OBSE64 directory for plugins
        m_SteamToGamePassPrefixes[steamBase + "\\OblivionRemastered\\Binaries\\Win64\\OBSE\\Plugins"] = 
            gamePassBase + "\\OblivionRemastered\\Binaries\\Win64\\OBSE\\Plugins";
        m_GamePassToSteamPrefixes[gamePassBase + "\\OblivionRemastered\\Binaries\\Win64\\OBSE\\Plugins"] = 
            steamBase + "\\OblivionRemastered\\Binaries\\Win64\\OBSE\\Plugins";
            
        // OBSE64 logs directory  
        m_SteamToGamePassPrefixes[std::string(localAppData) + "\\My Games\\Oblivion Remastered\\OBSE\\Logs"] = 
            std::string(localAppData) + "\\My Games\\Oblivion Remastered GP\\OBSE\\Logs";
        m_GamePassToSteamPrefixes[std::string(localAppData) + "\\My Games\\Oblivion Remastered GP\\OBSE\\Logs"] = 
            std::string(localAppData) + "\\My Games\\Oblivion Remastered\\OBSE\\Logs";
        
        // Log the mappings
        Log(LogLevel::Debug, "Path mappings created:");
        for (const auto& mapping : m_SteamToGamePassPrefixes) {
            Log(LogLevel::Debug, "  Steam -> Game Pass: '%s' -> '%s'", 
                mapping.first.c_str(), mapping.second.c_str());
        }
    }
    
    std::filesystem::path PathTranslator::SteamToGamePass(const std::filesystem::path& steamPath) {
        std::string pathStr = steamPath.string();
        
        // Check each prefix mapping
        for (const auto& mapping : m_SteamToGamePassPrefixes) {
            if (pathStr.find(mapping.first) == 0) {
                // Replace prefix
                std::string result = mapping.second + pathStr.substr(mapping.first.length());
                Log(LogLevel::Debug, "Translated Steam path '%s' to Game Pass path '%s'", 
                    pathStr.c_str(), result.c_str());
                return std::filesystem::path(result);
            }
        }
        
        // No mapping found, return original
        Log(LogLevel::Warning, "No mapping found for Steam path '%s'", pathStr.c_str());
        return steamPath;
    }
    
    std::filesystem::path PathTranslator::GamePassToSteam(const std::filesystem::path& gamePassPath) {
        std::string pathStr = gamePassPath.string();
        
        // Check each prefix mapping
        for (const auto& mapping : m_GamePassToSteamPrefixes) {
            if (pathStr.find(mapping.first) == 0) {
                // Replace prefix
                std::string result = mapping.second + pathStr.substr(mapping.first.length());
                Log(LogLevel::Debug, "Translated Game Pass path '%s' to Steam path '%s'", 
                    pathStr.c_str(), result.c_str());
                return std::filesystem::path(result);
            }
        }
        
        // No mapping found, return original
        Log(LogLevel::Warning, "No mapping found for Game Pass path '%s'", pathStr.c_str());
        return gamePassPath;
    }
    
    bool PathTranslator::IsSteamPath(const std::filesystem::path& path) {
        std::string pathStr = path.string();
        
        // Check if the path starts with any Steam prefix
        for (const auto& mapping : m_SteamToGamePassPrefixes) {
            if (pathStr.find(mapping.first) == 0) {
                return true;
            }
        }
        
        return false;
    }
    
    bool PathTranslator::IsGamePassPath(const std::filesystem::path& path) {
        std::string pathStr = path.string();
        
        // Check if the path starts with any Game Pass prefix
        for (const auto& mapping : m_GamePassToSteamPrefixes) {
            if (pathStr.find(mapping.first) == 0) {
                return true;
            }
        }
        
        return false;
    }

} // namespace ObseGPCompat