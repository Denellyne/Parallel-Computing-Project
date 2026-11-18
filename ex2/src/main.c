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

int main(int argc, char *argv[]) {
  int genProcRabbits, genProcFoxes, genFoodFoxes,
      nGen, // num of generations to compute
      r,    // board row count
      c,    // board column count
      n;    // num objects in the initial board

  fscanf(stdin, "%d %d %d %d %d %d %d", &genProcRabbits, &genProcFoxes,
         &genFoodFoxes, &nGen, &r, &c, &n);

  char boardM[r][c];
  char swapM[r][c];
  char (*board)[c] = boardM;
  char (*swap)[c] = swapM;

  short procArrM[r][c]; // track the number of generations since fox/rabbit at
                        // (R,C) last duplicated
  short procSwapM[r][c];
  short (*procArr)[c] = procArrM;
  short (*procSwap)[c] = procSwapM;

  short hungerArrM[r][c]; // track the number of generations since fox at (R,C)
                          // last ate a rabbit
  short hungerSwapM[r][c];
  short (*hungerArr)[c] = hungerArrM;
  short (*hungerSwap)[c] = hungerSwapM;

  // INIT BOARD
  // ----------------------------------------------------------------------------
  for (int i = 0; i < r; i++) {
    memset(board[i], EMPTY, c);
    memset(swap[i], EMPTY, c);

    memset(procArr[i], INIT_PROC, c * sizeof(procArr[0][0]));
    memset(procSwap[i], INIT_PROC, c * sizeof(procArr[0][0]));

    memset(hungerArr[i], INIT_HUNGER, c * sizeof(hungerArr[0][0]));
    memset(hungerSwap[i], INIT_HUNGER, c * sizeof(hungerArr[0][0]));
  }

  for (int i = 0; i < n; i++) {
    char buf[16];
    int x, y;
    fscanf(stdin, "%s %d %d", buf, &x, &y);
    if (!strcmp(buf, "ROCK")) {
      board[x][y] = ROCK;
    } else if (!strcmp(buf, "FOX")) {
      board[x][y] = FOX;
      hungerArr[x][y] = 0;
    } else if (!strcmp(buf, "RABBIT")) {
      board[x][y] = RABBIT;
    } else {
      printf("Not a valid option\n");
      return EXIT_FAILURE;
    }
  }

  // ADJACENT CELL HELPERS
  // ----------------------------------------------------------------------------
  char cellNorth(char board[][c], int row, int col) { // Up
    if (row == 0) {
      // return board[r-1][col];
      return OUT_OF_BOUNDS;
    } else {
      return board[row - 1][col];
    }
  }
  char cellSouth(char board[][c], int row, int col) { // Down
    if (row + 1 == r) {
      // return board[0][col];
      return OUT_OF_BOUNDS;
    } else {
      return board[row + 1][col];
    }
  }
  char cellEast(char board[][c], int row, int col) { // Right
    if (col + 1 == c) {
      // return board[row][0];
      return OUT_OF_BOUNDS;
    } else {
      return board[row][col + 1];
    }
  }
  char cellWest(char board[][c], int row, int col) { // Left
    if (col == 0) {
      // return board[row][c-1];
      return OUT_OF_BOUNDS;
    } else {
      return board[row][col - 1];
    }
  }

  // MOVE SELECTION LOGIC
  // ----------------------------------------------------------------------------
  // NOTE: Move selection should ALWAYS be made based on the board state and not
  // the swap state
  char _selectMove(char board[][c], char target, int g, int row, int col) {
    int i = 0;
    char moves[4]; // priority - N,E,S,W

    // Can move N?
    if (cellNorth(board, row, col) == target) {
      moves[i] = NORTH;
      i++;
    }
    // Can move E?
    if (cellEast(board, row, col) == target) {
      moves[i] = EAST;
      i++;
    }
    // Can move S?
    if (cellSouth(board, row, col) == target) {
      moves[i] = SOUTH;
      i++;
    }
    // Can move W?
    if (cellWest(board, row, col) == target) {
      moves[i] = WEST;
      i++;
    }

    DEBUG_PRINT(
        "<SelectMove>\tBoard[Row][Col]=%c, G=%d, Row=%d, Col=%d, I=%d\n",
        board[row][col], g, row, col, i);
    if (i != 0) {
      return moves[(g + row + col) % i];
    } else {
      return EMPTY;
    }
  }

  char selectMoveRabbit(int g, int row, int col) {
    return _selectMove(board, EMPTY, g, row, col);
  }

  char selectMoveFox(int g, int row, int col) {
    char rabbitDirection = _selectMove(board, RABBIT, g, row, col);
    if (rabbitDirection == EMPTY) {
      return _selectMove(board, EMPTY, g, row, col);
    } else {
      return rabbitDirection;
    }
  }

  // MOVE HANDLERS
  // ----------------------------------------------------------------------------
  void handleMoveRabbit(int gen, int i, int j) {
    int next_i = i, next_j = j;
    bool canProc = (procArr[i][j] == genProcRabbits);

    char move = selectMoveRabbit(gen, i, j);
    if (move == NORTH) {
      next_i--;
    } else if (move == SOUTH) {
      next_i++;
    } else if (move == EAST) {
      next_j++;
    } else if (move == WEST) {
      next_j--;
    }

    if (swap[i][j] != FOX) {
      if (canProc) { // Proc
        swap[i][j] = RABBIT;
        procSwap[i][j] = 0;
        n++;
      } else { // clear previous location
        swap[i][j] = EMPTY;
        procSwap[i][j] = INIT_PROC;
      }
    }

    if (swap[next_i][next_j] == RABBIT) {
      if (procSwap[next_i][next_j] < procArr[i][j]) { // max Proc
        procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
      }
      n--;
    } else if (swap[next_i][next_j] == EMPTY) {
      swap[next_i][next_j] = RABBIT;
      procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
    }
    DEBUG_PRINT(
        "<HandleMoveRab>\tR(%d,%d) next=(%d,%d) swap(next)='%c' proc=%d\n", i,
        j, next_i, next_j, swap[next_i][next_j], procSwap[next_i][next_j]);
  }

  void handleMoveFox(int gen, int i, int j) {
    int next_i = i, next_j = j;
    bool canProc = (procArr[i][j] == genProcFoxes);
    bool starved = (hungerArr[i][j] == genFoodFoxes - 1);

    char move = selectMoveFox(gen, i, j);
    if (move == NORTH) {
      next_i--;
    } else if (move == SOUTH) {
      next_i++;
    } else if (move == EAST) {
      next_j++;
    } else if (move == WEST) {
      next_j--;
    }

    if (starved && (swap[next_i][next_j] != RABBIT)) {
      // fox died of hunger
      swap[i][j] = EMPTY;
      procSwap[i][j] = INIT_PROC;
      hungerSwap[i][j] = INIT_HUNGER;
      n--;
    } else {
      if (canProc) { // Proc
        swap[i][j] = FOX;
        procSwap[i][j] = 0;
        hungerSwap[i][j] = 0;
        n++;
      } else {
        swap[i][j] = EMPTY;
        procSwap[i][j] = INIT_PROC;
        hungerSwap[i][j] = INIT_HUNGER;
      }

      switch (swap[next_i][next_j]) {
      case RABBIT: // eat Rabbit
        swap[next_i][next_j] = FOX;
        procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
        hungerSwap[next_i][next_j] = 0;
        n--;
        break;
      case FOX: // min Hunger
        if (hungerSwap[next_i][next_j] > hungerArr[i][j]) {
          procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
          hungerSwap[next_i][next_j] = hungerArr[i][j] + 1;
        }
        n--;
        break;
      case EMPTY:
        swap[next_i][next_j] = FOX;
        procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
        hungerSwap[next_i][next_j] = hungerArr[i][j] + 1;
        break;
      }
      DEBUG_PRINT("<HandleMoveFox>\tF(%d,%d) next=(%d,%d) swap(next)='%c' "
                  "proc=%d hunger(next)=%d\n",
                  i, j, next_i, next_j, swap[next_i][next_j],
                  procSwap[next_i][next_j], hungerSwap[next_i][next_j]);
    }
  }

  // UPDATE RABBIT POSITIONS
  // ----------------------------------------------------------------------------
  void updateRabbitState() {
    memcpy(board, swap, r * c);
    memcpy(procArr, procSwap, r * c * sizeof(procArr[0][0]));
  }
  // SWITCH ACTIVE BOARDS AND CLEAR SWAPs
  // ----------------------------------------------------------------------------
  void swapAndClearBoard() {
    char (*tmp)[c] = board;
    board = swap;
    swap = tmp;

    short (*t)[c] = procArr;
    procArr = procSwap;
    procSwap = t;

    t = hungerArr;
    hungerArr = hungerSwap;
    hungerSwap = t;

    for (int i = 0; i < r; i++) {
      memset(swap[i], EMPTY, c);
      memset(procSwap[i], INIT_PROC, c * sizeof(procSwap[0][0]));
      memset(hungerSwap[i], INIT_HUNGER, c * sizeof(hungerSwap[0][0]));
    }
  }
  // PRINT BOARD
  // ----------------------------------------------------------------------------
  void printProcRow(short m[r][c], int i) {
    for (int j = 0; j < c; j++) {
      if (board[i][j] == RABBIT || board[i][j] == FOX)
        printf("%hd", m[i][j]);
      else
        printf("%c", board[i][j]);
    }
  }
  void printHungerRow(short m[r][c], int i) {
    for (int j = 0; j < c; j++) {
      if (board[i][j] == FOX) {
        printf("%hd", m[i][j]);
      } else {
        printf("%c", board[i][j]);
      }
    }
  }
  void printDashes() {
    for (int i = 0; i < c + 2; i++)
      putchar('-');
    printf("   P");
    for (int i = 0; i < c + 1; i++)
      putchar('-');
    printf(" H");
    for (int i = 0; i < c + 1; i++)
      putchar('-');
    printf("\n");
  }
  void printState() {
    printDashes();

    for (int i = 0; i < r; i++) {
      printf("|");
      for (int j = 0; j < c; j++)
        printf("%c", board[i][j]);
      printf("|   |");

      printProcRow(procArr, i);
      printf("| |");

      printHungerRow(hungerArr, i);
      printf("|\n");
    }
    printDashes();
  }

#ifdef DEBUG
  printf("\nGen 0\n");
  printState();
#endif

  for (int gen = 0; gen < nGen; gen++) {
    for (int i = 0; i < r; i++) {   // i is a row
      for (int j = 0; j < c; j++) { // j is a col
        switch (board[i][j]) {
        // RABBIT NEXT POSITION
        // ----------------------------------------------------------------------------
        case RABBIT:
          handleMoveRabbit(gen, i, j);
          break;
        case FOX:
          swap[i][j] = FOX;
          procSwap[i][j] = procArr[i][j];
          break;
        case ROCK:
          swap[i][j] = ROCK;
          break;
        }
      }
    }
    updateRabbitState();

    for (int i = 0; i < r; i++) {   // i is a row
      for (int j = 0; j < c; j++) { // j is a col
        switch (board[i][j]) {
        // FOX NEXT POSITION
        // ----------------------------------------------------------------------------
        case FOX:
          handleMoveFox(gen, i, j);
          break;
        }
      }
    }
    // prepare for next generation
    swapAndClearBoard();
#ifdef DEBUG
    printf("\nGen %d\n", gen + 1);
    printState();
#endif
  }

  // PRINT FINAL STATE
  printf("%d %d %d %d %d %d %d\n", genProcRabbits, genProcFoxes, genFoodFoxes,
         0, r, c, n);
  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++) {
      switch (board[i][j]) {
      case RABBIT:
        printf("%s %d %d\n", "RABBIT", i, j);
        break;
      case FOX:
        printf("%s %d %d\n", "FOX", i, j);
        break;
      case ROCK:
        printf("%s %d %d\n", "ROCK", i, j);
        break;
      }
    }
  }

  return EXIT_SUCCESS;
}
