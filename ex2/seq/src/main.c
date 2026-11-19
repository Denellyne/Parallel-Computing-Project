#include "Misc/Misc.h"
#include "Misc/Vector.h"
#include <stdalign.h>
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

static inline char checkDirection(const char result, const int condition) {
  return (condition) ? OUT_OF_BOUNDS : result;
}

// MOVE SELECTION LOGIC
// ----------------------------------------------------------------------------
// NOTE: Move selection should ALWAYS be made based on the game->boardM state
// and not the game->swapM state
char _selectMove(const struct gameInfo *restrict const game, const char target,
                 const size_t g, const size_t row, const size_t col) {
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
void handleMoveRabbit(struct Vector *const restrict head,
                      const struct gameInfo *restrict game, const size_t gen,
                      const size_t i, const size_t j) {
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

    addNode(head, (struct Entry){.i = next_i,
                                 .j = next_j,
                                 .procreated = true,
                                 .proc = INIT_PROC});
    addNode(head, (struct Entry){
                      .i = i, .j = j, .procreated = false, .proc = INIT_PROC});

  } else {
  noMove:
    addNode(head, (struct Entry){.i = next_i,
                                 .j = next_j,
                                 .procreated = false,
                                 .proc = proc + 1});
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
  const short proc = game->procArrM[pos];
  const short hunger = game->hungerArrM[pos];
  const bool canProc = (proc >= game->genProcFoxes);

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

      addNode(head, (struct Entry){.i = next_i,
                                   .j = next_j,
                                   .procreated = true,
                                   .proc = INIT_PROC,
                                   .hunger = hunger + 1});
      addNode(head, (struct Entry){.i = i,
                                   .j = j,
                                   .procreated = false,
                                   .proc = INIT_PROC,
                                   .hunger = INIT_HUNGER});
    }
  } else {
  noMoveFox:

    addNode(head, (struct Entry){.i = next_i,
                                 .j = next_j,
                                 .procreated = false,
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

void calculateFoxState(struct Vector *const restrict head,
                       struct gameInfo *restrict game, const size_t gen) {
  const char *restrict const board = game->boardM;
  const size_t r = game->r, c = game->c;
  char *restrict const swap = game->swapM;
  short *restrict const procSwap = game->procSwapM;
  const short *restrict const proc = game->procArrM;
  size_t *restrict const n = &game->n;
#pragma GCC ivdep
  for (size_t i = 0; i < r; i++)
#pragma GCC ivdep
    for (size_t j = 0; j < c; j++) {
      const size_t pos = c * i + j;

      switch (board[pos]) {
      // FOX NEXT POSITION
      // ----------------------------------------------------------------------------
      case FOX:
        handleMoveFox(head, game, gen, i, j);
        break;

      case RABBIT:
        swap[pos] = RABBIT;
        procSwap[pos] = proc[pos];
        break;

      case ROCK:
        (*n)++;
        swap[pos] = ROCK;
        break;
      }
    }
}

// UPDATE RABBIT POSITIONS
// ----------------------------------------------------------------------------
void calculateRabbitState(struct Vector *restrict head,
                          struct gameInfo *restrict game, const size_t gen) {

  const char *restrict const board = game->boardM;
  const size_t r = game->r, c = game->c;
  char *restrict const swap = game->swapM;
  short *restrict const procSwap = game->procSwapM;
  const short *restrict const proc = game->procArrM;
  short *restrict const hungerSwap = game->hungerSwapM;
  const short *restrict const hunger = game->hungerArrM;
#pragma GCC ivdep
  for (size_t i = 0; i < r; i++)
#pragma GCC ivdep
    for (size_t j = 0; j < c; j++) {
      const size_t pos = c * i + j;
      switch (board[pos]) {
      // RABBIT NEXT POSITION
      // ----------------------------------------------------------------------------
      case RABBIT:
        handleMoveRabbit(head, game, gen, i, j);
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
void updateFoxState(struct Vector *restrict node,
                    struct gameInfo *restrict game) {

  const char *restrict const board = game->boardM;
  const size_t c = game->c;
  char *restrict const swap = game->swapM;
  short *restrict const procSwap = game->procSwapM;
  short *restrict const hungerSwap = game->hungerSwapM;
  const short genFoodFoxes = game->genFoodFoxes;
  size_t *restrict const n = &game->n;

#pragma GCC ivdep
  for (size_t idx = 0; idx < node->size; idx++) {
    struct Entry *restrict head = &node->data[idx];
    const size_t pos = c * head->i + head->j;
    if (head->hunger >= genFoodFoxes) {
      if (board[pos] != RABBIT) {
        if (head->procreated)
          idx++;
        continue;
      }
      head->hunger = 0;
    }

    (*n)++;
    switch (swap[pos]) {
    case EMPTY:
      swap[pos] = FOX;
      procSwap[pos] = head->proc;
      hungerSwap[pos] = head->hunger;
      break;
    case FOX:
      if (head->proc > procSwap[pos]) {
        procSwap[pos] = head->proc;
      } else if (head->proc == procSwap[pos]) {
        procSwap[pos] = head->proc;
        if (head->hunger < hungerSwap[pos])
          hungerSwap[pos] = head->hunger;
      }
      (*n)--;
      break;
    case RABBIT:
      swap[pos] = FOX;
      procSwap[pos] = head->proc;
      hungerSwap[pos] = INIT_HUNGER;
      (*n)--;
      break;
    }
  }
}
void updateRabbitState(struct Vector *restrict node,
                       struct gameInfo *restrict game) {

  const char *restrict const board = game->boardM;
  const size_t c = game->c;
  char *restrict const swap = game->swapM;
  short *restrict const procSwap = game->procSwapM;
  size_t *restrict const n = &game->n;
  (*n) = 0;
#pragma GCC ivdep
  for (size_t idx = 0; idx < node->size; idx++) {
    const struct Entry *const restrict head = &node->data[idx];
    (*n)++;
    const size_t pos = c * head->i + head->j;
    switch (swap[pos]) {
    case EMPTY:
      swap[pos] = RABBIT;
      procSwap[pos] = head->proc;
      break;
    case RABBIT:
      procSwap[pos] = MAX(head->proc, procSwap[pos]);
      (*n)--;
      break;
    }
  }

  simdMemcpyAndSet(game->boardM, swap, EMPTY, game->r * c);
  simdMemcpyAndSet(game->procArrM, procSwap, INIT_PROC, game->r * c);
  simdMemcpyAndSet(game->hungerArrM, game->hungerSwapM, INIT_HUNGER,
                   game->r * c);
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
  struct Vector vec = createVector(game.n);
  for (size_t gen = 0; gen < game.nGen; gen++) {
    calculateRabbitState(&vec, &game, gen);
    updateRabbitState(&vec, &game);
    vec.size = 0;
    calculateFoxState(&vec, &game, gen);
    updateFoxState(&vec, &game);
    vec.size = 0;

    // prepare for next generation
    swapAndClearBoard(&game);
#ifdef DEBUG
    printf("\nGen %ld\n", gen + 1);
    printState(&game);
#endif
  }
  freeVector(&vec);

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
