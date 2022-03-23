SHELL := /bin/bash

N := 1000
TEST_FILE := tests/test$(N)
ANS_FILE  := tests/answer$(N)

sudoku_solve: main.cpp nanoglass.h bit.h sudoku.h dancing.h bench.cpp threading.cpp
	g++ main.cpp -g -O3 -o sudoku_solve -std=c++11 -lpthread

clean:
	rm -rf sudoku sudoku.dSYM

run: sudoku_solve
	./sudoku_solve

verify: sudoku_solve
	@echo "Testing $(TEST_FILE)..."
	@diff $(ANS_FILE) <(echo "$(TEST_FILE)" | ./sudoku_solve) && echo -e "\033[1;32mYeah!!! Everything looks alright!\033[0m"

bench: sudoku_solve
	@hyperfine "make verify N=50" "make verify N=1000" "make verify N=10000"

test: sudoku_solve
	./Lab1.sh test_group answer_group

all: sodoku_solve
