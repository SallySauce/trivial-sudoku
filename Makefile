sudoku_solve: main.cpp nanoglass.h bit.h sudoku.h dancing.h
	g++ main.cpp -g -O3 -o sudoku_solve -std=c++11

clean:
	rm -rf sudoku sudoku.dSYM

run: sudoku_solve
	./sudoku_solve

all: sodoku_solve
