#include "Misc/Misc.h"
#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
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

struct node {
  struct node *next;
  size_t i, j;
  size_t hunger, proc;
  bool procreated;
};
struct LL {
  struct node *head, *tail;
};
struct LL *restrict createList() { return calloc(1, sizeof(struct LL)); }
void addNodes(struct LL *restrict head, struct node *restrict beg,
              struct node *restrict end) {
  if (!head->head) {
    head->head = beg;
    head->tail = end;
  } else {
    head->tail->next = beg;
    head->tail = end;
  }
}
void removeHead(struct LL *restrict head) {
  if (!head || !head->head)
    return;
  struct node *restrict next = head->head->next;
  free(head->head);
  head->head = next;
  if (!head->head)
    head->tail = NULL;
}
void freeList(struct LL *restrict head) {
  while (head->head)
    removeHead(head);
  free(head);
}

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
void printStateTemp(const struct gameInfo *restrict game);
// ADJACENT CEnode HELPERS
// ----------------------------------------------------------------------------
char cenodeNorth(const struct gameInfo *restrict game, const size_t row,
                 const size_t col) { // Up
  if (row == 0) {
    // return game->boardM[game->r*r-1+col];
    return OUT_OF_BOUNDS;
  } else {
    return game->boardM[game->c * (row - 1) + col];
  }
}
char cenodeSouth(const struct gameInfo *restrict game, const size_t row,
                 const size_t col) { // Down
  if (row + 1 == game->r) {
    // return game->boardM[game->r*0+col];
    return OUT_OF_BOUNDS;
  } else {
    return game->boardM[game->c * (row + 1) + col];
  }
}
char cenodeEast(const struct gameInfo *restrict game, const size_t row,
                const size_t col) { // Right
  if (col + 1 == game->c) {
    // return game->boardM[game->r*row+0];
    return OUT_OF_BOUNDS;
  } else {
    return game->boardM[game->c * row + col + 1];
  }
}
char cenodeWest(const struct gameInfo *restrict game, const size_t row,
                const size_t col) { // Left
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
                 const size_t g, const size_t row, const size_t col) {
  size_t i = 0;
  char moves[4]; // priority - N,E,S,W

  // Can move N?
  if (cenodeNorth(game, row, col) == target) {
    moves[i] = NORTH;
    i++;
  }
  // Can move E?
  if (cenodeEast(game, row, col) == target) {
    moves[i] = EAST;
    i++;
  }
  // Can move S?
  if (cenodeSouth(game, row, col) == target) {
    moves[i] = SOUTH;
    i++;
  }
  // Can move W?
  if (cenodeWest(game, row, col) == target) {
    moves[i] = WEST;
    i++;
  }

  DEBUG_PRINT(
      "<SelectMove>\tBoard[Row][Col]=%c, G=%zu, Row=%zu, Col=%zu, I=%zu\n",
      game->boardM[game->c * row + col], g, row, col, i);
  if (i != 0) {
    return moves[(g + row + col) % i];
  } else {
    return EMPTY;
  }
}

char selectMoveRabbit(const struct gameInfo *restrict game, const size_t g,
                      const size_t row, const size_t col) {
  return _selectMove(game, EMPTY, g, row, col);
}

char selectMoveFox(const struct gameInfo *restrict game, const size_t g,
                   const size_t row, const size_t col) {
  const char rabbitDirection = _selectMove(game, RABBIT, g, row, col);
  if (rabbitDirection == EMPTY) {
    return _selectMove(game, EMPTY, g, row, col);
  } else {
    return rabbitDirection;
  }
}

