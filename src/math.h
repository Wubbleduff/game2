
#pragma once

#ifndef MATH_SKIP_INCLUDES
#include "common.h"
#endif

// Vector structures. Not intended for storage. Only use as locals for convenience.
typedef struct v2Tag
{
    union
    {
        __m128 v;
        struct
        {
            f32 x;
            f32 y;
        };
    };
} v2;
typedef struct v3Tag
{
    union
    {
        __m128 v;
        struct
        {
            f32 x;
            f32 y;
            f32 z;
        };
    };
} v3;
typedef struct v4Tag
{
    union
    {
        __m128 v;
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
    };
} v4;

// Matrix structures. Not intended for storage. Only use as locals for convenience.
typedef struct m4x4Tag
{
    union
    {
        __m128 v[4];
        struct
        {
            _Alignas(16) f32 a[4][4];
        };
    };
} m4x4;


////////////////////////////////////////////////////////////////////////////////
// min
static inline u8  min_u8(u8 a, u8 b)    { return a < b ? a : b; }
static inline s8  min_s8(s8 a, s8 b)    { return a < b ? a : b; }
static inline u16 min_u16(u16 a, u16 b) { return a < b ? a : b; }
static inline s16 min_s16(s16 a, s16 b) { return a < b ? a : b; }
static inline u32 min_u32(u32 a, u32 b) { return a < b ? a : b; }
static inline s32 min_s32(s32 a, s32 b) { return a < b ? a : b; }
static inline u64 min_u64(u64 a, u64 b) { return a < b ? a : b; }
static inline s64 min_s64(s64 a, s64 b) { return a < b ? a : b; }
static inline f32 min_f32(f32 a, f32 b) { return a < b ? a : b; }
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// max
static inline u8  max_u8(u8 a, u8 b)    { return a > b ? a : b; }
static inline s8  max_s8(s8 a, s8 b)    { return a > b ? a : b; }
static inline u16 max_u16(u16 a, u16 b) { return a > b ? a : b; }
static inline s16 max_s16(s16 a, s16 b) { return a > b ? a : b; }
static inline u32 max_u32(u32 a, u32 b) { return a > b ? a : b; }
static inline s32 max_s32(s32 a, s32 b) { return a > b ? a : b; }
static inline u64 max_u64(u64 a, u64 b) { return a > b ? a : b; }
static inline s64 max_s64(s64 a, s64 b) { return a > b ? a : b; }
static inline f32 max_f32(f32 a, f32 b) { return a > b ? a : b; }
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// clamp
static inline u8  clamp_u8(u8 a, u8 min, u8 max)     { return min_u8(max_u8(a, min), max); }
static inline s8  clamp_s8(s8 a, s8 min, s8 max)     { return min_s8(max_s8(a, min), max); }
static inline u16 clamp_u16(u16 a, u16 min, u16 max) { return min_u16(max_u16(a, min), max); }
static inline s16 clamp_s16(s16 a, s16 min, s16 max) { return min_s16(max_s16(a, min), max); }
static inline u32 clamp_u32(u32 a, u32 min, u32 max) { return min_u32(max_u32(a, min), max); }
static inline s32 clamp_s32(s32 a, s32 min, s32 max) { return min_s32(max_s32(a, min), max); }
static inline u64 clamp_u64(u64 a, u64 min, u64 max) { return min_u64(max_u64(a, min), max); }
static inline s64 clamp_s64(s64 a, s64 min, s64 max) { return min_s64(max_s64(a, min), max); }
static inline f32 clamp_f32(f32 a, f32 min, f32 max) { return min_f32(max_f32(a, min), max); }
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// abs
static inline s8  abs_s8(s8 a)   { return a < 0 ? -a : a; }
static inline s16 abs_s16(s16 a) { return a < 0 ? -a : a; }
static inline s32 abs_s32(s32 a) { return a < 0 ? -a : a; }
static inline s64 abs_s64(s64 a) { return a < 0 ? -a : a; }
static inline f32 abs_f32(f32 a)
{
    return u32_bits_as_f32(f32_bits_as_u32(a) & 0x7FFFFFFF);
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// square
static inline u8  sq_u8(u8 a)   { return a * a; }
static inline s8  sq_s8(s8 a)   { return a * a; }
static inline u16 sq_u16(u16 a) { return a * a; }
static inline s16 sq_s16(s16 a) { return a * a; }
static inline u32 sq_u32(u32 a) { return a * a; }
static inline s32 sq_s32(s32 a) { return a * a; }
static inline u64 sq_u64(u64 a) { return a * a; }
static inline s64 sq_s64(s64 a) { return a * a; }
static inline f32 sq_f32(f32 a) { return a * a; }
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// round
static inline f32 round_neg_inf(f32 a)
{
    return _mm_cvtss_f32(_mm_round_ps(_mm_set1_ps(a), _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC));
}

static inline f32 round_pos_inf(f32 a)
{
    return _mm_cvtss_f32(_mm_round_ps(_mm_set1_ps(a), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// sqrt
static inline f32 sqrt_f32(f32 a)
{
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set1_ps(a)));
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// vector constructors
static inline v2 zero_v2()
{
    v2 r;
    r.v = _mm_setr_ps(0.0f, 0.0f, nan_f32(), nan_f32());
    return r;
}
static inline v3 zero_v3()
{
    v3 r;
    r.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, nan_f32());
    return r;
}
static inline v4 zero_v4()
{
    v4 r;
    r.v = _mm_set1_ps(0.0f);
    return r;
}

static inline v2 make_v2(f32 x, f32 y)
{
    v2 r;
    r.v = _mm_setr_ps(x, y, nan_f32(), nan_f32());
    return r;
}
static inline v3 make_v3(f32 x, f32 y, f32 z)
{
    v3 r;
    r.v = _mm_setr_ps(x, y, z, nan_f32());
    return r;
}
static inline v4 make_v4(f32 x, f32 y, f32 z, f32 w)
{
    v4 r;
    r.v = _mm_setr_ps(x, y, z, w);
    return r;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// add
static inline v2 add_v2(v2 a, v2 b)
{
    v2 r;
    r.v = _mm_add_ps(a.v, b.v);
    return r;
}
static inline v3 add_v3(v3 a, v3 b)
{
    v3 r;
    r.v = _mm_add_ps(a.v, b.v);
    return r;
}
static inline v4 add_v4(v4 a, v4 b)
{
    v4 r;
    r.v = _mm_add_ps(a.v, b.v);
    return r;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// sub
static inline v2 sub_v2(v2 a, v2 b)
{
    v2 r;
    r.v = _mm_sub_ps(a.v, b.v);
    return r;
}
static inline v3 sub_v3(v3 a, v3 b)
{
    v3 r;
    r.v = _mm_sub_ps(a.v, b.v);
    return r;
}
static inline v4 sub_v4(v4 a, v4 b)
{
    v4 r;
    r.v = _mm_sub_ps(a.v, b.v);
    return r;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// scale
static inline v2 scale_v2(v2 a, f32 b)
{
    v2 r;
    r.v = _mm_mul_ps(a.v, _mm_set1_ps(b));
    return r;
}
static inline v3 scale_v3(v3 a, f32 b)
{
    v3 r;
    r.v = _mm_mul_ps(a.v, _mm_set1_ps(b));
    return r;
}
static inline v4 scale_v4(v4 a, f32 b)
{
    v4 r;
    r.v = _mm_mul_ps(a.v, _mm_set1_ps(b));
    return r;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// lerp
static inline f32 lerp_f32(f32 a, f32 b, f32 t)
{
    return a * (1.0f - t) + b * t;
}
static inline v2 lerp_v2(v2 a, v2 b, f32 t)
{
    v2 result;
    result.v = _mm_add_ps(
            _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(1.0f), _mm_set1_ps(t)), a.v),
            _mm_mul_ps(_mm_set1_ps(t), b.v));
    return result;
}
static inline v3 lerp_v3(v3 a, v3 b, f32 t)
{
    v3 result;
    result.v = _mm_add_ps(
            _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(1.0f), _mm_set1_ps(t)), a.v),
            _mm_mul_ps(_mm_set1_ps(t), b.v));
    return result;
}
static inline v4 lerp_v4(v4 a, v4 b, f32 t)
{
    v4 result;
    result.v = _mm_add_ps(
            _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(1.0f), _mm_set1_ps(t)), a.v),
            _mm_mul_ps(_mm_set1_ps(t), b.v));
    return result;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// dot product
static inline f32 dot_v2(v2 a, v2 b)
{
    v2 r;
    r.v = _mm_mul_ps(a.v, b.v);
    r.v = _mm_add_ps(r.v, _mm_shuffle_ps(r.v, r.v, _MM_SHUFFLE(3, 2, 1, 1)));
    return r.x;
}
static inline f32 dot_v3(v3 a, v3 b)
{
    v3 r;
    r.v = _mm_mul_ps(a.v, b.v);
    __m128 tmp = r.v;
    r.v = _mm_add_ps(r.v, _mm_shuffle_ps(r.v, r.v, _MM_SHUFFLE(3, 2, 1, 1)));
    r.v = _mm_add_ps(r.v, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 2, 1, 2)));
    return r.x;
}
static inline f32 dot_v4(v4 a, v4 b)
{
    v4 r;
    r.v = _mm_mul_ps(a.v, b.v);
    r.v = _mm_add_ps(r.v, _mm_shuffle_ps(r.v, r.v, _MM_SHUFFLE(0, 3, 2, 1)));
    r.v = _mm_add_ps(r.v, _mm_shuffle_ps(r.v, r.v, _MM_SHUFFLE(1, 0, 3, 2)));
    return r.x;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// cross product
static inline v3 cross_v3(v3 a, v3 b)
{
    v3 r;
    r.v = _mm_fmsub_ps(
        _mm_shuffle_ps(a.v, a.v, 0b11001001),
        _mm_shuffle_ps(b.v, b.v, 0b11010010),
        _mm_mul_ps(
            _mm_shuffle_ps(a.v, a.v, 0b11010010),
            _mm_shuffle_ps(b.v, b.v, 0b11001001)
        )
    );
    return r;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// length
static inline f32 length_sq_v2(v2 a) { return dot_v2(a, a); }
static inline f32 length_sq_v3(v3 a) { return dot_v3(a, a); }
static inline f32 length_sq_v4(v4 a) { return dot_v4(a, a); }

static inline f32 length_v2(v2 a)
{
    return sqrt_f32(length_sq_v2(a));
}
static inline f32 length_v3(v3 a)
{
    return sqrt_f32(length_sq_v3(a));
}
static inline f32 length_v4(v4 a)
{
    return sqrt_f32(length_sq_v4(a));
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// normalize
static inline v2 normalize_v2(v2 a)
{
    v2 result;
    __m128 l2 = _mm_mul_ps(a.v, a.v);
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(l2, l2, _MM_SHUFFLE(3, 2, 0, 1)));
    result.v = _mm_div_ps(a.v, _mm_sqrt_ps(l2));
    return result;
}
static inline v3 normalize_v3(v3 a)
{
    v3 result;
    __m128 l2 = _mm_mul_ps(a.v, a.v);
    __m128 tmp = l2;
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(l2, l2, _MM_SHUFFLE(3, 0, 2, 1)));
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 1, 0, 2)));
    result.v = _mm_div_ps(a.v, _mm_sqrt_ps(l2));
    return result;
}
static inline v4 normalize_v4(v4 a)
{
    v4 result;
    __m128 l2 = _mm_mul_ps(a.v, a.v);
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(l2, l2, _MM_SHUFFLE(0, 3, 2, 1)));
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(l2, l2, _MM_SHUFFLE(1, 0, 3, 2)));
    result.v = _mm_div_ps(a.v, _mm_sqrt_ps(l2));
    return result;
}

