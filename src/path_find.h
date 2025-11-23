
#pragma once

#include "common.h"

#define MAX_PATH_LEN 4096

struct PathFind
{
    // 256x256 grid pathfind

    // Bitarray of valid cells.
    u8 grid[256 * 256 / 8 + 4];
    u32 grid_dist[256 * 256];
    u16 grid_prev[256 * 256];

    u32 num_open_list;
    u16 open_list[65536 + 8];
    u32 open_list_f_dist[65536 + 8];
};

struct Level;
void init_path_find(struct PathFind* path_find, const struct Level* level);

u32 run_path_find(
    struct PathFind* path_find,
    s32* r_path_x,
    s32* r_path_y,
    const struct Level* level,
    const s32 start_x,
    const s32 start_y,
    const s32 end_x,
    const s32 end_y);
