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
#define NONE -1

// #define DEBUG 1 //uncomment for more prints
#if DEBUG
  #define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG: " fmt, __VA_ARGS__)
#else
  #define DEBUG_PRINT(fmt, ...) // Do nothing
#endif

int main(int argc, char *argv[]) {
  int genProcRabbits, genProcFoxs, genFoodFoxes, 
    nGen, // num of generations to compute
    r,    // board row count
    c,    // board column count
    n;    // num objects in the initial board

  fscanf(stdin, "%d %d %d %d %d %d %d", &genProcRabbits, &genProcFoxs,
         &genFoodFoxes, &nGen, &r, &c, &n);
  
  char board[r][c]; 
  char swap[r][c];
  short procArr[r][c];    // track the number of generations since fox/rabbit at (R,C) last duplicated
  short procSwap[r][c];
  short hungerArr[r][c];  // track the number of generations since fox at (R,C) last ate a rabbit
  short hungerSwap[r][c];

  // INIT BOARD ----------------------------------------------------------------------------
  for (int i = 0; i < r; i++) {
    memset(&board[i], EMPTY, c);
    memset(&swap[i], EMPTY, c);
    memset(&procArr[i], NONE, c);
    memset(&hungerArr[i], NONE, c);
  }
  for (int i = 0; i < n; i++) {
    char buf[16];
    int x, y;
    fscanf(stdin, "%s %d %d", buf, &x, &y);
    if (!strcmp(buf, "ROCK")) {
      board[x][y] = ROCK;
    } else if (!strcmp(buf, "FOX")) {
      board[x][y] = FOX;
      procArr[x][y] = 0;
    } else if (!strcmp(buf, "RABBIT")) {
      board[x][y] = RABBIT;
      hungerArr[x][y] = 0;
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
    
    DEBUG_PRINT("<SelectMoveRabbit> Board[Row][Col]=%c, G=%d, Row=%d, Col=%d, I=%d\n", board[row][col], g, row, col, i);
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
    
    DEBUG_PRINT("<SelectMoveFox> Board[Row][Col]=%c, G=%d, Row=%d, Col=%d, I=%d\n\t", board[row][col], g, row, col, i);
    if (i != 0){
      return moves[(g + row + col)%i]; // RABBIT near
    }else {
      return selectMoveRabbit(g, row, col); // no RABBIT near
    }
  }

  // PRINT ----------------------------------------------------------------------------
  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++)
      printf("%c", board[i][j]);
    printf("\n");
  }

  for (int gen = 0; gen < nGen; gen++){
    printf("\nGen %d\n", gen);
    for (int i = 0; i < r; i++) { // i is a row
      for (int j = 0; j < c; j++){ // j is a col
        switch (board[i][j]){
          int next_i=i, next_j=j;
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

          // RABBIT NEXT POSITION ----------------------------------------------------------------------------
          case RABBIT:
            if (swap[next_i][next_j] == RABBIT){
              if (procSwap[next_i][next_j] < procArr[i][j]){ // max Proc
                procSwap[next_i][next_j] = procArr[i][j];
              }
            } else if swap[next_i][next_j] == EMPTY {
              swap[next_i][next_j] == RABBIT;
              procSwap[next_i][next_j] = procArr[i][j];
            }
            
            if (procArr[i][j] == genProcRabbits - 1){ // Proc
             // TODO 
            }
            break;
          
          // FOX NEXT POSITION ----------------------------------------------------------------------------
          case FOX:
            if (swap[next_i][next_j] == RABBIT){ // eat Rabbit
              swap[next_i][next_j] == FOX;
              procSwap[next_i][next_j] = procArr[i][j];
            } else if (swap[next_i][next_j] == FOX){
              if (hungerSwap[next_i][next_j] > hungerArr[i][j]){ // min Hunger
                hungerSwap[next_i][next_j] = hungerArr[i][j];
              }
            } else if swap[next_i][next_j] == EMPTY {
              swap[next_i][next_j] == FOX;
              procSwap[next_i][next_j] = procArr[i][j];
            }
            if (procArr[i][j] == genProcFoxs - 1){ // Proc
              // TODO 
            }
            break;
        }
        printf("%c", swap[i][j]);
      }
      printf("\n");
    }
  }

  return EXIT_SUCCESS;
}
