#include "Misc/Misc.h"
#include "Misc/Vector.h"
#include <omp.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define FOX 'F'
#define RABBIT 'R'
#define ROCK '*'
#define EMPTY ' '

#define NORTH 'N'
#define EAST 'E'
#define SOUTH 'S'
#define WEST 'W'
#define OUT_OF_BOUNDS '#'
#define INIT_HUNGER 0
#define INIT_PROC 0

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG: " fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) // Do nothing
#endif

struct gameInfo {
  short genProcRabbits, genProcFoxes, genFoodFoxes, r, c;
  size_t nGen, // num of generations to compute
      n;       // num objects in the initial game->boardM

  char *restrict boardM;
  char *restrict swapM;
  short *restrict procArrM;
  short *restrict procSwapM;
  short *restrict hungerArrM;
  short *restrict hungerSwapM;
};
void printStateTemp(const struct gameInfo *restrict game);
// ADJACENT CEnode HELPERS
// ----------------------------------------------------------------------------

static inline char checkDirection(const char result, const int condition) {
  return (condition) ? OUT_OF_BOUNDS : result;
}

// MOVE SELECTION LOGIC
// ----------------------------------------------------------------------------
// NOTE: Move selection should ALWAYS be made based on the game->boardM state
// and not the game->swapM state
static inline char _selectMove(const struct gameInfo *restrict const game,
                               const char target, const size_t g,
                               const size_t row, const size_t col) {
  size_t i = 0;
  const char *restrict const board = game->boardM;
  const size_t r = game->r, c = game->c;
  const size_t pos = c * row + col;
  char moves[4]; // priority - N,E,S,W

  // Can move N?
  if (checkDirection(board[pos - c], row == 0) == target)
    moves[i++] = NORTH;
  if (checkDirection(board[pos + 1], col + 1 == c) == target)
    moves[i++] = EAST;
  if (checkDirection(board[pos + c], row + 1 == r) == target)
    moves[i++] = SOUTH;
  if (checkDirection(board[pos - 1], col == 0) == target)
    moves[i++] = WEST;

  DEBUG_PRINT(
      "<SelectMove>\tBoard[Row][Col]=%c, G=%zu, Row=%zu, Col=%zu, I=%zu\n",
      game->boardM[game->c * row + col], g, row, col, i);
  return (i != 0) ? moves[(g + row + col) % i] : EMPTY;
}

static inline char selectMoveRabbit(const struct gameInfo *restrict const game,
                                    const size_t g, const size_t row,
                                    const size_t col) {
  return _selectMove(game, EMPTY, g, row, col);
}

static inline char selectMoveFox(const struct gameInfo *const restrict game,
                                 const size_t g, const size_t row,
                                 const size_t col) {
  const char rabbitDirection = _selectMove(game, RABBIT, g, row, col);
  return rabbitDirection == EMPTY ? _selectMove(game, EMPTY, g, row, col)
                                  : rabbitDirection;
}

// MOVE HANDLERS
// ----------------------------------------------------------------------------
static inline void handleMoveRabbit(struct Vector *const restrict head,
                                    const struct gameInfo *restrict game,
                                    const size_t gen, const size_t i,
                                    const size_t j) {
  size_t next_i = i, next_j = j;
  const size_t pos = game->c * i + j;
  const short proc = game->procArrM[pos];
  const bool canProc = (proc >= game->genProcRabbits);

  switch (selectMoveRabbit(game, gen, i, j)) {
  case NORTH:
    next_i--;
    break;
  case SOUTH:
    next_i++;
    break;
  case EAST:
    next_j++;
    break;
  case WEST:
    next_j--;
    break;
  case EMPTY:
    goto noMove;
    break;
  }

  if (canProc) { // Proc

    addNode(head, (struct Entry){
                      .i = next_i,
                      .j = next_j,
                      .procreated = NORTH,
                      .proc = INIT_PROC,
                  });
    addNode(head, (struct Entry){
                      .i = i,
                      .j = j,
                      .procreated = EMPTY,
                      .proc = INIT_PROC,
                  });

  } else {
  noMove:
    addNode(head, (struct Entry){.i = next_i,
                                 .j = next_j,
                                 .procreated = EMPTY,
                                 .proc = proc + 1});
  }

  DEBUG_PRINT("<HandleMoveRab>\tR(%zu,%zu) next=(%zu,%zu) "
              "game->swapM(next)='%c' proc=%d\n",
              i, j, next_i, next_j, game->swapM[game->c * next_i + next_j],
              game->procSwapM[game->c * next_i + next_j]);
}

