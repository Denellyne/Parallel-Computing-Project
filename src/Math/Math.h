#ifndef __MATH__HEADER__
#define __MATH__HEADER__
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

static inline void transpose(int *const restrict matrix,
                             const int *const restrict graph, const int size) {
  for (size_t i = 0; i < size; i++) {
    int *restrict mRow = &matrix[i * size];
    const int *restrict gCol = &graph[i];
#pragma GCC ivdep
    for (size_t j = 0; j < size; j++)
      mRow[j] = gCol[j * size];
  }
}

// static inline void matrixMultiply(int *restrict matrix,
//                                   const int *const restrict A,
//                                   const int *const restrict transposed,
//                                   const int size) {
//   for (int row = 0; row < size; row += 8) {
//     // int *const restrict mRow = &matrix[row * size];
//     // const int *const restrict graphRow = &A[row * size];
//
//     __m256i graphSIMD = _mm256_load_si256((__m256i *)&A[row * size]);
//     for (int col = 0; col < size; col += 8) {
//
//       __m256i transSIMD = _mm256_load_si256((__m256i *)&transposed[col *
//       size]);
//       __m256i matrixSIMD =
//           _mm256_load_si256((__m256i *)(__m256i *)&matrix[row * size + col]);
//       graphSIMD = _mm256_add_epi32(graphSIMD, transSIMD);
//       __m256i min = _mm256_min_epi32(graphSIMD,
//       _mm256_set1_epi32(INT16_MAX)); matrixSIMD =
//       _mm256_min_epi32(matrixSIMD, kin); _mm256_store_si256((__m256i
//       *)&matrix[row * size + col], matrixSIMD);
//     }
//   }
// }
static inline void matrixMultiply(int *restrict matrix,
                                  const int *const restrict A,
                                  const int *const restrict transposed,
                                  const int size) {
#pragma GCC ivdep
  for (size_t row = 0; row < size; row++) {
    int *const restrict mRow = &matrix[row * size];
    const int *const restrict graphRow = &A[row * size];

#pragma GCC ivdep
    for (size_t col = 0; col < size; col++) {
      const int *const restrict transRow = &transposed[col * size];

      size_t k = 0;
      int minCost = INT16_MAX;

#pragma GCC ivdep
      for (; k + 7 < size; k += 8) {
        const __m256i graphSIMD =
            _mm256_loadu_si256((const __m256i *)&graphRow[k]);
        const __m256i transSIMD =
            _mm256_loadu_si256((const __m256i *)&transRow[k]);
        const __m256i sumSIMD = _mm256_add_epi32(graphSIMD, transSIMD);
        const __m256i min =
            _mm256_min_epi32(sumSIMD, _mm256_set1_epi32(INT16_MAX));
        int temp[8] __attribute__((aligned(32)));
        _mm256_store_si256((__m256i *)temp, min);

#pragma GCC ivdep
#pragma GCC unroll 8
        for (size_t i = 0; i < 8; i++)
          if (temp[i] < minCost)
            minCost = temp[i];
      }

#pragma GCC ivdep
      for (; k + 3 < size; k += 4) {
        const __m128i graphSIMD =
            _mm_loadu_si128((const __m128i *)&graphRow[k]);
        const __m128i transSIMD =
            _mm_loadu_si128((const __m128i *)&transRow[k]);
        const __m128i sumSIMD = _mm_add_epi32(graphSIMD, transSIMD);
        const __m128i min = _mm_min_epi32(sumSIMD, _mm_set1_epi32(INT16_MAX));
        int temp[4] __attribute__((aligned(32)));
        _mm_store_si128((__m128i *)temp, min);

#pragma GCC ivdep
#pragma GCC unroll 4
        for (size_t i = 0; i < 4; i++)
          if (temp[i] < minCost)
            minCost = temp[i];
      }
#pragma GCC ivdep
      for (; k < size; k++) {
        const int graphCost = graphRow[k];
        const int transCost = transRow[k];
        if ((unsigned long long)(graphCost + transCost) < minCost)
          minCost = (graphCost + transCost);
      }
      mRow[col] = minCost;
    }
  }
}
#endif // !MATH.H