// MOVE HANDLERS
// ----------------------------------------------------------------------------
void handleMoveRabbit(struct LL *restrict head,
                      const struct gameInfo *restrict game, const size_t gen,
                      const size_t i, const size_t j) {
  size_t next_i = i, next_j = j;
  const bool canProc =
      (game->procArrM[game->c * i + j] >= game->genProcRabbits);

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
    struct node *restrict node = malloc(sizeof(struct node));
    struct node *restrict node2 = malloc(sizeof(struct node));
    node->next = node2;
    node->i = next_i;
    node->j = next_j;
    node->procreated = true;
    node->proc = INIT_PROC;

    node2->next = NULL;
    node2->proc = INIT_PROC;
    node2->i = i;
    node2->j = j;
    node2->procreated = false;
    addNodes(head, node, node2);

  } else {
  noMove:
    {
      struct node *restrict node = malloc(sizeof(struct node));
      node->next = NULL;
      node->i = next_i;
      node->j = next_j;
      node->procreated = false;
      node->proc = game->procArrM[game->c * i + j] + 1;
      addNodes(head, node, node);
    }
  }

  DEBUG_PRINT("<HandleMoveRab>\tR(%zu,%zu) next=(%zu,%zu) "
              "game->swapM(next)='%c' proc=%d\n",
              i, j, next_i, next_j, game->swapM[game->c * next_i + next_j],
              game->procSwapM[game->c * next_i + next_j]);
}

void handleMoveFox(struct LL *restrict head,
                   const struct gameInfo *const restrict game, const size_t gen,
                   const size_t i, const size_t j) {
  size_t next_i = i, next_j = j;
  const bool canProc = (game->procArrM[game->c * i + j] >= game->genProcFoxes);

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
      struct node *restrict node = malloc(sizeof(struct node));
      struct node *restrict node2 = malloc(sizeof(struct node));
      node->next = node2;
      node->hunger = game->hungerArrM[game->c * i + j] + 1;
      node->i = next_i;
      node->j = next_j;
      node->proc = INIT_PROC;
      node->procreated = true;

      node2->next = NULL;
      node2->hunger = INIT_HUNGER;
      node2->i = i;
      node2->j = j;
      node2->proc = INIT_PROC;
      node2->procreated = false;
      addNodes(head, node, node2);
    }
  } else {
  noMoveFox:
    {
      struct node *restrict node = malloc(sizeof(struct node));
      node->next = NULL;
      node->hunger = game->hungerArrM[game->c * i + j] + 1;
      node->i = next_i;
      node->j = next_j;
      node->proc = game->procArrM[game->c * i + j] + 1;
      node->procreated = false;
      addNodes(head, node, node);
    }
  }

  DEBUG_PRINT(
      "<HandleMoveFox>\tF(%zu,%zu) next=(%zu,%zu) game->swapM(next)='%c' "
      "proc=%d hunger(next)=%d\n",
      i, j, next_i, next_j, game->swapM[game->c * next_i + next_j],
      game->procSwapM[game->c * next_i + next_j],
      game->hungerSwapM[game->c * next_i + next_j]);
}

