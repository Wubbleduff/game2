
#pragma once

#include "common.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct PlatformWin32Core
{
    HWND hwnd;
    u32 client_width;
    u32 client_height;
    
    s64 clock_freq_hz;
};

// Avoid C runtime library
// https://hero.handmade.network/forums/code-discussion/t/94-guide_-_how_to_avoid_c_c++_runtime_on_windows
extern int _fltused;
void *memset(void *dst, int c, size_t count);
void *memcpy(void *dst, const void *src, size_t count);

struct PlatformWin32Core* platform_win32_get_core();
void platform_win32_init_core(struct PlatformWin32Core* mem);
// Runs the windows message loop.
// Return 0 if the program should quit. Return 1 otherwise.
u8 platform_win32_read_events();
s64 platform_win32_get_time_ns();
