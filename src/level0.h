
#pragma once

#include "common.h"

#define LEVEL0_WIDTH 128
#define LEVEL0_HEIGHT 64
#define LEVEL0_LEFT (-((f32)LEVEL0_WIDTH / 2))
#define LEVEL0_RIGHT ((f32)LEVEL0_WIDTH / 2)
#define LEVEL0_BOTTOM (-((f32)LEVEL0_HEIGHT / 2))
#define LEVEL0_TOP ((f32)LEVEL0_HEIGHT / 2)

static const f32 level0_wall_pos_x[] = {
    // Bounds
    LEVEL0_LEFT - 50.0f,
    LEVEL0_RIGHT + 50.0f,
    0.0f,
    0.0f,

    // Base
    LEVEL0_LEFT + 6.0f + 1.0f,
    LEVEL0_RIGHT - 6.0f - 1.0f,

    // Walls
    -28.0f,
    -28.0f,
    -28.0f,
    28.0f,
    28.0f,
    28.0f,
};

static const f32 level0_wall_pos_y[] = {
    // Bounds
    0.0f,
    0.0f,
    LEVEL0_BOTTOM - 50.0f,
    LEVEL0_TOP + 50.0f,

    // Base
    0.0f,
    0.0f,

    // Walls
    32.0f - 8.0f,
    0.0f,
    -32.0f + 8.0f,
    32.0f - 8.0f,
    0.0f,
    -32.0f + 8.0f,
};

static const f32 level0_wall_width[] = {
    // Bounds
    100.0f,
    100.0f,
    200.0f,
    200.0f,

    // Base
    12.0f,
    12.0f,

    // Walls
    2.0f,
    2.0f,
    2.0f,
    2.0f,
    2.0f,
    2.0f,
};

static const f32 level0_wall_height[] = {
    // Bounds
    200.0f,
    200.0f,
    100.0f,
    100.0f,

    // Base
    12.0f,
    12.0f,

    // Walls
    16.0f,
    16.0f,
    16.0f,
    16.0f,
    16.0f,
    16.0f,
};
