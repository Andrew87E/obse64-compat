#pragma once

// This wrapper ensures consistent Windows header includes
// with the right defines in the right order

#ifndef _WINDOWS_WRAPPER_H_
#define _WINDOWS_WRAPPER_H_

// Required defines before including Windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef _AMD64_
#define _AMD64_ // For x64 build
#endif

// Include Windows headers in the correct order
#include <Windows.h>
#include <Winternl.h>
#include <ShlObj.h>
#include <Shlwapi.h>

// Standard C++ includes that might be needed
#include <stddef.h>
#include <string>
#include <filesystem>

#endif // _WINDOWS_WRAPPER_H_