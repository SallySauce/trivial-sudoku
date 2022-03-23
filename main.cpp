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

// Solvers
#include "bit.h"
#include "dancing.h"

#include "threading.cpp"

int main() {
    //bench();
    sem_unlink("/sem_submit");
    sem_unlink("/sem_complete");
    sem_unlink("/sem_done");
    init_threads();
    start_scheduling();
    /*
    char path[4096]; // Hope that TA won't overrun my buffer.
    int r = scanf("%s", path);
    assert(r == 1);

    // @FIXME: Don't blow up the memory
    auto test_file = read_entire_file(path);

    auto puzzles = load_puzzles(test_file);

    for (auto p : puzzles) {
        dance_solve(p);
        //pretty_print_char_rep(p);
        print_char_rep(p); printf("\n");
    }
    */
    return 0;
}
