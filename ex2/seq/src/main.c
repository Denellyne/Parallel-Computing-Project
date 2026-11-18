#include "Misc/Misc.h"
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  size_t genProcRabbits, genProcFoxes, genFoodFoxes,
      nGen, // num of generations to compute
      c,    //
      r,    //
      n;    // num objects in the initial game->boardM

  char *restrict boardM;
  char *restrict swapM;
  short *restrict procArrM;
  short *restrict procSwapM;
  short *restrict hungerArrM;
  short *restrict hungerSwapM;
};
// ADJACENT CELL HELPERS
// ----------------------------------------------------------------------------
char cellNorth(const struct gameInfo *restrict game, const int row,
               const int col) { // Up
  if (row == 0) {
    // return game->boardM[game->r*r-1+col];
    return OUT_OF_BOUNDS;
  } else {
    return game->boardM[game->c * (row - 1) + col];
  }
}
char cellSouth(const struct gameInfo *restrict game, const int row,
               const int col) { // Down
  if (row + 1 == game->r) {
    // return game->boardM[game->r*0+col];
    return OUT_OF_BOUNDS;
  } else {
    return game->boardM[game->c * (row + 1) + col];
  }
}
char cellEast(const struct gameInfo *restrict game, const int row,
              const int col) { // Right
  if (col + 1 == game->c) {
    // return game->boardM[game->r*row+0];
    return OUT_OF_BOUNDS;
  } else {
    return game->boardM[game->c * row + col + 1];
  }
}
char cellWest(const struct gameInfo *restrict game, const int row,
              const int col) { // Left
  if (col == 0) {
    // return game->boardM[game->r*row+c-1];
    return OUT_OF_BOUNDS;
  } else {
    return game->boardM[game->c * row + col - 1];
  }
}

// MOVE SELECTION LOGIC
// ----------------------------------------------------------------------------
// NOTE: Move selection should ALWAYS be made based on the game->boardM state
// and not the game->swapM state
char _selectMove(const struct gameInfo *restrict game, const char target,
                 const int g, const int row, const int col) {
  int i = 0;
  char moves[4]; // priority - N,E,S,W

  // Can move N?
  if (cellNorth(game, row, col) == target) {
    moves[i] = NORTH;
    i++;
  }
  // Can move E?
  if (cellEast(game, row, col) == target) {
    moves[i] = EAST;
    i++;
  }
  // Can move S?
  if (cellSouth(game, row, col) == target) {
    moves[i] = SOUTH;
    i++;
  }
  // Can move W?
  if (cellWest(game, row, col) == target) {
    moves[i] = WEST;
    i++;
  }

  DEBUG_PRINT("<SelectMove>\tBoard[Row][Col]=%c, G=%d, Row=%d, Col=%d, I=%d\n",
              game->boardM[game->c * row + col], g, row, col, i);
  if (i != 0) {
    return moves[(g + row + col) % i];
  } else {
    return EMPTY;
  }
}

char selectMoveRabbit(const struct gameInfo *restrict game, const int g,
                      const int row, const int col) {
  return _selectMove(game, EMPTY, g, row, col);
}

char selectMoveFox(const struct gameInfo *restrict game, const int g,
                   const int row, const int col) {
  const char rabbitDirection = _selectMove(game, RABBIT, g, row, col);
  if (rabbitDirection == EMPTY) {
    return _selectMove(game, EMPTY, g, row, col);
  } else {
    return rabbitDirection;
  }
}

// MOVE HANDLERS
// ----------------------------------------------------------------------------
void handleMoveRabbit(struct gameInfo *restrict game, const int gen,
                      const int i, const int j) {
  int next_i = i, next_j = j;
  const bool canProc =
      (game->procArrM[game->c * i + j] == game->genProcRabbits);

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
  }

  if (game->swapM[game->c * i + j] != FOX) {
    if (canProc) { // Proc
      game->swapM[game->c * i + j] = RABBIT;
      game->procSwapM[game->c * i + j] = 0;
      game->n++;
    } else { // clear previous location
      game->swapM[game->c * i + j] = EMPTY;
      game->procSwapM[game->c * i + j] = INIT_PROC;
    }
  }

  if (game->swapM[game->c * next_i + next_j] == RABBIT) {
    if (game->procSwapM[game->c * next_i + next_j] <
        game->procArrM[game->c * i + j]) { // max Proc
      game->procSwapM[game->c * next_i + next_j] =
          (canProc) ? 0 : game->procArrM[game->c * i + j] + 1;
    }
    game->n--;
  } else if (game->swapM[game->c * next_i + next_j] == EMPTY) {
    game->swapM[game->c * next_i + next_j] = RABBIT;
    game->procSwapM[game->c * next_i + next_j] =
        (canProc) ? 0 : game->procArrM[game->c * i + j] + 1;
  }
  DEBUG_PRINT(
      "<HandleMoveRab>\tR(%d,%d) next=(%d,%d) game->swapM(next)='%c' proc=%d\n",
      i, j, next_i, next_j, game->swapM[game->c * next_i + next_j],
      game->procSwapM[game->c * next_i + next_j]);
}

