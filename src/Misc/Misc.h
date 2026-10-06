#ifndef __MISC__HEADER__
#define __MISC__HEADER__
#include <stdlib.h>

static inline int *alloc(const int size) {
  return malloc(sizeof(int) * (size * size));
}
#endif // !MISC.H
