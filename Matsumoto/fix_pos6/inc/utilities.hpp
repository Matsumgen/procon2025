#ifndef UTILITIES_HPP
#define UTILITIES_HPP 0

#include "state.hpp" // Stateや関連する型定義に必要
#include "field.hpp" // v_ope型定義に必要

#define API_URL "http://127.0.0.1:3000"
#define TOKEN "player1"

#define rep(i, n) for (int i = 0; i < n; i++)

extern int debug_val1;

// v_ope型を定義した後に、それを使用する関数を宣言
void print_ans(v_ope &ans, char *file_name);
int manhattan(Pos p1, Pos p2);
Ope rotateOpe(Ope ope, int fsize, int r);
#endif