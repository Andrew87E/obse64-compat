#pragma once

#include "ObseGPCompat.h"

namespace ObseGPCompat {

    class ProxyLauncher {
    public:
        ProxyLauncher();
        ~ProxyLauncher();
        
        // Launch the Game Pass version with the compatibility layer
        bool Launch();
        
        // Initialize the launcher
        bool Initialize();
        
        // Check if OBSE64 is installed
        bool IsObse64Installed();
        
        // Check if the Game Pass version is installed
        bool IsGamePassVersionInstalled();
        
        // Get the path to the Game Pass executable
        std::filesystem::path GetGamePassExecutablePath();
        
        // Get the path to the OBSE64 loader
        std::filesystem::path GetObse64LoaderPath();
        
        // Inject the compatibility layer into the Game Pass process
        bool InjectCompatibilityLayer(HANDLE processHandle);
        
    private:
        // Process information
        PROCESS_INFORMATION m_ProcessInfo;
        
        // Find the Game Pass installation
        bool FindGamePassInstallation();
        
        // Find the OBSE64 installation
        bool FindObse64Installation();
        
        // Launch process with suspended flag
        bool LaunchProcessSuspended(const std::filesystem::path& executablePath, const std::string& commandLine);
        
        // Resume suspended process
        bool ResumeProcess();
    };

} // namespace ObseGPCompat