#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace ObseGPCompat
{

    class PathTranslator
    {
    public:
        PathTranslator();
        ~PathTranslator();

        bool Initialize();

        // Path translation methods (simplified to focus on GamePass directories)
        std::filesystem::path TranslateObsePath(const std::filesystem::path &path);
        bool IsObsePath(const std::filesystem::path &path);
        bool IsGamePath(const std::filesystem::path &path);

    private:
        void BuildPathMappings();

        // Maps for path translation - using string keys for compatibility
        std::map<std::string, std::string> m_ObseToGamePaths;
        std::map<std::string, std::string> m_GameToObsePaths;
    };

} // namespace ObseGPCompat