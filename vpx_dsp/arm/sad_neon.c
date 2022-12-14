/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"

#include "vpx/vpx_integer.h"
#include "vpx_dsp/arm/mem_neon.h"
#include "vpx_dsp/arm/sum_neon.h"

uint32_t vpx_sad4x4_neon(const uint8_t *src_ptr, int src_stride,
                         const uint8_t *ref_ptr, int ref_stride) {
  const uint8x16_t src_u8 = load_unaligned_u8q(src_ptr, src_stride);
  const uint8x16_t ref_u8 = load_unaligned_u8q(ref_ptr, ref_stride);
#if defined(__aarch64__) && defined(__ARM_FEATURE_DOTPROD) && \
    (__ARM_FEATURE_DOTPROD == 1)
  const uint8x16_t sad_u8 = vabdq_u8(src_u8, ref_u8);
  const uint32x4_t dp = vdotq_u32(vdupq_n_u32(0), sad_u8, vdupq_n_u8(1));
  return horizontal_add_uint32x4(dp);
#else
  uint16x8_t abs = vabdl_u8(vget_low_u8(src_u8), vget_low_u8(ref_u8));
  abs = vabal_u8(abs, vget_high_u8(src_u8), vget_high_u8(ref_u8));
  return horizontal_add_uint16x8(abs);
#endif
}

uint32_t vpx_sad4x4_avg_neon(const uint8_t *src_ptr, int src_stride,
                             const uint8_t *ref_ptr, int ref_stride,
                             const uint8_t *second_pred) {
  const uint8x16_t src_u8 = load_unaligned_u8q(src_ptr, src_stride);
  const uint8x16_t ref_u8 = load_unaligned_u8q(ref_ptr, ref_stride);
  const uint8x16_t second_pred_u8 = vld1q_u8(second_pred);
  const uint8x16_t avg = vrhaddq_u8(ref_u8, second_pred_u8);
#if defined(__aarch64__) && defined(__ARM_FEATURE_DOTPROD) && \
    (__ARM_FEATURE_DOTPROD == 1)
  const uint8x16_t sad_u8 = vabdq_u8(src_u8, avg);
  const uint32x4_t prod = vdotq_u32(vdupq_n_u32(0), sad_u8, vdupq_n_u8(1));
  return horizontal_add_uint32x4(prod);
#else
  uint16x8_t abs = vabdl_u8(vget_low_u8(src_u8), vget_low_u8(avg));
  abs = vabal_u8(abs, vget_high_u8(src_u8), vget_high_u8(avg));
  return horizontal_add_uint16x8(abs);
#endif
}

uint32_t vpx_sad4x8_neon(const uint8_t *src_ptr, int src_stride,
                         const uint8_t *ref_ptr, int ref_stride) {
#if defined(__aarch64__) && defined(__ARM_FEATURE_DOTPROD) && \
    (__ARM_FEATURE_DOTPROD == 1)
  uint32x4_t prod = vdupq_n_u32(0);
  const uint8x16_t ones = vdupq_n_u8(1);
  const uint8x16_t src1_u8 = load_unaligned_u8q(src_ptr, src_stride);
  const uint8x16_t ref1_u8 = load_unaligned_u8q(ref_ptr, ref_stride);
  const uint8x16_t src2_u8 =
      load_unaligned_u8q(src_ptr + 4 * src_stride, src_stride);
  const uint8x16_t ref2_u8 =
      load_unaligned_u8q(ref_ptr + 4 * ref_stride, ref_stride);
  const uint8x16_t sad1_u8 = vabdq_u8(src1_u8, ref1_u8);
  const uint8x16_t sad2_u8 = vabdq_u8(src2_u8, ref2_u8);
  prod = vdotq_u32(prod, sad1_u8, ones);
  prod = vdotq_u32(prod, sad2_u8, ones);
  return horizontal_add_uint32x4(prod);
#else
  int i;
  uint16x8_t abs = vdupq_n_u16(0);
  for (i = 0; i < 8; i += 4) {
    const uint8x16_t src_u8 = load_unaligned_u8q(src_ptr, src_stride);
    const uint8x16_t ref_u8 = load_unaligned_u8q(ref_ptr, ref_stride);
    src_ptr += 4 * src_stride;
    ref_ptr += 4 * ref_stride;
    abs = vabal_u8(abs, vget_low_u8(src_u8), vget_low_u8(ref_u8));
    abs = vabal_u8(abs, vget_high_u8(src_u8), vget_high_u8(ref_u8));
  }

  return horizontal_add_uint16x8(abs);
#endif
}