static inline void handleMoveFox(struct Vector *const restrict head,
                                 const struct gameInfo *const restrict game,
                                 const size_t gen, const size_t i,
                                 const size_t j) {
  size_t next_i = i, next_j = j;
  const size_t pos = game->c * i + j;
  const short proc = game->procArrM[pos];
  const short hunger = game->hungerArrM[pos];
  const bool canProc = (proc >= game->genProcFoxes);

  const char direction = selectMoveFox(game, gen, i, j);
  switch (direction) {
  case NORTH:
    next_i--;
    break;
  case SOUTH:
    next_i++;
    break;
  case EAST:
    next_j++;
    break;
  case WEST:
    next_j--;
    break;
  case EMPTY:
    goto noMoveFox;
    break;
  }

  if (canProc) {

    addNode(head, (struct Entry){.i = next_i,
                                 .j = next_j,
                                 .procreated = direction,
                                 .proc = INIT_PROC,
                                 .hunger = hunger + 1});

  } else {
  noMoveFox:

    addNode(head, (struct Entry){.i = next_i,
                                 .j = next_j,
                                 .procreated = EMPTY,
                                 .proc = proc + 1,
                                 .hunger = hunger + 1});
  }

  DEBUG_PRINT(
      "<HandleMoveFox>\tF(%zu,%zu) next=(%zu,%zu) game->swapM(next)='%c' "
      "proc=%d hunger(next)=%d\n",
      i, j, next_i, next_j, game->swapM[game->c * next_i + next_j],
      game->procSwapM[game->c * next_i + next_j],
      game->hungerSwapM[game->c * next_i + next_j]);
}

static inline void calculateFoxState(struct Vector head[],
                                     struct gameInfo *restrict game,
                                     const size_t gen) {
  const char *restrict const board = game->boardM;
  const size_t r = game->r, c = game->c;
  char *restrict const swap = game->swapM;
  short *restrict const procSwap = game->procSwapM;
  const short *restrict const proc = game->procArrM;
#pragma omp parallel for collapse(1) schedule(guided)                          \
    shared(board, r, c, head, gen, swap, procSwap, proc)
  for (size_t i = 0; i < r; i++) {
    const int id = omp_get_thread_num();
    struct Vector *restrict const vec = &head[id];

#pragma GCC ivdep
    for (size_t j = 0; j < c; j++) {
      const size_t pos = c * i + j;

      switch (board[pos]) {
      case FOX:
        handleMoveFox(vec, game, gen, i, j);
        break;

      case RABBIT:
        swap[pos] = RABBIT;
        procSwap[pos] = proc[pos];
        break;

      case ROCK:
        swap[pos] = ROCK;
        break;
      }
    }
  }
}

// UPDATE RABBIT POSITIONS
// ----------------------------------------------------------------------------
static inline void calculateRabbitState(struct Vector head[],
                                        struct gameInfo *restrict const game,
                                        const size_t gen) {

  const char *restrict const board = game->boardM;
  const size_t r = game->r, c = game->c;
  char *restrict const swap = game->swapM;
  short *restrict const procSwap = game->procSwapM;
  short *restrict const proc = game->procArrM;
  short *restrict const hungerSwap = game->hungerSwapM;
  short *restrict const hunger = game->hungerArrM;

#pragma omp parallel for collapse(1) schedule(guided)                          \
    shared(board, r, c, head, gen)
  for (size_t i = 0; i < r; i++) {
    const int id = omp_get_thread_num();
    struct Vector *restrict const vec = &head[id];
#pragma GCC ivdep
    for (size_t j = 0; j < c; j++) {
      const size_t pos = c * i + j;
      switch (board[pos]) {
      case RABBIT:
        handleMoveRabbit(vec, game, gen, i, j);
        break;
      case FOX:
        swap[pos] = FOX;
        procSwap[pos] = proc[pos];
        hungerSwap[pos] = hunger[pos];
        break;

      case ROCK:
        swap[pos] = ROCK;
        break;
      }
    }
  }
}

