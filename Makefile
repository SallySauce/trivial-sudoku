sudoku: main.cpp nanoglass.h
	clang++ main.cpp -g -O3 -o sudoku

clean:
	rm -rf sudoku sudoku.dSYM

run: sudoku
	./sudoku

all: sodoku
