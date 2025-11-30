
#pragma once

#include "common.h"

#include "platform_win32/shaders/code_gen/shaders.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <d3d11.h>
#include <dxgi1_3.h>

enum StaticMeshType
{
    STATIC_MESH_QUAD,

    NUM_STATIC_MESH_TYPE
};

struct StaticMeshData
{
    ID3D11Buffer* vertex_buffer;
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

#define MAX_QUADS 32768
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

#define MAX_LINES 1024
struct RenderWorldLineData
{
    u32 num;
    f32 start_pos_x[MAX_LINES];
    f32 start_pos_y[MAX_LINES];
    f32 end_pos_x[MAX_LINES];
    f32 end_pos_y[MAX_LINES];
    f32 pos_z[MAX_LINES];
    f32 width[MAX_LINES];
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


    ID3D11InputLayout* shader_input_layouts[NUM_SHADERS];
    ID3D11VertexShader* vertex_shaders[NUM_SHADERS];
    ID3D11PixelShader* pixel_shaders[NUM_SHADERS];
    
    ID3D11Buffer* constant_buffer;

    struct StaticMeshData static_meshes[NUM_STATIC_MESH_TYPE];

    ID3D11Buffer* instance_buffer;
    ID3D11ShaderResourceView* instance_buffer_srv;

    ID3D11Buffer* fsq_vertex_buffer;
    ID3D11SamplerState* fsq_sampler;

    ID3D11RenderTargetView* hdr_fb_render_target_view;
    ID3D11ShaderResourceView* hdr_fb_texture_view;

    ID3D11RenderTargetView* bloom_render_target_view[4];
    ID3D11ShaderResourceView* bloom_texture_view[4];
    ID3D11BlendState* bloom_blend_state[4];

    struct RenderWorldQuadData world_quads;
    struct RenderWorldCircleData world_circles;
    struct RenderWorldLineData world_lines;
};

struct PlatformWin32Render* platform_win32_get_render();

void platform_win32_init_render(struct PlatformWin32Render* mem);

struct Engine;
void platform_win32_render(struct Engine* engine);

void platform_win32_swap_and_clear_buffer(f32 r, f32 g, f32 b);

void platform_win32_add_world_quad(f32 pos_x, f32 pos_y, f32 pos_z, f32 width, f32 height, f32 color_r, f32 color_g, f32 color_b, f32 color_a);

void platform_win32_add_world_circle(f32 pos_x, f32 pos_y, f32 pos_z, f32 radius, f32 color_r, f32 color_g, f32 color_b, f32 color_a);
