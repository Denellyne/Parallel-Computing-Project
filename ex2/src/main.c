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

// #define DEBUG 1 // uncomment this line for more prints
#if DEBUG
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

  short procArrM[r][c];    // track the number of generations since fox/rabbit at (R,C) last duplicated
  short procSwapM[r][c];
  short (*procArr)[c] = procArrM;
  short (*procSwap)[c] = procSwapM;

  short hungerArrM[r][c];  // track the number of generations since fox at (R,C) last ate a rabbit
  short hungerSwapM[r][c];
  short (*hungerArr)[c] = hungerArrM;
  short (*hungerSwap)[c] = hungerSwapM;

  // INIT BOARD ----------------------------------------------------------------------------
  for (int i = 0; i < r; i++) {
    memset(board[i], EMPTY, c);
    memset(swap[i], EMPTY, c);

    memset(procArr[i], INIT_PROC, c*sizeof(short));
    memset(procSwap[i], INIT_PROC, c*sizeof(short));

    memset(hungerArr[i], INIT_HUNGER, c*sizeof(short));
    memset(hungerSwap[i], INIT_HUNGER, c*sizeof(short));
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
  
  // ADJACENT CELL HELPERS ----------------------------------------------------------------------------
  char cellNorth(int row, int col){ //Up
    if (row == 0){
      // return board[r-1][col];
      return OUT_OF_BOUNDS;
    }else {
      return board[row-1][col];
    }
  }
  char cellSouth(int row, int col){ //Down
    if (row+1 == r){
      // return board[0][col];
      return OUT_OF_BOUNDS;
    }else {
      return board[row+1][col];
    }
  }
  char cellEast(int row, int col){ //Right
    if(col+1 == c){
      // return board[row][0];
      return OUT_OF_BOUNDS;
    }else {
      return board[row][col+1];
    }
  }
  char cellWest(int row, int col){ //Left
    if(col == 0){
      // return board[row][c-1];
      return OUT_OF_BOUNDS;
    }else {
      return board[row][col-1];
    }
  }

  // RABBIT MOVE SELECTOR----------------------------------------------------------------------------
  char selectMoveRabbit(int g, int row, int col) {
    int i = 0; //a
    char moves[4]; // priority - N,E,S,W

    // Can move N?
    if (cellNorth(row, col) == EMPTY){
      moves[i] = NORTH;
      i++;
    }
    // Can move E?
    if (cellEast(row, col) == EMPTY){
      moves[i] = EAST;
      i++;
    }
    // Can move S?
    if (cellSouth(row, col) == EMPTY){
      moves[i] = SOUTH;
      i++;
    }
    // Can move W?
    if (cellWest(row, col) == EMPTY){
      moves[i] = WEST;
      i++;
    }
    
    DEBUG_PRINT("<SelectMoveRab>\tBoard[Row][Col]=%c, G=%d, Row=%d, Col=%d, I=%d\n", board[row][col], g, row, col, i);
    if (i != 0){
      return moves[(g + row + col)%i];
    }else {
      return EMPTY;
    }
  }

  // FOX MOVE SELECTOR ----------------------------------------------------------------------------
  char selectMoveFox(int g, int row, int col){    
    int i = 0;
    char moves[4]; // priority - RABBIT in N,E,S,W; THEN EMPTY in N,E,S,W

    // Can move N?
    if (cellNorth(row, col) == RABBIT){
      moves[i] = NORTH;
      i++;
    }
    // Can move E?
    if (cellEast(row, col) == RABBIT){
      moves[i] = EAST;
      i++;
    }
    // Can move S?
    if (cellSouth(row, col) == RABBIT){
      moves[i] = SOUTH;
      i++;
    }
    // Can move W?
    if (cellWest(row, col) == RABBIT){
      moves[i] = WEST;
      i++;
    }
    
    DEBUG_PRINT("<SelectMoveFox>\tBoard[Row][Col]=%c, G=%d, Row=%d, Col=%d, I=%d\n\t", board[row][col], g, row, col, i);
    if (i != 0){
      return moves[(g + row + col)%i]; // RABBIT near
    }else {
      return selectMoveRabbit(g, row, col); // no RABBIT near
    }
  }

  // MOVE HANDLERS ----------------------------------------------------------------------------
  void handleMoveRabbit(int gen, int i, int j){
    int next_i=i, next_j=j;
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

    if(swap[i][j] != FOX){
      if (canProc){ // Proc
        swap[i][j] = RABBIT;
        procSwap[i][j] = 0;
      }else { // clear previous location
        swap[i][j] = EMPTY;
        procSwap[i][j] = INIT_PROC;
      }
    }

    if (swap[next_i][next_j] == RABBIT){
      if (procSwap[next_i][next_j] < procArr[i][j]){ // max Proc
        procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
      }
    } else if (swap[next_i][next_j] == EMPTY) {
      swap[next_i][next_j] = RABBIT;
      procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
    }
    DEBUG_PRINT("<HandleMoveRab>\tR(%d,%d) next=(%d,%d) swap(next)='%c' proc=%d\n", i, j, next_i, next_j, 
      swap[next_i][next_j], procSwap[next_i][next_j]);
  }

  void handleMoveFox(int gen, int i, int j){
    int next_i=i, next_j=j;
    bool canProc = (procArr[i][j] == genProcFoxes);
    bool starved = (hungerArr[i][j] == genFoodFoxes - 1);

    if (!starved){
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

      if (canProc){ // Proc
        swap[i][j] = FOX;
        procSwap[i][j] = 0;
        hungerSwap[i][j] = 0;
      } else {
        swap[i][j] = EMPTY;
        procSwap[i][j] = INIT_PROC;
        hungerSwap[i][j] = INIT_HUNGER;
      }

      switch (swap[next_i][next_j]){
        case RABBIT:  //eat Rabbit
          swap[next_i][next_j] = FOX;
          procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
          hungerSwap[next_i][next_j] = 0;
          break;
        case FOX:     //min Hunger
          if (hungerSwap[next_i][next_j] > hungerArr[i][j]) {
            procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
            hungerSwap[next_i][next_j] = hungerArr[i][j] + 1;
          }
          break;
        case EMPTY:
          swap[next_i][next_j] = FOX;
          procSwap[next_i][next_j] = (canProc) ? 0 : procArr[i][j] + 1;
          hungerSwap[next_i][next_j] = hungerArr[i][j] + 1;
          break;
      }
      DEBUG_PRINT("<HandleMoveFox>\tF(%d,%d) next=(%d,%d) swap(next)='%c' proc=%d hunger(next)=%d\n", i, j, next_i, next_j, 
        swap[next_i][next_j], procSwap[next_i][next_j], hungerSwap[next_i][next_j]);
    }
  }

  // SWITCH ACTIVE BOARDS AND CLEAR SWAPs ----------------------------------------------------------------------------
  void swapAndClearBoard(){
    char (*tmp)[c] = board;
    board = swap;
    swap = tmp;

    short (*t)[c] = procArr;
    procArr = procSwap;
    procSwap = t;

    t = hungerArr;
    hungerArr = hungerSwap;
    hungerSwap = t;

    for (int i = 0; i < r; i++)
    {
      memset(swap[i], EMPTY, c);
      memset(procSwap[i], INIT_PROC, c*sizeof(short));
      memset(hungerSwap[i], INIT_HUNGER, c*sizeof(short));
    }
  }
  // PRINT BOARD ----------------------------------------------------------------------------
  void printProcRow(short m[r][c], int i){
    for (int j = 0; j < c; j++){
      if (board[i][j] == RABBIT || board[i][j] == FOX )
        printf("%hd", m[i][j]);
      else 
        printf("%c", board[i][j]);
    }
  }
  void printHungerRow(short m[r][c], int i){
    for (int j = 0; j < c; j++){
      if (board[i][j] == FOX) {
        printf("%hd", m[i][j]);
      } else {
        printf("%c", board[i][j]);
      }
    }
  }
  void printDashes(){
    for (int i = 0; i < c+2; i++)
      putchar('-');
      printf("\t\tP");
    for (int i = 0; i < c+1; i++)
      putchar('-');
      printf("\tH");
    for (int i = 0; i < c+1; i++)
      putchar('-');
    printf("\n");
  }
  void printState(){
    printDashes();
    
    for (int i = 0; i < r; i++) {
      printf("|");
      for (int j = 0; j < c; j++)
        printf("%c", board[i][j]);
      printf("|\t\t|");
        
      printProcRow(procArr, i);
      printf("|\t|");

      printHungerRow(hungerArr, i);
      printf("|\n");
    }
    printDashes();
  }

  printf("\nGen 0\n");
  printState();

  for (int gen = 0; gen < nGen; gen++){
    printf("\nGen %d\n", gen+1);
    for (int i = 0; i < r; i++) { // i is a row
      for (int j = 0; j < c; j++){ // j is a col
        switch (board[i][j]){
          // RABBIT NEXT POSITION ----------------------------------------------------------------------------
          case RABBIT:
            handleMoveRabbit(gen, i, j);
            break;
          
          // FOX NEXT POSITION ----------------------------------------------------------------------------
          case FOX:
            handleMoveFox(gen, i, j);
            break;
            
          case ROCK:
            swap[i][j] = ROCK;
            break;
        }
      }
    } 
    // prepare for next generation
    swapAndClearBoard();
    printState();
  }

  return EXIT_SUCCESS;
}
