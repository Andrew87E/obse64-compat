#pragma once

#include "ObseGPCompat.h"

namespace ObseGPCompat {

    class VirtualFileSystem {
    public:
        VirtualFileSystem();
        ~VirtualFileSystem();
        
        // Initialize the virtual file system
        bool Initialize();
        
        // Map a virtual path to a real path
        bool MapPath(const std::filesystem::path& virtualPath, const std::filesystem::path& realPath);
        
        // Translate a virtual path to a real path
        std::filesystem::path TranslateToReal(const std::filesystem::path& virtualPath);
        
        // Translate a real path to a virtual path
        std::filesystem::path TranslateToVirtual(const std::filesystem::path& realPath);
        
        // Check if a path is virtual
        bool IsVirtualPath(const std::filesystem::path& path);
        
        // File operations on virtual paths
        bool FileExists(const std::filesystem::path& virtualPath);
        bool CreateDirectory(const std::filesystem::path& virtualPath);
        bool DeleteFile(const std::filesystem::path& virtualPath);
        bool CopyFile(const std::filesystem::path& srcVirtualPath, const std::filesystem::path& destVirtualPath, bool overwrite);
        
    private:
        // Maps for virtual to real paths
        std::unordered_map<std::string, std::string> m_VirtualToRealPaths;
        std::unordered_map<std::string, std::string> m_RealToVirtualPaths;
    };

} // namespace ObseGPCompat
