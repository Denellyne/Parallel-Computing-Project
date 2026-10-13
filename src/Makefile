CXX = mpicc
CFLAGS = -O2 -mtune=native -ftree-vectorize -fvect-cost-model=dynamic -fno-math-errno -fno-trapping-math -ffinite-loops -fpredictive-commoning -ffinite-math-only -march=native -ffast-math -funroll-loops -flto -pthread -falign-loops=32 -fno-semantic-interposition -fvisibility=hidden -fopt-info-vec-missed -lm
OBJ := $(shell find . -name '*.c')
TEST := *.md
DEBUG = -g -Wall -Wextra -pedantic
OUT = ../bin/fox
CORES = ./cores/core.*

all: run
cr : clean run

build: $(OBJ)
	@echo "Linking..."
	$(CXX) $(CFLAGS) $(OBJ) -o $(OUT) 
	

run: $(OBJ)
	@echo "Linking..."
	$(CXX) $(CFLAGS) $(OBJ) -o $(OUT) 
	mpirun -np 4 $(OUT)

debug: 
	@echo "Compiling..."
	$(CXX) $(DEBUG) $(CFLAGS) $(INCLUDES) -o $(OUT)

dr:
	@echo "Compiling..."
	$(CXX) $(DEBUG) $(CFLAGS) $(INCLUDES) -o $(OUT)
	@$(OUT)

clean:
	@echo "Deleting..."
	@rm -f $(OUT) $(TEST) $(CORES)
