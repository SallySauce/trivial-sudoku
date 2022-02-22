#include <chrono>

// FIXME: use better clocks
using Clock = std::chrono::high_resolution_clock;

Clock::time_point::rep elapsed(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template <typename F>
Clock::time_point::rep measure(F f) {
    auto start = Clock::now();
    f();
    auto end = Clock::now();
    return elapsed(start, end);
}

template <typename F>
Clock::time_point::rep measure_repeat(F f, int repeat) {
    auto start = Clock::now();
    while (repeat--) {
        f();
    }
    auto end = Clock::now();
    return elapsed(start, end);
}