uint32_t vpx_sad4x8_avg_neon(const uint8_t *src_ptr, int src_stride,
                             const uint8_t *ref_ptr, int ref_stride,
                             const uint8_t *second_pred) {
#if defined(__aarch64__) && defined(__ARM_FEATURE_DOTPROD) && \
    (__ARM_FEATURE_DOTPROD == 1)
  uint32x4_t prod = vdupq_n_u32(0);
  const uint8x16_t ones = vdupq_n_u8(1);
  const uint8x16_t src1_u8 = load_unaligned_u8q(src_ptr, src_stride);
  const uint8x16_t ref1_u8 = load_unaligned_u8q(ref_ptr, ref_stride);
  const uint8x16_t src2_u8 =
      load_unaligned_u8q(src_ptr + 4 * src_stride, src_stride);
  const uint8x16_t ref2_u8 =
      load_unaligned_u8q(ref_ptr + 4 * ref_stride, ref_stride);
  const uint8x16_t second_pred1_u8 = vld1q_u8(second_pred);
  const uint8x16_t second_pred2_u8 = vld1q_u8(second_pred + 16);
  const uint8x16_t avg1 = vrhaddq_u8(ref1_u8, second_pred1_u8);
  const uint8x16_t avg2 = vrhaddq_u8(ref2_u8, second_pred2_u8);
  const uint8x16_t sad1_u8 = vabdq_u8(src1_u8, avg1);
  const uint8x16_t sad2_u8 = vabdq_u8(src2_u8, avg2);
  prod = vdotq_u32(prod, sad1_u8, ones);
  prod = vdotq_u32(prod, sad2_u8, ones);
  return horizontal_add_uint32x4(prod);
#else
  int i;
  uint16x8_t abs = vdupq_n_u16(0);
  for (i = 0; i < 8; i += 4) {
    const uint8x16_t src_u8 = load_unaligned_u8q(src_ptr, src_stride);
    const uint8x16_t ref_u8 = load_unaligned_u8q(ref_ptr, ref_stride);
    const uint8x16_t second_pred_u8 = vld1q_u8(second_pred);
    const uint8x16_t avg = vrhaddq_u8(ref_u8, second_pred_u8);
    src_ptr += 4 * src_stride;
    ref_ptr += 4 * ref_stride;
    second_pred += 16;
    abs = vabal_u8(abs, vget_low_u8(src_u8), vget_low_u8(avg));
    abs = vabal_u8(abs, vget_high_u8(src_u8), vget_high_u8(avg));
  }

  return horizontal_add_uint16x8(abs);
#endif
}

#if defined(__aarch64__) && defined(__ARM_FEATURE_DOTPROD) && \
    (__ARM_FEATURE_DOTPROD == 1)
static INLINE uint32x2_t sad8x(const uint8_t *src_ptr, int src_stride,
                               const uint8_t *ref_ptr, int ref_stride,
                               const int height) {
  int i;
  uint32x2_t prod = vdup_n_u32(0);
  const uint8x8_t ones = vdup_n_u8(1);
  for (i = 0; i < height; ++i) {
    const uint8x8_t a_u8 = vld1_u8(src_ptr);
    const uint8x8_t b_u8 = vld1_u8(ref_ptr);
    const uint8x8_t sad_u8 = vabd_u8(a_u8, b_u8);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    prod = vdot_u32(prod, sad_u8, ones);
  }
  return prod;
}

static INLINE uint32x2_t sad8x_avg(const uint8_t *src_ptr, int src_stride,
                                   const uint8_t *ref_ptr, int ref_stride,
                                   const uint8_t *second_pred,
                                   const int height) {
  int i;
  uint32x2_t prod = vdup_n_u32(0);
  const uint8x8_t ones = vdup_n_u8(1);
  for (i = 0; i < height; ++i) {
    const uint8x8_t a_u8 = vld1_u8(src_ptr);
    const uint8x8_t b_u8 = vld1_u8(ref_ptr);
    const uint8x8_t c_u8 = vld1_u8(second_pred);
    const uint8x8_t avg = vrhadd_u8(b_u8, c_u8);
    const uint8x8_t sad_u8 = vabd_u8(a_u8, avg);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    second_pred += 8;
    prod = vdot_u32(prod, sad_u8, ones);
  }
  return prod;
}

