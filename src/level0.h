
#pragma once

#include "common.h"
#include "level.h"

#define LEVEL0_WIDTH 128
#define LEVEL0_HEIGHT 64
#define LEVEL0_LEFT (-LEVEL0_WIDTH / 2)
#define LEVEL0_RIGHT (LEVEL0_WIDTH / 2)
#define LEVEL0_BOTTOM (-LEVEL0_HEIGHT / 2)
#define LEVEL0_TOP (LEVEL0_HEIGHT / 2)

static const struct Level LEVEL0 = 
{
    .width = LEVEL0_WIDTH,
    .height = LEVEL0_HEIGHT,

    .num_walls = 12,
    .walls =
    {

        {LEVEL0_LEFT - 100, LEVEL0_BOTTOM - 100, 100, 300},
        {     LEVEL0_RIGHT, LEVEL0_BOTTOM - 100, 100, 300},

        {LEVEL0_LEFT - 100, LEVEL0_BOTTOM - 100, 300, 100},
        {LEVEL0_LEFT - 100,          LEVEL0_TOP, 300, 100},

        { LEVEL0_LEFT + 1,  -6, 12, 12 },
        { LEVEL0_RIGHT - 12 - 1, -6, 12, 12 },

        { -24,   LEVEL0_BOTTOM,  2, 16 },
        { -24,              -8,  2, 16 },
        { -24, LEVEL0_TOP - 16,  2, 16 },

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

        { 0.1f, 0.1f, 0.2f, },
        { 0.2f, 0.1f, 0.1f, },

        { 0.1f, 0.1f, 0.2f, },
        { 0.1f, 0.1f, 0.2f, },
        { 0.1f, 0.1f, 0.2f, },
        
        { 0.2f, 0.1f, 0.1f, },
        { 0.2f, 0.1f, 0.1f, },
        { 0.2f, 0.1f, 0.1f, },
    },

    .wall_color_hl =
    {
        { 4.25f, 0.6f, 4.75f, },
        { 4.25f, 0.6f, 4.75f, },

        { 4.25f, 0.6f, 4.75f, },
        { 4.25f, 0.6f, 4.75f, },

        { 0.4f, 2.5f, 5.0f, },
        { 5.0f, 0.4f, 0.4f, },

        { 0.4f, 2.5f, 5.0f, },
        { 0.4f, 2.5f, 5.0f, },
        { 0.4f, 2.5f, 5.0f, },

        { 5.0f, 0.4f, 0.4f, },
        { 5.0f, 0.4f, 0.4f, },
        { 5.0f, 0.4f, 0.4f, },
    },
}; 
