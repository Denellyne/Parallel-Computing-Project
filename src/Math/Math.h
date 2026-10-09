#ifndef __MATH__HEADER__
#define __MATH__HEADER__
#include <immintrin.h>
#define INF 1000000

// Transposes the matrix for better cache locality when multiplying
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

static inline void matrixMultiply(int *restrict matrix,
                                  const int *const restrict A,
                                  const int *const restrict transposed,
                                  const int size) {
  // These function like some others has a lot of 'weird' functions, these
  // functions take advantage of SIMD (Single Instruction Multiple Data)
  // registers to operate on multiple elements on the vector at the same time
  // making the code run significantly faster than scalar code
  // We also take caution to add the keywords restrict and const whenever
  // possible to give more information to the compiler for better compile time
  // optimization
  //
  // Multiple times throughout the code we can see
  // #pragma GCC ivdep, this serves to tell the compiler the vectors have no
  // dependencies extern to that loop it's basically just another hint to the
  // compiler so we can work faster
#pragma GCC ivdep
  for (size_t row = 0; row < size; row++) {
    int *const restrict mRow = &matrix[row * size];
    const int *const restrict graphRow = &A[row * size];

#pragma GCC ivdep
    for (size_t col = 0; col < size; col++) {
      const int *const restrict transRow = &transposed[col * size];

      size_t k = 0;
      int minCost = mRow[col];

      // Here we operate on 8 32bit integers at the same time
#if defined(__AVX2__)
      {
        int temp[8] __attribute__((aligned(ALIGNMENT)));
        __m256i min = _mm256_set1_epi32(minCost);
#pragma GCC ivdep
        for (; k + 7 < size; k += 8) {
          const __m256i graphSIMD =
              _mm256_loadu_si256((const __m256i *)&graphRow[k]);
          const __m256i transSIMD =
              _mm256_loadu_si256((const __m256i *)&transRow[k]);
          const __m256i sumSIMD = _mm256_add_epi32(graphSIMD, transSIMD);
          min = _mm256_min_epu32(sumSIMD, min);
        }

        _mm256_store_si256((__m256i *)temp, min);
#pragma GCC ivdep
        for (size_t i = 0; i < 8; i++)
          if (temp[i] < minCost)
            minCost = temp[i];
      }

#endif
      // Here we operate on 4 32bit integers at the same time
#if defined(__SSE4_1__)
      {
        int temp[4] __attribute__((aligned(ALIGNMENT)));
        __m128i min = _mm_set1_epi32(minCost);
#pragma GCC ivdep
        for (; k + 3 < size; k += 4) {
          const __m128i graphSIMD =
              _mm_loadu_si128((const __m128i *)&graphRow[k]);
          const __m128i transSIMD =
              _mm_loadu_si128((const __m128i *)&transRow[k]);
          const __m128i sumSIMD = _mm_add_epi32(graphSIMD, transSIMD);
          min = _mm_min_epu32(sumSIMD, min);
        }
        _mm_store_si128((__m128i *)temp, min);
#pragma GCC ivdep
        for (size_t i = 0; i < 4; i++)
          if (temp[i] < minCost)
            minCost = temp[i];
      }

#endif
      // If by any chance there is still some elements to process, less than 4,
      // we go back to scalar code and finish the calculations
#pragma GCC ivdep
      for (; k < size; k++) {
        const int graphCost = graphRow[k];
        const int transCost = transRow[k];
        if (graphCost + transCost < minCost)
          minCost = (graphCost + transCost);
      }
      mRow[col] = minCost;
    }
  }
}
#endif // !MATH.H
