sudoku_solve: main.cpp nanoglass.h bit.h sudoku.h dancing.h bench.cpp
	g++ main.cpp -g -O3 -o sudoku_solve -std=c++11

clean:
	rm -rf sudoku sudoku.dSYM

run: sudoku_solve
	./sudoku_solve

test: sudoku_solve
	./Lab1.sh test_group answer_group

all: sodoku_solve
