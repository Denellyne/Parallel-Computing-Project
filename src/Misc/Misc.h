#ifndef __MISC__HEADER__
#define __MISC__HEADER__
#include <immintrin.h>
#include <stdlib.h>
#include <string.h>

static inline int *alloc(const int size) {
  const int alignment = 32;
  int totalSize = size * size * sizeof(int);
  totalSize += alignment;
  totalSize &= ~31;
  return (int *)aligned_alloc(alignment, totalSize);
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
#endif // !MISC.H