#define SAD8XN(n)                                                            \
  uint32_t vpx_sad8x##n##_neon(const uint8_t *src_ptr, int src_stride,       \
                               const uint8_t *ref_ptr, int ref_stride) {     \
    const uint32x2_t prod =                                                  \
        sad8x(src_ptr, src_stride, ref_ptr, ref_stride, n);                  \
    return horizontal_add_uint32x2(prod);                                    \
  }                                                                          \
                                                                             \
  uint32_t vpx_sad8x##n##_avg_neon(const uint8_t *src_ptr, int src_stride,   \
                                   const uint8_t *ref_ptr, int ref_stride,   \
                                   const uint8_t *second_pred) {             \
    const uint32x2_t prod =                                                  \
        sad8x_avg(src_ptr, src_stride, ref_ptr, ref_stride, second_pred, n); \
    return horizontal_add_uint32x2(prod);                                    \
  }

#else
static INLINE uint16x8_t sad8x(const uint8_t *src_ptr, int src_stride,
                               const uint8_t *ref_ptr, int ref_stride,
                               const int height) {
  int i;
  uint16x8_t abs = vdupq_n_u16(0);

  for (i = 0; i < height; ++i) {
    const uint8x8_t a_u8 = vld1_u8(src_ptr);
    const uint8x8_t b_u8 = vld1_u8(ref_ptr);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    abs = vabal_u8(abs, a_u8, b_u8);
  }
  return abs;
}

static INLINE uint16x8_t sad8x_avg(const uint8_t *src_ptr, int src_stride,
                                   const uint8_t *ref_ptr, int ref_stride,
                                   const uint8_t *second_pred,
                                   const int height) {
  int i;
  uint16x8_t abs = vdupq_n_u16(0);

  for (i = 0; i < height; ++i) {
    const uint8x8_t a_u8 = vld1_u8(src_ptr);
    const uint8x8_t b_u8 = vld1_u8(ref_ptr);
    const uint8x8_t c_u8 = vld1_u8(second_pred);
    const uint8x8_t avg = vrhadd_u8(b_u8, c_u8);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    second_pred += 8;
    abs = vabal_u8(abs, a_u8, avg);
  }
  return abs;
}

#define SAD8XN(n)                                                              \
  uint32_t vpx_sad8x##n##_neon(const uint8_t *src_ptr, int src_stride,         \
                               const uint8_t *ref_ptr, int ref_stride) {       \
    const uint16x8_t abs = sad8x(src_ptr, src_stride, ref_ptr, ref_stride, n); \
    return horizontal_add_uint16x8(abs);                                       \
  }                                                                            \
                                                                               \
  uint32_t vpx_sad8x##n##_avg_neon(const uint8_t *src_ptr, int src_stride,     \
                                   const uint8_t *ref_ptr, int ref_stride,     \
                                   const uint8_t *second_pred) {               \
    const uint16x8_t abs =                                                     \
        sad8x_avg(src_ptr, src_stride, ref_ptr, ref_stride, second_pred, n);   \
    return horizontal_add_uint16x8(abs);                                       \
  }
#endif

SAD8XN(4)
SAD8XN(8)
SAD8XN(16)

#if defined(__aarch64__) && defined(__ARM_FEATURE_DOTPROD) && \
    (__ARM_FEATURE_DOTPROD == 1)
static INLINE uint32x4_t sad16x(const uint8_t *src_ptr, int src_stride,
                                const uint8_t *ref_ptr, int ref_stride,
                                const int height) {
  int i;
  uint32x4_t prod = vdupq_n_u32(0);
  const uint8x16_t ones = vdupq_n_u8(1);
  for (i = 0; i < height; ++i) {
    const uint8x16_t src_u8 = vld1q_u8(src_ptr);
    const uint8x16_t ref_u8 = vld1q_u8(ref_ptr);
    const uint8x16_t sad_u8 = vabdq_u8(src_u8, ref_u8);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    prod = vdotq_u32(prod, sad_u8, ones);
  }
  return prod;
}

