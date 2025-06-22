#ifndef SOLVE1_HPP
#define SOLVE1_HPP 0
#include "main.hpp"
void preprocess(vvv_ope &grid_ope, vvv_bool &is_can_ope);
void solve1(vvv_ope &grid_ope, vvv_bool &is_can_ope, vvvvv_ope &result);
void solve_recode(int recode, vvv_ope &grid_ope, vvv_bool &is_can_ope, vvvvv_ope &result);
void solve_colum(int recode, vvv_ope &grid_ope, vvv_bool &is_can_ope, vvvvv_ope &result);
#endif