struct LL *restrict calculateFoxState(struct LL *restrict head,
                                      struct gameInfo *restrict game,
                                      const size_t gen) {
  for (size_t idx = 0; idx < game->r * game->c; idx++) { // i is a row
    const size_t i = idx / game->c;
    const size_t j = idx % game->c;
    switch (game->boardM[game->c * i + j]) {
    // FOX NEXT POSITION
    // ----------------------------------------------------------------------------
    case FOX:
      handleMoveFox(head, game, gen, i, j);
      break;

    case RABBIT:
      game->swapM[game->c * i + j] = RABBIT;
      game->procSwapM[game->c * i + j] = game->procArrM[game->c * i + j];
      break;

    case ROCK:
      game->n++;
      game->swapM[game->c * i + j] = ROCK;
      break;
    }
  }
  return head;
}
// UPDATE RABBIT POSITIONS
// ----------------------------------------------------------------------------
struct LL *calculateRabbitState(struct LL *restrict head,
                                struct gameInfo *restrict game,
                                const size_t gen) {
  for (size_t idx = 0; idx < game->r * game->c; idx++) { // i is a row
    const size_t i = idx / game->c;
    const size_t j = idx % game->c;
    switch (game->boardM[game->c * i + j]) {
    // RABBIT NEXT POSITION
    // ----------------------------------------------------------------------------
    case RABBIT:
      handleMoveRabbit(head, game, gen, i, j);
      break;
    case FOX:
      game->swapM[game->c * i + j] = FOX;
      game->procSwapM[game->c * i + j] = game->procArrM[game->c * i + j];
      game->hungerSwapM[game->c * i + j] = game->hungerArrM[game->c * i + j];
      break;

    case ROCK:
      game->swapM[game->c * i + j] = ROCK;
      break;
    }
  }
  return head;
}
void updateFoxState(struct LL *restrict node, struct gameInfo *restrict game) {

  while (node->head) {
    struct node *restrict head = node->head;
    const size_t pos = game->c * head->i + head->j;
    if (head->hunger >= game->genFoodFoxes && game->swapM[pos] != RABBIT) {
      if (head->procreated)
        removeHead(node);
      removeHead(node);
      continue;
    }

    game->n++;
    switch (game->swapM[pos]) {
    case EMPTY:
      game->swapM[pos] = FOX;
      game->procSwapM[pos] = head->proc;
      game->hungerSwapM[pos] = head->hunger;
      break;
    case FOX:
      game->procSwapM[pos] = MAX(head->proc, game->procSwapM[pos]);
      game->hungerSwapM[pos] = MIN(head->hunger, game->hungerSwapM[pos]);
      game->n--;
      break;
    case RABBIT:
      game->swapM[pos] = FOX;
      game->procSwapM[pos] = head->proc;
      game->hungerSwapM[pos] = INIT_HUNGER;
      game->n--;
      break;
    case ROCK:
      printf("Impossible to move to ROCK\n");
      assert(1 == 2);
      break;
    }

    removeHead(node);
  }
}
void updateRabbitState(struct LL *restrict node,
                       struct gameInfo *restrict game) {

  game->n = 0;
  while (node->head) {
    struct node *restrict head = node->head;
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
    case FOX:
      printf("Impossible to move to FOX\n");
      assert(1 == 2);
      break;
    case ROCK:
      printf("Impossible to move to ROCK\n");
      assert(1 == 2);
      break;
    }

    removeHead(node);
  }

  simdMemcpy(game->boardM, game->swapM, game->r * game->c);
  simdMemcpy(game->procArrM, game->procSwapM, game->r * game->c);
  simdMemcpy(game->hungerArrM, game->hungerSwapM, game->r * game->c);

  simdMemset(game->swapM, EMPTY, game->r * game->c);
  simdMemset(game->procSwapM, INIT_PROC, game->r * game->c);
  simdMemset(game->hungerSwapM, INIT_HUNGER, game->r * game->c);
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
    if (game->swapM[game->c * i + j] == RABBIT ||
        game->swapM[game->c * i + j] == FOX)
      printf("%hd", m[game->c * i + j]);
    else
      printf("%c", game->swapM[game->c * i + j]);
  }
}
void printHungerRowT(const struct gameInfo *restrict game,
                     const short *restrict m, const size_t i) {
  for (size_t j = 0; j < game->c; j++) {
    if (game->swapM[game->c * i + j] == FOX) {
      printf("%hd", m[game->c * i + j]);
    } else {
      printf("%c", game->swapM[game->c * i + j]);
    }
  }
}
void printProcRow(const struct gameInfo *restrict game, const short *restrict m,
                  const size_t i) {
  for (size_t j = 0; j < game->c; j++) {
    if (game->boardM[game->c * i + j] == RABBIT ||
        game->boardM[game->c * i + j] == FOX)
      printf("%hd", m[game->c * i + j]);
    else
      printf("%c", game->boardM[game->c * i + j]);
  }
}
void printHungerRow(const struct gameInfo *restrict game,
                    const short *restrict m, const size_t i) {
  for (size_t j = 0; j < game->c; j++) {
    if (game->boardM[game->c * i + j] == FOX) {
      printf("%hd", m[game->c * i + j]);
    } else {
      printf("%c", game->boardM[game->c * i + j]);
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
  if (fscanf(stdin, "%zu %zu %zu %zu %zu %zu %zu", &game.genProcRabbits,
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
  struct LL *restrict head = createList();
  for (size_t gen = 0; gen < game.nGen; gen++) {
    head = calculateRabbitState(head, &game, gen);
    updateRabbitState(head, &game);
    head = calculateFoxState(head, &game, gen);
    assert(game.n <= game.r * game.c);
    updateFoxState(head, &game);

    // prepare for next generation
    swapAndClearBoard(&game);
#ifdef DEBUG
    printf("\nGen %ld\n", gen + 1);
    printState(&game);
#endif
  }
  freeList(head);

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
