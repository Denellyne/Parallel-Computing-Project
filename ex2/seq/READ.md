## Data
$$
C \in \N \text{: Number of Columns x in the board} \\
R \in \N \text{: Number of Rows y in the board} \\
GenProcRabbits \in \N \text{: Number of generations for a rabbit to duplicate} \\
GenProcFoxes \in \N \text{: Number of generations for a fox to duplicate} \\
GenFoodFoxes \in \N \text{: Number of generations for a fox to starve} \\
$$

$$
board = R \times C \\
rocks \sube R \times C \\
$$

$$
LastProcRabbit \in [0, GenProcRabbits] \\
rabbits: R \times C \longrightarrow LastProcRabbit \text{, Number of generations since rabbit at position (R,C) duplicated} \\
$$

$$
LastProcFox \in [0, GenProcFoxes] \\
foxes: R \times C \longrightarrow LastProcFox \text{, Number of generations since fox at position (R,C) duplicated} \\
$$

$$
LastMealFox \in [0, GenFoodFoxes] \\
foxesHunger: R \times C \longrightarrow LastMealcFox \text{, Number of generations since fox at position (R,C) ate a rabbit} \\
$$

## Moving

$$
G \in \N, \text{Current generation number - initial board has G=0} \\
X \in C,
Y \in R, \text{where (Y,X) is the board position of some rabbit or fox}
$$

$$
freeCellsRabbit(R,C) \sube \{N,E,S,W\}\text{: unoccupied adjacent board cells in the North,East,South,West direcitons}\\
|freeCellsRabbit| \neq 0 \implies \text{rabbit moves}
$$

$$
moveRabbit(g, y, x) = (g + x + y) mod (|freeCellsRabbit|) \\
rabbits(y,x) \longleftarrow max(LastProcRabbit), \text{ assume the highest LastProc among tha rabbits moving to (y,x)} \\
$$

Foxes will move to an adj cell with a rabbits ...