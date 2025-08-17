#ifndef UTILITIES_HPP
#define UTILITIES_HPP 0
#include "main.hpp"

void input_data(State &s, char* file_name);
void input_file(State &s, char* file_name);
void input_bfs_result(vvvvv_ope &out);
void set_pos(Pos *base_ent_pos, vv_ent &field, vv_pos &ent_pos, int *val, Pos *another_ent_pos, Pos *diff);
void rotate(State &s, Ope &ope);
void exe_ope(State &s, v_ope &ope_list, v_bool &is_exe);
void stateToChar(State &s, uint8_t *out);
int getPairCnt(vv_pos &ent_pos);
bool isEnd(State &s);
int manhattan(Pos &p1, Pos &p2);
void shuffle(v_ope &ope_list);
void print_ans(v_ope &ans, char* file_name);
int weighted_random(v_int &random_weight);
vv_int setOK2x2(State &s);
void setSum(vv_int &array);
int getSum(vv_int &sum_array, int x1, int y1, int x2, int y2);
BeamNode* createNewBeamNode(State &s);
BeamNode* getBeamNodeCopy(BeamNode *origin);
bool addPriorityQueue(priority_queue<BeamNode2> &p_queue, BeamNode2 data, int max_size);
uint8_t popcount(unsigned long long int x);
int getMaxBit(unsigned long long int x);
int getMinBit(unsigned long long int x);
#endif