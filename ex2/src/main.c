#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FOX 'F'
#define RABBIT 'R'
#define ROCK '*'
int main(int argc, char *argv[]) {
  int genProcRabbits, genProcFoxs, genFoodFoxes, nGen, r, c, n;
  fscanf(stdin, "%d %d %d %d %d %d %d", &genProcRabbits, &genProcFoxs,
         &genFoodFoxes, &nGen, &r, &c, &n);
  char board[r][c];
  char swap[r][c];
  for (int i = 0; i < r; i++) {
    memset(&board[i], ' ', c);
    memset(&swap[i], ' ', c);
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

  return EXIT_SUCCESS;
}
