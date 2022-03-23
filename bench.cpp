
#define VALIDATION 0

void bench_bit(const Slice<char *> &puzzles) {
    vector<Bit_Rep> bit_reps;
    std::transform(puzzles.begin(), puzzles.end(), std::back_inserter(bit_reps),
                   [](char *str_rep) -> Bit_Rep { return Bit_Rep(str_rep); });

    for (auto &bit_rep : bit_reps) {
        bool solved = bit_solve(bit_rep);
        
#if VALIDATION
        assert(check_validty(bit_rep.board));
        assert(solved);
        //bit_rep.pretty_print(); printf("\n");
#endif
    }
}

void bench_dance(const Slice<char *> &puzzles) {

    for (auto p : puzzles) {
        //pretty_print_char_rep(p); printf("\n");
        bool solved = dance_solve(p);
#if VALIDATION
        printf("hey\n");
        assert(check_validty(p));
        assert(solved);
        //pretty_print_char_rep(p); printf("\n");
#endif
    }
}

void bench() {
    auto test_file = read_entire_file("test1000");
    auto puzzles = load_puzzles(test_file);
    printf("%lu puzzles\n", puzzles.size());

    //auto bench_name = "bit"; auto bench_func = bench_bit;
    auto bench_name = "dance"; auto bench_func = bench_dance;

    {
        auto elapsed_time =
            measure([puzzles, bench_func]() { bench_func(puzzles); });
        printf("(%s) elapsed: %ld us (%.2f puzzles/sec)\n", bench_name,
               elapsed_time, puzzles.size() * 1e6 / (float)(elapsed_time));
    }

}
