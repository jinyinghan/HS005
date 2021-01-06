#ifndef __JZ_CORE_SSE_UTILS_HPP__
#define __JZ_CORE_SSE_UTILS_HPP__

#ifndef __cplusplus
#  error sse_utils.hpp header must be compiled as C++
#endif

#include "mxu_types_c.h"

#if MXU_SSE2

inline void _mm_interleave_ps(__m128 & v_r0, __m128 & v_r1, __m128 & v_g0,
                              __m128 & v_g1, __m128 & v_b0, __m128 & v_b1)
{
    const int mask_lo = _MM_SHUFFLE(2, 0, 2, 0), mask_hi = _MM_SHUFFLE(3, 1, 3, 1);

    __m128 layer2_chunk0 = _mm_shuffle_ps(v_r0, v_r1, mask_lo);
    __m128 layer2_chunk3 = _mm_shuffle_ps(v_r0, v_r1, mask_hi);
    __m128 layer2_chunk1 = _mm_shuffle_ps(v_g0, v_g1, mask_lo);
    __m128 layer2_chunk4 = _mm_shuffle_ps(v_g0, v_g1, mask_hi);
    __m128 layer2_chunk2 = _mm_shuffle_ps(v_b0, v_b1, mask_lo);
    __m128 layer2_chunk5 = _mm_shuffle_ps(v_b0, v_b1, mask_hi);

    __m128 layer1_chunk0 = _mm_shuffle_ps(layer2_chunk0, layer2_chunk1, mask_lo);
    __m128 layer1_chunk3 = _mm_shuffle_ps(layer2_chunk0, layer2_chunk1, mask_hi);
    __m128 layer1_chunk1 = _mm_shuffle_ps(layer2_chunk2, layer2_chunk3, mask_lo);
    __m128 layer1_chunk4 = _mm_shuffle_ps(layer2_chunk2, layer2_chunk3, mask_hi);
    __m128 layer1_chunk2 = _mm_shuffle_ps(layer2_chunk4, layer2_chunk5, mask_lo);
    __m128 layer1_chunk5 = _mm_shuffle_ps(layer2_chunk4, layer2_chunk5, mask_hi);

    v_r0 = _mm_shuffle_ps(layer1_chunk0, layer1_chunk1, mask_lo);
    v_g1 = _mm_shuffle_ps(layer1_chunk0, layer1_chunk1, mask_hi);
    v_r1 = _mm_shuffle_ps(layer1_chunk2, layer1_chunk3, mask_lo);
    v_b0 = _mm_shuffle_ps(layer1_chunk2, layer1_chunk3, mask_hi);
    v_g0 = _mm_shuffle_ps(layer1_chunk4, layer1_chunk5, mask_lo);
    v_b1 = _mm_shuffle_ps(layer1_chunk4, layer1_chunk5, mask_hi);
}

#endif // MXU_SSE2

#endif //__JZ_CORE_SSE_UTILS_HPP__
