#ifndef ALGO_HPP
#define ALGO_HPP 0
#include <bits/stdc++.h>
#include "main.hpp"
typedef struct _solve_data{
    Pos base_ent_pos;
    int type;
    int bfs_idx;
} SolveData;

typedef vector<SolveData> v_solve_data;

void convert_bfs_result(vvvvv_ope &bfs_result);
v_ope solve(State &s, vvvvv_ope &bfs_result, int start_clock, int max_time);
void set_solve_data_recode(int recode, int *bfs_idx, v_solve_data &solve_data);
void set_solve_data_colum(int colum, int *bfs_idx, v_solve_data &solve_data);
int getNextFieldCnt(State *s, int cnt, v_solve_data &solve_data, vvvvv_ope &bfs_result);
int getNextField(State *s, int cnt, v_solve_data &solve_data, vvvvv_ope &bfs_result, int idx, v_ope *ope_log);
v_ope getAnswer(State &s, v_solve_data &solve_data, vvvvv_ope &bfs_result, v_int &idx_list);
pair<int, int> dfs(State &s, v_solve_data &solve_data, int cnt, vvvvv_ope &bfs_result, int depth, int max_depth);
BeamNode* beamSearch(State &s, int cnt, v_solve_data &solve_data, vvvvv_ope &bfs_result, int beam_width);
#endif