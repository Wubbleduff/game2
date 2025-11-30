typedef unsigned char BYTE;

#include "platform_win32/shaders/code_gen/d3d11_vshader_basic.h"
#include "platform_win32/shaders/code_gen/d3d11_pshader_basic.h"

#include "platform_win32/shaders/code_gen/d3d11_vshader_basic_color.h"
#include "platform_win32/shaders/code_gen/d3d11_pshader_basic_color.h"

#include "platform_win32/shaders/code_gen/d3d11_vshader_circle_basic_color.h"
#include "platform_win32/shaders/code_gen/d3d11_pshader_circle_basic_color.h"

#include "platform_win32/shaders/code_gen/d3d11_vshader_fsq.h"
#include "platform_win32/shaders/code_gen/d3d11_pshader_fsq.h"

#include "platform_win32/shaders/code_gen/d3d11_vshader_line_basic_color.h"
#include "platform_win32/shaders/code_gen/d3d11_pshader_line_basic_color.h"

#include "platform_win32/shaders/code_gen/d3d11_vshader_lit_circle_outline.h"
#include "platform_win32/shaders/code_gen/d3d11_pshader_lit_circle_outline.h"

const void* VERTEX_SHADER_BYTE_CODE[] = {
    D3D11_VSHADER_basic,
    D3D11_VSHADER_basic_color,
    D3D11_VSHADER_circle_basic_color,
    D3D11_VSHADER_fsq,
    D3D11_VSHADER_line_basic_color,
    D3D11_VSHADER_lit_circle_outline,
};

const size_t VERTEX_SHADER_BYTE_CODE_SIZE[] = {
    sizeof(D3D11_VSHADER_basic),
    sizeof(D3D11_VSHADER_basic_color),
    sizeof(D3D11_VSHADER_circle_basic_color),
    sizeof(D3D11_VSHADER_fsq),
    sizeof(D3D11_VSHADER_line_basic_color),
    sizeof(D3D11_VSHADER_lit_circle_outline),
};

const void* PIXEL_SHADER_BYTE_CODE[] = {
    D3D11_PSHADER_basic,
    D3D11_PSHADER_basic_color,
    D3D11_PSHADER_circle_basic_color,
    D3D11_PSHADER_fsq,
    D3D11_PSHADER_line_basic_color,
    D3D11_PSHADER_lit_circle_outline,
};

const size_t PIXEL_SHADER_BYTE_CODE_SIZE[] = {
    sizeof(D3D11_PSHADER_basic),
    sizeof(D3D11_PSHADER_basic_color),
    sizeof(D3D11_PSHADER_circle_basic_color),
    sizeof(D3D11_PSHADER_fsq),
    sizeof(D3D11_PSHADER_line_basic_color),
    sizeof(D3D11_PSHADER_lit_circle_outline),
};

