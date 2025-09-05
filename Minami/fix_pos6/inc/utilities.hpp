#ifndef UTILITIES_HPP
#define UTILITIES_HPP 0

// #include "all.hpp"
#include "state.hpp"

#define rep(i, n) for (int i = 0; i < n; i++)

extern int debug_val1;  // デバッグの際に利用できるグローバル変数

State loadProblem(char *file_name);
void print_ans(v_ope &ans, char* file_name);
int manhattan(Pos p1, Pos p2);
Ope rotateOpe(Ope ope, int fsize, int r);
#endif