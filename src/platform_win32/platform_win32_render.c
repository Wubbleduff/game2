
#include "math.h"
#include "engine.h"
#include "platform.h"
#include "debug_draw.h"

// TODO: remove
#include "level0.h"

#include "platform_win32/platform_win32_render.h"
#include "platform_win32/platform_win32_core.h"

#include "platform_win32/include_shaders.h"

struct PlatformWin32Render* g_platform_win32_render;

struct Vertex
{
    f32 pos[2];
    f32 tex_coord[2];
};

struct FsqVertex
{
    f32 pos[2];
    f32 tex_coord[2];
};

struct InstanceData
{
    f32 xform[3][4];
    f32 color[4];
};

struct PlatformWin32Render* platform_win32_get_render()
{
    return g_platform_win32_render;
}

static void create_static_mesh_data(
    struct StaticMeshData* out_mesh_data,
    ID3D11Device* device,
    const u32 vertex_buf_size,
    const void* init_vertex_data,
    const u32 index_buf_size,
    const void* init_index_data,
    const u32 num_indices,
    const u32 instance_buf_size)
{
    {
        D3D11_BUFFER_DESC buffer_desc = {
            .ByteWidth = vertex_buf_size,
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_VERTEX_BUFFER,
        };
        D3D11_SUBRESOURCE_DATA init_data = { .pSysMem = init_vertex_data };
        HRESULT hr = ID3D11Device_CreateBuffer(
            device,
            &buffer_desc,
            &init_data,
            &out_mesh_data->vertex_buffer
        );
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        D3D11_BUFFER_DESC buffer_desc = {
            .ByteWidth = instance_buf_size,
            .Usage = D3D11_USAGE_DYNAMIC,
            .BindFlags = D3D11_BIND_VERTEX_BUFFER,
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
        };
        HRESULT hr = ID3D11Device_CreateBuffer(
            device,
            &buffer_desc,
            NULL,
            &out_mesh_data->instance_buffer
        );
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        D3D11_BUFFER_DESC buffer_desc = {
            .ByteWidth = index_buf_size,
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_INDEX_BUFFER,
        };
        D3D11_SUBRESOURCE_DATA buffer_init_data = { .pSysMem = init_index_data };
        HRESULT hr = ID3D11Device_CreateBuffer(
            device,
            &buffer_desc,
            &buffer_init_data,
            &out_mesh_data->index_buffer
        );
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
        out_mesh_data->num_indices = num_indices;
    }
}

