#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>

#include "nanoglass.h"

/* Design choices
 * 1) We're using uint8_t for each number, for now.
 */

struct EntireFile {
    size_t size;
    char   *data;
};

EntireFile read_entire_file(const char* filename) {

    EntireFile file = {};

    FILE *fp = fopen(filename, "rb");

    if (!fp) {
        fprintf(stderr, "cannot open file %s\n", filename);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    auto length = ftell(fp);
    assert(length != -1);
    rewind(fp);

    file.data = (char *)malloc(length+1);

    if (!file.data) {
        fprintf(stderr, "not enough memory\n");
        exit(1);
    }

    if (fread(file.data, 1, length, fp) == 0) {
        fprintf(stderr, "unable to read file\n");
        exit(1);
    }

    // convenient terminator for string processnig
    file.data[length] = '\0';
    file.size         = length;

    fclose(fp);

    return file;
}

void parse_sodoku_string(char *text, uint8_t sudoku[]) {
    for (int i = 0; i < 81; i++) {
        assert(isdigit(text[i]));
        sudoku[i] = text[i] - '0';
    }
}

void print_sudoku(uint8_t sudoku[]) {
    for (int i = 0; i < 81; i++) {
        printf("%c", (char)(sudoku[i] + '0'));
    }
}

void pretty_print_sudoku(uint8_t sudoku[]) {
    for (int i = 0; i < 81; i++) {
        printf("%c ", ".123456789"[sudoku[i]]);

        if ((i + 1) % 9 == 0) {
            printf("\n");
        }
    }
}

// This kind of performance is crap.
bool check_validty(uint8_t sudoku[]) {

    // check rows
    for (int i = 0; i < 9; i++) {
        bool mark[10] = {};
        for (int j = 0; j < 9; j++) {
            int n = i * 9 + j;
            if (sudoku[n] == 0) return false;
            if (mark[sudoku[n]]) return false;
            mark[sudoku[n]] = true;
        }
    }

    // check columns
    for (int i = 0; i < 9; i++) {
        bool mark[10] = {};
        for (int j = 0; j < 9; j++) {
            int n = j * 9 + i;
            if (sudoku[n] == 0) return false;
            if (mark[sudoku[n]]) return false;
            mark[sudoku[n]] = true;
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
            if (mark[sudoku[n]]) return false;
            mark[sudoku[n]] = true;
        }
    }

    return true;
}

static size_t ticks = 0;

bool basic_solve(uint8_t *sudoku, int pos = 0) {

    // sudoku is completed, check validty
    if (pos == 81) {
        bool res = check_validty(sudoku);
        if (res) {printf("found solution\n");}
        return res;
    }

    ticks++;

    if (ticks % (1 << 25) == 0) {
    //if (ticks % (100) == 0) {
    //if (true) {
        printf("ticks: %zd..., pos=%d\n", ticks, pos);
        pretty_print_sudoku(sudoku);
    }

    //printf("pos: %d\n", pos);

    // find the first empty cell to fill
    while (sudoku[pos] != 0) pos++;

    // don't fill numbers that already appeared
    // on the row or column.
    bool mark[10] = {};
    int r = pos / 9;
    int c = pos % 9;
    for (int i = 0; i < 9; i++) {
        mark[sudoku[r * 9 + i]] = 1;
        mark[sudoku[i * 9 + c]] = 1;
    }

    int base_i = (r / 3) * 3;
    int base_j = (c / 3) * 3;

    // for each number in the grid
    for (int x = 0; x < 9; x++) {
        int i = base_i + (x % 3);
        int j = base_j + (x / 3);
        int n = i * 9 + j;
        mark[sudoku[n]] = true;
    }

    // try filling in 1~9
    for (int n = 1; n < 10; n++) {
        if (mark[n]) continue;

        sudoku[pos] = n;
        if (basic_solve(sudoku, pos + 1)) {
            return true;
        }
        sudoku[pos] = 0;
    }

    return false;

}

struct Puzzles {
    size_t  count;
    uint8_t puzzles;
};

uint8_t **load_puzzles(EntireFile file) {

    size_t newline_count = 0;
    char *p = file.data;
    while (*p++) {
        if (*p == '\n') newline_count++;
    }

    Puzzles puzzles;
    puzzles.count = newline_count;
    printf("%d puzzles\n", puzzles.count);

    return nullptr;
}

int main() {
    EntireFile test_file = read_entire_file("test1000");
    load_puzzles(test_file);
    //EntireFile test_file = read_entire_file("test1");
    //printf("file length: %zu\n", test_file.size);
    //printf("Hello World\n%s\n", test_file.data);
    uint8_t sudoku[81];
    auto psudoku = (uint8_t *) sudoku;
    parse_sodoku_string(test_file.data, sudoku);
    pretty_print_sudoku(sudoku);
    printf("\n\n");
    auto elapsed_time = measure([psudoku]() { basic_solve(psudoku); });
    //basic_solve(sudoku);
    printf("ticks: %zd...\n", ticks);
    printf("elapsed: %lld us (%f puzzles/sec)\n", elapsed_time, 1e6/(float)(elapsed_time));
    pretty_print_sudoku(sudoku);
    printf("solved: %d\n", check_validty(sudoku));

    return 0;
}
