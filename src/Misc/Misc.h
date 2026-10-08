#ifndef __MISC__HEADER__
#define __MISC__HEADER__
#include <immintrin.h>
#include <stdlib.h>
#include <string.h>
#define ALIGNMENT 32

static inline int *alloc(const int size) {
  int totalSize = size * size * sizeof(int);
  totalSize += ALIGNMENT;
  totalSize &= ~(ALIGNMENT - 1);
  return (int *)aligned_alloc(ALIGNMENT, totalSize);
}

static inline void simdMemset(int *restrict dest, const int val,
                              const size_t size) {

  size_t idx = 0;
  const __m256i valSIMD256 = _mm256_set1_epi32(val);
#pragma GCC ivdep
  for (; idx + 7 < size; idx += 8)
    _mm256_store_si256((__m256i *)&dest[idx], valSIMD256);

  const __m128i valSIMD = _mm_set1_epi32(val);
#pragma GCC ivdep
  for (; idx + 3 < size; idx += 4)
    _mm_store_si128((__m128i *)&dest[idx], valSIMD);

  if (idx < size)
    memset(&dest[idx], val, (size - idx) * sizeof(int));
}
static inline void simdMemcpy(int *restrict dest, const int *restrict src,
                              const size_t size) {

  size_t idx = 0;
#pragma GCC ivdep
  for (; idx + 7 < size; idx += 8) {
    const __m256i srcSIMD = _mm256_load_si256((const __m256i *)&src[idx]);
    _mm256_store_si256((__m256i *)&dest[idx], srcSIMD);
  }
#pragma GCC ivdep
  for (; idx + 3 < size; idx += 4) {
    const __m128i srcSIMD = _mm_load_si128((const __m128i *)&src[idx]);
    _mm_store_si128((__m128i *)&dest[idx], srcSIMD);
  }

  if (idx < size)
    memcpy(&dest[idx], &src[idx], (size - idx) * sizeof(int));
}

static inline void simdMemcpyAndSet(int *restrict dest, int *restrict src,
                                    const int val, const size_t size) {

  size_t idx = 0;

#pragma GCC ivdep
  for (; idx + 7 < size; idx += 8) {
    const __m256i srcSIMD = _mm256_load_si256((__m256i *)&src[idx]);
    _mm256_store_si256((__m256i *)&dest[idx], srcSIMD);
    _mm256_store_si256((__m256i *)&src[idx], _mm256_set1_epi32(val));
  }
#pragma GCC ivdep
  for (; idx + 3 < size; idx += 4) {
    const __m128i srcSIMD = _mm_load_si128((__m128i *)&src[idx]);
    _mm_store_si128((__m128i *)&dest[idx], srcSIMD);
    _mm_store_si128((__m128i *)&src[idx], _mm_set1_epi32(val));
  }

  if (idx < size) {
    memcpy(&dest[idx], &src[idx], (size - idx) * sizeof(int));
    memset(&src[idx], val, (size - idx) * sizeof(int));
  }
}
static inline void simdMinimum(int *restrict dest, const int *restrict src,
                               const size_t size) {

  size_t idx = 0;
#pragma GCC ivdep
  for (; idx + 7 < size * size; idx += 8) {
    const __m256i srcSIMD = _mm256_load_si256((const __m256i *)&src[idx]);
    const __m256i destSIMD = _mm256_load_si256((__m256i *)&dest[idx]);
    const __m256i min = _mm256_min_epi32(destSIMD, srcSIMD);
    _mm256_store_si256((__m256i *)&dest[idx], min);
  }
#pragma GCC ivdep
  for (; idx + 3 < size * size; idx += 4) {
    const __m128i srcSIMD = _mm_load_si128((const __m128i *)&src[idx]);
    const __m128i destSIMD = _mm_load_si128((__m128i *)&dest[idx]);
    const __m128i min = _mm_min_epi32(destSIMD, srcSIMD);
    _mm_store_si128((__m128i *)&dest[idx], min);
  }

#pragma GCC ivdep
  for (; idx < size * size; idx++)
    if ((long)(src[idx]) < dest[idx])
      dest[idx] = src[idx];
}
#endif // !MISC.H
