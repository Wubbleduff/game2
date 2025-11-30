
#include "path_find.h"
#include "level.h"
#include "math.h"

#include "debug_draw.h"

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
        const struct LevelWallGeometry* wall = &level->walls[i];
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
        path_find->grid_dist[i] = s32_MAX;
    }

    {
        const u8 cur_x = grid_start_x;
        const u8 cur_y = grid_start_y;
        const u16 cur_idx = (u16)((u64)cur_y * 256ULL + (u64)cur_x);
        path_find->num_open_list = 1;
        path_find->open_list[0] = cur_idx;
        path_find->open_list_f_dist[0] = 0;
        path_find->grid_dist[cur_idx] = 0;
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
        u32 min_val = u32_MAX;

        if(num_open_list < 8)
        {
            for(u64 i = 0; i < num_open_list; i++)
            {
                const u32 d = path_find->open_list_f_dist[i];
                i_min = d < min_val ? (u16)i : i_min;
                min_val = d < min_val ? d : min_val;
            }
        }
        else
        {
            __m256i i_min8 = _mm256_set1_epi32(0);
            // Use s32_MAX because _mm256_min_epi32 is a signed compare.
            __m256i min_val8 = _mm256_set1_epi32(s32_MAX);
            for(s32 i = 0; i < (s32)num_open_list - 8; i += 8)
            {
                const __m256i i8 = _mm256_setr_epi32(i + 0, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7);
                const __m256i d = _mm256_loadu_si256((__m256i*)(path_find->open_list_f_dist + i));
                const __m256i mask = _mm256_cmpgt_epi32(min_val8, d);
                i_min8 =
                    _mm256_blendv_epi8(
                        i_min8, // 0
                        i8, // 1
                        mask
                );
                min_val8 = _mm256_min_epi32(min_val8, d);
            }
            {
                const s32 i = (s32)num_open_list - 8;
                const __m256i i8 = _mm256_setr_epi32(i + 0, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7);
                const __m256i d = _mm256_loadu_si256((__m256i*)(path_find->open_list_f_dist + i));
                const __m256i mask = _mm256_cmpgt_epi32(min_val8, d);
                i_min8 =
                    _mm256_blendv_epi8(
                        i_min8, // 0
                        i8, // 1
                        mask
                );
                min_val8 = _mm256_min_epi32(min_val8, d);
            }
            u32 min_val8_array[8];
            _mm256_storeu_si256((__m256i*)min_val8_array, min_val8);
            u32 i_min8_array[8];
            _mm256_storeu_si256((__m256i*)i_min8_array, i_min8);
            for(u64 i = 0; i < 8; i++)
            {
                if(min_val8_array[i] < min_val)
                {
                    min_val = min_val8_array[i];
                    i_min = (u16)i_min8_array[i];
                }
            }
        }
        ASSERT(min_val != u32_MAX, "Bad val");
        ASSERT(i_min < num_open_list, "Bad val");

        const u16 cur_idx = path_find->open_list[i_min];
        const u8 cur_x = (u8)((cur_idx >> 0U) & 0xFF);
        const u8 cur_y = (u8)((cur_idx >> 8U) & 0xFF);
        const u32 cur_dist = path_find->grid_dist[cur_idx];

        path_find->open_list[i_min] = path_find->open_list[num_open_list - 1];
        path_find->open_list_f_dist[i_min] = path_find->open_list_f_dist[num_open_list - 1];
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

        #if 1
        {
            const __m256i cur_x8 = _mm256_set1_epi32(cur_x);
            const __m256i cur_y8 = _mm256_set1_epi32(cur_y);
            const __m256i cur_dist8 = _mm256_set1_epi32(cur_dist);
        
            const __m256i n_x = _mm256_add_epi32(cur_x8, _mm256_setr_epi32(-1,  0,  1, -1,  1, -1,  0,  1));
            const __m256i n_y = _mm256_add_epi32(cur_y8, _mm256_setr_epi32(-1, -1, -1,  0,  0,  1,  1,  1));
            const __m256i n_idx = _mm256_add_epi32(_mm256_slli_epi32(n_y, 8), n_x);
            const __m256i n_dist = _mm256_add_epi32(cur_dist8, _mm256_setr_epi32(1500, 1000, 1500, 1000, 1000, 1500, 1000, 1500));

            const __m256i grid_end_x8 = _mm256_set1_epi32(grid_end_x);
            const __m256i grid_end_y8 = _mm256_set1_epi32(grid_end_y);
            __m256i n_hdist = 
                _mm256_max_epi32(
                    _mm256_abs_epi32(_mm256_sub_epi32(n_x, grid_end_x8)),
                    _mm256_abs_epi32(_mm256_sub_epi32(n_y, grid_end_y8))
                    );
            n_hdist = _mm256_mullo_epi32(n_hdist, _mm256_set1_epi32(1000));
        
            __m256i mask = _mm256_set1_epi8(0xFF);
        
            // x >= 0  ->  x > -1
            mask = _mm256_and_si256(mask, _mm256_cmpgt_epi32(n_x, _mm256_set1_epi32(-1)));
            // x < 256  ->  256 > x
            mask = _mm256_and_si256(mask, _mm256_cmpgt_epi32(_mm256_set1_epi32(256), n_x));
            // y >= 0  ->  y > -1
            mask = _mm256_and_si256(mask, _mm256_cmpgt_epi32(n_y, _mm256_set1_epi32(-1)));
            // y < 256  ->  256 > y
            mask = _mm256_and_si256(mask, _mm256_cmpgt_epi32(_mm256_set1_epi32(256), n_y));
        
            const __m256i existing_dists =
                _mm256_mask_i32gather_epi32(
                    _mm256_set1_epi32(u32_MAX),
                    (s32*)&path_find->grid_dist[0],
                    n_idx,
                    mask,
                    4);
            mask = _mm256_and_si256(mask, _mm256_cmpgt_epi32(existing_dists, n_dist));

            // cell = path->grid[n_idx / 8]
            __m256i cell =
                _mm256_mask_i32gather_epi32(
                    _mm256_set1_epi32(u32_MAX),
                    (s32*)&path_find->grid[0],
                    _mm256_srli_epi32(n_idx, 3),
                    mask,
                    1);
            // cell_mask = 1 << (n_idx % 8)
            __m256i cell_mask = _mm256_sllv_epi32(
                _mm256_set1_epi32(1),
                _mm256_and_si256(n_idx, _mm256_set1_epi32(7)));
            // cell = cell & cell_mask
            cell = _mm256_and_si256(cell, cell_mask);
            // cell = cell == 0;
            cell = _mm256_cmpeq_epi32(cell, _mm256_set1_epi32(0));
            mask = _mm256_and_si256(mask, cell);

            u32 scalar_mask[8];
            _mm256_storeu_si256((__m256i*)scalar_mask, mask);
            u32 scalar_n_idx[8];
            _mm256_storeu_si256((__m256i*)scalar_n_idx, n_idx);
            u32 scalar_n_dist[8];
            _mm256_storeu_si256((__m256i*)scalar_n_dist, n_dist);
            u32 scalar_n_hdist[8];
            _mm256_storeu_si256((__m256i*)scalar_n_hdist, n_hdist);
            for(u64 i = 0; i < 8; i++)
            {
                const u32 add = scalar_mask[i] != 0;
                if(add)
                {
                    ASSERT(num_open_list < ARRAY_COUNT(path_find->open_list), "Path find open list overflow.");
                    path_find->grid_dist[scalar_n_idx[i]] = scalar_n_dist[i];
                    path_find->grid_prev[scalar_n_idx[i]] = cur_idx;
                    path_find->open_list[num_open_list] = (u16)scalar_n_idx[i];
                    path_find->open_list_f_dist[num_open_list] = scalar_n_dist[i] + scalar_n_hdist[i];
                    num_open_list++;
                }
            }
        }
        #endif
        
        #if 0
        for(u64 i_dir = 0; i_dir < 8; i_dir++)
        {
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
            const s64 n_x = (s64)cur_x + dirs[i_dir][0];
            const s64 n_y = (s64)cur_y + dirs[i_dir][1];
            const u16 n_idx = (u16)((u64)n_y * 256ULL + (u64)n_x);
            const u32 dists[8] =
            {
                1500,
                1000,
                1500,
                1000,
                1000,
                1500,
                1000,
                1500,
            };
            const u32 n_dist = cur_dist + dists[i_dir];
            const u32 n_hdist = 
                (u32)max_s32(
                    abs_s32((s32)n_x - (s32)grid_end_x), 
                    abs_s32((s32)n_y - (s32)grid_end_y)
                ) * 1000;
            u32 mask = 1;
            
            mask = mask && (n_x >= 0);
            mask = mask && (n_x < 256);
            mask = mask && (n_y >= 0);
            mask = mask && (n_y < 256);
            mask = mask && is_open_cell(path_find, (u8)n_x, (u8)n_y);
            mask = mask && path_find->grid_dist[n_idx] > n_dist;
            const u32 add = mask;
            ASSERT(!add || num_open_list < ARRAY_COUNT(path_find->open_list), "Path find open list overflow.");
            path_find->grid_dist[n_idx] = add ? n_dist : path_find->grid_dist[n_idx];
            path_find->grid_prev[n_idx] = add ? cur_idx : path_find->grid_prev[n_idx];
            path_find->open_list[num_open_list] = n_idx;
            path_find->open_list_f_dist[num_open_list] = n_dist + n_hdist;
            num_open_list += add;
        }
        #endif

        path_find->num_open_list = num_open_list;
    }
}
