SHELL := /bin/bash

ifdef CI
	HYPERFINE_COLOR := --style color
endif
N := 50
TEST_FILE := tests/test$(N)
ANS_FILE  := tests/answer$(N)

sudoku_solve: main.cpp nanoglass.h bit.h sudoku.h dancing.h bench.cpp threading.cpp
	g++ main.cpp -g -O3 -o sudoku_solve -std=c++14 -lpthread

.PHONY: clean
clean:
	rm -rf sudoku_solve sudoku_solve.dSYM

.PHONY: run
run: sudoku_solve
	./sudoku_solve

.PHONY: verify
verify: sudoku_solve
	@echo "Testing $(TEST_FILE)..."
	@diff $(ANS_FILE) <(echo "$(TEST_FILE)" | ./sudoku_solve) && echo -e "\033[1;32mYeah!!! Everything looks alright!\033[0m"

.PHONY: bench
bench: sudoku_solve
	@hyperfine "make verify N=50" "make verify N=1000" "make verify N=10000" --export-json benchmark_report.json $(HYPERFINE_COLOR)

.PHONY: test
test: sudoku_solve
	./Lab1.sh test_group answer_group

all: sodoku_solve