static inline int handleRabbit(const short proc, const size_t pos,
                               short *restrict const procSwap,
                               char *restrict const swap) {

  switch (swap[pos]) {
  case EMPTY:
    swap[pos] = RABBIT;
    procSwap[pos] = proc;
    return 0;
  case RABBIT:
    procSwap[pos] = MAX(proc, procSwap[pos]);
    return -1;
  }
  return 0;
}
static inline int handleFox(const short hunger, const short proc,
                            const size_t pos, short *restrict const hungerSwap,
                            short *restrict const procSwap,
                            char *restrict const swap) {

  switch (swap[pos]) {
  case EMPTY:
    swap[pos] = FOX;
    procSwap[pos] = proc;
    hungerSwap[pos] = hunger;
    return 0;
  case FOX:
    if (proc > procSwap[pos]) {
      procSwap[pos] = proc;
    } else if (proc == procSwap[pos]) {
      procSwap[pos] = proc;
      if (hunger < hungerSwap[pos])
        hungerSwap[pos] = hunger;
    }
    return -1;
  case RABBIT:
    swap[pos] = FOX;
    procSwap[pos] = proc;
    hungerSwap[pos] = INIT_HUNGER;
    return -1;
  }
  return 0;
}

static inline void updateFoxState(struct Vector node[],
                                  struct gameInfo *restrict game) {
  const char *restrict const board = game->boardM;
  const size_t c = game->c;
  char *restrict const swap = game->swapM;
  short *restrict const procSwap = game->procSwapM;
  short *restrict const hungerSwap = game->hungerSwapM;
  const short genFoodFoxes = game->genFoodFoxes;
  int n = 0;

  for (size_t id = 0; id < omp_get_max_threads(); id++) {
    struct Vector *restrict const vec = &node[id];
    for (size_t idx = 0; vec->size > idx; idx++) {
      struct Entry head = vec->data[idx];
      const size_t pos = c * head.i + head.j;

      if (board[pos] == RABBIT)
        head.hunger = 0;
      else if (head.hunger >= genFoodFoxes) {
        n--;
        continue;
      }

      if (head.procreated != EMPTY) {
        size_t childPos = pos;
        switch (head.procreated) {
        case NORTH:
          childPos += c;
          break;
        case SOUTH:
          childPos -= c;
          break;
        case EAST:
          childPos--;
          break;
        case WEST:
          childPos++;
          break;
        }
        n++;
        swap[childPos] = FOX;
        procSwap[childPos] = INIT_PROC;
        hungerSwap[childPos] = INIT_HUNGER;
      }
      n += handleFox(head.hunger, head.proc, pos, hungerSwap, procSwap, swap);
    }
    node[id].size = 0;
  }
  game->n += n;
}
static inline void updateRabbitState(struct Vector node[],
                                     struct gameInfo *restrict game) {

  const char *restrict const board = game->boardM;
  const size_t c = game->c;
  char *restrict const swap = game->swapM;
  short *restrict const procSwap = game->procSwapM;
  int n = 0;
  for (size_t id = 0; id < omp_get_max_threads(); id++) {
    struct Vector *restrict const vec = &node[id];
    for (size_t idx = 0; idx < vec->size; idx++) {
      const struct Entry head = vec->data[idx];
      const size_t pos = c * head.i + head.j;

      if (head.procreated != EMPTY)
        n++;

      n += handleRabbit(head.proc, pos, procSwap, swap);
    }
    node[id].size = 0;
  }
  game->n += n;

  simdMemcpyAndSet(game->boardM, swap, EMPTY, game->r * c);
  simdMemcpyAndSet(game->procArrM, procSwap, INIT_PROC, game->r * c);
  simdMemcpyAndSet(game->hungerArrM, game->hungerSwapM, INIT_HUNGER,
                   game->r * c);
}
// SWITCH ACTIVE BOARDS AND CLEAR SWAPs
// ----------------------------------------------------------------------------
static inline void swapAndClearBoard(struct gameInfo *restrict game) {
  game->boardM = (char *)((size_t)game->boardM ^ (size_t)game->swapM);
  game->swapM = (char *)((size_t)game->boardM ^ (size_t)game->swapM);
  game->boardM = (char *)((size_t)game->boardM ^ (size_t)game->swapM);

  game->procArrM = (short *)((size_t)game->procArrM ^ (size_t)game->procSwapM);
  game->procSwapM = (short *)((size_t)game->procArrM ^ (size_t)game->procSwapM);
  game->procArrM = (short *)((size_t)game->procArrM ^ (size_t)game->procSwapM);

  game->hungerArrM =
      (short *)((size_t)game->hungerArrM ^ (size_t)game->hungerSwapM);
  game->hungerSwapM =
      (short *)((size_t)game->hungerArrM ^ (size_t)game->hungerSwapM);
  game->hungerArrM =
      (short *)((size_t)game->hungerArrM ^ (size_t)game->hungerSwapM);

  simdMemset(game->swapM, EMPTY, game->r * game->c);
  simdMemset(game->procSwapM, INIT_PROC, game->r * game->c);
  simdMemset(game->hungerSwapM, INIT_HUNGER, game->r * game->c);
}
// PRINT game->BOARDM
// ----------------------------------------------------------------------------

