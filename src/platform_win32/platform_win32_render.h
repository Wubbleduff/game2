
#pragma once

#include "common.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define COBJMACROS
#include <d3d11.h>
#include <dxgi1_3.h>

enum ShaderType
{
    SHADER_BASIC,
    SHADER_BASIC_COLOR,
    SHADER_CIRCLE_BASIC_COLOR,
    
    NUM_SHADER_TYPE
};

enum StaticMeshType
{
    STATIC_MESH_QUAD,

    NUM_STATIC_MESH_TYPE
};

struct StaticMeshData
{
    ID3D11Buffer* vertex_buffer;
    ID3D11Buffer* instance_buffer;
    ID3D11Buffer* index_buffer;
    u32 num_indices;
};

enum RasterizerStateType
{
    RASTERIZER_STATE_SOLID,
    RASTERIZER_STATE_WIREFRAME,

    NUM_RASTERIZER_STATE_TYPE
};

#define MAX_INSTANCES 32768

#define MAX_QUADS 1024
struct RenderWorldQuadData
{
    u32 num;
    f32 pos_x[MAX_QUADS];
    f32 pos_y[MAX_QUADS];
    f32 pos_z[MAX_QUADS];
    f32 width[MAX_QUADS];
    f32 height[MAX_QUADS];
    f32 color_r[MAX_QUADS];
    f32 color_g[MAX_QUADS];
    f32 color_b[MAX_QUADS];
    f32 color_a[MAX_QUADS];
};

#define MAX_CIRCLES 1024
struct RenderWorldCircleData
{
    u32 num;
    f32 pos_x[MAX_CIRCLES];
    f32 pos_y[MAX_CIRCLES];
    f32 pos_z[MAX_CIRCLES];
    f32 radius[MAX_CIRCLES];
    f32 color_r[MAX_CIRCLES];
    f32 color_g[MAX_CIRCLES];
    f32 color_b[MAX_CIRCLES];
    f32 color_a[MAX_CIRCLES];
};

struct PlatformWin32Render
{
    ID3D11Device* device;
    ID3D11DeviceContext* device_context;
    IDXGISwapChain1* swap_chain;
    ID3D11RasterizerState* rasterizer_states[NUM_RASTERIZER_STATE_TYPE];
    ID3D11DepthStencilState* depth_state;
    ID3D11RenderTargetView* render_target_view;
    ID3D11DepthStencilView* depth_stencil_view;

    ID3D11InputLayout* layout;

    ID3D11VertexShader* vertex_shaders[NUM_SHADER_TYPE];
    ID3D11PixelShader* pixel_shaders[NUM_SHADER_TYPE];
    
    ID3D11Buffer* constant_buffer;

    struct StaticMeshData static_meshes[NUM_STATIC_MESH_TYPE];

    struct RenderWorldQuadData world_quads;
    struct RenderWorldCircleData world_circles;
};

struct PlatformWin32Render* platform_win32_get_render();

void platform_win32_init_render(struct PlatformWin32Render* mem);

struct GameState;
void platform_win32_render(struct GameState* game_state);

void platform_win32_swap_and_clear_buffer(u8 r, u8 g, u8 b);
