// Solve sudoku using a naive bit representation
#pragma once

#include <vector>
#include <cctype>

#include "sudoku.h"

using std::vector;

struct Bit_Rep {
    char *board;
	u32 rows[9];
	u32 cols[9];
	u32 boxes[9];

    Bit_Rep(char *str_rep) {
        for (int i = 0; i < 9; i++) {
            rows[i] = cols[i] = boxes[i] = 0x1ff;
        }

        board = str_rep;
        for (int i = 0; i < 81; i++) {
            char n;
            if ((n = board[i]) == '0') continue;
            assert(isdigit(n));

            int r = i / 9, c = i % 9;
            int box = (r / 3) * 3 + c / 3;

            u32 bit     = 1 << (n - '1');
            rows[r]    ^= bit;
            cols[c]    ^= bit;
            boxes[box] ^= bit;
        }
    }

    void pretty_print() {
        pretty_print_char_rep(board);
    }
};

i32 get_low_bit_pos(u32 x) {
    if (x == 0) return -1;

    i32 pos = 0;
    while (!(x & 1)) {
        x >>= 1;
        pos++;        
    }

    return pos;
}

bool bit_solve(Bit_Rep &p, int empty_index = 0) {

    // Find next empty cell to fill
    while (p.board[empty_index] != '0' && empty_index < 81) empty_index++;

    if (empty_index == 81) {
        return true;
    }

    int r = empty_index / 9, c = empty_index % 9;
    int box = (r / 3) * 3 + c / 3;

    u32 avail = p.rows[r] & p.cols[c] & p.boxes[box];

    while (avail) {
        auto n = get_low_bit_pos(avail) + 1;
        auto bit = 1 << (n - 1);

        // Fill the cell with n, also making it unavilable
        p.rows[r]    ^= bit;
        p.cols[c]    ^= bit;
        p.boxes[box] ^= bit;

        if (bit_solve(p, empty_index + 1)) {
            p.board[empty_index] = n + '0';
            return true;
        }

        // Backtrack
        p.rows[r]    ^= bit;
        p.cols[c]    ^= bit;
        p.boxes[box] ^= bit;
        avail        ^= bit;
    }

    return false;
}
