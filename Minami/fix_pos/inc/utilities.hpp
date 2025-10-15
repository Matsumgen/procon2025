#ifndef UTILITIES_HPP
#define UTILITIES_HPP 0
#include "main.hpp"
void input_data(State &s, char* file_name);
void input_file(State &s, char* file_name);
void input_bfs_result(vvvvv_ope &out);
void set_pos(Pos *base_ent_pos, State &s, int *val, Pos *another_ent_pos, Pos *diff);
void rotate(State &s, Ope &ope);
void rotate_back(State &s, Ope &ope);
void print_ans(v_ope &ans, char* file_name);
void print_field(State &s);
int weighted_random(v_int &random_weight);
int choice_bfs_result(vv_ope &bfs_result);
BeamNode* createNewBeamNode(State &s);
BeamNode* getBeamNodeCopy(BeamNode *origin);
void addPriorityQueue(priority_queue<BeamNode2> &p_queue, BeamNode2 data, int max_size);
#endif