
#include "path_find.h"
#include "level.h"
#include "math.h"

static void path_find_set_wall(struct PathFind* path_find, u8 bl_x, u8 bl_y, u16 w, u16 h)
{
    for(u64 y = bl_y; y < bl_y + h; y++)
    {
        for(u64 x = bl_x; x < bl_x + w; x++)
        {
            const u64 idx = y * 256ULL + x;
            const u64 byte = idx / 8;
            const u64 bit = idx % 8;
            path_find->grid[byte] |= 1ULL << bit;
        }
    }
}

static u8 is_open_cell(const struct PathFind* path_find, const u8 x, const u8 y)
{
    const u64 idx = (u64)y * 256ULL + (u64)x;
    const u64 byte = idx / 8;
    const u64 bit = idx % 8;
    return !(path_find->grid[byte] & (1ULL << bit));
}

void init_path_find(struct PathFind* path_find, 
                    const struct Level* level)
{
    ZERO_ARRAY(path_find->grid);

    const s32 level_hw = level->width / 2;
    const s32 level_hh = level->height / 2;
    for(u64 i = 0; i < level->num_walls; i++)
    {
        const struct LevelWall* wall = &level->walls[i];
        const s32 gx0 = clamp_s32(level_hw + wall->x,           0, 255);
        const s32 gx1 = clamp_s32(level_hw + wall->x + wall->w, 0, 255);
        const s32 gy0 = clamp_s32(level_hh + wall->y,           0, 255);
        const s32 gy1 = clamp_s32(level_hh + wall->y + wall->h, 0, 255);

        path_find_set_wall(path_find, (u8)gx0, (u8)gy0, (u8)(gx1 - gx0), (u8)(gy1 - gy0));
    }
}

