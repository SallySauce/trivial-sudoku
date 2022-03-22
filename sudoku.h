#pragma once

#include <vector>

#include "file.h"

using std::vector;

vector<char *> load_puzzles(EntireFile file) {
    vector<char *> puzzles;

    for (char *p = file.data; p - file.data < file.size; p += 82) {
        puzzles.push_back(p);
    }

    return puzzles;
}

void pretty_print_char_rep(char p[81]) {
    for (int i = 0; i < 81; i++) {
        printf("%c ", ".123456789"[p[i] - '0']);

        if ((i + 1) % 9 == 0) {
            printf("\n");
        }
    }
}

// @WARNING: Only for validation purpose.
// This kind of performance is crap.
bool check_validty(char sudoku[]) {

    // check rows
    for (int i = 0; i < 9; i++) {
        bool mark[10] = {};
        for (int j = 0; j < 9; j++) {
            int n = i * 9 + j;
            if (sudoku[n] == '0') return false;
            if (mark[sudoku[n] - '0']) return false;
            mark[sudoku[n] - '0'] = true;
        }
    }

    // check columns
    for (int i = 0; i < 9; i++) {
        bool mark[10] = {};
        for (int j = 0; j < 9; j++) {
            int n = j * 9 + i;
            if (sudoku[n] == '0') return false;
            if (mark[sudoku[n] - '0']) return false;
            mark[sudoku[n] - '0'] = true;
        }
    }

    // check each of the 9 grids
    for (int g = 0; g < 9; g++)  {
        bool mark[10] = {};

        // (0, 0), (0, 3), (0, 6)
        // (3, 0), (3, 3), (3, 6)
        // (6, 0), (6, 3), (6, 6)
        int base_i = (g / 3) * 3;
        int base_j = (g % 3) * 3;

        // for each number in the grid
        for (int x = 0; x < 9; x++) {
            int i = base_i + (x % 3);
            int j = base_j + (x / 3);
            int n = i * 9 + j;
            if (mark[sudoku[n] - '0']) return false;
            mark[sudoku[n] - '0'] = true;
        }
    }

    return true;
}
