
#include "platform_win32/platform_win32_core.h"
#include "platform_win32/platform_win32_input.h"

#include <stdarg.h>

#define STB_SPRINTF_IMPLEMENTATION
#include "external/stb_sprintf.h"

struct PlatformWin32Core* g_platform_win32_core;

int _fltused = 0;

void assert_fn(const u64 c, const char* msg, ...)
{
    if(!c)
    {
        char buf[1024] = {0};
        va_list args;
        va_start(args, msg);
        stbsp_vsnprintf(buf, sizeof(buf), msg, args);
        va_end(args);
        MessageBox(NULL, buf, "Assertion failed", MB_OK | MB_ICONERROR);
        DebugBreak();
    }
}

#pragma function(memset)
void *memset(void *dst, int c, size_t count)
{
    if(count < 32)
    {
        char *bytes = (char *)dst;
        while (count--)
        {
            *bytes++ = (char)c;
        }
        return dst;
    }
    else
    {
        const __m256i val = _mm256_set1_epi8((u8)c);
        for(u64 i = 0; i < count - 32; i += 32)
        {
            _mm256_storeu_si256((__m256i*)((u8*)dst + i), val);
        }
        for(u64 i = count - 32; i < count; i++)
        {
            ((u8*)dst)[i] = (u8)c;
        }
        return dst;
    }
}

#pragma function(memcpy)
void *memcpy(void *dst, const void *src, size_t count)
{
    if(count < 32)
    {
        for(u64 i = 0; i < count; i++)
        {
            ((u8*)dst)[i] = ((const u8*)src)[i];
        }
    }
    else
    {
        for(u64 i = 0; i < count - 32; i += 32)
        {
            _mm256_storeu_si256((__m256i*)((u8*)dst + i), _mm256_loadu_si256((const __m256i*)((const u8*)src + i)));
        }
        for(u64 i = count - 32; i < count; i++)
        {
            ((u8*)dst)[i] = ((const u8*)src)[i];
        }
    }
    return (u8*)dst + count;
}

struct PlatformWin32Core* platform_win32_get_core()
{
    return g_platform_win32_core;
}

static LRESULT CALLBACK platform_win32_wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (uMsg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        break;

        case WM_CLOSE: 
        {
            DestroyWindow(hwnd);
            return 0;
        }  
        break;

        case WM_PAINT:
        {
            ValidateRect(hwnd, 0);
        }
        break;

        case WM_KEYDOWN: 
        {
            platform_win32_set_keyboard_key((u8)wParam);
        }
        break;

        case WM_KEYUP:
        {
            platform_win32_clear_keyboard_key((u8)wParam);
        }
        break;

        case WM_LBUTTONDOWN:
        {
            platform_win32_set_mouse_button(WIN32_MB_LEFT);
        }
        break;
        case WM_LBUTTONUP:
        {
            platform_win32_clear_mouse_button(WIN32_MB_LEFT);
        }
        break;

        case WM_RBUTTONDOWN:
        {
            platform_win32_set_mouse_button(WIN32_MB_RIGHT);
        }
        break;
        case WM_RBUTTONUP:
        {
            platform_win32_clear_mouse_button(WIN32_MB_RIGHT);
        }
        break;

        default:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        break;
    }

    return result;
}

void platform_win32_init_core(struct PlatformWin32Core* mem)
{
    g_platform_win32_core = mem;

    struct PlatformWin32Core* win32_core = platform_win32_get_core();

    const HMODULE hinstance = GetModuleHandle(0);
    ASSERT(hinstance, "GetModuleHandle failed.");

    const char window_class_name[] = "Window Class";
    WNDCLASS wc = {};
    wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    wc.lpfnWndProc   = platform_win32_wnd_proc;
    wc.hInstance     = hinstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = window_class_name;
    const ATOM register_class_result = RegisterClass(&wc);
    ASSERT(register_class_result, "RegisterClass failed.");

    const u32 monitor_width = GetSystemMetrics(SM_CXSCREEN);
    const u32 monitor_height = GetSystemMetrics(SM_CYSCREEN);
    const u32 window_width = (monitor_width * 3) / 4;
    const u32 window_height = (monitor_height * 3) / 4;
    const u32 window_x = monitor_width / 2 - window_width / 2;
    const u32 window_y = monitor_height / 2 - window_height / 2;

    win32_core->hwnd = CreateWindowEx(
        0,
        window_class_name,
        "Game",
        WS_POPUP | WS_VISIBLE,
        window_x,
        window_y,
        window_width,
        window_height,
        NULL,
        NULL,
        hinstance,
        NULL);
    ASSERT(win32_core->hwnd, "CreateWindowEx failed.");

    LARGE_INTEGER clock_freq;
    QueryPerformanceFrequency(&clock_freq);
    win32_core->clock_freq_hz = clock_freq.QuadPart;
    ASSERT(win32_core->clock_freq_hz < 1000000000LL, "platform_win32_get_time_ns will break.");

    RECT client_rect;
    const BOOL get_client_rect_success = GetClientRect(win32_core->hwnd, &client_rect);
    ASSERT(get_client_rect_success, "GetClientRect failed: %i", GetLastError());
    win32_core->client_width = client_rect.right;
    win32_core->client_height = client_rect.bottom;
}

u8 platform_win32_read_events()
{
    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if(msg.message == WM_QUIT)
        {
            return 0;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}

s64 platform_win32_get_time_ns()
{
    const struct PlatformWin32Core* win32_core = platform_win32_get_core();
    LARGE_INTEGER cy;
    QueryPerformanceCounter(&cy);
    return cy.QuadPart * (1000000000LL / win32_core->clock_freq_hz);
}