static INLINE uint32x4_t sad16x_avg(const uint8_t *src_ptr, int src_stride,
                                    const uint8_t *ref_ptr, int ref_stride,
                                    const uint8_t *second_pred,
                                    const int height) {
  int i;
  uint32x4_t prod = vdupq_n_u32(0);
  const uint8x16_t ones = vdupq_n_u8(1);
  for (i = 0; i < height; ++i) {
    const uint8x16_t a_u8 = vld1q_u8(src_ptr);
    const uint8x16_t b_u8 = vld1q_u8(ref_ptr);
    const uint8x16_t c_u8 = vld1q_u8(second_pred);
    const uint8x16_t avg = vrhaddq_u8(b_u8, c_u8);
    const uint8x16_t sad_u8 = vabdq_u8(a_u8, avg);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    second_pred += 16;
    prod = vdotq_u32(prod, sad_u8, ones);
  }
  return prod;
}

#define SAD16XN(n)                                                            \
  uint32_t vpx_sad16x##n##_neon(const uint8_t *src_ptr, int src_stride,       \
                                const uint8_t *ref_ptr, int ref_stride) {     \
    const uint32x4_t prod =                                                   \
        sad16x(src_ptr, src_stride, ref_ptr, ref_stride, n);                  \
    return horizontal_add_uint32x4(prod);                                     \
  }                                                                           \
                                                                              \
  uint32_t vpx_sad16x##n##_avg_neon(const uint8_t *src_ptr, int src_stride,   \
                                    const uint8_t *ref_ptr, int ref_stride,   \
                                    const uint8_t *second_pred) {             \
    const uint32x4_t prod =                                                   \
        sad16x_avg(src_ptr, src_stride, ref_ptr, ref_stride, second_pred, n); \
    return horizontal_add_uint32x4(prod);                                     \
  }
#else
static INLINE uint16x8_t sad16x(const uint8_t *src_ptr, int src_stride,
                                const uint8_t *ref_ptr, int ref_stride,
                                const int height) {
  int i;
  uint16x8_t abs = vdupq_n_u16(0);

  for (i = 0; i < height; ++i) {
    const uint8x16_t a_u8 = vld1q_u8(src_ptr);
    const uint8x16_t b_u8 = vld1q_u8(ref_ptr);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    abs = vabal_u8(abs, vget_low_u8(a_u8), vget_low_u8(b_u8));
    abs = vabal_u8(abs, vget_high_u8(a_u8), vget_high_u8(b_u8));
  }
  return abs;
}

static INLINE uint16x8_t sad16x_avg(const uint8_t *src_ptr, int src_stride,
                                    const uint8_t *ref_ptr, int ref_stride,
                                    const uint8_t *second_pred,
                                    const int height) {
  int i;
  uint16x8_t abs = vdupq_n_u16(0);

  for (i = 0; i < height; ++i) {
    const uint8x16_t a_u8 = vld1q_u8(src_ptr);
    const uint8x16_t b_u8 = vld1q_u8(ref_ptr);
    const uint8x16_t c_u8 = vld1q_u8(second_pred);
    const uint8x16_t avg = vrhaddq_u8(b_u8, c_u8);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    second_pred += 16;
    abs = vabal_u8(abs, vget_low_u8(a_u8), vget_low_u8(avg));
    abs = vabal_u8(abs, vget_high_u8(a_u8), vget_high_u8(avg));
  }
  return abs;
}

#define SAD16XN(n)                                                            \
  uint32_t vpx_sad16x##n##_neon(const uint8_t *src_ptr, int src_stride,       \
                                const uint8_t *ref_ptr, int ref_stride) {     \
    const uint16x8_t abs =                                                    \
        sad16x(src_ptr, src_stride, ref_ptr, ref_stride, n);                  \
    return horizontal_add_uint16x8(abs);                                      \
  }                                                                           \
                                                                              \
  uint32_t vpx_sad16x##n##_avg_neon(const uint8_t *src_ptr, int src_stride,   \
                                    const uint8_t *ref_ptr, int ref_stride,   \
                                    const uint8_t *second_pred) {             \
    const uint16x8_t abs =                                                    \
        sad16x_avg(src_ptr, src_stride, ref_ptr, ref_stride, second_pred, n); \
    return horizontal_add_uint16x8(abs);                                      \
  }
