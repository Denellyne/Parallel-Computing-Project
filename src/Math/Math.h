#ifndef __MATH__HEADER__
#define __MATH__HEADER__
#include <stdint.h>
#include <stdio.h>

static inline void transpose(int *restrict matrix,
                             const int *const restrict graph, const int size) {
  for (int i = 0; i < size; i++) {
    int *restrict mRow = &matrix[i * size];
    for (int j = 0; j < size; j++)
      mRow[j] = graph[j * size + i];
  }
}

static inline void matrixMultiply(int *restrict matrix,
                                  const int *const restrict trans,
                                  const int *const restrict graph,
                                  const int size) {
  for (int row = 0; row < size; row++) {
    int *restrict mRow = &matrix[row * size];
    const int *restrict graphRow = &graph[row * size];

    for (int col = 0; col < size; col++) {
      const int *restrict transRow = &trans[col * size];
      int minCost = INT32_MAX;

#pragma GCC ivdep
      for (int k = 0; k < size; k++) {
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
