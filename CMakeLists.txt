cmake_minimum_required(VERSION 3.10)
project(OBSE64GP VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add custom CMake modules directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Windows-specific settings
if(WIN32)
    # Add Windows target version macros
    add_definitions(-D_WIN32_WINNT=0x0601)  # Target Windows 7 or later
    
    # Set WINDOWS subsystem for the DLL only
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
    
    # Set CONSOLE subsystem for the launcher executable
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find Detours using our custom module
find_package(Detours REQUIRED)

if(NOT DETOURS_FOUND)
    message(FATAL_ERROR "Microsoft Detours library not found. Please ensure you have detours.h and detours.lib in your library paths.")
else()
    message(STATUS "Using Detours include directory: ${DETOURS_INCLUDE_DIR}")
    message(STATUS "Using Detours library: ${DETOURS_LIBRARY}")
endif()

# Add include directories
include_directories(${DETOURS_INCLUDE_DIR})

# Define sources
set(SOURCES
    src/main.cpp
    src/PathTranslator.cpp
    src/APIHookManager.cpp
    src/VirtualFileSystem.cpp
    src/ConfigurationManager.cpp
    src/ProxyLauncher.cpp
)

# Define headers
set(HEADERS
    include/ObseGPCompat.h
    include/PathTranslator.h
    include/APIHookManager.h
    include/VirtualFileSystem.h
    include/ConfigurationManager.h
    include/ProxyLauncher.h
)

# Add resource files for versioning (optional)
if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/resources/version.rc.in")
    configure_file(
        ${CMAKE_SOURCE_DIR}/resources/version.rc.in
        ${CMAKE_BINARY_DIR}/version.rc
        @ONLY
    )
    set(RESOURCE_FILES ${CMAKE_BINARY_DIR}/version.rc)
else()
    set(RESOURCE_FILES "")
endif()

# Add executable
add_executable(OBSE64GP_Launcher ${SOURCES} ${HEADERS} ${RESOURCE_FILES})

# Add DLL library
add_library(OBSE64GP SHARED ${SOURCES} ${HEADERS} ${RESOURCE_FILES})

# Link libraries
target_link_libraries(OBSE64GP_Launcher PRIVATE 
    ${DETOURS_LIBRARY}
    shlwapi.lib
    shell32.lib  # Added for SHGetFolderPath
    version.lib
    ws2_32.lib
)

target_link_libraries(OBSE64GP PRIVATE 
    ${DETOURS_LIBRARY}
    shlwapi.lib
    shell32.lib  # Added for SHGetFolderPath
    version.lib
    ws2_32.lib
)

# Add compile definitions
target_compile_definitions(OBSE64GP PRIVATE 
    OBSE64GP_EXPORTS 
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_compile_definitions(OBSE64GP_Launcher PRIVATE 
    OBSE64GP_LAUNCHER
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# Set output directories
set_target_properties(OBSE64GP PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    PREFIX ""  # Remove "lib" prefix from DLL name
)

set_target_properties(OBSE64GP_Launcher PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install rules
install(TARGETS OBSE64GP OBSE64GP_Launcher
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

# Install additional files
if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    install(FILES README.md DESTINATION .)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE.txt")
    install(FILES LICENSE.txt DESTINATION .)
endif()

# Copy DLL to bin directory for executable
add_custom_command(TARGET OBSE64GP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:OBSE64GP>
    $<TARGET_FILE_DIR:OBSE64GP_Launcher>
)