#include "ConfigurationManager.h"
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <shlobj.h>

namespace ObseGPCompat {

    ConfigurationManager::ConfigurationManager() {
        // Constructor
    }

    ConfigurationManager::~ConfigurationManager() {
        // Save configuration on destruction
        Save();
    }

    bool ConfigurationManager::Initialize() {
        Log(LogLevel::Info, "Initializing ConfigurationManager");
        
        // Set default config path
        char localAppData[MAX_PATH];
        if (FAILED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData))) {
            Log(LogLevel::Error, "Failed to get Local AppData path");
            return false;
        }
        
        m_ConfigPath = std::filesystem::path(localAppData) / "OBSE64GP" / "config.ini";
        Log(LogLevel::Info, "Config path: %s", m_ConfigPath.string().c_str());
        
        // Create directory if it doesn't exist
        std::filesystem::create_directories(m_ConfigPath.parent_path());
        
        // Parse configuration file if it exists, otherwise create a default one
        if (std::filesystem::exists(m_ConfigPath)) {
            if (!ParseConfig()) {
                Log(LogLevel::Error, "Failed to parse configuration file");
                return false;
            }
        } else {
            // Create default configuration
            SetString("Paths", "GamePassInstall", "");
            SetString("Paths", "SteamInstall", "");
            SetBool("Settings", "AutoDetectPaths", true);
            SetBool("Settings", "EnableLogging", true);
            SetInt("Settings", "LogLevel", static_cast<int>(LogLevel::Info));
            
            // Save default configuration
            if (!Save()) {
                Log(LogLevel::Error, "Failed to save default configuration");
                return false;
            }
        }
        
        Log(LogLevel::Info, "ConfigurationManager initialized successfully");
        return true;
    }

    std::string ConfigurationManager::GetString(const std::string& section, const std::string& key, const std::string& defaultValue) {
        // Check if section exists
        auto sectionIt = m_ConfigData.find(section);
        if (sectionIt == m_ConfigData.end()) {
            return defaultValue;
        }
        
        // Check if key exists in section
        auto keyIt = sectionIt->second.find(key);
        if (keyIt == sectionIt->second.end()) {
            return defaultValue;
        }
        
        return keyIt->second;
    }

    int ConfigurationManager::GetInt(const std::string& section, const std::string& key, int defaultValue) {
        std::string strValue = GetString(section, key, "");
        if (strValue.empty()) {
            return defaultValue;
        }
        
        try {
            return std::stoi(strValue);
        } catch (const std::exception& e) {
            Log(LogLevel::Warning, "Failed to parse integer value for %s::%s: %s", 
                section.c_str(), key.c_str(), e.what());
            return defaultValue;
        }
    }

    bool ConfigurationManager::GetBool(const std::string& section, const std::string& key, bool defaultValue) {
        std::string strValue = GetString(section, key, "");
        if (strValue.empty()) {
            return defaultValue;
        }
        
        // Convert to lowercase
        std::transform(strValue.begin(), strValue.end(), strValue.begin(), 
            [](unsigned char c) { return std::tolower(c); });
        
        // Check for true values
        return (strValue == "true" || strValue == "1" || strValue == "yes" || strValue == "on");
    }

    void ConfigurationManager::SetString(const std::string& section, const std::string& key, const std::string& value) {
        m_ConfigData[section][key] = value;
    }

    void ConfigurationManager::SetInt(const std::string& section, const std::string& key, int value) {
        m_ConfigData[section][key] = std::to_string(value);
    }

    void ConfigurationManager::SetBool(const std::string& section, const std::string& key, bool value) {
        m_ConfigData[section][key] = value ? "true" : "false";
    }

    bool ConfigurationManager::Save() {
        Log(LogLevel::Info, "Saving configuration to %s", m_ConfigPath.string().c_str());
        
        try {
            // Open file for writing
            std::ofstream configFile(m_ConfigPath);
            if (!configFile.is_open()) {
                Log(LogLevel::Error, "Failed to open configuration file for writing");
                return false;
            }
            
            // Write each section
            for (const auto& section : m_ConfigData) {
                configFile << "[" << section.first << "]" << std::endl;
                
                // Write each key-value pair in the section
                for (const auto& kvp : section.second) {
                    configFile << kvp.first << "=" << kvp.second << std::endl;
                }
                
                // Add a blank line between sections
                configFile << std::endl;
            }
            
            configFile.close();
            Log(LogLevel::Info, "Configuration saved successfully");
            return true;
        } catch (const std::exception& e) {
            Log(LogLevel::Error, "Failed to save configuration: %s", e.what());
            return false;
        }
    }

    std::filesystem::path ConfigurationManager::GetConfigPath() const {
        return m_ConfigPath;
    }

    bool ConfigurationManager::ParseConfig() {
        Log(LogLevel::Info, "Parsing configuration file: %s", m_ConfigPath.string().c_str());
        
        try {
            // Open file for reading
            std::ifstream configFile(m_ConfigPath);
            if (!configFile.is_open()) {
                Log(LogLevel::Error, "Failed to open configuration file for reading");
                return false;
            }
            
            // Clear existing configuration data
            m_ConfigData.clear();
            
            std::string line;
            std::string currentSection;
            
            // Parse each line
            while (std::getline(configFile, line)) {
                // Remove leading and trailing whitespace
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);
                
                // Skip empty lines and comments
                if (line.empty() || line[0] == ';' || line[0] == '#') {
                    continue;
                }
                
                // Check for section
                if (line[0] == '[' && line[line.length() - 1] == ']') {
                    currentSection = line.substr(1, line.length() - 2);
                    continue;
                }
                
                // Parse key-value pair
                size_t equalsPos = line.find('=');
                if (equalsPos != std::string::npos) {
                    std::string key = line.substr(0, equalsPos);
                    std::string value = line.substr(equalsPos + 1);
                    
                    // Remove leading and trailing whitespace from key and value
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    
                    // Store key-value pair in current section
                    if (!currentSection.empty()) {
                        m_ConfigData[currentSection][key] = value;
                    }
                }
            }
            
            configFile.close();
            Log(LogLevel::Info, "Configuration parsed successfully");
            return true;
        } catch (const std::exception& e) {
            Log(LogLevel::Error, "Failed to parse configuration: %s", e.what());
            return false;
        }
    }

} // namespace ObseGPCompat