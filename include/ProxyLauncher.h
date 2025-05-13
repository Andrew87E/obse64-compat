#pragma once

#include <Windows.h>
#include <string>
#include <filesystem>

namespace ObseGPCompat
{

    class ProxyLauncher
    {
    public:
        ProxyLauncher();
        ~ProxyLauncher();

        bool Initialize();
        bool Launch();

        bool IsObse64Installed();
        bool IsGamePassVersionInstalled();

        std::filesystem::path GetGamePassExecutablePath();
        std::filesystem::path GetObse64LoaderPath();

    private:
        bool FindGamePassInstallation();
        bool FindObse64Installation();
        bool LaunchProcessSuspended(const std::filesystem::path &executablePath, const std::string &commandLine);
        bool ResumeProcess();
        bool InjectCompatibilityLayer(HANDLE processHandle);

        PROCESS_INFORMATION m_ProcessInfo;
    };

} // namespace ObseGPCompat