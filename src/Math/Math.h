#ifndef __MATH__HEADER__
#define __MATH__HEADER__
#include <immintrin.h>
#include <stdint.h>
#define INF INT16_MAX

// Transposes the matrix for better cache locality when multiplying
static inline void transpose(unsigned short *const restrict matrix,
                             const unsigned short *const restrict graph,
                             const size_t size) {
#pragma GCC ivdep
  for (size_t i = 0; i < size; i++) {
    unsigned short *restrict mRow = &matrix[i * size];
    const unsigned short *const restrict gCol = &graph[i];
#pragma GCC ivdep
    for (size_t j = 0; j < size; j++)
      mRow[j] = gCol[j * size];
  }
}

static inline void matrixMultiply(
    unsigned short *restrict matrix, const unsigned short *const restrict A,
    const unsigned short *const restrict transposed, const size_t size) {
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
    unsigned short *const restrict mRow = &matrix[row * size];
    const unsigned short *const restrict graphRow = &A[row * size];

#pragma GCC ivdep
    for (size_t col = 0; col < size; col++) {
      const unsigned short *const restrict transRow = &transposed[col * size];

      unsigned short temp[16] __attribute__((aligned(ALIGNMENT))) = {
          INF, INF, INF, INF, INF, INF, INF, INF,
          INF, INF, INF, INF, INF, INF, INF, INF,
      };
      size_t k = 0;
      // Here we operate on 16 32bit integers at the same time
#if defined(__AVX2__)
      {
        __m256i min = _mm256_load_si256((__m256i *)temp);
#pragma GCC ivdep
        for (; k + 15 < size; k += 16) {
          const __m256i graphSIMD =
              _mm256_loadu_si256((const __m256i *)&graphRow[k]);
          const __m256i transSIMD =
              _mm256_loadu_si256((const __m256i *)&transRow[k]);
          const __m256i sumSIMD = _mm256_add_epi16(graphSIMD, transSIMD);
          min = _mm256_min_epu16(sumSIMD, min);
        }

        _mm256_store_si256((__m256i *)temp, min);
      }

#endif
      // Here we operate on 8 32bit integers at the same time
#if defined(__SSE4_1__)
      {
        __m128i min = _mm_load_si128((__m128i *)temp);
#pragma GCC ivdep
        for (; k + 7 < size; k += 8) {
          const __m128i graphSIMD =
              _mm_loadu_si128((const __m128i *)&graphRow[k]);
          const __m128i transSIMD =
              _mm_loadu_si128((const __m128i *)&transRow[k]);
          const __m128i sumSIMD = _mm_add_epi16(graphSIMD, transSIMD);
          min = _mm_min_epu16(sumSIMD, min);
        }
        _mm_store_si128((__m128i *)temp, min);
      }

#endif

      int minCost = mRow[col];
#pragma GCC ivdep
      for (size_t i = 0; i < 16; i++)
        if (temp[i] < minCost)
          minCost = temp[i];
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
