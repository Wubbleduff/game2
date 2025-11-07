
#pragma once

#include "common.h"

enum KeyboardKey
{
    KB_NOT_SUPPORTED,

    KB_LCTRL = 0x11,
    KB_ESCAPE = 0x1B,
    KB_SPACE = 0x20,

    KB_0 = '0',
    KB_1 = '1',
    KB_2 = '2',
    KB_3 = '3',
    KB_4 = '4',
    KB_5 = '5',
    KB_6 = '6',
    KB_7 = '7',
    KB_8 = '8',
    KB_9 = '9',

    KB_A = 'A',
    KB_B = 'B',
    KB_C = 'C',
    KB_D = 'D',
    KB_E = 'E',
    KB_F = 'F',
    KB_G = 'G',
    KB_H = 'H',
    KB_I = 'I',
    KB_J = 'J',
    KB_K = 'K',
    KB_L = 'L',
    KB_M = 'M',
    KB_N = 'N',
    KB_O = 'O',
    KB_P = 'P',
    KB_Q = 'Q',
    KB_R = 'R',
    KB_S = 'S',
    KB_T = 'T',
    KB_U = 'U',
    KB_V = 'V',
    KB_W = 'W',
    KB_X = 'X',
    KB_Y = 'Y',
    KB_Z = 'Z',

    NUM_KEYBOARD_KEY
};

enum GameMouseButton
{
    GAME_MB_LEFT,
    GAME_MB_RIGHT,

    NUM_GAME_MOUSE_BUTTON
};

u32 is_keyboard_key_down(enum KeyboardKey k);
u32 is_keyboard_key_toggled_down(enum KeyboardKey k);

u32 is_mouse_button_down(enum GameMouseButton m);

void get_mouse_screen_delta(s32* dx, s32* dy);
