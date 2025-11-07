
#include "platform_win32/platform_win32_input.h"
#include "platform_win32/platform_win32_core.h"

struct PlatformWin32Input* g_platform_win32_input;

struct PlatformWin32Input* platform_win32_get_input()
{
    return g_platform_win32_input;
}

void platform_win32_init_input(struct PlatformWin32Input* mem)
{
    struct PlatformWin32Core* win32_core = platform_win32_get_core();

    g_platform_win32_input = mem;

    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ZERO_ARRAY(win32_input->keyboard_buf);
    ZERO_ARRAY(win32_input->prev_keyboard_buf);

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(win32_core->hwnd, &p);

    win32_input->mouse_screen_x = p.x;
    win32_input->mouse_screen_y = p.y;
    win32_input->prev_mouse_screen_x = p.x;
    win32_input->prev_mouse_screen_y = p.y;
}

void platform_win32_input_end_frame()
{
    struct PlatformWin32Core* win32_core = platform_win32_get_core();
    struct PlatformWin32Input* win32_input = platform_win32_get_input();

    COPY_ARRAY(win32_input->prev_keyboard_buf, win32_input->keyboard_buf);
    COPY_ARRAY(win32_input->prev_mouse_button, win32_input->mouse_button);
    win32_input->prev_mouse_screen_x = win32_input->mouse_screen_x;
    win32_input->prev_mouse_screen_y = win32_input->mouse_screen_y;

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(win32_core->hwnd, &p);

    win32_input->mouse_screen_x = p.x;
    win32_input->mouse_screen_y = p.y;
}

static u8 is_valid_vk(const u8 k)
{
    return k >= 0x01 && k <= 0xFE;
}

void platform_win32_set_keyboard_key(const u8 vk)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_vk(vk), "Invalid keyboard key %u", vk);
    win32_input->keyboard_buf[vk >> 3] |= (1 << (vk & 7));
}

void platform_win32_clear_keyboard_key(const u8 vk)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_vk(vk), "Invalid keyboard key %u", vk);
    win32_input->keyboard_buf[vk >> 3] &= ~(1 << (vk & 7));
}

u32 platform_win32_is_keyboard_key_down(const u8 vk)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_vk(vk), "Invalid keyboard key %u", vk);
    return (win32_input->keyboard_buf[vk >> 3] & (1 << (vk & 7))) != 0;
}

u32 platform_win32_is_keyboard_key_toggled_down(const u8 vk)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_vk(vk), "Invalid keyboard key %u", vk);
    const u32 cur_frame = (win32_input->keyboard_buf[vk >> 3] & (1 << (vk & 7))) != 0;
    const u32 prev_frame = (win32_input->prev_keyboard_buf[vk >> 3] & (1 << (vk & 7))) != 0;
    return cur_frame && !prev_frame;
}

static u32 is_valid_mouse_button(const enum Win32MouseButton m)
{
    return m >= WIN32_MB_LEFT && m < NUM_WIN32_MOUSE_BUTTONS;
}

void platform_win32_set_mouse_button(const enum Win32MouseButton m)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_mouse_button(m), "Invalid mouse button %u", m);
    win32_input->mouse_button[m >> 3] |= (1 << (m & 7));
}

void platform_win32_clear_mouse_button(const enum Win32MouseButton m)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_mouse_button(m), "Invalid mouse button %u", m);
    win32_input->mouse_button[m >> 3] &= ~(1U << (m & 7));
}

u32 platform_win32_is_mouse_button_down(const enum Win32MouseButton m)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_mouse_button(m), "Invalid mouse button %u", m);
    return (win32_input->mouse_button[m >> 3] & (1U << (m & 7))) != 0;
}

u32 platform_win32_is_mouse_button_toggled_down(const enum Win32MouseButton m)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    ASSERT(is_valid_mouse_button(m), "Invalid mouse button %u", m);
    const u32 cur_frame = (win32_input->mouse_button[m >> 3] & (1U << (m & 7))) != 0;
    const u32 prev_frame = (win32_input->prev_mouse_button[m >> 3] & (1U << (m & 7))) != 0;
    return cur_frame && !prev_frame;
}

void platform_win32_get_mouse_screen_position(s32* x, s32* y)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    *x = win32_input->mouse_screen_x;
    *y = win32_input->mouse_screen_y;
}

void platform_win32_get_mouse_screen_delta(s32* x, s32* y)
{
    struct PlatformWin32Input* win32_input = platform_win32_get_input();
    *x = win32_input->mouse_screen_x - win32_input->prev_mouse_screen_x;
    *y = win32_input->mouse_screen_y - win32_input->prev_mouse_screen_y;
}


////////////////////////////////////////////////////////////////////////////////
// Game specific impl


static const u8 keyboard_key_to_vk_code[NUM_KEYBOARD_KEY] = {
    [KB_LCTRL] = VK_CONTROL,
    [KB_ESCAPE] = VK_ESCAPE,
    [KB_SPACE] = VK_SPACE,

    [KB_0] = '0',
    [KB_1] = '1',
    [KB_2] = '2',
    [KB_3] = '3',
    [KB_4] = '4',
    [KB_5] = '5',
    [KB_6] = '6',
    [KB_7] = '7',
    [KB_8] = '8',
    [KB_9] = '9',

    [KB_A] = 'A',
    [KB_B] = 'B',
    [KB_C] = 'C',
    [KB_D] = 'D',
    [KB_E] = 'E',
    [KB_F] = 'F',
    [KB_G] = 'G',
    [KB_H] = 'H',
    [KB_I] = 'I',
    [KB_J] = 'J',
    [KB_K] = 'K',
    [KB_L] = 'L',
    [KB_M] = 'M',
    [KB_N] = 'N',
    [KB_O] = 'O',
    [KB_P] = 'P',
    [KB_Q] = 'Q',
    [KB_R] = 'R',
    [KB_S] = 'S',
    [KB_T] = 'T',
    [KB_U] = 'U',
    [KB_V] = 'V',
    [KB_W] = 'W',
    [KB_X] = 'X',
    [KB_Y] = 'Y',
    [KB_Z] = 'Z',
};

u32 is_keyboard_key_down(enum KeyboardKey k)
{
    ASSERT((u32)k < ARRAY_COUNT(keyboard_key_to_vk_code), "Invalid key %u", k);
    return platform_win32_is_keyboard_key_down(keyboard_key_to_vk_code[(u32)k]);
}

u32 is_keyboard_key_toggled_down(enum KeyboardKey k)
{
    ASSERT((u32)k < ARRAY_COUNT(keyboard_key_to_vk_code), "Invalid key %u", k);
    return platform_win32_is_keyboard_key_toggled_down(keyboard_key_to_vk_code[(u32)k]);
}

static const u8 mouse_button_to_win32_mouse_button[] = {
    [GAME_MB_LEFT] = WIN32_MB_LEFT,
    [GAME_MB_RIGHT] = WIN32_MB_RIGHT,
};

u32 is_mouse_button_down(enum GameMouseButton m)
{
    ASSERT((u32)m < ARRAY_COUNT(mouse_button_to_win32_mouse_button), "Invalid mouse button %u", m);
    return platform_win32_is_mouse_button_down(mouse_button_to_win32_mouse_button[m]);
}

void get_mouse_screen_delta(s32* dx, s32* dy)
{
    platform_win32_get_mouse_screen_delta(dx, dy);
}
