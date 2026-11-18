#include "Misc/Misc.h"
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static inline void resetVector(struct Vector *const restrict vec) {
  vec->size = 0;
}
static inline struct Vector *restrict createVector() {
  struct Vector *restrict vec = malloc(sizeof(struct Vector));
  vec->size = 0;
  vec->capacity = 0;
  vec->data = NULL;
  return vec;
}
static inline void addNode(struct Vector *const restrict vec,
                           const struct Entry entry) {
  if (!vec->data) {
    vec->capacity = 4;
    vec->data = malloc(vec->capacity * sizeof(struct Entry));
  } else if (vec->size == vec->capacity) {
    vec->capacity <<= 1;
    vec->data = realloc(vec->data, vec->capacity * sizeof(struct Entry));
  }

  vec->data[vec->size++] = entry;
}
static inline void freeVector(struct Vector *const restrict vec) {
  if (vec->data)
    free(vec->data);
  free(vec);
}

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG: " fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) // Do nothing
#endif

struct gameInfo {
  short genProcRabbits, genProcFoxes, genFoodFoxes;
  size_t nGen, // num of generations to compute
      c,       //
      r,       //
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
static inline char cenodeNorth(const struct gameInfo *const restrict game,
                               const size_t row,
                               const size_t col) { // Up
  return (row == 0) ? OUT_OF_BOUNDS : game->boardM[game->c * (row - 1) + col];
}
static inline char cenodeSouth(const struct gameInfo *const restrict game,
                               const size_t row,
                               const size_t col) { // Down
  return (row + 1 == game->r) ? OUT_OF_BOUNDS
                              : game->boardM[game->c * (row + 1) + col];
}
static inline char cenodeEast(const struct gameInfo *const restrict game,
                              const size_t row,
                              const size_t col) { // Right
  return (col + 1 == game->c) ? OUT_OF_BOUNDS
                              : game->boardM[game->c * row + col + 1];
}
static inline char cenodeWest(const struct gameInfo *const restrict game,
                              const size_t row,
                              const size_t col) { // Left
  return (col == 0) ? OUT_OF_BOUNDS : game->boardM[game->c * row + col - 1];
}

// MOVE SELECTION LOGIC
// ----------------------------------------------------------------------------
// NOTE: Move selection should ALWAYS be made based on the game->boardM state
// and not the game->swapM state
char _selectMove(const struct gameInfo *restrict game, const char target,
                 const size_t g, const size_t row, const size_t col) {
  size_t i = 0;
  char moves[4]; // priority - N,E,S,W

  // Can move N?
  if (cenodeNorth(game, row, col) == target)
    moves[i++] = NORTH;
  // Can move E?
  if (cenodeEast(game, row, col) == target)
    moves[i++] = EAST;

  // Can move S?
  if (cenodeSouth(game, row, col) == target)
    moves[i++] = SOUTH;

  // Can move W?
  if (cenodeWest(game, row, col) == target)
    moves[i++] = WEST;

  DEBUG_PRINT(
      "<SelectMove>\tBoard[Row][Col]=%c, G=%zu, Row=%zu, Col=%zu, I=%zu\n",
      game->boardM[game->c * row + col], g, row, col, i);
  return (i != 0) ? moves[(g + row + col) % i] : EMPTY;
}

static inline char selectMoveRabbit(const struct gameInfo *const restrict game,
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
void handleMoveRabbit(struct Vector *const restrict head,
                      const struct gameInfo *restrict game, const size_t gen,
                      const size_t i, const size_t j) {
  size_t next_i = i, next_j = j;
  const size_t pos = game->c * i + j;
  const bool canProc = (game->procArrM[pos] >= game->genProcRabbits);

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
    struct Entry node;
    struct Entry node2;
    node.i = next_i;
    node.j = next_j;
    node.procreated = true;
    node.proc = INIT_PROC;

    node2.proc = INIT_PROC;
    node2.i = i;
    node2.j = j;
    node2.procreated = false;

    addNode(head, node);
    addNode(head, node2);

  } else {
  noMove:
    {
      struct Entry node;
      node.i = next_i;
      node.j = next_j;
      node.procreated = false;
      node.proc = game->procArrM[pos] + 1;
      addNode(head, node);
    }
  }

  DEBUG_PRINT("<HandleMoveRab>\tR(%zu,%zu) next=(%zu,%zu) "
              "game->swapM(next)='%c' proc=%d\n",
              i, j, next_i, next_j, game->swapM[game->c * next_i + next_j],
              game->procSwapM[game->c * next_i + next_j]);
}

void handleMoveFox(struct Vector *const restrict head,
                   const struct gameInfo *const restrict game, const size_t gen,
                   const size_t i, const size_t j) {
  size_t next_i = i, next_j = j;
  const size_t pos = game->c * i + j;
  const bool canProc = (game->procArrM[pos] >= game->genProcFoxes);

  switch (selectMoveFox(game, gen, i, j)) {
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
    {
      struct Entry node;
      struct Entry node2;
      node.hunger = game->hungerArrM[pos] + 1;
      node.i = next_i;
      node.j = next_j;
      node.proc = INIT_PROC;
      node.procreated = true;

      node2.hunger = INIT_HUNGER;
      node2.i = i;
      node2.j = j;
      node2.proc = INIT_PROC;
      node2.procreated = false;
      addNode(head, node);
      addNode(head, node2);
    }
  } else {
  noMoveFox:
    {
      struct Entry node;
      node.hunger = game->hungerArrM[pos] + 1;
      node.i = next_i;
      node.j = next_j;
      node.proc = game->procArrM[pos] + 1;
      node.procreated = false;
      addNode(head, node);
    }
  }

  DEBUG_PRINT(
      "<HandleMoveFox>\tF(%zu,%zu) next=(%zu,%zu) game->swapM(next)='%c' "
      "proc=%d hunger(next)=%d\n",
      i, j, next_i, next_j, game->swapM[game->c * next_i + next_j],
      game->procSwapM[game->c * next_i + next_j],
      game->hungerSwapM[game->c * next_i + next_j]);
}

void calculateFoxState(struct Vector *const restrict head,
                       struct gameInfo *restrict game, const size_t gen) {
#pragma GCC ivdep
  for (size_t i = 0; i < game->r; i++)
#pragma GCC ivdep
    for (size_t j = 0; j < game->c; j++) {

      const size_t pos = game->c * i + j;

      switch (game->boardM[pos]) {
      // FOX NEXT POSITION
      // ----------------------------------------------------------------------------
      case FOX:
        handleMoveFox(head, game, gen, i, j);
        break;

      case RABBIT:
        game->swapM[pos] = RABBIT;
        game->procSwapM[pos] = game->procArrM[pos];
        break;

      case ROCK:
        game->n++;
        game->swapM[pos] = ROCK;
        break;
      }
    }
}

// UPDATE RABBIT POSITIONS
// ----------------------------------------------------------------------------
void calculateRabbitState(struct Vector *restrict head,
                          struct gameInfo *restrict game, const size_t gen) {
#pragma GCC ivdep
  for (size_t i = 0; i < game->r; i++)
#pragma GCC ivdep
    for (size_t j = 0; j < game->c; j++) {
      const size_t pos = game->c * i + j;
      switch (game->boardM[pos]) {
      // RABBIT NEXT POSITION
      // ----------------------------------------------------------------------------
      case RABBIT:
        handleMoveRabbit(head, game, gen, i, j);
        break;
      case FOX:
        game->swapM[pos] = FOX;
        game->procSwapM[pos] = game->procArrM[pos];
        game->hungerSwapM[pos] = game->hungerArrM[pos];
        break;

      case ROCK:
        game->swapM[pos] = ROCK;
        break;
      }
    }
}
void updateFoxState(struct Vector *restrict node,
                    struct gameInfo *restrict game) {

#pragma GCC ivdep
  for (size_t idx = 0; idx < node->size; idx++) {
    struct Entry *restrict head = &node->data[idx];
    const size_t pos = game->c * head->i + head->j;
    if (head->hunger >= game->genFoodFoxes) {
      if (game->boardM[pos] != RABBIT) {
        if (head->procreated)
          idx++;
        continue;
      }
      head->hunger = 0;
    }

    game->n++;
    switch (game->swapM[pos]) {
    case EMPTY:
      game->swapM[pos] = FOX;
      game->procSwapM[pos] = head->proc;
      game->hungerSwapM[pos] = head->hunger;
      break;
    case FOX:
      if (head->proc > game->procSwapM[pos]) {
        game->procSwapM[pos] = head->proc;
      } else if (head->proc == game->procSwapM[pos]) {
        game->procSwapM[pos] = head->proc;
        if (head->hunger < game->hungerSwapM[pos])
          game->hungerSwapM[pos] = head->hunger;
      }
      game->n--;
      break;
    case RABBIT:
      game->swapM[pos] = FOX;
      game->procSwapM[pos] = head->proc;
      game->hungerSwapM[pos] = INIT_HUNGER;
      game->n--;
      break;
    }
  }
}
void updateRabbitState(struct Vector *restrict node,
                       struct gameInfo *restrict game) {

