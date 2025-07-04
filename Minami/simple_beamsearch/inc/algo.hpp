#ifndef ALGO_HPP
#define ALGO_HPP 0
#include <bits/stdc++.h>
#include "main.hpp"
v_ope solve(State &s);
v_ope greedy(State &s, v_ope &ope_list);
BeamNode* beamSearch(State &s, v_ope &ope_list, int (*score_func)(State &s), int depth, int beam_width);
void eraseOpe(State &first, v_ope &ope_log);
#endif