#endif

SAD16XN(8)
SAD16XN(16)
SAD16XN(32)

#if defined(__aarch64__) && defined(__ARM_FEATURE_DOTPROD) && \
    (__ARM_FEATURE_DOTPROD == 1)
static INLINE uint32x4_t sad32x(const uint8_t *src_ptr, int src_stride,
                                const uint8_t *ref_ptr, int ref_stride,
                                const int height) {
  int i;
  uint32x4_t prod = vdupq_n_u32(0);
  const uint8x16_t ones = vdupq_n_u8(1);
  for (i = 0; i < height; ++i) {
    const uint8x16_t a_lo = vld1q_u8(src_ptr);
    const uint8x16_t a_hi = vld1q_u8(src_ptr + 16);
    const uint8x16_t b_lo = vld1q_u8(ref_ptr);
    const uint8x16_t b_hi = vld1q_u8(ref_ptr + 16);
    const uint8x16_t sad_lo_u8 = vabdq_u8(a_lo, b_lo);
    const uint8x16_t sad_hi_u8 = vabdq_u8(a_hi, b_hi);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    prod = vdotq_u32(prod, sad_lo_u8, ones);
    prod = vdotq_u32(prod, sad_hi_u8, ones);
  }
  return prod;
}

static INLINE uint32x4_t sad32x_avg(const uint8_t *src_ptr, int src_stride,
                                    const uint8_t *ref_ptr, int ref_stride,
                                    const uint8_t *second_pred,
                                    const int height) {
  int i;
  uint32x4_t prod = vdupq_n_u32(0);
  const uint8x16_t ones = vdupq_n_u8(1);
  for (i = 0; i < height; ++i) {
    const uint8x16_t a_lo = vld1q_u8(src_ptr);
    const uint8x16_t a_hi = vld1q_u8(src_ptr + 16);
    const uint8x16_t b_lo = vld1q_u8(ref_ptr);
    const uint8x16_t b_hi = vld1q_u8(ref_ptr + 16);
    const uint8x16_t c_lo = vld1q_u8(second_pred);
    const uint8x16_t c_hi = vld1q_u8(second_pred + 16);
    const uint8x16_t avg_lo = vrhaddq_u8(b_lo, c_lo);
    const uint8x16_t avg_hi = vrhaddq_u8(b_hi, c_hi);
    const uint8x16_t sad_lo_u8 = vabdq_u8(a_lo, avg_lo);
    const uint8x16_t sad_hi_u8 = vabdq_u8(a_hi, avg_hi);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    second_pred += 32;
    prod = vdotq_u32(prod, sad_lo_u8, ones);
    prod = vdotq_u32(prod, sad_hi_u8, ones);
  }
  return prod;
}

#define SAD32XN(n)                                                            \
  uint32_t vpx_sad32x##n##_neon(const uint8_t *src_ptr, int src_stride,       \
                                const uint8_t *ref_ptr, int ref_stride) {     \
    const uint32x4_t prod =                                                   \
        sad32x(src_ptr, src_stride, ref_ptr, ref_stride, n);                  \
    return horizontal_add_uint32x4(prod);                                     \
  }                                                                           \
                                                                              \
  uint32_t vpx_sad32x##n##_avg_neon(const uint8_t *src_ptr, int src_stride,   \
                                    const uint8_t *ref_ptr, int ref_stride,   \
                                    const uint8_t *second_pred) {             \
    const uint32x4_t prod =                                                   \
        sad32x_avg(src_ptr, src_stride, ref_ptr, ref_stride, second_pred, n); \
    return horizontal_add_uint32x4(prod);                                     \
  }

#else
static INLINE uint16x8_t sad32x(const uint8_t *src_ptr, int src_stride,
                                const uint8_t *ref_ptr, int ref_stride,
                                const int height) {
  int i;
  uint16x8_t abs = vdupq_n_u16(0);

  for (i = 0; i < height; ++i) {
    const uint8x16_t a_lo = vld1q_u8(src_ptr);
    const uint8x16_t a_hi = vld1q_u8(src_ptr + 16);
    const uint8x16_t b_lo = vld1q_u8(ref_ptr);
    const uint8x16_t b_hi = vld1q_u8(ref_ptr + 16);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    abs = vabal_u8(abs, vget_low_u8(a_lo), vget_low_u8(b_lo));
    abs = vabal_u8(abs, vget_high_u8(a_lo), vget_high_u8(b_lo));
    abs = vabal_u8(abs, vget_low_u8(a_hi), vget_low_u8(b_hi));
    abs = vabal_u8(abs, vget_high_u8(a_hi), vget_high_u8(b_hi));
  }
  return abs;
}

