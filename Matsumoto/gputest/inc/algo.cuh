#ifndef ALGO_CUH_
#define ALGO_CUH_

#include <string>
#include <cstdint>

struct SolveResult {
    std::string filename;
    double time_seconds;
    int moves;
    bool solved;
    uint32_t fsize;
};

void test_rotate();
void test_beam_search();

SolveResult solve_from_file(const std::string& filepath);

#endif
