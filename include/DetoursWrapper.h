#pragma once

// This is a wrapper for detours.h to ensure proper include order

// Include our Windows wrapper first for consistent header inclusion
#include "WindowsWrapper.h"

// Include the actual detours header - use direct path
#include "../libs/Detours/include/detours.h"