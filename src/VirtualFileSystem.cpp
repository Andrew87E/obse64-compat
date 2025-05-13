#include "VirtualFileSystem.h"
#include <Windows.h>

namespace ObseGPCompat {

    VirtualFileSystem::VirtualFileSystem() {
        // Constructor
    }

    VirtualFileSystem::~VirtualFileSystem() {
        // Destructor
    }

    bool VirtualFileSystem::Initialize() {
        Log(LogLevel::Info, "Initializing VirtualFileSystem");
        
        // Clear existing mappings
        m_VirtualToRealPaths.clear();
        m_RealToVirtualPaths.clear();
        
        // Add default mappings based on path translator
        // These mappings will handle common OBSE64 paths
        
        // Map plugins directory
        std::filesystem::path steamPluginsPath = g_SteamInstallPath / "OblivionRemastered" / "Binaries" / "Win64" / "OBSE" / "Plugins";
        std::filesystem::path gamePassPluginsPath = g_GamePassInstallPath / "Content" / "OblivionRemastered" / "Binaries" / "Win64" / "OBSE" / "Plugins";
        
        MapPath(steamPluginsPath, gamePassPluginsPath);
        
        // Map data directory
        std::filesystem::path steamDataPath = g_SteamInstallPath / "OblivionRemastered" / "Content" / "Dev" / "ObvData" / "data";
        std::filesystem::path gamePassDataPath = g_GamePassInstallPath / "Content" / "OblivionRemastered" / "Content" / "Dev" / "ObvData" / "data";
        
        MapPath(steamDataPath, gamePassDataPath);
        
        // Map logs directory
        char localAppData[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData))) {
            std::filesystem::path steamLogsPath = std::filesystem::path(localAppData) / "My Games" / "Oblivion Remastered" / "OBSE" / "Logs";
            std::filesystem::path gamePassLogsPath = std::filesystem::path(localAppData) / "My Games" / "Oblivion Remastered GP" / "OBSE" / "Logs";
            
            MapPath(steamLogsPath, gamePassLogsPath);
        }
        
        // Ensure all mapped directories exist
        for (const auto& mapping : m_VirtualToRealPaths) {
            std::filesystem::path realPath(mapping.second);
            if (!std::filesystem::exists(realPath)) {
                std::filesystem::create_directories(realPath);
                Log(LogLevel::Info, "Created directory: %s", realPath.string().c_str());
            }
        }
        
        Log(LogLevel::Info, "VirtualFileSystem initialized successfully");
        return true;
    }

    bool VirtualFileSystem::MapPath(const std::filesystem::path& virtualPath, const std::filesystem::path& realPath) {
        Log(LogLevel::Info, "Mapping path: %s -> %s", 
            virtualPath.string().c_str(), realPath.string().c_str());
        
        // Add mappings in both directions
        m_VirtualToRealPaths[virtualPath.string()] = realPath.string();
        m_RealToVirtualPaths[realPath.string()] = virtualPath.string();
        
        return true;
    }

    std::filesystem::path VirtualFileSystem::TranslateToReal(const std::filesystem::path& virtualPath) {
        std::string pathStr = virtualPath.string();
        
        // Check each prefix mapping
        for (const auto& mapping : m_VirtualToRealPaths) {
            if (pathStr.find(mapping.first) == 0) {
                // Replace prefix
                std::string result = mapping.second + pathStr.substr(mapping.first.length());
                Log(LogLevel::Debug, "Translated virtual path '%s' to real path '%s'", 
                    pathStr.c_str(), result.c_str());
                return std::filesystem::path(result);
            }
        }
        
        // No mapping found, return original
        return virtualPath;
    }

    std::filesystem::path VirtualFileSystem::TranslateToVirtual(const std::filesystem::path& realPath) {
        std::string pathStr = realPath.string();
        
        // Check each prefix mapping
        for (const auto& mapping : m_RealToVirtualPaths) {
            if (pathStr.find(mapping.first) == 0) {
                // Replace prefix
                std::string result = mapping.second + pathStr.substr(mapping.first.length());
                Log(LogLevel::Debug, "Translated real path '%s' to virtual path '%s'", 
                    pathStr.c_str(), result.c_str());
                return std::filesystem::path(result);
            }
        }
        
        // No mapping found, return original
        return realPath;
    }

    bool VirtualFileSystem::IsVirtualPath(const std::filesystem::path& path) {
        std::string pathStr = path.string();
        
        // Check if the path starts with any virtual prefix
        for (const auto& mapping : m_VirtualToRealPaths) {
            if (pathStr.find(mapping.first) == 0) {
                return true;
            }
        }
        
        return false;
    }

    bool VirtualFileSystem::FileExists(const std::filesystem::path& virtualPath) {
        // Translate to real path
        std::filesystem::path realPath = TranslateToReal(virtualPath);
        
        // Check if file exists
        return std::filesystem::exists(realPath);
    }

    bool VirtualFileSystem::CreateDirectory(const std::filesystem::path& virtualPath) {
        // Translate to real path
        std::filesystem::path realPath = TranslateToReal(virtualPath);
        
        // Create directory
        try {
            return std::filesystem::create_directories(realPath);
        } catch (const std::exception& e) {
            Log(LogLevel::Error, "Failed to create directory '%s': %s", 
                realPath.string().c_str(), e.what());
            return false;
        }
    }

    bool VirtualFileSystem::DeleteFile(const std::filesystem::path& virtualPath) {
        // Translate to real path
        std::filesystem::path realPath = TranslateToReal(virtualPath);
        
        // Delete file
        try {
            return std::filesystem::remove(realPath);
        } catch (const std::exception& e) {
            Log(LogLevel::Error, "Failed to delete file '%s': %s", 
                realPath.string().c_str(), e.what());
            return false;
        }
    }

    bool VirtualFileSystem::CopyFile(const std::filesystem::path& srcVirtualPath, const std::filesystem::path& destVirtualPath, bool overwrite) {
        // Translate to real paths
        std::filesystem::path srcRealPath = TranslateToReal(srcVirtualPath);
        std::filesystem::path destRealPath = TranslateToReal(destVirtualPath);
        
        // Check if destination file exists and we're not allowed to overwrite
        if (!overwrite && std::filesystem::exists(destRealPath)) {
            Log(LogLevel::Error, "Destination file '%s' already exists and overwrite is not allowed", 
                destRealPath.string().c_str());
            return false;
        }
        
        // Copy file
        try {
            std::filesystem::copy_file(
                srcRealPath, 
                destRealPath, 
                overwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::none
            );
            return true;
        } catch (const std::exception& e) {
            Log(LogLevel::Error, "Failed to copy file from '%s' to '%s': %s", 
                srcRealPath.string().c_str(), destRealPath.string().c_str(), e.what());
            return false;
        }
    }

} // namespace ObseGPCompat