void printProcRowT(const struct gameInfo *restrict game,
                   const short *restrict m, const size_t i) {
  for (size_t j = 0; j < game->c; j++) {

    const size_t pos = game->c * i + j;
    if (game->swapM[pos] == RABBIT || game->swapM[pos] == FOX)
      printf("%hd", m[pos]);
    else
      printf("%c", game->swapM[pos]);
  }
}
void printHungerRowT(const struct gameInfo *restrict game,
                     const short *restrict m, const size_t i) {
  for (size_t j = 0; j < game->c; j++) {
    const size_t pos = game->c * i + j;
    if (game->swapM[pos] == FOX) {
      printf("%hd", m[pos]);
    } else {
      printf("%c", game->swapM[pos]);
    }
  }
}
void printProcRow(const struct gameInfo *restrict game, const short *restrict m,
                  const size_t i) {
  for (size_t j = 0; j < game->c; j++) {
    const size_t pos = game->c * i + j;
    if (game->boardM[pos] == RABBIT || game->boardM[pos] == FOX)
      printf("%hd", m[pos]);
    else
      printf("%c", game->boardM[pos]);
  }
}
void printHungerRow(const struct gameInfo *restrict game,
                    const short *restrict m, const size_t i) {
  for (size_t j = 0; j < game->c; j++) {
    const size_t pos = game->c * i + j;
    if (game->boardM[pos] == FOX) {
      printf("%hd", m[pos]);
    } else {
      printf("%c", game->boardM[pos]);
    }
  }
}
void printDashes(const size_t c) {
  for (size_t i = 0; i < c + 2; i++)
    putchar('-');
  printf("   P");
  for (size_t i = 0; i < c + 1; i++)
    putchar('-');
  printf(" H");
  for (size_t i = 0; i < c + 1; i++)
    putchar('-');
  printf("\n");
}

void printStateTemp(const struct gameInfo *restrict game) {
  printDashes(game->c);

  for (size_t i = 0; i < game->r; i++) {
    printf("|");
    for (size_t j = 0; j < game->c; j++)
      printf("%c", game->swapM[game->c * i + j]);
    printf("|   |");

    printProcRowT(game, game->procSwapM, i);
    printf("| |");

    printHungerRowT(game, game->hungerSwapM, i);
    printf("|\n");
  }
  printDashes(game->c);
}
void printState(const struct gameInfo *restrict game) {
  printDashes(game->c);

  for (size_t i = 0; i < game->r; i++) {
    printf("|");
    for (size_t j = 0; j < game->c; j++)
      printf("%c", game->boardM[game->c * i + j]);
    printf("|   |");

    printProcRow(game, game->procArrM, i);
    printf("| |");

    printHungerRow(game, game->hungerArrM, i);
    printf("|\n");
  }
  printDashes(game->c);
}

