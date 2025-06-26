#ifndef UTILITIES_HPP
#define UTILITIES_HPP 0
#include "main.hpp"
void set_pos(Pos *base_ent_pos, vv_ent &field, vv_pos &ent_pos, int *val, Pos *another_ent_pos, Pos *diff);
vv_ent rotate(vv_ent &field, vv_pos &ent_pos, Ope ope);
vvvv_ope bfs(Pos goal_pos, vvv_ope &grid_ope, vvv_bool &is_can_ope);
void get_best_answer(Pos base_pos, Pos goal_pos, Pos last_pos, vvv_ope &grid_ope, vvv_bool &is_can_ope, vvvvv_ope &result);
void update_adj(Pos confirm_pos, vvv_ope &grid_ope, vvv_bool &is_can_ope);
void save_result(vvvvv_ope &result, char* file_name);
#endif