void platform_win32_init_render(struct PlatformWin32Render* mem)
{
    g_platform_win32_render = mem;

    struct PlatformWin32Core* win32_core = platform_win32_get_core();
    struct PlatformWin32Render* win32_render = platform_win32_get_render();

    HRESULT hr;
    
    {
        UINT flags = 0;
        D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
        hr = D3D11CreateDevice(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            flags,
            levels,
            ARRAY_COUNT(levels),
            D3D11_SDK_VERSION,
            &win32_render->device,
            NULL,
            &win32_render->device_context);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        IDXGIDevice* dxgi_device;
        hr = ID3D11Device_QueryInterface(win32_render->device, &IID_IDXGIDevice, (void**)&dxgi_device);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");

        IDXGIAdapter* dxgi_adapter;
        hr = IDXGIDevice_GetAdapter(dxgi_device, &dxgi_adapter);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");

        IDXGIFactory2* dxgi_factory;
        hr = IDXGIAdapter_GetParent(dxgi_adapter, &IID_IDXGIFactory2, (void**)&dxgi_factory);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");

        // https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_desc1
        DXGI_SWAP_CHAIN_DESC1 desc =
        {
            .Width = win32_core->client_width,
            .Height = win32_core->client_height,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = { 1, 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,
            .Scaling = DXGI_SCALING_NONE,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        };

        hr = IDXGIFactory2_CreateSwapChainForHwnd(dxgi_factory, (IUnknown*)win32_render->device, win32_core->hwnd, &desc, NULL, NULL, &win32_render->swap_chain);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");

        hr = IDXGIFactory_MakeWindowAssociation(dxgi_factory, win32_core->hwnd, DXGI_MWA_NO_ALT_ENTER);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");

        hr = IDXGIFactory2_Release(dxgi_factory);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
        hr = IDXGIAdapter_Release(dxgi_adapter);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
        hr = IDXGIDevice_Release(dxgi_device);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        _Static_assert(NUM_RASTERIZER_STATE_TYPE == 2, "Inconsistent size.");

        D3D11_RASTERIZER_DESC solid_desc =
        {
            .FillMode = D3D11_FILL_SOLID,
            .CullMode = D3D11_CULL_BACK,
            .FrontCounterClockwise = TRUE,
            .DepthClipEnable = TRUE,
        };
        hr = ID3D11Device_CreateRasterizerState(win32_render->device, &solid_desc, &win32_render->rasterizer_states[RASTERIZER_STATE_SOLID]);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");

        D3D11_RASTERIZER_DESC wireframe_desc =
        {
            .FillMode = D3D11_FILL_WIREFRAME,
            .CullMode = D3D11_CULL_NONE,
            .FrontCounterClockwise = TRUE,
            .DepthClipEnable = TRUE,
        };
        hr = ID3D11Device_CreateRasterizerState(win32_render->device, &wireframe_desc, &win32_render->rasterizer_states[RASTERIZER_STATE_WIREFRAME]);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        D3D11_DEPTH_STENCIL_DESC desc =
        {
            .DepthEnable = TRUE,
            .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
            .DepthFunc = D3D11_COMPARISON_LESS,
            .StencilEnable = FALSE,
            .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
            .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
        };
        hr = ID3D11Device_CreateDepthStencilState(win32_render->device, &desc, &win32_render->depth_state);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        ID3D11Texture2D* back_buffer;
        hr = IDXGISwapChain1_GetBuffer(win32_render->swap_chain, 0, &IID_ID3D11Texture2D, (void* *)&back_buffer);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
        hr = ID3D11Device_CreateRenderTargetView(win32_render->device, (ID3D11Resource*)back_buffer, NULL, &win32_render->render_target_view);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        D3D11_TEXTURE2D_DESC fb_desc =
        {
            .Width = win32_core->client_width,
            .Height = win32_core->client_height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_R16G16B16A16_FLOAT,
            .SampleDesc = { 1, 0 },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
        };
        ID3D11Texture2D* fb_texture;
        hr = ID3D11Device_CreateTexture2D(win32_render->device, &fb_desc, NULL, &fb_texture);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");

        D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc =
        {
            .Format = DXGI_FORMAT_R16G16B16A16_FLOAT,
            .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
            .Texture2D = {
                .MipSlice = 0,
            },
        };
        hr = ID3D11Device_CreateRenderTargetView(win32_render->device, (ID3D11Resource*)fb_texture, &render_target_view_desc, &win32_render->fsq_render_target_view);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
        hr = ID3D11Texture2D_Release(fb_texture);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");

        hr = ID3D11Device_CreateShaderResourceView(
            win32_render->device,
            (ID3D11Resource*)fb_texture,
            NULL,
            &win32_render->fb_texture_view
        );
        ASSERT(SUCCEEDED(hr), "d3d11 error.");

        D3D11_TEXTURE2D_DESC depth_desc =
        {
            .Width = win32_core->client_width,
            .Height = win32_core->client_height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D32_FLOAT,
            .SampleDesc = { 1, 0 },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
        };
        ID3D11Texture2D* depth;
        hr = ID3D11Device_CreateTexture2D(win32_render->device, &depth_desc, NULL, &depth);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
        hr = ID3D11Device_CreateDepthStencilView(win32_render->device, (ID3D11Resource*)depth, NULL, &win32_render->depth_stencil_view);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
        hr = ID3D11Texture2D_Release(depth);
    }

    {
        D3D11_BUFFER_DESC buffer_desc = {
            .ByteWidth = sizeof(f32) * 4 * 4,
            .Usage = D3D11_USAGE_DYNAMIC,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
        };
        hr = ID3D11Device_CreateBuffer(
            win32_render->device,
            &buffer_desc,
            0,
            &win32_render->constant_buffer
        );
    }

    const u32 static_instance_data_buffer_size = sizeof(struct InstanceData) * MAX_INSTANCES;

    {
        const struct Vertex quad_verts[] = {
            {
                .pos = { -0.5f, -0.5f, }
            },
            {
                .pos = { 0.5f, -0.5f, }
            },
            {
                .pos = { -0.5f, 0.5f, }
            },
            {
                .pos = { 0.5f, 0.5f, }
            },
        };
        const u32 quad_indices[] = {
            0, 1, 2,
            2, 1, 3,
        };
        create_static_mesh_data(
            &win32_render->static_meshes[STATIC_MESH_QUAD],
            win32_render->device,
            sizeof(quad_verts),
            quad_verts,
            sizeof(quad_indices),
            quad_indices,
            ARRAY_COUNT(quad_indices),
            static_instance_data_buffer_size
        );
    }

    {
        const struct Vertex fsq_quad_verts[] = {
            {
                .pos = { -1.0f, -1.0f, },
                .tex_coord = { 0.0f, 1.0f, },
            },
            {
                .pos = { 1.0f, -1.0f, },
                .tex_coord = { 1.0f, 1.0f, },
            },
            {
                .pos = { -1.0f, 1.0f, },
                .tex_coord = { 0.0f, 0.0f, },
            },
            {
                .pos = { 1.0f, 1.0f, },
                .tex_coord = { 1.0f, 0.0f, },
            },
        };

        D3D11_BUFFER_DESC buffer_desc = {
            .ByteWidth = sizeof(fsq_quad_verts),
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_VERTEX_BUFFER,
        };
        D3D11_SUBRESOURCE_DATA init_data = { .pSysMem = fsq_quad_verts };
        hr = ID3D11Device_CreateBuffer(
            win32_render->device,
            &buffer_desc,
            &init_data,
            &win32_render->fsq_vertex_buffer
        );
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        D3D11_SAMPLER_DESC sampler_desc = {
            .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
            .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
            .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
        };
        hr = ID3D11Device_CreateSamplerState(
            win32_render->device,
            &sampler_desc,
            &win32_render->fsq_sampler
        );
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        D3D11_INPUT_ELEMENT_DESC layout_descs[] = {
            {
                .SemanticName = "POSITION",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
            },
            {
                .SemanticName = "TEXCOORD",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
            },
        };
        hr = ID3D11Device_CreateInputLayout(win32_render->device,
                                       layout_descs,
                                       ARRAY_COUNT(layout_descs),
                                       d3d11_vshader_fsq,
                                       sizeof(d3d11_vshader_fsq),
                                       &win32_render->fsq_layout);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }

    {
        D3D11_INPUT_ELEMENT_DESC layout_descs[] = {
            {
                .SemanticName = "POSITION",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
            },
            {
                .SemanticName = "TEXCOORD",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
            },
            {
                .SemanticName = "POSITION",
                .SemanticIndex = 1,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 1,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 1,
            },
            {
                .SemanticName = "POSITION",
                .SemanticIndex = 2,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 1,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 1,
            },
            {
                .SemanticName = "POSITION",
                .SemanticIndex = 3,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 1,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 1,
            },
            {
                .SemanticName = "COLOR",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 1,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 1,
            },
        };
        hr = ID3D11Device_CreateInputLayout(win32_render->device,
                                       layout_descs,
                                       ARRAY_COUNT(layout_descs),
                                       d3d11_vshader_basic,
                                       sizeof(d3d11_vshader_basic),
                                       &win32_render->layout);
        ASSERT(SUCCEEDED(hr), "d3d11 error.");
    }
    
    {

        const void* vertex_shader_byte_code[] = {
            d3d11_vshader_fsq,
            d3d11_vshader_basic,
            d3d11_vshader_basic_color,
            d3d11_vshader_circle_basic_color,
            d3d11_vshader_line_basic_color,
        };
        _Static_assert(ARRAY_COUNT(vertex_shader_byte_code) == NUM_SHADER_TYPE, "Inconsistent size.");
        const u64 vertex_shader_byte_code_size[] = {
            sizeof(d3d11_vshader_fsq),
            sizeof(d3d11_vshader_basic),
            sizeof(d3d11_vshader_basic_color),
            sizeof(d3d11_vshader_circle_basic_color),
            sizeof(d3d11_vshader_line_basic_color),
        };
        _Static_assert(ARRAY_COUNT(vertex_shader_byte_code_size) == NUM_SHADER_TYPE, "Inconsistent size.");
        const void* pixel_shader_byte_code[] = {
            d3d11_pshader_fsq,
            d3d11_pshader_basic,
            d3d11_pshader_basic_color,
            d3d11_pshader_circle_basic_color,
            d3d11_pshader_line_basic_color,
        };
        _Static_assert(ARRAY_COUNT(pixel_shader_byte_code) == NUM_SHADER_TYPE, "Inconsistent size.");
        const u64 pixel_shader_byte_code_size[] = {
            sizeof(d3d11_pshader_fsq),
            sizeof(d3d11_pshader_basic),
            sizeof(d3d11_pshader_basic_color),
            sizeof(d3d11_pshader_circle_basic_color),
            sizeof(d3d11_pshader_line_basic_color),
        };
        _Static_assert(ARRAY_COUNT(pixel_shader_byte_code_size) == NUM_SHADER_TYPE, "Inconsistent size.");

        for(u64 i = 0; i < NUM_SHADER_TYPE; i++)
        {
            hr = ID3D11Device_CreateVertexShader(win32_render->device,
                                                 vertex_shader_byte_code[i],
                                                 vertex_shader_byte_code_size[i],
                                                 NULL,
                                                 &win32_render->vertex_shaders[i]);
            ASSERT(SUCCEEDED(hr), "d3d11 error.");
            hr = ID3D11Device_CreatePixelShader(win32_render->device,
                                                pixel_shader_byte_code[i],
                                                pixel_shader_byte_code_size[i],
                                                NULL,
                                                &win32_render->pixel_shaders[i]);
            ASSERT(SUCCEEDED(hr), "d3d11 error.");
        }
    }

    win32_render->world_quads.num = 0;
    win32_render->world_circles.num = 0;
    win32_render->world_lines.num = 0;
}

void platform_win32_add_world_quad(
    f32 pos_x,
    f32 pos_y,
    f32 pos_z,
    f32 width,
    f32 height,
    f32 color_r,
    f32 color_g,
    f32 color_b,
    f32 color_a)
{
    struct PlatformWin32Render* win32_render = platform_win32_get_render();

    struct RenderWorldQuadData* data = &win32_render->world_quads;
    const u32 num = data->num;
    ASSERT(num < ARRAY_COUNT(data->pos_x), "'platform_win32_add_world_quad' overflow %u", ARRAY_COUNT(data->pos_x));
    data->pos_x[num] = pos_x;
    data->pos_y[num] = pos_y;
    data->pos_z[num] = pos_z;
    data->width[num] = width;
    data->height[num] = height;
    data->color_r[num] = color_r;
    data->color_g[num] = color_g;
    data->color_b[num] = color_b;
    data->color_a[num] = color_a;
    data->num++;
}

void platform_win32_add_world_circle(
    f32 pos_x,
    f32 pos_y,
    f32 pos_z,
    f32 radius,
    f32 color_r,
    f32 color_g,
    f32 color_b,
    f32 color_a)
{
    struct PlatformWin32Render* win32_render = platform_win32_get_render();

    struct RenderWorldCircleData* data = &win32_render->world_circles;
    const u32 num = data->num;
    ASSERT(num < ARRAY_COUNT(data->pos_x), "'platform_win32_add_world_circle' overflow %u", ARRAY_COUNT(data->pos_x));
    data->pos_x[num] = pos_x;
    data->pos_y[num] = pos_y;
    data->pos_z[num] = pos_z;
    data->radius[num] = radius;
    data->color_r[num] = color_r;
    data->color_g[num] = color_g;
    data->color_b[num] = color_b;
    data->color_a[num] = color_a;
    data->num++;
}

void platform_win32_add_world_line(
    f32 start_pos_x,
    f32 start_pos_y,
    f32 end_pos_x,
    f32 end_pos_y,
    f32 pos_z,
    f32 width,
    f32 color_r,
    f32 color_g,
    f32 color_b,
    f32 color_a)
{
    struct PlatformWin32Render* win32_render = platform_win32_get_render();

    struct RenderWorldLineData* data = &win32_render->world_lines;
    const u32 num = data->num;
    ASSERT(num < ARRAY_COUNT(data->start_pos_x), "'platform_win32_add_world_line' overflow %u", ARRAY_COUNT(data->start_pos_x));
    data->start_pos_x[num] = start_pos_x;
    data->start_pos_y[num] = start_pos_y;
    data->end_pos_x[num] = end_pos_x;
    data->end_pos_y[num] = end_pos_y;
    data->pos_z[num] = pos_z;
    data->width[num] = width;
    data->color_r[num] = color_r;
    data->color_g[num] = color_g;
    data->color_b[num] = color_b;
    data->color_a[num] = color_a;
    data->num++;
}

void debug_draw_add_world_quad(
    f32 pos_x,
    f32 pos_y,
    f32 pos_z,
    f32 width,
    f32 height,
    f32 color_r,
    f32 color_g,
    f32 color_b,
    f32 color_a)
{
    struct PlatformWin32Render* win32_render = platform_win32_get_render();

    struct RenderWorldQuadData* data = &win32_render->world_quads;
    const u32 num = data->num;
    ASSERT(num < ARRAY_COUNT(data->pos_x), "'platform_win32_add_world_quad' overflow %u", ARRAY_COUNT(data->pos_x));
    data->pos_x[num] = pos_x;
    data->pos_y[num] = pos_y;
    data->pos_z[num] = pos_z;
    data->width[num] = width;
    data->height[num] = height;
    data->color_r[num] = color_r;
    data->color_g[num] = color_g;
    data->color_b[num] = color_b;
    data->color_a[num] = color_a;
    data->num++;
}

void platform_win32_render(struct Engine* engine)
{
    struct PlatformWin32Core* win32_core = platform_win32_get_core();    
    struct PlatformWin32Render* win32_render = platform_win32_get_render();

    // Flip prev and next becuse 'tick_engine' updated which is whic.
    struct GameState* next_game_state = &engine->game_states[(engine->cur_game_state_idx + 1) & 1];
    //struct GameState* prev_game_state = &engine->game_states[engine->cur_game_state_idx];

    ID3D11DeviceContext* const device_context = win32_render->device_context;

    D3D11_VIEWPORT viewport =
    {
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width = (f32)win32_core->client_width,
        .Height = (f32)win32_core->client_height,
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };
    ID3D11DeviceContext_RSSetViewports(device_context, 1, &viewport);

    ID3D11DeviceContext_OMSetDepthStencilState(device_context, win32_render->depth_state, 0);

    ID3D11DeviceContext_OMSetRenderTargets(device_context, 1, &win32_render->fsq_render_target_view, win32_render->depth_stencil_view);

    m4x4 cam_proj_m4x4;
    {
        const f32 cam_pos_x = next_game_state->cam_pos_x;
        const f32 cam_pos_y = next_game_state->cam_pos_y;

        const f32 aspect_ratio = platform_get_screen_aspect_ratio();
        const f32 cam_w = next_game_state->cam_w;
        const f32 cam_h = cam_w * aspect_ratio;
        m4x4 cam_translation_m4x4 = {
            .a = {
                { 1.0f, 0.0f, 0.0f, -cam_pos_x },
                { 0.0f, 1.0f, 0.0f, -cam_pos_y },
                { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f, 0.0f, 0.0f, 1.0f },
            }
        };
        m4x4 cam_m4x4 = cam_translation_m4x4;

        m4x4 proj_m4x4 = {
            .a = {
                { 2.0f / cam_w, 0.0f, 0.0f, 0.0f, },
                { 0.0f, 2.0f / cam_h, 0.0f, 0.0f, },
                { 0.0f, 0.0f, 1.0f, 0.0f, },
                { 0.0f, 0.0f, 0.0f, 1.0f, },
            }
        };

        mul_m4x4(&cam_proj_m4x4, &proj_m4x4, &cam_m4x4);

        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            ID3D11Resource* mapped_resource = (ID3D11Resource*)win32_render->constant_buffer;
            HRESULT hr = ID3D11DeviceContext_Map(
                device_context,
                mapped_resource,
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mapped
            );
            ASSERT(SUCCEEDED(hr), "d3d11 error.");
            volatile f32* mapped_data = (volatile f32*)mapped.pData;
            for(u64 i0 = 0; i0 < 4; i0++)
            {
                for(u64 i1 = 0; i1 < 4; i1++)
                {
                    mapped_data[i0 * 4 + i1] = cam_proj_m4x4.a[i1][i0];
                }
            }
            ID3D11DeviceContext_Unmap(device_context, mapped_resource, 0);

            ID3D11DeviceContext_VSSetConstantBuffers(
                device_context,
                0,
                1,
                &win32_render->constant_buffer
            );
        }
    }

    {
        v4 color = make_v4(0.05f, 0.05f, 0.05f, 1.0f);
        for(s64 x = -128; x < 128; x++)
        {
            platform_win32_add_world_quad(
                (f32)x,
                0.0f,
                0.99f,
                0.05f,
                256.0f,
                color.x,
                color.y,
                color.z,
                color.w
            );
        }
        for(s64 y = -128; y < 128; y++)
        {
            platform_win32_add_world_quad(
                0.0f,
                (f32)y,
                0.99f,
                256.0f,
                0.05f,
                color.x,
                color.y,
                color.z,
                color.w
            );
        }
    }

    for(u64 i = 0; i < next_game_state->num_players; i++)
    {
        // v4 color =
        //     next_game_state->player_team_id[i] == 0
        //     ? make_v4(0.0f, 5.8f, 7.0f, 1.0f)
        //     : make_v4(6.0f, 5.4f, 0.0f, 1.0f);

        v4 color =
            next_game_state->player_team_id[i] == 0
            ? make_v4(0.4f, 1.5f, 2.0f, 1.0f)
            : make_v4(2.0f, 0.2f, 0.2f, 1.0f);


        color =
            next_game_state->player_health[i] == 0
            ? scale_v4(color, 0.025f)
            : color;

        platform_win32_add_world_circle(
            next_game_state->player_pos_x[i],
            next_game_state->player_pos_y[i],
            0.5f,
            0.5f,
            color.x,
            color.y,
            color.z,
            color.w
        );
    }

    for(u64 i = 0; i < next_game_state->num_bullets; i++)
    {
        v4 color =
            next_game_state->bullet_team_id[i] == 0
            ? make_v4(0.4f, 1.5f, 2.0f, 1.0f)
            : make_v4(2.0f, 0.2f, 0.2f, 1.0f);

        const f32 prev_pos_x = next_game_state->bullet_prev_pos_x[i];
        const f32 prev_pos_y = next_game_state->bullet_prev_pos_y[i];
        const f32 next_pos_x = next_game_state->bullet_pos_x[i];
        const f32 next_pos_y = next_game_state->bullet_pos_y[i];
        
        platform_win32_add_world_line(
            prev_pos_x,
            prev_pos_y,
            next_pos_x,
            next_pos_y,
            0.5f,
            0.2f,
            color.x,
            color.y,
            color.z,
            color.w
        );
    }

    {
        ASSERT(next_game_state->cur_level == 0, "TODO levels");

        const struct Level* level = &LEVEL0;
        const v4 color = make_v4(0.05f, 0.05f, 0.05f, 1.0f);
        for(u64 i = 0; i < level->num_walls; i++)
        {
            const struct LevelWall* wall = &level->walls[i];
            platform_win32_add_world_quad(
                (f32)wall->x + (f32)wall->w * 0.5f,
                (f32)wall->y + (f32)wall->h * 0.5f,
                0.6f,
                (f32)wall->w,
                (f32)wall->h,
                color.x,
                color.y,
                color.z,
                color.w
            );
        }
        
    }

    {
        const enum StaticMeshType mesh_type = STATIC_MESH_QUAD;
        ID3D11Buffer* vertex_buffer = win32_render->static_meshes[mesh_type].vertex_buffer;
        ID3D11Buffer* instance_buffer = win32_render->static_meshes[mesh_type].instance_buffer;
        ID3D11Buffer* index_buffer = win32_render->static_meshes[mesh_type].index_buffer;
        u32 num_indices = win32_render->static_meshes[mesh_type].num_indices;
        const enum ShaderType shader_type = SHADER_BASIC_COLOR;

        u32 num_instances = 0;
     
        ID3D11DeviceContext_RSSetState(device_context, win32_render->rasterizer_states[RASTERIZER_STATE_SOLID]);
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            ID3D11Resource* mapped_resource = (ID3D11Resource*)instance_buffer;
            HRESULT hr = ID3D11DeviceContext_Map(
                device_context,
                mapped_resource,
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mapped
            );
            ASSERT(SUCCEEDED(hr), "d3d11 error.");
            volatile struct InstanceData* mapped_data = (volatile struct InstanceData*)mapped.pData;
            num_instances = win32_render->world_quads.num;
            for(u64 i = 0; i < num_instances; i++)
            {
                const f32 pos_x = win32_render->world_quads.pos_x[i];
                const f32 pos_y = win32_render->world_quads.pos_y[i];
                const f32 pos_z = win32_render->world_quads.pos_z[i];
                const f32 width = win32_render->world_quads.width[i];
                const f32 height = win32_render->world_quads.height[i];
                const f32 color_r = win32_render->world_quads.color_r[i];
                const f32 color_g = win32_render->world_quads.color_g[i];
                const f32 color_b = win32_render->world_quads.color_b[i];
                const f32 color_a = win32_render->world_quads.color_a[i];

                mapped_data[i].xform[0][0] = width; mapped_data[i].xform[0][1] = 0.0f; mapped_data[i].xform[0][2] = 0.0f; mapped_data[i].xform[0][3] = pos_x;
                mapped_data[i].xform[1][0] = 0.0f; mapped_data[i].xform[1][1] = height; mapped_data[i].xform[1][2] = 0.0f; mapped_data[i].xform[1][3] = pos_y;
                mapped_data[i].xform[2][0] = 0.0f; mapped_data[i].xform[2][1] = 0.0f; mapped_data[i].xform[2][2] = 1.0f; mapped_data[i].xform[2][3] = pos_z;
                mapped_data[i].color[0] = color_r;
                mapped_data[i].color[1] = color_g;
                mapped_data[i].color[2] = color_b;
                mapped_data[i].color[3] = color_a;
            }
     
            ID3D11DeviceContext_Unmap(device_context, mapped_resource, 0);
        }
     
        ID3D11DeviceContext_VSSetShader(device_context, win32_render->vertex_shaders[shader_type], NULL, 0);
        ID3D11DeviceContext_PSSetShader(device_context, win32_render->pixel_shaders[shader_type], NULL, 0);
     
        ID3D11DeviceContext_IASetPrimitiveTopology(device_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ID3D11DeviceContext_IASetInputLayout(device_context, win32_render->layout);
        ID3D11DeviceContext_IASetIndexBuffer(device_context, index_buffer, DXGI_FORMAT_R32_UINT, 0);
        ID3D11Buffer* vertex_buffers[] = {
            vertex_buffer,
            instance_buffer,
        };
        const u32 strides[] = {
            sizeof(struct Vertex),
            sizeof(struct InstanceData),
        };
        const u32 offsets[] = {
            0,
            0,
        };
        ID3D11DeviceContext_IASetVertexBuffers(
            device_context,
            0,
            ARRAY_COUNT(vertex_buffers),
            vertex_buffers,
            strides,
            offsets
        );
        ID3D11DeviceContext_DrawIndexedInstanced(
            device_context,
            num_indices,
            num_instances,
            0,
            0,
            0);

        win32_render->world_quads.num = 0;
    }

    {
        const enum StaticMeshType mesh_type = STATIC_MESH_QUAD;
        ID3D11Buffer* vertex_buffer = win32_render->static_meshes[mesh_type].vertex_buffer;
        ID3D11Buffer* instance_buffer = win32_render->static_meshes[mesh_type].instance_buffer;
        ID3D11Buffer* index_buffer = win32_render->static_meshes[mesh_type].index_buffer;
        u32 num_indices = win32_render->static_meshes[mesh_type].num_indices;
        const enum ShaderType shader_type = SHADER_CIRCLE_BASIC_COLOR;

        u32 num_instances = 0;
     
        ID3D11DeviceContext_RSSetState(device_context, win32_render->rasterizer_states[RASTERIZER_STATE_SOLID]);
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            ID3D11Resource* mapped_resource = (ID3D11Resource*)instance_buffer;
            HRESULT hr = ID3D11DeviceContext_Map(
                device_context,
                mapped_resource,
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mapped
            );
            ASSERT(SUCCEEDED(hr), "d3d11 error.");
            volatile struct InstanceData* mapped_data = (volatile struct InstanceData*)mapped.pData;
            num_instances = win32_render->world_circles.num;
            for(u64 i = 0; i < num_instances; i++)
            {
                const f32 pos_x = win32_render->world_circles.pos_x[i];
                const f32 pos_y = win32_render->world_circles.pos_y[i];
                const f32 pos_z = win32_render->world_circles.pos_z[i];
                const f32 radius = win32_render->world_circles.radius[i];
                const f32 color_r = win32_render->world_circles.color_r[i];
                const f32 color_g = win32_render->world_circles.color_g[i];
                const f32 color_b = win32_render->world_circles.color_b[i];
                const f32 color_a = win32_render->world_circles.color_a[i];

                mapped_data[i].xform[0][0] = radius * 2.0f; mapped_data[i].xform[0][1] = 0.0f; mapped_data[i].xform[0][2] = 0.0f; mapped_data[i].xform[0][3] = pos_x;
                mapped_data[i].xform[1][0] = 0.0f; mapped_data[i].xform[1][1] = radius * 2.0f; mapped_data[i].xform[1][2] = 0.0f; mapped_data[i].xform[1][3] = pos_y;
                mapped_data[i].xform[2][0] = 0.0f; mapped_data[i].xform[2][1] = 0.0f; mapped_data[i].xform[2][2] = 1.0f; mapped_data[i].xform[2][3] = pos_z;
                mapped_data[i].color[0] = color_r;
                mapped_data[i].color[1] = color_g;
                mapped_data[i].color[2] = color_b;
                mapped_data[i].color[3] = color_a;
            }
     
            ID3D11DeviceContext_Unmap(device_context, mapped_resource, 0);
        }
     
        ID3D11DeviceContext_VSSetShader(device_context, win32_render->vertex_shaders[shader_type], NULL, 0);
        ID3D11DeviceContext_PSSetShader(device_context, win32_render->pixel_shaders[shader_type], NULL, 0);
     
        ID3D11DeviceContext_IASetPrimitiveTopology(device_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ID3D11DeviceContext_IASetInputLayout(device_context, win32_render->layout);
        ID3D11DeviceContext_IASetIndexBuffer(device_context, index_buffer, DXGI_FORMAT_R32_UINT, 0);
        ID3D11Buffer* vertex_buffers[] = {
            vertex_buffer,
            instance_buffer,
        };
        const u32 strides[] = {
            sizeof(struct Vertex),
            sizeof(struct InstanceData),
        };
        const u32 offsets[] = {
            0,
            0,
        };
        ID3D11DeviceContext_IASetVertexBuffers(
            device_context,
            0,
            ARRAY_COUNT(vertex_buffers),
            vertex_buffers,
            strides,
            offsets
        );
        ID3D11DeviceContext_DrawIndexedInstanced(
            device_context,
            num_indices,
            num_instances,
            0,
            0,
            0);

        win32_render->world_circles.num = 0;
    }

    {
        const enum StaticMeshType mesh_type = STATIC_MESH_QUAD;
        ID3D11Buffer* vertex_buffer = win32_render->static_meshes[mesh_type].vertex_buffer;
        ID3D11Buffer* instance_buffer = win32_render->static_meshes[mesh_type].instance_buffer;
        ID3D11Buffer* index_buffer = win32_render->static_meshes[mesh_type].index_buffer;
        u32 num_indices = win32_render->static_meshes[mesh_type].num_indices;
        const enum ShaderType shader_type = SHADER_LINE_BASIC_COLOR;

        u32 num_instances = 0;
     
        ID3D11DeviceContext_RSSetState(device_context, win32_render->rasterizer_states[RASTERIZER_STATE_SOLID]);
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            ID3D11Resource* mapped_resource = (ID3D11Resource*)instance_buffer;
            HRESULT hr = ID3D11DeviceContext_Map(
                device_context,
                mapped_resource,
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mapped
            );
            ASSERT(SUCCEEDED(hr), "d3d11 error.");
            volatile struct InstanceData* mapped_data = (volatile struct InstanceData*)mapped.pData;
            num_instances = win32_render->world_lines.num;
            for(u64 i = 0; i < num_instances; i++)
            {
                const f32 start_pos_x = win32_render->world_lines.start_pos_x[i];
                const f32 start_pos_y = win32_render->world_lines.start_pos_y[i];
                const f32 end_pos_x = win32_render->world_lines.end_pos_x[i];
                const f32 end_pos_y = win32_render->world_lines.end_pos_y[i];
                const f32 pos_z = win32_render->world_lines.pos_z[i];
                const f32 width = win32_render->world_lines.width[i];
                const f32 color_r = win32_render->world_lines.color_r[i];
                const f32 color_g = win32_render->world_lines.color_g[i];
                const f32 color_b = win32_render->world_lines.color_b[i];
                const f32 color_a = win32_render->world_lines.color_a[i];

                const f32 mid_x = (start_pos_x + end_pos_x) * 0.5f;
                const f32 mid_y = (start_pos_y + end_pos_y) * 0.5f;
                v2 I = make_v2(end_pos_x - start_pos_x, end_pos_y - start_pos_y);
                const f32 I_len = length_v2(I);
                I = normalize_or_v2(I, zero_v2());
                v2 J = make_v2(-I.y, I.x);
                J = scale_v2(J, width * 0.5f);
                I = scale_v2(I, I_len * 0.5f + width);

                mapped_data[i].xform[0][0] =  I.x; mapped_data[i].xform[0][1] =  J.x; mapped_data[i].xform[0][2] = 0.0f; mapped_data[i].xform[0][3] = mid_x;
                mapped_data[i].xform[1][0] =  I.y; mapped_data[i].xform[1][1] =  J.y; mapped_data[i].xform[1][2] = 0.0f; mapped_data[i].xform[1][3] = mid_y;
                mapped_data[i].xform[2][0] = 0.0f; mapped_data[i].xform[2][1] = 0.0f; mapped_data[i].xform[2][2] = 1.0f; mapped_data[i].xform[2][3] = pos_z;
                mapped_data[i].color[0] = color_r;
                mapped_data[i].color[1] = color_g;
                mapped_data[i].color[2] = color_b;
                mapped_data[i].color[3] = color_a;
            }
     
            ID3D11DeviceContext_Unmap(device_context, mapped_resource, 0);
        }
     
        ID3D11DeviceContext_VSSetShader(device_context, win32_render->vertex_shaders[shader_type], NULL, 0);
        ID3D11DeviceContext_PSSetShader(device_context, win32_render->pixel_shaders[shader_type], NULL, 0);
     
        ID3D11DeviceContext_IASetPrimitiveTopology(device_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ID3D11DeviceContext_IASetInputLayout(device_context, win32_render->layout);
        ID3D11DeviceContext_IASetIndexBuffer(device_context, index_buffer, DXGI_FORMAT_R32_UINT, 0);
        ID3D11Buffer* vertex_buffers[] = {
            vertex_buffer,
            instance_buffer,
        };
        const u32 strides[] = {
            sizeof(struct Vertex),
            sizeof(struct InstanceData),
        };
        const u32 offsets[] = {
            0,
            0,
        };
        ID3D11DeviceContext_IASetVertexBuffers(
            device_context,
            0,
            ARRAY_COUNT(vertex_buffers),
            vertex_buffers,
            strides,
            offsets
        );
        ID3D11DeviceContext_DrawIndexedInstanced(
            device_context,
            num_indices,
            num_instances,
            0,
            0,
            0);

        win32_render->world_lines.num = 0;
    }

    {
        ID3D11Buffer* vertex_buffer = win32_render->fsq_vertex_buffer;
        const enum ShaderType shader_type = SHADER_FSQ;

        ID3D11DeviceContext_OMSetRenderTargets(device_context, 1, &win32_render->render_target_view, win32_render->depth_stencil_view);

        ID3D11DeviceContext_RSSetState(device_context, win32_render->rasterizer_states[RASTERIZER_STATE_SOLID]);
     
        ID3D11DeviceContext_VSSetShader(device_context, win32_render->vertex_shaders[shader_type], NULL, 0);
        ID3D11DeviceContext_PSSetShader(device_context, win32_render->pixel_shaders[shader_type], NULL, 0);

        ID3D11ShaderResourceView* shader_views[] = {
            win32_render->fb_texture_view,
        };
        ID3D11DeviceContext_PSSetShaderResources(
            device_context,
            0,
            1,
            shader_views
        );
        ID3D11SamplerState* samplers[] = {
            win32_render->fsq_sampler,
        };
        ID3D11DeviceContext_PSSetSamplers(
            device_context,
            0,
            1,
            samplers
        );
     
        ID3D11DeviceContext_IASetPrimitiveTopology(device_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        ID3D11DeviceContext_IASetInputLayout(device_context, win32_render->fsq_layout);
        ID3D11Buffer* vertex_buffers[] = {
            vertex_buffer,
        };
        const u32 strides[] = {
            sizeof(struct FsqVertex),
        };
        const u32 offsets[] = {
            0,
        };
        ID3D11DeviceContext_IASetVertexBuffers(
            device_context,
            0,
            ARRAY_COUNT(vertex_buffers),
            vertex_buffers,
            strides,
            offsets
        );
        ID3D11DeviceContext_Draw(device_context, 4, 0);
    }
}

void platform_win32_swap_and_clear_buffer(f32 r, f32 g, f32 b)
{
    struct PlatformWin32Render* win32_render = platform_win32_get_render();

    ID3D11DeviceContext* const device_context = win32_render->device_context;

    HRESULT hr;

    hr = IDXGISwapChain1_Present(win32_render->swap_chain, 1, 0);
    if(FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) || hr == DXGI_STATUS_OCCLUDED, "d3d11 error.");
        HRESULT reason = ID3D11Device_GetDeviceRemovedReason(win32_render->device);
        ASSERT(reason = S_OK, "fail");
    }

    f32 clear_color[] = { r, g, b, 1.0f };
    f32 black[4] = {};
    ID3D11DeviceContext_ClearRenderTargetView(device_context,
                                              win32_render->render_target_view,
                                              black);
    ID3D11DeviceContext_ClearRenderTargetView(device_context,
                                              win32_render->fsq_render_target_view,
                                              clear_color);
    ID3D11DeviceContext_ClearDepthStencilView(device_context,
                                              win32_render->depth_stencil_view,
                                              D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                              1.0f,
                                              0);
}
