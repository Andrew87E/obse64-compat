# OBSE64GP - Game Pass Compatibility Layer for OBSE64

## Overview

OBSE64GP is a compatibility layer that allows the Oblivion Remastered Script Extender (OBSE64) to work with the Microsoft Store/Game Pass version of The Elder Scrolls IV: Oblivion Remastered. This tool acts as a bridge between the Game Pass version and OBSE64, enabling mod support that would otherwise be unavailable.

## Features

- Allows OBSE64 plugins to work with the Game Pass version
- Transparently redirects file operations between Steam and Game Pass paths
- Provides a virtual file system layer to work around UWP sandbox restrictions
- Automatically detects game and OBSE64 installations
- Detailed logging for troubleshooting

## Requirements

- The Elder Scrolls IV: Oblivion Remastered (Game Pass version)
- OBSE64 (Steam version) installed
- Microsoft Visual C++ Redistributable 2019 or newer
- Windows 10/11 with Game Pass subscription

## Installation Instructions

1. **Enable Mods in Xbox App**:

   - Open the Xbox app
   - Find Oblivion Remastered in your library
   - Click on the three dots (...)
   - Select "Enable Mods"

2. **Install OBSE64**:

   - Download OBSE64 from [Nexus Mods](https://www.nexusmods.com/oblivionremastered/mods/282)
   - Install it to a temporary location (you don't need to install it to your Game Pass Oblivion folder)

3. **Install OBSE64GP**:

   - Extract the OBSE64GP files to a folder of your choice
   - Run the OBSE64GP_Setup.exe installer
   - Follow the installation wizard, which will:
     - Detect your Game Pass Oblivion installation
     - Detect your OBSE64 installation
     - Configure paths automatically

4. **Launch the Game**:
   - Always use the OBSE64GP_Launcher.exe to start the game
   - Do NOT use the Xbox app to launch the game if you want to use OBSE64

## Installing OBSE64 Plugins

1. Download the plugin from Nexus Mods or another source
2. Extract the plugin files to: `C:\Program Files\ModifiableWindowsApps\The Elder Scrolls IV- Oblivion Remastered\Content\OblivionRemastered\Binaries\Win64\OBSE\Plugins`
   - This directory is created automatically by OBSE64GP
   - If your Game Pass installation is in a different location, check the log files for the correct path

## Troubleshooting

### Logs

Log files are stored in: `%LOCALAPPDATA%\OBSE64GP\Logs\`

Common issues:

1. **Game crashes on startup**:

   - Check that you have enabled mods in the Xbox app
   - Verify that both OBSE64 and Oblivion Remastered are up to date
   - Check the logs for detailed error messages

2. **OBSE64 not detected in-game**:

   - Run the OBSE64GP Configuration Utility and verify paths
   - Make sure you're launching through OBSE64GP_Launcher.exe

3. **Plugins not loading**:
   - Check that plugins are installed to the correct directory
   - Verify plugin compatibility with the current version of OBSE64
   - Check the logs for plugin loading errors

## Configuration

The OBSE64GP configuration file is located at: `%LOCALAPPDATA%\OBSE64GP\config.ini`

Key settings:

```ini
[Paths]
GamePassInstall=C:\Program Files\ModifiableWindowsApps\The Elder Scrolls IV- Oblivion Remastered
SteamInstall=C:\Path\To\Steam\OBSE64\Installation

[Settings]
AutoDetectPaths=true
EnableLogging=true
LogLevel=1
```

## Technical Details

OBSE64GP works by:

1. Launching the Game Pass version of Oblivion Remastered
2. Injecting a compatibility layer DLL into the game process
3. Hooking Windows API functions for file operations
4. Translating paths between Game Pass and Steam formats
5. Providing a virtual file system layer to bypass UWP restrictions

## Credits and Thanks

- Ian Patterson (ianpatt) and the OBSE team for creating OBSE64
- Bethesda Game Studios for Oblivion Remastered
- Microsoft Detours library for API hooking

## License

This software is provided as-is under the MIT License. See LICENSE.txt for details.

## Disclaimer

This tool is not affiliated with, endorsed by, or related to Bethesda Softworks, ZeniMax Media, or Microsoft. Use at your own risk. Always back up your saves before using mods.
