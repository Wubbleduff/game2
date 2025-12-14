
#pragma once

#include "common.h"
#include "level.h"

#define LEVEL0_WIDTH 128
#define LEVEL0_HEIGHT 64
#define LEVEL0_LEFT (-LEVEL0_WIDTH / 2)
#define LEVEL0_RIGHT (LEVEL0_WIDTH / 2)
#define LEVEL0_BOTTOM (-LEVEL0_HEIGHT / 2)
#define LEVEL0_TOP (LEVEL0_HEIGHT / 2)

#define BLUE_TEAM_COLOR_BG { 0.1f, 0.1f, 0.2f, }
#define BLUE_TEAM_COLOR_HL { 0.4f, 2.5f, 5.0f, }

#define RED_TEAM_COLOR_BG { 0.2f, 0.1f, 0.1f, }
#define RED_TEAM_COLOR_HL { 5.0f, 0.4f, 0.4f, }

static const struct Level LEVEL0 = 
{
    .width = LEVEL0_WIDTH,
    .height = LEVEL0_HEIGHT,

    .num_walls = 14,
    .walls =
    {
        {LEVEL0_LEFT - 100, LEVEL0_BOTTOM - 100, 100, 300},
        {     LEVEL0_RIGHT, LEVEL0_BOTTOM - 100, 100, 300},
        {LEVEL0_LEFT - 100, LEVEL0_BOTTOM - 100, 300, 100},
        {LEVEL0_LEFT - 100,          LEVEL0_TOP, 300, 100},

        { LEVEL0_LEFT + 1,  -6, 12,  2 },
        { LEVEL0_LEFT + 1,   4, 12,  2 },
        { -24,   LEVEL0_BOTTOM,  2, 16 },
        { -24,              -8,  2, 16 },
        { -24, LEVEL0_TOP - 16,  2, 16 },

        { LEVEL0_RIGHT - 12 - 1, -6, 12,  2 },
        { LEVEL0_RIGHT - 12 - 1,  4, 12,  2 },
        { 24,   LEVEL0_BOTTOM,  2, 16 },
        { 24,              -8,  2, 16 },
        { 24, LEVEL0_TOP - 16,  2, 16 },
    },

    .wall_color_bg =
    {
        { 0.1f, 0.1f, 0.1f, },
        { 0.1f, 0.1f, 0.1f, },
        { 0.1f, 0.1f, 0.1f, },
        { 0.1f, 0.1f, 0.1f, },

        BLUE_TEAM_COLOR_BG,
        BLUE_TEAM_COLOR_BG,
        BLUE_TEAM_COLOR_BG,
        BLUE_TEAM_COLOR_BG,
        BLUE_TEAM_COLOR_BG,
        
        RED_TEAM_COLOR_BG,
        RED_TEAM_COLOR_BG,
        RED_TEAM_COLOR_BG,
        RED_TEAM_COLOR_BG,
        RED_TEAM_COLOR_BG,
    },

    .wall_color_hl =
    {
        { 4.25f, 0.6f, 4.75f, },
        { 4.25f, 0.6f, 4.75f, },
        { 4.25f, 0.6f, 4.75f, },
        { 4.25f, 0.6f, 4.75f, },

        BLUE_TEAM_COLOR_HL,
        BLUE_TEAM_COLOR_HL,
        BLUE_TEAM_COLOR_HL,
        BLUE_TEAM_COLOR_HL,
        BLUE_TEAM_COLOR_HL,

        RED_TEAM_COLOR_HL,
        RED_TEAM_COLOR_HL,
        RED_TEAM_COLOR_HL,
        RED_TEAM_COLOR_HL,
        RED_TEAM_COLOR_HL,
    },

    .num_flags = 2,
    .flag_pos_x = {
        LEVEL0_LEFT + 7.0f,
        LEVEL0_RIGHT - 7.0f,
    },
    .flag_pos_y = {
        0.0f,
        0.0f,
    },
};
