#ifndef __MISC__HEADER__
#define __MISC__HEADER__
#include <immintrin.h>
#include <stdlib.h>
#include <string.h>
#define ALIGNMENT 32

static inline void simdMemsetChar(char *restrict dest, const char val,
                                  const size_t size) {

  // Has the same functionality as memset except we use SIMD instructions
  size_t idx = 0;
#if defined(__AVX2__)
  const __m256i valSIMD256 = _mm256_set1_epi8(val);
#pragma GCC ivdep
  for (; idx + 31 < size; idx += 32)
    _mm256_store_si256((__m256i *)&dest[idx], valSIMD256);
#endif

#if defined(__SSE4_1__)
  const __m128i valSIMD = _mm_set1_epi8(val);
#pragma GCC ivdep
  for (; idx + 15 < size; idx += 16)
    _mm_store_si128((__m128i *)&dest[idx], valSIMD);
#pragma GCC ivdep
  for (; idx + 7 < size; idx += 8)
    _mm_store_si128((__m128i *)&dest[idx], valSIMD);

#endif

  if (idx < size)
    memset(&dest[idx], val, (size - idx) * sizeof(char));
}
static inline void simdMemcpyChar(char *restrict dest, const char *restrict src,
                                  const size_t size) {

  size_t idx = 0;
  // Has the same functionality as memcpy except we use SIMD instructions
#if defined(__AVX2__)
#pragma GCC ivdep
  for (; idx + 31 < size; idx += 32) {
    const __m256i srcSIMD = _mm256_load_si256((const __m256i *)&src[idx]);
    _mm256_store_si256((__m256i *)&dest[idx], srcSIMD);
  }
#endif
#if defined(__SSE4_1__)
#pragma GCC ivdep
  for (; idx + 15 < size; idx += 16) {
    const __m128i srcSIMD = _mm_load_si128((const __m128i *)&src[idx]);
    _mm_store_si128((__m128i *)&dest[idx], srcSIMD);
  }
#pragma GCC ivdep
  for (; idx + 7 < size; idx += 8) {
    const __m128i srcSIMD = _mm_load_si128((const __m128i *)&src[idx]);
    _mm_store_si128((__m128i *)&dest[idx], srcSIMD);
  }
#endif

  if (idx < size)
    memcpy(&dest[idx], &src[idx], (size - idx) * sizeof(char));
}

static inline void simdMemcpyAndSetChar(char *restrict dest, char *restrict src,
                                        const char val, const size_t size) {

  size_t idx = 0;

  // Has the same functionality as memcpy and memset except we use SIMD
  // instructions
#if defined(__AVX2__)
#pragma GCC ivdep
  for (; idx + 31 < size; idx += 32) {
    const __m256i srcSIMD = _mm256_load_si256((__m256i *)&src[idx]);
    _mm256_store_si256((__m256i *)&dest[idx], srcSIMD);
    _mm256_store_si256((__m256i *)&src[idx], _mm256_set1_epi8(val));
  }
#endif
#if defined(__SSE4_1__)
#pragma GCC ivdep
  for (; idx + 15 < size; idx += 16) {
    const __m128i srcSIMD = _mm_load_si128((__m128i *)&src[idx]);
    _mm_store_si128((__m128i *)&dest[idx], srcSIMD);
    _mm_store_si128((__m128i *)&src[idx], _mm_set1_epi8(val));
  }
#pragma GCC ivdep
  for (; idx + 7 < size; idx += 8) {
    const __m128i srcSIMD = _mm_load_si128((__m128i *)&src[idx]);
    _mm_store_si128((__m128i *)&dest[idx], srcSIMD);
    _mm_store_si128((__m128i *)&src[idx], _mm_set1_epi8(val));
  }
#endif

  if (idx < size) {
    memcpy(&dest[idx], &src[idx], (size - idx) * sizeof(char));
    memset(&src[idx], val, (size - idx) * sizeof(char));
  }
}

static inline void simdMemsetShort(short *restrict dest, const short val,
                                   const size_t size) {

  // Has the same functionality as memset except we use SIMD instructions
  size_t idx = 0;
#if defined(__AVX2__)
  const __m256i valSIMD256 = _mm256_set1_epi16(val);
#pragma GCC ivdep
  for (; idx + 15 < size; idx += 16)
    _mm256_store_si256((__m256i *)&dest[idx], valSIMD256);
#endif

#if defined(__SSE4_1__)
  const __m128i valSIMD = _mm_set1_epi16(val);
#pragma GCC ivdep
  for (; idx + 7 < size; idx += 8)
    _mm_store_si128((__m128i *)&dest[idx], valSIMD);
#endif

  if (idx < size)
    memset(&dest[idx], val, (size - idx) * sizeof(short));
}
static inline void simdMemcpyShort(short *restrict dest,
                                   const short *restrict src,
                                   const size_t size) {

  size_t idx = 0;
  // Has the same functionality as memcpy except we use SIMD instructions
#if defined(__AVX2__)
#pragma GCC ivdep
  for (; idx + 15 < size; idx += 16) {
    const __m256i srcSIMD = _mm256_load_si256((const __m256i *)&src[idx]);
    _mm256_store_si256((__m256i *)&dest[idx], srcSIMD);
  }
#endif
#if defined(__SSE4_1__)
#pragma GCC ivdep
  for (; idx + 7 < size; idx += 8) {
    const __m128i srcSIMD = _mm_load_si128((const __m128i *)&src[idx]);
    _mm_store_si128((__m128i *)&dest[idx], srcSIMD);
  }
#endif

  if (idx < size)
    memcpy(&dest[idx], &src[idx], (size - idx) * sizeof(short));
}

static inline void simdMemcpyAndSetShort(short *restrict dest,
                                         short *restrict src, const short val,
                                         const size_t size) {

  size_t idx = 0;

  // Has the same functionality as memcpy and memset except we use SIMD
  // instructions
#if defined(__AVX2__)
#pragma GCC ivdep
  for (; idx + 15 < size; idx += 16) {
    const __m256i srcSIMD = _mm256_load_si256((__m256i *)&src[idx]);
    _mm256_store_si256((__m256i *)&dest[idx], srcSIMD);
    _mm256_store_si256((__m256i *)&src[idx], _mm256_set1_epi16(val));
  }
#endif
#if defined(__SSE4_1__)
#pragma GCC ivdep
  for (; idx + 7 < size; idx += 8) {
    const __m128i srcSIMD = _mm_load_si128((__m128i *)&src[idx]);
    _mm_store_si128((__m128i *)&dest[idx], srcSIMD);
    _mm_store_si128((__m128i *)&src[idx], _mm_set1_epi16(val));
  }
#endif

  if (idx < size) {
    memcpy(&dest[idx], &src[idx], (size - idx) * sizeof(short));
    memset(&src[idx], val, (size - idx) * sizeof(short));
  }
}

#define simdMemset(x, y, z)                                                    \
  _Generic((x), short *: simdMemsetShort, char *: simdMemsetChar)(x, y, z)

#define simdMemcpy(x, y, z)                                                    \
  _Generic((x), short *: simdMemcpyShort, char *: simdMemcpyChar)(x, y, z)
#define simdMemcpyAndSet(x, y, val, z)                                         \
  _Generic((x), short *: simdMemcpyAndSetShort, char *: simdMemcpyAndSetChar)( \
      x, y, val, z)
#endif // !MISC.H