// void a(int x) {
//   if (x == 3)
//     return;
//
// #pragma omp task
//   {
//
// #pragma omp task untied
//     {
//       printf("%d %d\n", x, omp_get_thread_num());
//       a(x + 1);
//     }
//     printf("%d done:%d\n", x, omp_get_thread_num());
// #pragma omp task untied
//     {
//       printf("%d %d\n", x, omp_get_thread_num());
//       a(x + 1);
//     }
//   }
// }
//
// #pragma omp parallel
//   {
// #pragma omp single
//     a(0);
//   }
int main(int argc, char *argv[]) {

  struct gameInfo game;
  if (fscanf(stdin, "%hu %hu %hu %zu %hu %hu %zu", &game.genProcRabbits,
             &game.genProcFoxes, &game.genFoodFoxes, &game.nGen, &game.r,
             &game.c, &game.n) < 0) {
    printf("Unable to read initial values\n");
    return EXIT_FAILURE;
  }

  alignas(ALIGNMENT) char boardM[game.r * game.c];
  alignas(ALIGNMENT) char swapM[game.r * game.c];

  alignas(ALIGNMENT) short
      procArrM[game.r * game.c]; // track the number of generations since
  alignas(ALIGNMENT) short procSwapM[game.r * game.c];

  alignas(ALIGNMENT) short
      hungerArrM[game.r * game.c]; // track the number of generations since
  alignas(ALIGNMENT) short hungerSwapM[game.r * game.c];

  game.boardM = boardM;
  game.swapM = swapM;
  game.procArrM = procArrM;
  game.procSwapM = procSwapM;
  game.hungerArrM = hungerArrM;
  game.hungerSwapM = hungerSwapM;

  // INIT game->BOARDM
  // ----------------------------------------------------------------------------
  simdMemset(game.boardM, EMPTY, game.r * game.c);
  simdMemset(game.swapM, EMPTY, game.r * game.c);

  simdMemset(game.procArrM, INIT_PROC, game.r * game.c);
  simdMemset(game.procSwapM, INIT_PROC, game.r * game.c);

  simdMemset(game.hungerArrM, INIT_HUNGER, game.r * game.c);
  simdMemset(game.hungerSwapM, INIT_HUNGER, game.r * game.c);

  for (size_t i = 0; i < game.n; i++) {
    char buf[16];
    size_t x, y;
    if (fscanf(stdin, "%s %zu %zu", buf, &x, &y) < 0) {
      printf("Unable to read values of game\n");
      return EXIT_FAILURE;
    }
    if (!strcmp(buf, "ROCK")) {
      game.boardM[game.c * x + y] = ROCK;
    } else if (!strcmp(buf, "FOX")) {
      game.boardM[game.c * x + y] = FOX;
      game.hungerArrM[game.c * x + y] = 0;
    } else if (!strcmp(buf, "RABBIT")) {
      game.boardM[game.c * x + y] = RABBIT;
    } else {
      printf("Not a valid option\n");
      return EXIT_FAILURE;
    }
  }

#ifdef DEBUG
  printf("\nGen 0\n");
  printState(&game);
#endif
  const size_t threads = omp_get_max_threads();
  const double startTime = omp_get_wtime();
  struct Vector vec[threads];

#pragma omp parallel for
  for (size_t idx = 0; idx < threads; idx++)
    vec[idx] = createVector(game.n);

  for (size_t gen = 0; gen < game.nGen; gen++) {
    calculateRabbitState(vec, &game, gen);
    updateRabbitState(vec, &game);

    calculateFoxState(vec, &game, gen);
    updateFoxState(vec, &game);

    // prepare for next generation
    swapAndClearBoard(&game);
#ifdef DEBUG
    printf("\nGen %ld\n", gen + 1);
    printState(&game);
#endif
  }
  const double endTime = omp_get_wtime();

#pragma omp parallel for
  for (size_t idx = 0; idx < threads; idx++)
    freeVector(&vec[idx]);

#ifdef PRINT
  // PRINT FINAL STATE
  printf("%d %d %d %d %zd %zd %zd\n", game.genProcRabbits, game.genProcFoxes,
         game.genFoodFoxes, 0, game.r, game.c, game.n);
  for (size_t i = 0; i < game.r; i++) {
    for (size_t j = 0; j < game.c; j++) {
      switch (game.boardM[game.c * i + j]) {
      case RABBIT:
        printf("%s %zu %zu\n", "RABBIT", i, j);
        break;
      case FOX:
        printf("%s %zu %zu\n", "FOX", i, j);
        break;
      case ROCK:
        printf("%s %zu %zu\n", "ROCK", i, j);
        break;
      }
    }
  }
  printf("\n");
#endif

#ifdef BENCHMARK
  printf("Execution time: %f seconds\n", endTime - startTime);
#endif

  return EXIT_SUCCESS;
}