  game->n = 0;
#pragma GCC ivdep
  for (size_t idx = 0; idx < node->size; idx++) {
    const struct Entry *const restrict head = &node->data[idx];
    game->n++;
    const size_t pos = game->c * head->i + head->j;
    switch (game->swapM[pos]) {
    case EMPTY:
      game->swapM[pos] = RABBIT;
      game->procSwapM[pos] = head->proc;
      break;
    case RABBIT:
      game->procSwapM[pos] = MAX(head->proc, game->procSwapM[pos]);
      game->n--;
      break;
    }
  }

  simdMemcpyAndSet(game->boardM, game->swapM, EMPTY, game->r * game->c);
  simdMemcpyAndSet(game->procArrM, game->procSwapM, INIT_PROC,
                   game->r * game->c);
  simdMemcpyAndSet(game->hungerArrM, game->hungerSwapM, INIT_HUNGER,
                   game->r * game->c);
}
// SWITCH ACTIVE BOARDS AND CLEAR SWAPs
// ----------------------------------------------------------------------------
void swapAndClearBoard(struct gameInfo *restrict game) {
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

int main(int argc, char *argv[]) {

  struct gameInfo game;
  if (fscanf(stdin, "%hu %hu %hu %zu %zu %zu %zu", &game.genProcRabbits,
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
  struct Vector *restrict vec = createVector();
  for (size_t gen = 0; gen < game.nGen; gen++) {
    calculateRabbitState(vec, &game, gen);
    updateRabbitState(vec, &game);
    resetVector(vec);
    calculateFoxState(vec, &game, gen);
    updateFoxState(vec, &game);
    resetVector(vec);

    // prepare for next generation
    swapAndClearBoard(&game);
#ifdef DEBUG
    printf("\nGen %ld\n", gen + 1);
    printState(&game);
#endif
  }
  freeVector(vec);

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

  return EXIT_SUCCESS;
}
