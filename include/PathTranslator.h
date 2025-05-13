#pragma once

#include "ObseGPCompat.h"

namespace ObseGPCompat {

    class PathTranslator {
    public:
        PathTranslator();
        ~PathTranslator();
        
        // Convert a Steam path to Game Pass path
        std::filesystem::path SteamToGamePass(const std::filesystem::path& steamPath);
        
        // Convert a Game Pass path to Steam path
        std::filesystem::path GamePassToSteam(const std::filesystem::path& gamePassPath);
        
        // Check if a path is a Steam path
        bool IsSteamPath(const std::filesystem::path& path);
        
        // Check if a path is a Game Pass path
        bool IsGamePassPath(const std::filesystem::path& path);
        
        // Initialize path mappings
        bool Initialize();
        
    private:
        // Maps for path prefixes
        std::unordered_map<std::string, std::string> m_SteamToGamePassPrefixes;
        std::unordered_map<std::string, std::string> m_GamePassToSteamPrefixes;
        
        // Special path handling for known directories
        void BuildPathMappings();
    };

} // namespace ObseGPCompat
