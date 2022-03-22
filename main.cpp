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

#include "bench.cpp"

int main() {
    bench();
    return 0;
}
