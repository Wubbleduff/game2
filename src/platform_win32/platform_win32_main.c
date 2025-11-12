
#include "engine.h"

#include "platform_win32/platform_win32_core.h"
#include "platform_win32/platform_win32_input.h"
#include "platform_win32/platform_win32_render.h"

struct PlatformWin32
{
    struct PlatformWin32Core core;

    struct PlatformWin32Input input;

    struct PlatformWin32Render render;
};

struct MainMemory
{
    struct PlatformWin32 win32;

    struct Engine engine;
};
struct MainMemory* g_main_memory;

void platform_win32_init()
{
    platform_win32_init_core(&g_main_memory->win32.core);

    platform_win32_init_input(&g_main_memory->win32.input);

    platform_win32_init_render(&g_main_memory->win32.render);
}

void WinMainCRTStartup()
{
    g_main_memory = VirtualAlloc(
        NULL,
        (sizeof(struct MainMemory) + 4095) & ~4095,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);
    ASSERT(g_main_memory, "Could not allocate memory.");
    ASSERT(((u64)g_main_memory & 4095) == 0, "g_main_memory not aligned.");
    memset(g_main_memory, 0xCD, sizeof(*g_main_memory));

    platform_win32_init();
    init_engine(&g_main_memory->engine);

    s64 frame_timer_ns = 0;
    s64 last_frame_time_ns = platform_win32_get_time_ns();
    u64 running = 1;
    while(running)
    {
        const s64 frame_start_ns = platform_win32_get_time_ns();
        s64 frame_duration_ns = frame_start_ns - last_frame_time_ns;
        last_frame_time_ns = frame_start_ns;
        frame_timer_ns += frame_duration_ns;

        running &= platform_win32_read_events();

        if(frame_timer_ns < FRAME_DURATION_NS)
        {
            _mm_pause();
            continue;
        }
        frame_timer_ns -= FRAME_DURATION_NS;

        running &= !platform_win32_is_keyboard_key_down(VK_ESCAPE);

        tick_engine(&g_main_memory->engine);

        platform_win32_render(&g_main_memory->engine);

        platform_win32_swap_and_clear_buffer(20, 0, 50);

        platform_win32_input_end_frame();
    }

    ExitProcess(0);
}