static INLINE uint16x8_t sad32x_avg(const uint8_t *src_ptr, int src_stride,
                                    const uint8_t *ref_ptr, int ref_stride,
                                    const uint8_t *second_pred,
                                    const int height) {
  int i;
  uint16x8_t abs = vdupq_n_u16(0);

  for (i = 0; i < height; ++i) {
    const uint8x16_t a_lo = vld1q_u8(src_ptr);
    const uint8x16_t a_hi = vld1q_u8(src_ptr + 16);
    const uint8x16_t b_lo = vld1q_u8(ref_ptr);
    const uint8x16_t b_hi = vld1q_u8(ref_ptr + 16);
    const uint8x16_t c_lo = vld1q_u8(second_pred);
    const uint8x16_t c_hi = vld1q_u8(second_pred + 16);
    const uint8x16_t avg_lo = vrhaddq_u8(b_lo, c_lo);
    const uint8x16_t avg_hi = vrhaddq_u8(b_hi, c_hi);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    second_pred += 32;
    abs = vabal_u8(abs, vget_low_u8(a_lo), vget_low_u8(avg_lo));
    abs = vabal_u8(abs, vget_high_u8(a_lo), vget_high_u8(avg_lo));
    abs = vabal_u8(abs, vget_low_u8(a_hi), vget_low_u8(avg_hi));
    abs = vabal_u8(abs, vget_high_u8(a_hi), vget_high_u8(avg_hi));
  }
  return abs;
}

#define SAD32XN(n)                                                            \
  uint32_t vpx_sad32x##n##_neon(const uint8_t *src_ptr, int src_stride,       \
                                const uint8_t *ref_ptr, int ref_stride) {     \
    const uint16x8_t abs =                                                    \
        sad32x(src_ptr, src_stride, ref_ptr, ref_stride, n);                  \
    return horizontal_add_uint16x8(abs);                                      \
  }                                                                           \
                                                                              \
  uint32_t vpx_sad32x##n##_avg_neon(const uint8_t *src_ptr, int src_stride,   \
                                    const uint8_t *ref_ptr, int ref_stride,   \
                                    const uint8_t *second_pred) {             \
    const uint16x8_t abs =                                                    \
        sad32x_avg(src_ptr, src_stride, ref_ptr, ref_stride, second_pred, n); \
    return horizontal_add_uint16x8(abs);                                      \
  }
#endif

SAD32XN(16)
SAD32XN(32)
SAD32XN(64)

#if defined(__aarch64__) && defined(__ARM_FEATURE_DOTPROD) && \
    (__ARM_FEATURE_DOTPROD == 1)
static INLINE uint32x4_t sad64x(const uint8_t *src_ptr, int src_stride,
                                const uint8_t *ref_ptr, int ref_stride,
                                const int height) {
  int i;
  uint32x4_t prod = vdupq_n_u32(0);
  const uint8x16_t ones = vdupq_n_u8(1);
  for (i = 0; i < height; ++i) {
    const uint8x16_t a_0 = vld1q_u8(src_ptr);
    const uint8x16_t a_1 = vld1q_u8(src_ptr + 16);
    const uint8x16_t a_2 = vld1q_u8(src_ptr + 32);
    const uint8x16_t a_3 = vld1q_u8(src_ptr + 48);
    const uint8x16_t b_0 = vld1q_u8(ref_ptr);
    const uint8x16_t b_1 = vld1q_u8(ref_ptr + 16);
    const uint8x16_t b_2 = vld1q_u8(ref_ptr + 32);
    const uint8x16_t b_3 = vld1q_u8(ref_ptr + 48);
    const uint8x16_t sad_0_u8 = vabdq_u8(a_0, b_0);
    const uint8x16_t sad_1_u8 = vabdq_u8(a_1, b_1);
    const uint8x16_t sad_2_u8 = vabdq_u8(a_2, b_2);
    const uint8x16_t sad_3_u8 = vabdq_u8(a_3, b_3);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    prod = vdotq_u32(prod, sad_0_u8, ones);
    prod = vdotq_u32(prod, sad_1_u8, ones);
    prod = vdotq_u32(prod, sad_2_u8, ones);
    prod = vdotq_u32(prod, sad_3_u8, ones);
  }
  return prod;
}

