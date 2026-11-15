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
  for (int i = 0; i < r; i++) {
    memset(&board[i], EMPTY, c);
    memset(&swap[i], EMPTY, c);
  }
  for (int i = 0; i < n; i++) {
    char buf[16];
    int x, y;
    fscanf(stdin, "%s %d %d", buf, &x, &y);
    if (!strcmp(buf, "ROCK")) {
      board[x][y] = ROCK;
    } else if (!strcmp(buf, "FOX")) {
      board[x][y] = FOX;
    } else if (!strcmp(buf, "RABBIT")) {
      board[x][y] = RABBIT;
    } else {
      printf("Not a valid option\n");
      return EXIT_FAILURE;
    }
  }
  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++)
      printf("%c", board[i][j]);
    printf("\n");
  }

  char selectMoveRabbit(int g, int row, int col){
    // Rabbits - priority N,E,S,W
    
    int i = 0;
    char moves[4];

    // Can move N?
    if ((row == 0) && (board[r-1][col] == EMPTY)) {
      moves[i]= NORTH;
      i ++;
    } else if ((row > 0) && board[row - 1][col] == EMPTY){
      moves[i]= NORTH;
      i ++;
    }

    // Can move E?
    if ((col+1 == c) && (board[row][0] == EMPTY)) {
      moves[i]= EAST;
      i ++;
    } else if ((col+1 < c) && (board[row][col+1] == EMPTY)){
      moves[i]= EAST;
      i ++;
    }

    // Can move S?
    if ((row+1 == r) && (board[0][col] == EMPTY)) {
      moves[i]= SOUTH;
      i ++;
    } else if ((row+1 < r) && (board[row+1][col] == EMPTY)){
      moves[i]= 'E';
      i ++;
    }

    // Can move W?
    if ((col == 0) && (board[row][c-1] == EMPTY)) {
      moves[i]= WEST;
      i ++;
    } else if ((col > 0) && (board[row][col-1] == EMPTY)){
      moves[i]= WEST;
      i ++;
    }
    
    if (i == 0){
      return EMPTY
    }else {
      return moves[(g + row + col)%i]
    }
  }

  return EXIT_SUCCESS;
}
