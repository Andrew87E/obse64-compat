#pragma once

// Include our Windows wrapper first
#include "WindowsWrapper.h"

// Standard includes
#include <filesystem>
#include <map>
#include <string>

namespace ObseGPCompat
{

    class VirtualFileSystem
    {
    public:
        VirtualFileSystem();
        ~VirtualFileSystem();

        bool Initialize();

        // Path mapping methods
        bool MapPath(const std::filesystem::path &virtualPath, const std::filesystem::path &realPath);

        // Path translation methods
        std::filesystem::path TranslateToReal(const std::filesystem::path &virtualPath);
        std::filesystem::path TranslateToVirtual(const std::filesystem::path &realPath);

        // Filesystem operation methods
        bool IsVirtualPath(const std::filesystem::path &path);
        bool FileExists(const std::filesystem::path &virtualPath);
        bool CreateDirectory(const std::filesystem::path &virtualPath);
        bool DeleteFile(const std::filesystem::path &virtualPath);
        bool CopyFile(const std::filesystem::path &srcVirtualPath, const std::filesystem::path &destVirtualPath, bool overwrite = false);

    private:
        // Maps for path translation
        std::map<std::string, std::string> m_VirtualToRealPaths;
        std::map<std::string, std::string> m_RealToVirtualPaths;
    };

} // namespace ObseGPCompat