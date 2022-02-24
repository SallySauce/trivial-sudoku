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