static inline v2 normalize_or_v2(v2 a, v2 def)
{
    v2 result;
    __m128 l2 = _mm_mul_ps(a.v, a.v);
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(l2, l2, _MM_SHUFFLE(3, 2, 0, 1)));
    result.v = _mm_div_ps(a.v, _mm_sqrt_ps(l2));
    result.v = _mm_blendv_ps(
        result.v,
        def.v,
        _mm_cmp_ps(l2, _mm_set1_ps(0.0f), _CMP_EQ_OQ)
    );
    return result;
}
static inline v3 normalize_or_v3(v3 a, v3 def)
{
    v3 result;
    __m128 l2 = _mm_mul_ps(a.v, a.v);
    __m128 tmp = l2;
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(l2, l2, _MM_SHUFFLE(3, 0, 2, 1)));
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 1, 0, 2)));
    result.v = _mm_div_ps(a.v, _mm_sqrt_ps(l2));
    result.v = _mm_blendv_ps(
        result.v,
        def.v,
        _mm_cmp_ps(l2, _mm_set1_ps(0.0f), _CMP_EQ_OQ)
    );
    return result;
}
static inline v4 normalize_or_v4(v4 a, v4 def)
{
    v4 result;
    __m128 l2 = _mm_mul_ps(a.v, a.v);
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(l2, l2, _MM_SHUFFLE(0, 3, 2, 1)));
    l2 = _mm_add_ps(l2, _mm_shuffle_ps(l2, l2, _MM_SHUFFLE(1, 0, 3, 2)));
    result.v = _mm_div_ps(a.v, _mm_sqrt_ps(l2));
    result.v = _mm_blendv_ps(
        result.v,
        def.v,
        _mm_cmp_ps(l2, _mm_set1_ps(0.0f), _CMP_EQ_OQ)
    );
    return result;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// matrix ops
