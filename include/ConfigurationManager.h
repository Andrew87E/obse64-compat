#pragma once

#include <string>
#include <filesystem>
#include <map>

namespace ObseGPCompat
{

    class ConfigurationManager
    {
    public:
        ConfigurationManager();
        ~ConfigurationManager();

        bool Initialize();
        bool Save();

        std::string GetString(const std::string &section, const std::string &key, const std::string &defaultValue = "");
        int GetInt(const std::string &section, const std::string &key, int defaultValue = 0);
        bool GetBool(const std::string &section, const std::string &key, bool defaultValue = false);

        void SetString(const std::string &section, const std::string &key, const std::string &value);
        void SetInt(const std::string &section, const std::string &key, int value);
        void SetBool(const std::string &section, const std::string &key, bool value);

        std::filesystem::path GetConfigPath() const;

    private:
        bool ParseConfig();

        std::filesystem::path m_ConfigPath;
        std::map<std::string, std::map<std::string, std::string>> m_ConfigData;
    };

} // namespace ObseGPCompat