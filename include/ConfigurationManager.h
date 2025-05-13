#pragma once

#include "ObseGPCompat.h"

namespace ObseGPCompat {

    class ConfigurationManager {
    public:
        ConfigurationManager();
        ~ConfigurationManager();
        
        // Initialize configuration
        bool Initialize();
        
        // Get a string value from the configuration
        std::string GetString(const std::string& section, const std::string& key, const std::string& defaultValue = "");
        
        // Get an integer value from the configuration
        int GetInt(const std::string& section, const std::string& key, int defaultValue = 0);
        
        // Get a boolean value from the configuration
        bool GetBool(const std::string& section, const std::string& key, bool defaultValue = false);
        
        // Set a string value in the configuration
        void SetString(const std::string& section, const std::string& key, const std::string& value);
        
        // Set an integer value in the configuration
        void SetInt(const std::string& section, const std::string& key, int value);
        
        // Set a boolean value in the configuration
        void SetBool(const std::string& section, const std::string& key, bool value);
        
        // Save configuration to file
        bool Save();
        
        // Get the path to the configuration file
        std::filesystem::path GetConfigPath() const;
        
    private:
        // Configuration data
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_ConfigData;
        
        // Path to the configuration file
        std::filesystem::path m_ConfigPath;
        
        // Parse configuration file
        bool ParseConfig();
    };

} // namespace ObseGPCompat