static inline void identity_m4x4(m4x4* a)
{
    a->v[0] = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    a->v[1] = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    a->v[2] = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    a->v[3] = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
}

// 4x4 matrix multiply : r = a * b.
// NOTE: Matrices are assumed to be row-major.
static inline void mul_m4x4(m4x4* r, m4x4* a, m4x4* b)
{
    /*
     *
     * | a00  a01  a02  a03 |   | b00  b01  b02  b03 |   
     * | a10  a11  a12  a13 | * | b10  b11  b12  b13 | = 
     * | a20  a21  a22  a23 |   | b20  b21  b22  b23 |   
     * | a30  a31  a32  a33 |   | b30  b31  b32  b33 |   
     *
     * | a00*b00 + a01*b10 + a02*b20 + a03*b30    a00*b01 + a01*b11 + a02*b21 + a03*b31    a00*b02 + a01*b12 + a02*b22 + a03*b32    a00*b03 + a01*b13 + a02*b23 + a03*b33 |
     * | a10*b00 + a11*b10 + a12*b20 + a13*b30    a10*b01 + a11*b11 + a12*b21 + a13*b31    a10*b02 + a11*b12 + a12*b22 + a13*b32    a10*b03 + a11*b13 + a12*b23 + a13*b33 |
     * | a20*b00 + a21*b10 + a22*b20 + a23*b30    a20*b01 + a21*b11 + a22*b21 + a23*b31    a20*b02 + a21*b12 + a22*b22 + a23*b32    a20*b03 + a21*b13 + a22*b23 + a23*b33 |
     * | a30*b00 + a31*b10 + a32*b20 + a33*b30    a30*b01 + a31*b11 + a32*b21 + a33*b31    a30*b02 + a31*b12 + a32*b22 + a33*b32    a30*b03 + a31*b13 + a32*b23 + a33*b33 |
     *
     */

    /*
    Reference
    for(u64 i = 0; i < 4; i++)
    {
        for(u64 j = 0; j < 4; j++)
        {
            r[i*4 + j] =
                a[i*4 + 0] * b[0*4 + j] + 
                a[i*4 + 1] * b[1*4 + j] + 
                a[i*4 + 2] * b[2*4 + j] + 
                a[i*4 + 3] * b[3*4 + j];
        }
    }
    */

    for(u64 row = 0; row < 4; row++)
    {
        const __m128 v = _mm_fmadd_ps(
            _mm_broadcast_ss(&a->a[row][0]),
            _mm_loadu_ps(&b->a[0][0]),
            _mm_fmadd_ps(
                _mm_broadcast_ss(&a->a[row][1]),
                _mm_loadu_ps(&b->a[1][0]),
                _mm_fmadd_ps(
                    _mm_broadcast_ss(&a->a[row][2]),
                    _mm_loadu_ps(&b->a[2][0]),
                    _mm_mul_ps(
                        _mm_broadcast_ss(&a->a[row][3]),
                        _mm_loadu_ps(&b->a[3][0])
                    )
                )
            )
        );
        _mm_storeu_ps(&r->a[row][0], v);
    }
}
////////////////////////////////////////////////////////////////////////////////
