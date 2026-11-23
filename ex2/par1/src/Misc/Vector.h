#ifndef __VECTOR__HEADER__
#define __VECTOR__HEADER__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
struct Entry {
  size_t i, j;
  short hunger, proc;
  bool procreated;
};
struct Vector {
  size_t size;
  size_t capacity;
  struct Entry *data;
};

static inline struct Vector createVector(const size_t initialCapacity) {
  return (struct Vector){.size = 0,
                         .capacity = initialCapacity,
                         .data =
                             malloc(initialCapacity * sizeof(struct Entry))};
}
static inline void addNode(struct Vector *restrict const vec,
                           const struct Entry entry) {

  if (vec->size == vec->capacity) {
    vec->capacity <<= 1;
    vec->data = realloc(vec->data, vec->capacity * sizeof(struct Entry));
  }
  vec->data[vec->size++] = entry;
}
static inline void freeVector(struct Vector *restrict const vec) {
  if (vec->data)
    free(vec->data);
}

#endif // !VECTOR