u32 run_path_find(
    struct PathFind* path_find,
    s32* r_path_x,
    s32* r_path_y,
    const struct Level* level,
    const s32 start_x,
    const s32 start_y,
    const s32 end_x,
    const s32 end_y)
{
    const s32 level_hw = level->width / 2;
    const s32 level_hh = level->height / 2;
    const u8 grid_start_x = (u8)(clamp_s32(start_x, -level_hw, level_hw - 1) + level_hw);
    const u8 grid_start_y = (u8)(clamp_s32(start_y, -level_hh, level_hh - 1) + level_hh);
    const u8 maybe_grid_end_x = (u8)(clamp_s32(end_x, -level_hw, level_hw - 1) + level_hw);
    const u8 maybe_grid_end_y = (u8)(clamp_s32(end_y, -level_hh, level_hh - 1) + level_hh);

    ASSERT(is_open_cell(path_find, grid_start_x, grid_start_y), "Invalid starting cell for path_find.");

    u8 grid_end_x = maybe_grid_end_x;
    u8 grid_end_y = maybe_grid_end_y;
    if(!is_open_cell(path_find, maybe_grid_end_x, maybe_grid_end_y))
    {
        u32 found = 0;
        const s32 dirs[4][2] = {
            { -1,  0 },
            {  1,  0 },
            {  0, -1 },
            {  0,  1 },
        };
        // For now, the closest end cell will be in one of the cardinal directions.
        for(s32 iter = 0; iter < 256; iter++)
        {
            for(u64 i_dir = 0; i_dir < 4; i_dir++)
            {
                s32 test_x = (s32)maybe_grid_end_x + dirs[i_dir][0] * iter;
                s32 test_y = (s32)maybe_grid_end_y + dirs[i_dir][1] * iter;

                if(test_x >= 0 && test_x < 256 && is_open_cell(path_find, (u8)test_x, (u8)test_y))
                {
                    grid_end_x = (u8)test_x;
                    grid_end_y = (u8)test_y;
                    found = 1;
                    break;
                }
            }
            if(found)
            {
                break;
            }
        }
        if(!found)
        {
            ASSERT(0, "Could not find any valid path end point for (%i, %i). Check level geometry", (s32)maybe_grid_end_x, (s32)maybe_grid_end_y);
            return 0;
        }
    }

    path_find->num_open_list = 0;
    for(u64 i = 0; i < ARRAY_COUNT(path_find->grid_dist); i++)
    {
        path_find->grid_dist[i] = INFINITY;
    }

    {
        const u8 cur_x = grid_start_x;
        const u8 cur_y = grid_start_y;
        const u16 cur_idx = (u16)((u64)cur_y * 256ULL + (u64)cur_x);
        path_find->num_open_list = 1;
        path_find->open_list[0] = cur_idx;
        path_find->grid_dist[cur_idx] = 0.0f;
        path_find->grid_prev[cur_idx] = cur_idx;
    }

    while(1)
    {
        u32 num_open_list = path_find->num_open_list;

        if(num_open_list == 0)
        {
            return 0;
        }

        u16 i_min = 0;
        f32 min_val = INFINITY;
        for(u64 i = 0; i < num_open_list; i++)
        {
            const u16 idx = path_find->open_list[i];
            const u8 x = (u8)((idx >> 0U) & 0xFF);
            const u8 y = (u8)((idx >> 8U) & 0xFF);
            const f32 hdist = sqrt_f32(sq_f32((f32)x - (f32)grid_end_x) + sq_f32((f32)y - (f32)grid_end_y));

            const f32 d = path_find->grid_dist[idx] + hdist;

            i_min = d < min_val ? (u16)i : i_min;
            min_val = d < min_val ? d : min_val;
        }
        ASSERT(min_val != INFINITY, "Bad val");
        ASSERT(i_min < num_open_list, "Bad val");

        const u16 cur_idx = path_find->open_list[i_min];
        const u8 cur_x = (u8)((cur_idx >> 0U) & 0xFF);
        const u8 cur_y = (u8)((cur_idx >> 8U) & 0xFF);
        const f32 cur_dist = path_find->grid_dist[cur_idx];

        path_find->open_list[i_min] = path_find->open_list[num_open_list - 1];
        num_open_list--;

        if(cur_x == grid_end_x && cur_y == grid_end_y)
        {
            u32 r_num_path = 0;

            u16 rewind_idx = cur_idx;
            while(rewind_idx != (grid_start_y * 256 + grid_start_x))
            {
                ASSERT(r_num_path < MAX_PATH_LEN, "Path find result path overflow.");

                const u8 x = (u8)(rewind_idx & 0xFF);
                const u8 y = (u8)(rewind_idx >> 8);

                r_path_x[r_num_path] = (s32)x - level_hw;
                r_path_y[r_num_path] = (s32)y - level_hh;

                r_num_path++;

                rewind_idx = path_find->grid_prev[rewind_idx];
            }
            // Add the start node.
            {
                ASSERT(r_num_path < MAX_PATH_LEN, "Path find result path overflow.");

                const u8 x = (u8)(rewind_idx & 0xFF);
                const u8 y = (u8)(rewind_idx >> 8);

                r_path_x[r_num_path] = (s32)x - level_hw;
                r_path_y[r_num_path] = (s32)y - level_hh;

                r_num_path++;
            }

            for(u64 i = 0; i < r_num_path / 2; i++)
            {
                swap_s32(&r_path_x[i], &r_path_x[r_num_path - 1 - i]);
                swap_s32(&r_path_y[i], &r_path_y[r_num_path - 1 - i]);
            }

            return r_num_path;
        }

        const s64 dirs[8][2] =
        {
            { -1, -1 },
            {  0, -1 },
            {  1, -1 },
            { -1,  0 },
            {  1,  0 },
            { -1,  1 },
            {  0,  1 },
            {  1,  1 },
        };
        const f32 dists[8] =
        {
            1.5f,
            1.0f,
            1.5f,
            1.0f,
            1.0f,
            1.5f,
            1.0f,
            1.5f,
        };

        for(u64 i_dir = 0; i_dir < 8; i_dir++)
        {
            const s64 n_x = (s64)cur_x + dirs[i_dir][0];
            const s64 n_y = (s64)cur_y + dirs[i_dir][1];
            const u16 n_idx = (u16)((u64)n_y * 256ULL + (u64)n_x);
            const f32 n_dist = cur_dist + dists[i_dir];
            const u32 add =
                n_x >= 0 &&
                n_x < 256 &&
                n_y >= 0 &&
                n_y < 256 &&
                is_open_cell(path_find, (u8)n_x, (u8)n_y) &&
                path_find->grid_dist[n_idx] > n_dist;
            ASSERT(!add || num_open_list < ARRAY_COUNT(path_find->open_list), "Path find open list overflow.");
            path_find->grid_dist[n_idx] = add ? n_dist : path_find->grid_dist[n_idx];
            path_find->grid_prev[n_idx] = add ? cur_idx : path_find->grid_prev[n_idx];
            path_find->open_list[num_open_list] = n_idx;
            num_open_list += add;
        }

        path_find->num_open_list = num_open_list;
    }
}