void handleMoveFox(struct gameInfo *restrict game, int gen, int i, int j) {
  int next_i = i, next_j = j;
  bool canProc = (game->procArrM[game->c * i + j] == game->genProcFoxes);
  bool starved = (game->hungerArrM[game->c * i + j] == game->genFoodFoxes - 1);

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
  }

  if (starved && (game->swapM[game->c * next_i + next_j] != RABBIT)) {
    // fox died of hunger
    game->swapM[game->c * i + j] = EMPTY;
    game->procSwapM[game->c * i + j] = INIT_PROC;
    game->hungerSwapM[game->c * i + j] = INIT_HUNGER;
    game->n--;
  } else {
    if (canProc) { // Proc
      game->swapM[game->c * i + j] = FOX;
      game->procSwapM[game->c * i + j] = 0;
      game->hungerSwapM[game->c * i + j] = 0;
      game->n++;
    } else {
      game->swapM[game->c * i + j] = EMPTY;
      game->procSwapM[game->c * i + j] = INIT_PROC;
      game->hungerSwapM[game->c * i + j] = INIT_HUNGER;
    }

    switch (game->swapM[game->c * next_i + next_j]) {
    case RABBIT: // eat Rabbit
      game->swapM[game->c * next_i + next_j] = FOX;
      game->procSwapM[game->c * next_i + next_j] =
          (canProc) ? 0 : game->procArrM[game->c * i + j] + 1;
      game->hungerSwapM[game->c * next_i + next_j] = 0;
      game->n--;
      break;
    case FOX: // min Hunger
      if (game->hungerSwapM[game->c * next_i + next_j] >
          game->hungerArrM[game->c * i + j]) {
        game->procSwapM[game->c * next_i + next_j] =
            (canProc) ? 0 : game->procArrM[game->c * i + j] + 1;
        game->hungerSwapM[game->c * next_i + next_j] =
            game->hungerArrM[game->c * i + j] + 1;
      }
      game->n--;
      break;
    case EMPTY:
      game->swapM[game->c * next_i + next_j] = FOX;
      game->procSwapM[game->c * next_i + next_j] =
          (canProc) ? 0 : game->procArrM[game->c * i + j] + 1;
      game->hungerSwapM[game->c * next_i + next_j] =
          game->hungerArrM[game->c * i + j] + 1;
      break;
    }
    DEBUG_PRINT("<HandleMoveFox>\tF(%d,%d) next=(%d,%d) game->swapM(next)='%c' "
                "proc=%d hunger(next)=%d\n",
                i, j, next_i, next_j, game->swapM[game->c * next_i + next_j],
                game->procSwapM[game->c * next_i + next_j],
                game->hungerSwapM[game->c * next_i + next_j]);
  }
}

void updateFoxState(struct gameInfo *restrict game, const int gen) {
  for (size_t idx = 0; idx < game->r * game->c; idx++) { // i is a row
    const int i = idx / game->c;
    const int j = idx % game->c;
    switch (game->boardM[game->c * i + j]) {
    // FOX NEXT POSITION
    // ----------------------------------------------------------------------------
    case FOX:
      handleMoveFox(game, gen, i, j);
      break;
    }
  }
}
// UPDATE RABBIT POSITIONS
// ----------------------------------------------------------------------------
void calculateRabbitState(struct gameInfo *restrict game, const int gen) {
  for (size_t idx = 0; idx < game->r * game->c; idx++) { // i is a row
    const int i = idx / game->c;
    const int j = idx % game->c;
    switch (game->boardM[game->c * i + j]) {
    // RABBIT NEXT POSITION
    // ----------------------------------------------------------------------------
    case RABBIT:
      handleMoveRabbit(game, gen, i, j);
      break;
    case FOX:
      game->swapM[game->c * i + j] = FOX;
      game->procSwapM[game->c * i + j] = game->procArrM[game->c * i + j];
      break;
    case ROCK:
      game->swapM[game->c * i + j] = ROCK;
      break;
    }
  }
}
void updateRabbitState(struct gameInfo *restrict game) {
  simdMemcpy(game->boardM, game->swapM, game->r * game->c);
  simdMemcpy(game->procArrM, game->procSwapM, game->r * game->c);
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
void printProcRow(const struct gameInfo *restrict game, const short *restrict m,
                  const int i) {
  for (size_t j = 0; j < game->c; j++) {
    if (game->boardM[game->c * i + j] == RABBIT ||
        game->boardM[game->c * i + j] == FOX)
      printf("%hd", m[game->c * i + j]);
    else
      printf("%c", game->boardM[game->c * i + j]);
  }
}
void printHungerRow(const struct gameInfo *restrict game,
                    const short *restrict m, const int i) {
  for (size_t j = 0; j < game->c; j++) {
    if (game->boardM[game->c * i + j] == FOX) {
      printf("%hd", m[game->c * i + j]);
    } else {
      printf("%c", game->boardM[game->c * i + j]);
    }
  }
}
void printDashes(const int c) {
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
  if (fscanf(stdin, "%zd %zd %zd %zd %zd %zd %zd", &game.genProcRabbits,
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
    int x, y;
    if (fscanf(stdin, "%s %d %d", buf, &x, &y) < 0) {
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

  for (size_t gen = 0; gen < game.nGen; gen++) {

    calculateRabbitState(&game, gen);
    updateRabbitState(&game);
    updateFoxState(&game, gen);

    // prepare for next generation
    swapAndClearBoard(&game);
#ifdef DEBUG
    printf("\nGen %ld\n", gen + 1);
    printState(&game);
#endif
  }

  // PRINT FINAL STATE
  printf("%zd %zd %zd %d %zd %zd %zd\n", game.genProcRabbits, game.genProcFoxes,
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
