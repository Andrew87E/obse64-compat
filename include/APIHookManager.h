#pragma once

#include <string>
#include <vector>

namespace ObseGPCompat
{

    struct HookInfo
    {
        void *original;
        void *hooked;
        const char *moduleName;
        const char *functionName;
    };

    class APIHookManager
    {
    public:
        APIHookManager();
        ~APIHookManager();

        bool Initialize();
        void Shutdown();

        bool AddHook(const char *moduleName, const char *functionName, void *hookFunction, void **originalFunction);
        bool RemoveHook(void *hookFunction);

    private:
        std::vector<HookInfo> m_Hooks;
    };

} // namespace ObseGPCompat