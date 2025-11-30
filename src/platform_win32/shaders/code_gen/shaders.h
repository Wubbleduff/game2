#pragma once

enum ShaderType
{
    SHADER_basic,
    SHADER_basic_color,
    SHADER_circle_basic_color,
    SHADER_fsq,
    SHADER_line_basic_color,
    SHADER_lit_circle_outline,
    NUM_SHADERS,
};

extern const void* VERTEX_SHADER_BYTE_CODE[6];
extern const size_t VERTEX_SHADER_BYTE_CODE_SIZE[6];
extern const void* PIXEL_SHADER_BYTE_CODE[6];
extern const size_t PIXEL_SHADER_BYTE_CODE_SIZE[6];