static INLINE uint32x4_t sad64x_avg(const uint8_t *src_ptr, int src_stride,
                                    const uint8_t *ref_ptr, int ref_stride,
                                    const uint8_t *second_pred,
                                    const int height) {
  int i;
  uint32x4_t prod = vdupq_n_u32(0);
  const uint8x16_t ones = vdupq_n_u8(1);
  for (i = 0; i < height; ++i) {
    const uint8x16_t a_0 = vld1q_u8(src_ptr);
    const uint8x16_t a_1 = vld1q_u8(src_ptr + 16);
    const uint8x16_t a_2 = vld1q_u8(src_ptr + 32);
    const uint8x16_t a_3 = vld1q_u8(src_ptr + 48);
    const uint8x16_t b_0 = vld1q_u8(ref_ptr);
    const uint8x16_t b_1 = vld1q_u8(ref_ptr + 16);
    const uint8x16_t b_2 = vld1q_u8(ref_ptr + 32);
    const uint8x16_t b_3 = vld1q_u8(ref_ptr + 48);
    const uint8x16_t c_0 = vld1q_u8(second_pred);
    const uint8x16_t c_1 = vld1q_u8(second_pred + 16);
    const uint8x16_t c_2 = vld1q_u8(second_pred + 32);
    const uint8x16_t c_3 = vld1q_u8(second_pred + 48);
    const uint8x16_t avg_0 = vrhaddq_u8(b_0, c_0);
    const uint8x16_t avg_1 = vrhaddq_u8(b_1, c_1);
    const uint8x16_t avg_2 = vrhaddq_u8(b_2, c_2);
    const uint8x16_t avg_3 = vrhaddq_u8(b_3, c_3);
    const uint8x16_t sad_0_u8 = vabdq_u8(a_0, avg_0);
    const uint8x16_t sad_1_u8 = vabdq_u8(a_1, avg_1);
    const uint8x16_t sad_2_u8 = vabdq_u8(a_2, avg_2);
    const uint8x16_t sad_3_u8 = vabdq_u8(a_3, avg_3);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    second_pred += 64;
    prod = vdotq_u32(prod, sad_0_u8, ones);
    prod = vdotq_u32(prod, sad_1_u8, ones);
    prod = vdotq_u32(prod, sad_2_u8, ones);
    prod = vdotq_u32(prod, sad_3_u8, ones);
  }
  return prod;
}
#else
static INLINE uint32x4_t sad64x(const uint8_t *src_ptr, int src_stride,
                                const uint8_t *ref_ptr, int ref_stride,
                                const int height) {
  int i;
  uint16x8_t abs_0 = vdupq_n_u16(0);
  uint16x8_t abs_1 = vdupq_n_u16(0);

  for (i = 0; i < height; ++i) {
    const uint8x16_t a_0 = vld1q_u8(src_ptr);
    const uint8x16_t a_1 = vld1q_u8(src_ptr + 16);
    const uint8x16_t a_2 = vld1q_u8(src_ptr + 32);
    const uint8x16_t a_3 = vld1q_u8(src_ptr + 48);
    const uint8x16_t b_0 = vld1q_u8(ref_ptr);
    const uint8x16_t b_1 = vld1q_u8(ref_ptr + 16);
    const uint8x16_t b_2 = vld1q_u8(ref_ptr + 32);
    const uint8x16_t b_3 = vld1q_u8(ref_ptr + 48);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    abs_0 = vabal_u8(abs_0, vget_low_u8(a_0), vget_low_u8(b_0));
    abs_0 = vabal_u8(abs_0, vget_high_u8(a_0), vget_high_u8(b_0));
    abs_0 = vabal_u8(abs_0, vget_low_u8(a_1), vget_low_u8(b_1));
    abs_0 = vabal_u8(abs_0, vget_high_u8(a_1), vget_high_u8(b_1));
    abs_1 = vabal_u8(abs_1, vget_low_u8(a_2), vget_low_u8(b_2));
    abs_1 = vabal_u8(abs_1, vget_high_u8(a_2), vget_high_u8(b_2));
    abs_1 = vabal_u8(abs_1, vget_low_u8(a_3), vget_low_u8(b_3));
    abs_1 = vabal_u8(abs_1, vget_high_u8(a_3), vget_high_u8(b_3));
  }

  {
    const uint32x4_t sum = vpaddlq_u16(abs_0);
    return vpadalq_u16(sum, abs_1);
  }
}

