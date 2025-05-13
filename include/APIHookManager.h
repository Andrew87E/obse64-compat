#pragma once

#include "ObseGPCompat.h"

namespace ObseGPCompat {

    // Typedef for hooked function handlers
    using HookFunction = std::function<void*()>;

    class APIHookManager {
    public:
        APIHookManager();
        ~APIHookManager();
        
        // Initialize API hooks
        bool Initialize();
        
        // Remove all hooks
        void Shutdown();
        
        // Add a hook for a specific API function
        bool AddHook(const char* moduleName, const char* functionName, void* hookFunction, void** originalFunction);
        
        // Remove a specific hook
        bool RemoveHook(void* hookFunction);
        
    private:
        // Store information about hooked functions
        struct HookInfo {
            void* original;
            void* hooked;
            std::string moduleName;
            std::string functionName;
        };
        
        std::vector<HookInfo> m_Hooks;
    };

} // namespace ObseGPCompat
