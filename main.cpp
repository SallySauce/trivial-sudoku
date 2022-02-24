#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <algorithm>

typedef uint32_t u32;
typedef int32_t  i32;

#include "sudoku.h"
#include "nanoglass.h"
#include "file.h"

#include "bit.h"

void bench_bit(const vector<char *> &puzzles) {
    vector<Bit_Rep> bit_reps;
    std::transform(puzzles.begin(), puzzles.end(), std::back_inserter(bit_reps),
                   [](char *str_rep) -> Bit_Rep { return Bit_Rep(str_rep); });

    for (auto &bit_rep : bit_reps) {
        assert(bit_solve(bit_rep));
        //bit_rep.pretty_print(); printf("\n");
    }
}

int main() {
    EntireFile test_file = read_entire_file("kaggle.txt");
    auto puzzles = load_puzzles(test_file);
    printf("%lu puzzles\n", puzzles.size());

    //bench_bit(puzzles);

    auto elapsed_time = measure([puzzles]() { bench_bit(puzzles); });
    //basic_solve(sudoku);
    printf("elapsed: %lld us (%f puzzles/sec)\n", elapsed_time,
           puzzles.size() * 1e6 / (float)(elapsed_time));

    return 0;
}