static INLINE uint32x4_t sad64x_avg(const uint8_t *src_ptr, int src_stride,
                                    const uint8_t *ref_ptr, int ref_stride,
                                    const uint8_t *second_pred,
                                    const int height) {
  int i;
  uint16x8_t abs_0 = vdupq_n_u16(0);
  uint16x8_t abs_1 = vdupq_n_u16(0);

  for (i = 0; i < height; ++i) {
    const uint8x16_t a_0 = vld1q_u8(src_ptr);
    const uint8x16_t a_1 = vld1q_u8(src_ptr + 16);
    const uint8x16_t a_2 = vld1q_u8(src_ptr + 32);
    const uint8x16_t a_3 = vld1q_u8(src_ptr + 48);
    const uint8x16_t b_0 = vld1q_u8(ref_ptr);
    const uint8x16_t b_1 = vld1q_u8(ref_ptr + 16);
    const uint8x16_t b_2 = vld1q_u8(ref_ptr + 32);
    const uint8x16_t b_3 = vld1q_u8(ref_ptr + 48);
    const uint8x16_t c_0 = vld1q_u8(second_pred);
    const uint8x16_t c_1 = vld1q_u8(second_pred + 16);
    const uint8x16_t c_2 = vld1q_u8(second_pred + 32);
    const uint8x16_t c_3 = vld1q_u8(second_pred + 48);
    const uint8x16_t avg_0 = vrhaddq_u8(b_0, c_0);
    const uint8x16_t avg_1 = vrhaddq_u8(b_1, c_1);
    const uint8x16_t avg_2 = vrhaddq_u8(b_2, c_2);
    const uint8x16_t avg_3 = vrhaddq_u8(b_3, c_3);
    src_ptr += src_stride;
    ref_ptr += ref_stride;
    second_pred += 64;
    abs_0 = vabal_u8(abs_0, vget_low_u8(a_0), vget_low_u8(avg_0));
    abs_0 = vabal_u8(abs_0, vget_high_u8(a_0), vget_high_u8(avg_0));
    abs_0 = vabal_u8(abs_0, vget_low_u8(a_1), vget_low_u8(avg_1));
    abs_0 = vabal_u8(abs_0, vget_high_u8(a_1), vget_high_u8(avg_1));
    abs_1 = vabal_u8(abs_1, vget_low_u8(a_2), vget_low_u8(avg_2));
    abs_1 = vabal_u8(abs_1, vget_high_u8(a_2), vget_high_u8(avg_2));
    abs_1 = vabal_u8(abs_1, vget_low_u8(a_3), vget_low_u8(avg_3));
    abs_1 = vabal_u8(abs_1, vget_high_u8(a_3), vget_high_u8(avg_3));
  }

  {
    const uint32x4_t sum = vpaddlq_u16(abs_0);
    return vpadalq_u16(sum, abs_1);
  }
}
#endif

#define SAD64XN(n)                                                            \
  uint32_t vpx_sad64x##n##_neon(const uint8_t *src_ptr, int src_stride,       \
                                const uint8_t *ref_ptr, int ref_stride) {     \
    const uint32x4_t abs =                                                    \
        sad64x(src_ptr, src_stride, ref_ptr, ref_stride, n);                  \
    return horizontal_add_uint32x4(abs);                                      \
  }                                                                           \
                                                                              \
  uint32_t vpx_sad64x##n##_avg_neon(const uint8_t *src_ptr, int src_stride,   \
                                    const uint8_t *ref_ptr, int ref_stride,   \
                                    const uint8_t *second_pred) {             \
    const uint32x4_t abs =                                                    \
        sad64x_avg(src_ptr, src_stride, ref_ptr, ref_stride, second_pred, n); \
    return horizontal_add_uint32x4(abs);                                      \
  }

SAD64XN(32)
SAD64XN(64)
